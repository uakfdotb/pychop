/*
   Copyright [2008]  Spoof.3D (Michael Höferle) & jampe (Daniel Jampen)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   This code is a heavily modified derivative from GHost++:	http://forum.codelain.com
   GHost++ is a port from the original GHost project:		http://ghost.pwner.org
*/

#include "chop.h"
#include "util.h"
#include "config.h"
#include "language.h"
#include "socket.h"
#include "commandpacket.h"
#include "db.h"
#include "bncsutilinterface.h"
#include "bnlsclient.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include "user.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

using namespace boost :: filesystem;

//
// CBNET
//

CBNET :: CBNET( CChOP *nChOP, string nServer, string nServerAlias, string nBNLSServer, uint16_t nBNLSPort, uint32_t nBNLSWardenCookie, string nCDKeyROC, string nCDKeyTFT, string nCountryAbbrev, string nCountry, string nUserName, string nUserPassword, string nFirstChannel, string nRootAdmin, char nCommandTrigger, unsigned char nWar3Version, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType, uint32_t nMaxMessageLength, bool nTFT )
{
	// todotodo: append path seperator to Warcraft3Path if needed

	m_ChOP = nChOP;
	m_Socket = new CTCPClient( );
	m_Protocol = new CBNETProtocol( );
	m_BNLSClient = NULL;
	m_BNCSUtil = new CBNCSUtilInterface( nUserName, nUserPassword );
	m_CallableBanList = m_ChOP->m_DB->ThreadedBanList( nServer );
	m_CallableUserList = m_ChOP->m_DB->ThreadedUserList( nServer );
	m_Exiting = false;
	m_Server = nServer;
	string LowerServer = m_Server;
	transform( LowerServer.begin( ), LowerServer.end( ), LowerServer.begin( ), (int(*)(int))tolower );

	if( !nServerAlias.empty( ) )
		m_ServerAlias = nServerAlias;
	else if( LowerServer == "useast.battle.net" )
		m_ServerAlias = "USEast";
	else if( LowerServer == "uswest.battle.net" )
		m_ServerAlias = "USWest";
	else if( LowerServer == "asia.battle.net" )
		m_ServerAlias = "Asia";
	else if( LowerServer == "europe.battle.net" )
		m_ServerAlias = "Europe";
	else
		m_ServerAlias = m_Server;

	m_BNLSServer = nBNLSServer;
	m_BNLSPort = nBNLSPort;
	m_BNLSWardenCookie = nBNLSWardenCookie;
	m_CDKeyROC = nCDKeyROC;
	m_CDKeyTFT = nCDKeyTFT;

	// remove dashes from CD keys and convert to uppercase

	m_CDKeyROC.erase( remove( m_CDKeyROC.begin( ), m_CDKeyROC.end( ), '-' ), m_CDKeyROC.end( ) );
	transform( m_CDKeyROC.begin( ), m_CDKeyROC.end( ), m_CDKeyROC.begin( ), (int(*)(int))toupper );

	m_CDKeyTFT.erase( remove( m_CDKeyTFT.begin( ), m_CDKeyTFT.end( ), '-' ), m_CDKeyTFT.end( ) );
	transform( m_CDKeyTFT.begin( ), m_CDKeyTFT.end( ), m_CDKeyTFT.begin( ), (int(*)(int))toupper );
	
	m_TFT = nTFT;

	if( m_CDKeyROC.size( ) != 26 )
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] warning - your ROC CD key is not 26 characters long and is probably invalid" );

	if( m_TFT && m_CDKeyTFT.size( ) != 26 )
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] warning - your TFT CD key is not 26 characters long and is probably invalid" );

	m_CountryAbbrev = nCountryAbbrev;
	m_Country = nCountry;
	m_UserName = nUserName;
	m_UserPassword = nUserPassword;
	m_FirstChannel = nFirstChannel;
	m_RootAdmin = nRootAdmin;
	transform( m_RootAdmin.begin( ), m_RootAdmin.end( ), m_RootAdmin.begin( ), (int(*)(int))tolower );
	m_CommandTrigger = nCommandTrigger;
	m_War3Version = nWar3Version;
	m_EXEVersion = nEXEVersion;
	m_EXEVersionHash = nEXEVersionHash;
	m_PasswordHashType = nPasswordHashType;
	m_MaxMessageLength = nMaxMessageLength;
	m_NextConnectTime = GetTime( );
	m_LastNullTime = 0;
	m_LastOutPacketTicks = 0;
	m_LastOutPacketSize = 0;
	m_FrequencyDelayTimes = 0;
	m_LastBanRefreshTime = GetTime( );
	m_LastUserRefreshTime = GetTime( );
	m_WaitingToConnect = true;
	m_LoggedIn = false;
	m_InChat = false;
	m_LastInviteCreation = false;

	string CommFile = m_ChOP->m_CFGPath + "command.txt";
	m_CFG.Read( CommFile );
}

CBNET :: ~CBNET( )
{
	delete m_Socket;
	delete m_Protocol;
	delete m_BNLSClient;

	while( !m_Packets.empty( ) )
	{
		delete m_Packets.front( );
		m_Packets.pop( );
	}

	delete m_BNCSUtil;

	for( vector<CIncomingFriendList *> :: iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
		delete *i;

	for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
		delete *i;

	for( vector<PairedBanCount> :: iterator i = m_PairedBanCounts.begin( ); i != m_PairedBanCounts.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<PairedBanAdd> :: iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<PairedBanRemove> :: iterator i = m_PairedBanRemoves.begin( ); i != m_PairedBanRemoves.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<PairedGPSCheck> :: iterator i = m_PairedGPSChecks.begin( ); i != m_PairedGPSChecks.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<PairedDPSCheck> :: iterator i = m_PairedDPSChecks.begin( ); i != m_PairedDPSChecks.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<PairedUserCount> :: iterator i = m_PairedUserCounts.begin( ); i != m_PairedUserCounts.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<PairedUserAdd> :: iterator i = m_PairedUserAdds.begin( ); i != m_PairedUserAdds.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<PairedUserDelete> :: iterator i = m_PairedUserDeletes.begin( ); i != m_PairedUserDeletes.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<PairedUserSeen> :: iterator i = m_PairedUserSeens.begin( ); i != m_PairedUserSeens.end( ); i++ )
		m_ChOP->m_Callables.push_back( i->second );

	for( vector<CCallableUserSetSeen *> :: iterator i = m_CallableUserSeens.begin( ); i != m_CallableUserSeens.end( ); i++ )
		m_ChOP->m_Callables.push_back( *i );

	if( m_CallableBanList )
		m_ChOP->m_Callables.push_back( m_CallableBanList );

	if( m_CallableUserList )
		m_ChOP->m_Callables.push_back( m_CallableUserList );

	for( vector<CDBBan *> :: iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
		delete *i;
}

BYTEARRAY CBNET :: GetUniqueName( )
{
	return m_Protocol->GetUniqueName( );
}

unsigned int CBNET :: SetFD( void *fd, void *send_fd, int *nfds )
{
	unsigned int NumFDs = 0;

	if( !m_Socket->HasError( ) && m_Socket->GetConnected( ) )
	{
		m_Socket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
		NumFDs++;

		if( m_BNLSClient )
			NumFDs += m_BNLSClient->SetFD( fd, send_fd, nfds );
	}

	return NumFDs;
}

bool CBNET :: Update( void *fd, void *send_fd )
{
	//
	// update callables
	//

	for( vector<PairedBanCount> :: iterator i = m_PairedBanCounts.begin( ); i != m_PairedBanCounts.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			uint32_t Count = i->second->GetResult( );

			if( Count == 0 )
				QueueChatCommand( m_ChOP->m_Language->ThereAreNoBannedUsers( m_Server ), i->first, !i->first.empty( ) );
			else if( Count == 1 )
				QueueChatCommand( m_ChOP->m_Language->ThereIsBannedUser( m_Server ), i->first, !i->first.empty( ) );
			else
				QueueChatCommand( m_ChOP->m_Language->ThereAreBannedUsers( m_Server, UTIL_ToString( Count ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanCounts.erase( i );
		}
		else
			i++;
	}

	for( vector<PairedBanAdd> :: iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				AddBan( i->second->GetUser( ), i->second->GetIP( ), i->second->GetGameName( ), i->second->GetAdmin( ), i->second->GetReason( ) );
				QueueChatCommand( m_ChOP->m_Language->BannedUser( i->second->GetServer( ), i->second->GetUser( ) ), i->first, !i->first.empty( ) );
			}
			else
				QueueChatCommand( m_ChOP->m_Language->ErrorBanningUser( i->second->GetUser( ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanAdds.erase( i );
		}
		else
			i++;
	}

	for( vector<PairedBanRemove> :: iterator i = m_PairedBanRemoves.begin( ); i != m_PairedBanRemoves.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				RemoveBan( i->second->GetUser( ) );
				QueueChatCommand( m_ChOP->m_Language->UnbannedUser( i->second->GetServer( ), i->second->GetUser( ) ), i->first, !i->first.empty( ) );
			}
			else
				QueueChatCommand( m_ChOP->m_Language->ErrorUnbanningUser( i->second->GetUser( ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanRemoves.erase( i );
		}
		else
			i++;
	}

	for( vector<PairedGPSCheck> :: iterator i = m_PairedGPSChecks.begin( ); i != m_PairedGPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBGamePlayerSummary *GamePlayerSummary = i->second->GetResult( );

			if( GamePlayerSummary )
				QueueChatCommand( m_ChOP->m_Language->HasPlayedGames( i->second->GetName( ), GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), UTIL_ToString( GamePlayerSummary->GetTotalGames( ) ), UTIL_ToString( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 2 ), UTIL_ToString( GamePlayerSummary->GetAvgLeftPercent( ) ) ), i->first, !i->first.empty( ) );
			else
				QueueChatCommand( m_ChOP->m_Language->HasntPlayedGames( i->second->GetName( ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedGPSChecks.erase( i );
		}
		else
			i++;
	}

	for( vector<PairedDPSCheck> :: iterator i = m_PairedDPSChecks.begin( ); i != m_PairedDPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBDotAPlayerSummary *DotAPlayerSummary = i->second->GetResult( );

			if( DotAPlayerSummary )
			{
				string Summary = m_ChOP->m_Language->HasPlayedDotAGames(	i->second->GetName( ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalGames( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalWins( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalLosses( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalKills( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalDeaths( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalCreepKills( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalCreepDenies( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalAssists( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalNeutralKills( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalTowerKills( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalRaxKills( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetTotalCourierKills( ) ),
																			UTIL_ToString( DotAPlayerSummary->GetAvgKills( ), 2 ),
																			UTIL_ToString( DotAPlayerSummary->GetAvgDeaths( ), 2 ),
																			UTIL_ToString( DotAPlayerSummary->GetAvgCreepKills( ), 2 ),
																			UTIL_ToString( DotAPlayerSummary->GetAvgCreepDenies( ), 2 ),
																			UTIL_ToString( DotAPlayerSummary->GetAvgAssists( ), 2 ),
																			UTIL_ToString( DotAPlayerSummary->GetAvgNeutralKills( ), 2 ),
																			UTIL_ToString( DotAPlayerSummary->GetAvgRaxKills( ), 2 ),
																			UTIL_ToString( DotAPlayerSummary->GetAvgCourierKills( ), 2 ) );

				QueueChatCommand( Summary, i->first, !i->first.empty( ) );
			}
			else
				QueueChatCommand( m_ChOP->m_Language->HasntPlayedDotAGames( i->second->GetName( ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedDPSChecks.erase( i );
		}
		else
			i++;
	}

	for( vector<PairedUserCount> :: iterator i = m_PairedUserCounts.begin( ); i != m_PairedUserCounts.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			uint32_t Count = i->second->GetResult( );

			if( Count == 0 )
				QueueChatCommand( m_ChOP->m_Language->ThereAreNoUsers( m_Server ), i->first, !i->first.empty( ) );
			else if( Count == 1 )
				QueueChatCommand( m_ChOP->m_Language->ThereIsUser( m_Server ), i->first, !i->first.empty( ) );
			else
				QueueChatCommand( m_ChOP->m_Language->ThereAreUsers( m_Server, UTIL_ToString( Count ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedUserCounts.erase( i );
		}
		else
			i++;
	}

	for( vector<PairedUserAdd> :: iterator i = m_PairedUserAdds.begin( ); i != m_PairedUserAdds.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				AddUser( i->second->GetName( ), i->second->GetAccess( ) );
				QueueChatCommand( m_ChOP->m_Language->AddedUser( m_Server, i->second->GetName( ), UTIL_ToString( i->second->GetAccess( ) ) ), i->first, !i->first.empty( ) );
			}
			else
				QueueChatCommand( m_ChOP->m_Language->ErrorAddingUser( i->second->GetName( ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedUserAdds.erase( i );
		}
		else
			i++;
	}

	for( vector<PairedUserDelete> :: iterator i = m_PairedUserDeletes.begin( ); i != m_PairedUserDeletes.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				RemoveUser( i->second->GetName( ) );
				QueueChatCommand( m_ChOP->m_Language->DeletedUser( m_Server, i->second->GetName( ) ), i->first, !i->first.empty( ) );
			}
			else
				QueueChatCommand( m_ChOP->m_Language->ErrorDeletingUser( i->second->GetName( ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedUserDeletes.erase( i );
		}
		else
			i++;
	}

	for( vector<PairedUserSeen> :: iterator i = m_PairedUserSeens.begin( ); i != m_PairedUserSeens.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) != 0 )
				QueueChatCommand( m_ChOP->m_Language->UserSeen( i->second->GetName( ), FormatTime( GetTicks()/1000 - i->second->GetResult( ) / 1000 ) ), i->first, !i->first.empty( ) );
			else
				QueueChatCommand( m_ChOP->m_Language->UserNotSeen( i->second->GetName( ) ), i->first, !i->first.empty( ) );

			m_ChOP->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedUserSeens.erase( i );
		}
		else
			i++;
	}

	for( vector<CCallableUserSetSeen *> :: iterator i = m_CallableUserSeens.begin( ); i != m_CallableUserSeens.end( ); )
	{
		if( (*i)->GetReady( ) )
		{
			m_ChOP->m_DB->RecoverCallable( *i );
			delete *i;
			i = m_CallableUserSeens.erase( i );
		}
		else
			i++;
	}

	// refresh the user list every minute

	if( !m_CallableUserList && GetTime( ) >= m_LastUserRefreshTime + 60 ) {
		m_CallableUserList = m_ChOP->m_DB->ThreadedUserList( m_Server );
		m_LastUserRefreshTime = GetTime();
	}

	if( m_CallableUserList && m_CallableUserList->GetReady( ) )
	{
		// CONSOLE_Print( "[BNET: " + m_ServerAlias + "] refreshed user list (" + UTIL_ToString( m_Users.size( ) ) + " -> " + UTIL_ToString( m_CallableUserList->GetResult( ).size( ) ) + " users with access)" );

		map<string, CUser *> tmpUsers = m_Users;
		m_Users = m_CallableUserList->GetResult( );
		
		// first, look for users in old list that are also in new list or who were deleted
		
		for( map<string, CUser *> :: iterator i = tmpUsers.begin( ); i != tmpUsers.end( ); i++ ) {
			map<string, CUser *> :: iterator newIndex = m_Users.find( (*i).first );
			map<string, CUser *> :: iterator channelIndex = m_Channel.find( (*i).first );
			
			if( newIndex != m_Users.end( ) ) {
				// this user was simply updated, so we want to update channel version as well
				
				if( channelIndex != m_Channel.end( ) ) {
					m_Channel.erase(channelIndex);
					m_Channel[(*i).first] = m_Users[(*i).first];
				}
				
				delete (*i).second;
			} else {
				// this user was deleted; if not in channel, delete, otherwise update access
				
				if( channelIndex != m_Channel.end( ) ) {
					(*channelIndex).second->SetAccess(0);
				} else {
					delete (*i).second;
				}
			}
		}
		
		// now, look for users in new list not in old list (added users)
		
		for( map<string, CUser *> :: iterator i = m_Users.begin( ); i != m_Users.end( ); i++ ) {
			map<string, CUser *> :: iterator oldIndex = tmpUsers.find( (*i).first );
			map<string, CUser *> :: iterator channelIndex = m_Channel.find( (*i).first );
			
			if( oldIndex == tmpUsers.end( ) ) {
				// this user was added; if in channel, update channel user
				
				if( channelIndex != m_Channel.end( ) ) {
					delete (*channelIndex).second;
					m_Channel.erase(channelIndex);
					m_Channel[(*i).first] = (*i).second;
				}
			}
		}
		
		m_ChOP->m_DB->RecoverCallable( m_CallableUserList );
		delete m_CallableUserList;
		m_CallableUserList = NULL;
		m_LastUserRefreshTime = GetTime();
	}

	// refresh the ban list every 5 minutes

	if( !m_CallableBanList && GetTime( ) >= m_LastBanRefreshTime + 300 )

		m_CallableBanList = m_ChOP->m_DB->ThreadedBanList( m_Server );

	if( m_CallableBanList && m_CallableBanList->GetReady( ) )
	{
		// CONSOLE_Print( "[BNET: " + m_ServerAlias + "] refreshed ban list (" + UTIL_ToString( m_Bans.size( ) ) + " -> " + UTIL_ToString( m_CallableBanList->GetResult( ).size( ) ) + " bans)" );

		for( vector<CDBBan *> :: iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
			delete *i;

		m_Bans = m_CallableBanList->GetResult( );
		m_ChOP->m_DB->RecoverCallable( m_CallableBanList );
		delete m_CallableBanList;
		m_CallableBanList = NULL;
		m_LastBanRefreshTime = GetTime( );
	}

	// we return at the end of each if statement so we don't have to deal with errors related to the order of the if statements
	// that means it might take a few ms longer to complete a task involving multiple steps (in this case, reconnecting) due to blocking or sleeping
	// but it's not a big deal at all, maybe 100ms in the worst possible case (based on a 50ms blocking time)

	if( m_Socket->HasError( ) )
	{
		// the socket has an error

		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] disconnected from battle.net due to socket error" );
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect" );
		m_ChOP->EventBNETDisconnected( this );
		delete m_BNLSClient;
		m_BNLSClient = NULL;
		m_BNCSUtil->Reset( m_UserName, m_UserPassword );
		m_Socket->Reset( );
		m_NextConnectTime = GetTime( ) + 90;
		m_LoggedIn = false;
		m_InChat = false;
		m_WaitingToConnect = true;
		return m_Exiting;
	}

	if( !m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && !m_WaitingToConnect )
	{
		// the socket was disconnected

		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] disconnected from battle.net due to socket not connected" );
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect" );
		m_ChOP->EventBNETDisconnected( this );
		delete m_BNLSClient;
		m_BNLSClient = NULL;
		m_BNCSUtil->Reset( m_UserName, m_UserPassword );
		m_Socket->Reset( );
		m_NextConnectTime = GetTime( ) + 90;
		m_LoggedIn = false;
		m_InChat = false;
		m_WaitingToConnect = true;
		return m_Exiting;
	}

	if( m_Socket->GetConnected( ) )
	{
		// the socket is connected and everything appears to be working properly

		m_Socket->DoRecv( (fd_set *)fd );
		ExtractPackets( );
		ProcessPackets( );

		// update the BNLS client

		if( m_BNLSClient )
		{
			if( m_BNLSClient->Update( fd, send_fd ) )
			{
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] deleting BNLS client" );
				delete m_BNLSClient;
				m_BNLSClient = NULL;
			}
			else
			{
				BYTEARRAY WardenResponse = m_BNLSClient->GetWardenResponse( );

				if( !WardenResponse.empty( ) )
					m_OutPackets.push( m_Protocol->SEND_SID_WARDEN( WardenResponse ) );
			}
		}
		
		int64_t RemainingWait = DelayTime( );

		if( !m_OutPackets.empty( ) && RemainingWait <= 0 )
		{
			if( m_OutPackets.size( ) > 7 )
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] packet queue warning - there are " + UTIL_ToString( m_OutPackets.size( ) ) + " packets waiting to be sent" );

			m_Socket->PutBytes( m_OutPackets.front( ) );
			m_LastOutPacketSize = m_OutPackets.front( ).size( );
			m_OutPackets.pop( );

			// reset frequency delay (or increment it)
			
			if( m_FrequencyDelayTimes >= 100 || RemainingWait <= -500 )
				m_FrequencyDelayTimes = 0;
			else
				m_FrequencyDelayTimes++;

			m_LastOutPacketTicks = GetTicks( );			
		}

		// send a null packet every 60 seconds to detect disconnects

		if( GetTime( ) >= m_LastNullTime + 60 && GetTicks( ) >= m_LastOutPacketTicks + 60000 )
		{
			m_Socket->PutBytes( m_Protocol->SEND_SID_NULL( ) );
			m_LastNullTime = GetTime( );
		}

		m_Socket->DoSend( (fd_set *)send_fd );
		return m_Exiting;
	}

	if( m_Socket->GetConnecting( ) )
	{
		// we are currently attempting to connect to battle.net

		if( m_Socket->CheckConnect( ) )
		{
			// the connection attempt completed

			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connected" );
			m_ChOP->EventBNETConnected( this );
			m_Socket->PutBytes( m_Protocol->SEND_PROTOCOL_INITIALIZE_SELECTOR( ) );
			m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_INFO( m_War3Version, m_TFT, 1033, m_CountryAbbrev, m_Country ) );
			m_Socket->DoSend( (fd_set *)send_fd );
			m_LastNullTime = GetTime( );
			m_LastOutPacketTicks = GetTicks( );

			while( !m_OutPackets.empty( ) )
				m_OutPackets.pop( );

			return m_Exiting;
		}
		else if( GetTime( ) >= m_NextConnectTime + 15 )
		{
			// the connection attempt timed out (15 seconds)

			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connect timed out" );
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect" );
			m_ChOP->EventBNETConnectTimedOut( this );
			m_Socket->Reset( );
			m_NextConnectTime = GetTime( ) + 90;
			m_WaitingToConnect = true;
			return m_Exiting;
		}
	}

	if( !m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && GetTime( ) >= m_NextConnectTime )
	{
		// attempt to connect to battle.net

		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connecting to server [" + m_Server + "] on port 6112" );
		m_ChOP->EventBNETConnecting( this );

		if( m_ServerIP.empty( ) )
		{
			m_Socket->Connect( string( ), m_Server, 6112 );

			if( !m_Socket->HasError( ) )
			{
				m_ServerIP = m_Socket->GetIPString( );
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] resolved and cached server IP address " + m_ServerIP );
			}
		}
		else
		{
			// use cached server IP address since resolving takes time and is blocking

			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using cached server IP address " + m_ServerIP );
			m_Socket->Connect( string( ), m_ServerIP, 6112 );
		}

		m_WaitingToConnect = false;
		return m_Exiting;
	}

	return m_Exiting;
}

void CBNET :: ExtractPackets( )
{
	// extract as many packets as possible from the socket's receive buffer and put them in the m_Packets queue

	string *RecvBuffer = m_Socket->GetBytes( );
	BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( Bytes.size( ) >= 4 )
	{
		// byte 0 is always 255

		if( Bytes[0] == BNET_HEADER_CONSTANT )
		{
			// bytes 2 and 3 contain the length of the packet

			uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

			if( Length >= 4 )
			{
				if( Bytes.size( ) >= Length )
				{
					m_Packets.push( new CCommandPacket( BNET_HEADER_CONSTANT, Bytes[1], BYTEARRAY( Bytes.begin( ), Bytes.begin( ) + Length ) ) );
					*RecvBuffer = RecvBuffer->substr( Length );
					Bytes = BYTEARRAY( Bytes.begin( ) + Length, Bytes.end( ) );
				}
				else
					return;
			}
			else
			{
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error - received invalid packet from battle.net (bad length), disconnecting" );
				m_Socket->Disconnect( );
				return;
			}
		}
		else
		{
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error - received invalid packet from battle.net (bad header constant), disconnecting" );
			m_Socket->Disconnect( );
			return;
		}
	}
}

void CBNET :: ProcessPackets( )
{
	CIncomingGameHost *GameHost = NULL;
	CIncomingChatEvent *ChatEvent = NULL;
	BYTEARRAY WardenData;
	vector<CIncomingFriendList *> Friends;
	vector<CIncomingClanList *> Clans;

	// process all the received packets in the m_Packets queue
	// this normally means sending some kind of response

	while( !m_Packets.empty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.pop( );

		if( Packet->GetPacketType( ) == BNET_HEADER_CONSTANT )
		{
			switch( Packet->GetID( ) )
			{
			case CBNETProtocol :: SID_NULL:
				// warning: we do not respond to NULL packets with a NULL packet of our own
				// this is because PVPGN servers are programmed to respond to NULL packets so it will create a vicious cycle of useless traffic
				// official battle.net servers do not respond to NULL packets

				m_Protocol->RECEIVE_SID_NULL( Packet->GetData( ) );
				break;

			case CBNETProtocol :: SID_ENTERCHAT:
				if( m_Protocol->RECEIVE_SID_ENTERCHAT( Packet->GetData( ) ) )
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] joining channel [" + m_FirstChannel + "]" );
					m_InChat = true;
					m_Socket->PutBytes( m_Protocol->SEND_SID_JOINCHANNEL( m_FirstChannel ) );
				}

				break;

			case CBNETProtocol :: SID_CHATEVENT:
				ChatEvent = m_Protocol->RECEIVE_SID_CHATEVENT( Packet->GetData( ) );

				if( ChatEvent )
					ProcessChatEvent( ChatEvent );

				delete ChatEvent;
				ChatEvent = NULL;
				break;

			case CBNETProtocol :: SID_CHECKAD:
				m_Protocol->RECEIVE_SID_CHECKAD( Packet->GetData( ) );
				break;

			case CBNETProtocol :: SID_PING:
				m_Socket->PutBytes( m_Protocol->SEND_SID_PING( m_Protocol->RECEIVE_SID_PING( Packet->GetData( ) ) ) );
				break;

			case CBNETProtocol :: SID_AUTH_INFO:
				if( m_Protocol->RECEIVE_SID_AUTH_INFO( Packet->GetData( ) ) )
				{
					if( m_BNCSUtil->HELP_SID_AUTH_CHECK( m_TFT, m_ChOP->m_Warcraft3Path, m_CDKeyROC, m_CDKeyTFT, m_Protocol->GetValueStringFormulaString( ), m_Protocol->GetIX86VerFileNameString( ), m_Protocol->GetClientToken( ), m_Protocol->GetServerToken( ) ) )
					{
						// override the exe information generated by bncsutil if specified in the config file
						// apparently this is useful for pvpgn users

						if( m_EXEVersion.size( ) == 4 )
						{
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using custom exe version bnet_custom_exeversion = " + UTIL_ToString( m_EXEVersion[0] ) + " " + UTIL_ToString( m_EXEVersion[1] ) + " " + UTIL_ToString( m_EXEVersion[2] ) + " " + UTIL_ToString( m_EXEVersion[3] ) );
							m_BNCSUtil->SetEXEVersion( m_EXEVersion );
						}

						if( m_EXEVersionHash.size( ) == 4 )
						{
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using custom exe version hash bnet_custom_exeversionhash = " + UTIL_ToString( m_EXEVersionHash[0] ) + " " + UTIL_ToString( m_EXEVersionHash[1] ) + " " + UTIL_ToString( m_EXEVersionHash[2] ) + " " + UTIL_ToString( m_EXEVersionHash[3] ) );
							m_BNCSUtil->SetEXEVersionHash( m_EXEVersionHash );
						}

						if( m_TFT )
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] attempting to auth as Warcraft III: The Frozen Throne" );
						else
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] attempting to auth as Warcraft III: Reign of Chaos" );

						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_CHECK( m_TFT, m_Protocol->GetClientToken( ), m_BNCSUtil->GetEXEVersion( ), m_BNCSUtil->GetEXEVersionHash( ), m_BNCSUtil->GetKeyInfoROC( ), m_BNCSUtil->GetKeyInfoTFT( ), m_BNCSUtil->GetEXEInfo( ), "ChOP" ) );

						// the Warden seed is the first 4 bytes of the ROC key hash
						// initialize the Warden handler

						if( !m_BNLSServer.empty( ) )
						{
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] creating BNLS client" );
							delete m_BNLSClient;
							m_BNLSClient = new CBNLSClient( m_BNLSServer, m_BNLSPort, m_BNLSWardenCookie );
							m_BNLSClient->QueueWardenSeed( UTIL_ByteArrayToUInt32( m_BNCSUtil->GetKeyInfoROC( ), false, 16 ) );
						}
					}
					else
					{
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - bncsutil key hash failed (check your Warcraft 3 path and cd key), disconnecting" );
						m_Socket->Disconnect( );
						delete Packet;
						return;
					}
				}

				break;

			case CBNETProtocol :: SID_AUTH_CHECK:
				if( m_Protocol->RECEIVE_SID_AUTH_CHECK( Packet->GetData( ) ) )
				{
					// cd keys accepted

					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] cd keys accepted" );
					m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGON( );
					m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGON( m_BNCSUtil->GetClientKey( ), m_UserName ) );
				}
				else
				{
					// cd keys not accepted

					switch( UTIL_ByteArrayToUInt32( m_Protocol->GetKeyState( ), false ) )
					{
					case CBNETProtocol :: KR_ROC_KEY_IN_USE:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - ROC CD key in use by user [" + m_Protocol->GetKeyStateDescription( ) + "], disconnecting" );
						break;
					case CBNETProtocol :: KR_TFT_KEY_IN_USE:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - TFT CD key in use by user [" + m_Protocol->GetKeyStateDescription( ) + "], disconnecting" );
						break;
					case CBNETProtocol :: KR_OLD_GAME_VERSION:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - game version is too old, disconnecting" );
						break;
					case CBNETProtocol :: KR_INVALID_VERSION:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - game version is invalid, disconnecting" );
						break;
					default:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - cd keys not accepted, disconnecting" );
						break;
					}

					m_Socket->Disconnect( );
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_AUTH_ACCOUNTLOGON:
				if( m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGON( Packet->GetData( ) ) )
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] username [" + m_UserName + "] accepted" );

					if( m_PasswordHashType == "pvpgn" )
					{
						// pvpgn logon

						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using pvpgn logon type (for pvpgn servers only)" );
						m_BNCSUtil->HELP_PvPGNPasswordHash( m_UserPassword );
						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF( m_BNCSUtil->GetPvPGNPasswordHash( ) ) );
					}
					else
					{
						// battle.net logon

						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using battle.net logon type (for official battle.net servers only)" );
						m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGONPROOF( m_Protocol->GetSalt( ), m_Protocol->GetServerPublicKey( ) );
						m_Socket->PutBytes( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF( m_BNCSUtil->GetM1( ) ) );
					}
				}
				else
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - invalid username, disconnecting" );
					m_Socket->Disconnect( );
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_AUTH_ACCOUNTLOGONPROOF:
				if( m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( Packet->GetData( ) ) )
				{
					// logon successful

					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon successful" );
					m_LoggedIn = true;
					m_ChOP->EventBNETLoggedIn( this );
					m_Socket->PutBytes( m_Protocol->SEND_SID_NETGAMEPORT( 6112 ) );
					m_Socket->PutBytes( m_Protocol->SEND_SID_ENTERCHAT( ) );
					m_Socket->PutBytes( m_Protocol->SEND_SID_FRIENDSLIST( ) );
					m_Socket->PutBytes( m_Protocol->SEND_SID_CLANMEMBERLIST( ) );
				}
				else
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - invalid password, disconnecting" );

					// try to figure out if the user might be using the wrong logon type since too many people are confused by this

					string Server = m_Server;
					transform( Server.begin( ), Server.end( ), Server.begin( ), (int(*)(int))tolower );

					if( m_PasswordHashType == "pvpgn" && ( Server == "useast.battle.net" || Server == "uswest.battle.net" || Server == "asia.battle.net" || Server == "europe.battle.net" ) )
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] it looks like you're trying to connect to a battle.net server using a pvpgn logon type, check your config file's \"battle.net custom data\" section" );
					else if( m_PasswordHashType != "pvpgn" && ( Server != "useast.battle.net" && Server != "uswest.battle.net" && Server != "asia.battle.net" && Server != "europe.battle.net" ) )
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] it looks like you're trying to connect to a pvpgn server using a battle.net logon type, check your config file's \"battle.net custom data\" section" );

					m_Socket->Disconnect( );
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_WARDEN:
				WardenData = m_Protocol->RECEIVE_SID_WARDEN( Packet->GetData( ) );

				if( m_BNLSClient )
					m_BNLSClient->QueueWardenRaw( WardenData );
				else
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] warning - received warden packet but no BNLS server is available, you will be kicked from battle.net soon" );

				break;

			case CBNETProtocol :: SID_FRIENDSLIST:
				Friends = m_Protocol->RECEIVE_SID_FRIENDSLIST( Packet->GetData( ) );

				for( vector<CIncomingFriendList *> :: iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
					delete *i;

				m_Friends = Friends;

				/* DEBUG_Print( "received " + UTIL_ToString( Friends.size( ) ) + " friends" );
				for( vector<CIncomingFriendList *> :: iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
					DEBUG_Print( "friend: " + (*i)->GetAccount( ) ); */


				break;

			case CBNETProtocol :: SID_CLANMEMBERLIST: {
				vector<CIncomingClanList *> Clans = m_Protocol->RECEIVE_SID_CLANMEMBERLIST( Packet->GetData( ) );

				for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
					delete *i;

				m_Clans = Clans;
				
				// Save ChOP User ClanRank
				string chopname = m_UserName;
				transform( chopname.begin( ), chopname.end( ), chopname.begin( ), (int(*)(int))tolower );
				for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
				{
					string lowerName = (*i)->GetName( );
					transform( lowerName.begin( ), lowerName.end( ), lowerName.begin( ), (int(*)(int))tolower );

					if( lowerName == chopname )
					{
						string rank = (*i)->GetRank( );
						if( rank == "Recruit" )
							m_ClanRank = CBNETProtocol :: CLAN_RECRUIT;
						else if( rank == "Peon" )
							m_ClanRank = CBNETProtocol :: CLAN_PEON;
						else if( rank == "Grunt" )
							m_ClanRank = CBNETProtocol :: CLAN_GRUNT;
						else if( rank == "Shaman" )
							m_ClanRank = CBNETProtocol :: CLAN_SHAMAN;
						else if( rank == "Chieftain" )
							m_ClanRank = CBNETProtocol :: CLAN_CHIEF;
						else
							m_ClanRank = CBNETProtocol :: CLAN_RECRUIT;
						break;
					}
				}

				break;
			}
			
			case CBNETProtocol :: SID_CLANCREATIONINVITATION: {
				string ClanCreateName = m_Protocol->RECEIVE_SID_CLANCREATIONINVITATION( Packet->GetData( ) );
				
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] Invited (creation) to clan " + ClanCreateName + ", !accept to accept" );
				m_LastInviteCreation = true;
				break;
			}
			
			case CBNETProtocol :: SID_CLANINVITATIONRESPONSE: {
				string ClanInviteName = m_Protocol->RECEIVE_SID_CLANINVITATIONRESPONSE( Packet->GetData( ) );
				
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] Invited to clan " + ClanInviteName + ", !accept to accept" );
				m_LastInviteCreation = false;
				break;
			}
			}
		}

		delete Packet;
	}
}

void CBNET :: ProcessChatEvent( CIncomingChatEvent *chatEvent )
{
	CBNETProtocol :: IncomingChatEvent Event = chatEvent->GetChatEvent( );
	bool Whisper = ( Event == CBNETProtocol :: EID_WHISPER );
	string User = chatEvent->GetUser( );
	string Message = chatEvent->GetMessage( );

	if( Event == CBNETProtocol :: EID_WHISPER || Event == CBNETProtocol :: EID_TALK )
	{
		bool isWhisper = Event == CBNETProtocol :: EID_WHISPER;
		
		try
		{ 
			EXECUTE_HANDLER("ChatReceived", true, boost::ref(this), User, Message)
		}
		catch(...) 
		{ 
			return;
		}
		
		try
		{ 
			EXECUTE_HANDLER("ChatReceivedExtended", true, boost::ref(this), User, Message, isWhisper)
		}
		catch(...) 
		{ 
			return;
		}
	
		if( isWhisper )
			CONSOLE_Print( "[WHISPER: " + m_ServerAlias + "] [" + User + "] " + Message );
		else
			CONSOLE_Print( "[LOCAL: " + m_ServerAlias + "] [" + User + "] " + Message );

		EXECUTE_HANDLER("ChatReceived", false, boost::ref(this), User, Message)
		EXECUTE_HANDLER("ChatReceivedExtended", false, boost::ref(this), User, Message, isWhisper)

		// handle bot commands

		if( !Message.empty( ) )
		{
			string lowerUser = User;
			transform( lowerUser.begin( ), lowerUser.end( ), lowerUser.begin( ), (int(*)(int))tolower );
			
			CUser *user = GetUserByName( User );

			if( !user ) {
				user = new CUser( User, 0 );
				m_Users[lowerUser] = user;
			}

			user->SetPing( chatEvent->GetPing( ) );

			if( IsRootAdmin( User ) )
				user->SetAccess( 10 );

			if( Message == "?trigger" && ( user->GetAccess( ) > 0 || ( !m_ChOP->m_DisablePublic && m_OutPackets.size( ) > 3 ) ) )
			{
				QueueChatCommand( "Command trigger: " + string( 1, m_CommandTrigger ), User, Whisper);
			}
			else if( Message[0] == m_CommandTrigger )
			{
				// extract the command trigger, the command, and the payload
				// e.g. "!say hello world" -> command: "say", payload: "hello world"

				string Reply, Command, Payload;
				string :: size_type PayloadStart = Message.find( " " );
				uint32_t Type = Whisper ? 1 : 2;

				if( PayloadStart != string :: npos )
				{
					Command = Message.substr( 1, PayloadStart - 1 );
					Payload = Message.substr( PayloadStart + 1 );
				}
				else
					Command = Message.substr( 1 );

				transform( Command.begin( ), Command.end( ), Command.begin( ), (int(*)(int))tolower );

				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] user [" + User + ":" + UTIL_ToString( user->GetAccess( ) ) + "] sent command [" + Command + "] with payload [" + Payload + "]" );

				Reply = ProcessCommand( user, Command, Payload, Type );

				if( !Reply.empty( ) ) {
					if( user->GetAccess( ) != 0 || m_OutPackets.size( ) <= 3 ) {
						QueueChatCommand( Reply, User, Whisper );
					}
				}
			}
			else
			{
				ProcessCorePlugins( user, Message );
			}
		}
	}
	else if( Event == CBNETProtocol :: EID_JOIN || Event == CBNETProtocol :: EID_SHOWUSER )
	{
		string lowerUser = User;
		transform( lowerUser.begin( ), lowerUser.end( ), lowerUser.begin( ), (int(*)(int))tolower );

		bool Banned = IsBannedName( User );

		if( Banned && m_IsOperator && m_ChOP->m_BanlistChannel )
			QueueChatCommand( "/ban " + User + " banlist" );
		else
		{
			CUser *user = GetUserByName( User );
			if( !user )
				user = new CUser( User, 0 );
				
			m_Channel[lowerUser] = user;
			user->SetPing( chatEvent->GetPing( ) );
			
			if( IsClanMember( User ) ) {
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] clanmember [" + User + "] has joined the channel." );
			}
			else {
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] user [" + User + "] has joined the channel." );
			}

			bool isShowUser = Event == CBNETProtocol :: EID_SHOWUSER;
			EXECUTE_HANDLER("UserJoined", false, boost::ref(this), user, isShowUser)
		}
	}
	else if( Event == CBNETProtocol :: EID_LEAVE )
	{
		string lowerName = User;
		transform( lowerName.begin( ), lowerName.end( ), lowerName.begin( ), (int(*)(int))tolower );
		map<string, CUser *> :: iterator i = m_Channel.find( lowerName );

		if( i != m_Channel.end( ) ) // if the user left the channel
			m_Channel.erase( i );

		if( IsClanMember( User ) )
		{
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] clanmember [" + User + "] has left the channel." );
			UpdateSeen( User );
			
			if(m_ChOP->m_Follow >= 1) {
				QueueChatCommand( "/whois " + User );
			}
		}
		else {
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] user [" + User + "] has left the channel." );
			
			if(m_ChOP->m_SeenAllUsers) {
				UpdateSeen( User );
			}
			
			if(m_ChOP->m_Follow >= 2) {
				QueueChatCommand( "/whois " + User );
			}
		}

		EXECUTE_HANDLER("UserLeft", false, boost::ref(this), User)
	}
	else if( Event == CBNETProtocol :: EID_CHANNEL )
	{
		// keep track of current channel so we can rejoin it after hosting a game



		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] joined channel [" + Message + "]" );
		m_CurrentChannel = Message;
		m_IsOperator = false;
		m_Channel.clear( );
	}
	else if( Event == CBNETProtocol :: EID_USERFLAGS )
	{


		if( chatEvent->GetUser( ) == m_UserName )
			m_IsOperator = chatEvent->GetOperator( );
	}
	else if( Event == CBNETProtocol :: EID_INFO )
	{
		EXECUTE_HANDLER("BNETInfo", false, boost::ref(this), Message)
		
		CONSOLE_Print( "[INFO: " + m_ServerAlias + "] " + Message );

		// extract the first word which we hope is the username
		// this is not necessarily true though since info messages also include channel MOTD's and such

		string UserName;
		string :: size_type Split = Message.find( " " );

		if( Split != string :: npos )
			UserName = Message.substr( 0, Split );
		else
			UserName = Message.substr( 0 );

		if( Message.find( "is using Warcraft III The Frozen Throne in game" ) != string :: npos || Message.find( "is using Warcraft III Frozen Throne and is currently in game" ) != string :: npos )
		{
			bool displayFollow = ( m_ChOP->m_Follow >= 1 && IsClanMember( UserName ) ) || m_ChOP->m_Follow >= 2;
			if( displayFollow )
				QueueChatCommand( "/me " + UserName + " has joined the game " + Message.substr( Message.find( "game" ) + 5 ) );
		}
	}
	else if( Event == CBNETProtocol :: EID_ERROR ) {
		EXECUTE_HANDLER("BNETError", false, boost::ref(this), Message)
		
		CONSOLE_Print( "[ERROR: " + m_ServerAlias + "] " + Message );
	}
	else if( Event == CBNETProtocol :: EID_EMOTE ) {
		EXECUTE_HANDLER("EmoteReceived", false, boost::ref(this), User, Message)
		
		string lowerUser = User;
		transform( lowerUser.begin( ), lowerUser.end( ), lowerUser.begin( ), (int(*)(int))tolower );
		
		CUser *user = GetUserByName( User );

		if( !user ) {
			user = new CUser( User, 0 );
			m_Users[lowerUser] = user;
		}
			
		ProcessCorePlugins( user, Message );
		
		CONSOLE_Print( "[EMOTE: " + m_ServerAlias + "] [" + User + "] " + Message );
	}
}

void CBNET :: ProcessCorePlugins( CUser *user, string Message )
{
	uint32_t kick = 0;
	string User = user->GetName( );

	// AntiSpam

	if( m_ChOP->m_AntiSpam )
	{
		string message = Message + ";" + User;

		if( m_SpamCache.size( ) == m_ChOP->m_SpamCacheSize )
			m_SpamCache.erase( m_SpamCache.begin( ) );

		m_SpamCache.push_back( message );

		if(	user->GetAccess( ) == 0 )
		{
			int matches = 0;

			for( vector<string> :: iterator i = m_SpamCache.begin( ); i != m_SpamCache.end( ); i++ )
				if( (*i) == message )
					matches++;

			if( matches > 2 || ( message.length( ) > 3 && matches > 1 ) )
				kick = 1;
		}
	}

	// AntiYell

	if( !kick && m_ChOP->m_AntiYell && user->GetAccess( ) == 0 )
	{
		int charCount = 0;

		for( int i = 0; i != Message.length( ); i++ )
		{
			if( isalpha( Message[i] ) )
				charCount++;
		}

		if( charCount > 3 && charCount >= Message.length( ) / 2 )
		{
			string upperMessage = Message;
			transform( upperMessage.begin( ), upperMessage.end( ), upperMessage.begin( ), (int(*)(int))toupper );

			if(	Message == upperMessage )
				kick = 2;
		}
	}

	// PhraseKick

	if( !kick && m_ChOP->m_PhraseKick && user->GetAccess( ) == 0 )
	{
		string message = Message;
		transform( message.begin( ), message.end( ), message.begin( ), (int(*)(int))tolower );

		for( vector<string> :: iterator i = m_ChOP->m_Phrases.begin( ); i != m_ChOP->m_Phrases.end( ); i++ )
		{
			if( message.find( *i ) != string :: npos )
				kick = 3;
		}
	}

	// kick ?
	if( kick )
	{
		switch( kick )
		{
		case 1:
			QueueChatCommand( "/kick " + User + " " + m_ChOP->m_Language->KickSpam( ) );
			break;
		case 2:
			QueueChatCommand( "/kick " + User + " " + m_ChOP->m_Language->KickYell( ) );
			break;
		case 3:
			QueueChatCommand( "/kick " + User + " " + m_ChOP->m_Language->KickPhrase( ) );
			break;
		}
	}
}

void CBNET :: SendJoinChannel( string channel )
{
	if( m_LoggedIn && m_InChat )
		m_Socket->PutBytes( m_Protocol->SEND_SID_JOINCHANNEL( channel ) );
}

void CBNET :: SendGetFriendsList( )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_FRIENDSLIST( ) );
}

void CBNET :: SendGetClanList( )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_CLANMEMBERLIST( ) );
}

void CBNET :: QueueEnterChat( )
{
	if( m_LoggedIn )
		m_OutPackets.push( m_Protocol->SEND_SID_ENTERCHAT( ) );
}

void CBNET :: QueueChatCommand( string chatCommand )
{
	if( chatCommand.empty( ) )
		return;

	try
	{ 
		EXECUTE_HANDLER("QueueChatCommand", true, boost::ref(this), chatCommand)
	}
	catch(...) 
	{ 
		return;
	}

	EXECUTE_HANDLER("QueueChatCommand", false, boost::ref(this), chatCommand)

	if( m_LoggedIn )
	{
		if( m_PasswordHashType == "pvpgn" && chatCommand.size( ) > m_MaxMessageLength )
			chatCommand = chatCommand.substr( 0, m_MaxMessageLength );

		if( chatCommand.size( ) > 255 )
			chatCommand = chatCommand.substr( 0, 255 );

		if( m_OutPackets.size( ) > 10 )
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] attempted to queue chat command [" + chatCommand + "] but there are too many (" + UTIL_ToString( m_OutPackets.size( ) ) + ") packets queued, discarding" );
		else
		{
			CONSOLE_Print( "[QUEUED: " + m_ServerAlias + "] " + chatCommand );
			m_OutPackets.push( m_Protocol->SEND_SID_CHATCOMMAND( chatCommand ) );
		}
	}
}

void CBNET :: QueueChatCommand( string chatCommand, string user, bool whisper )
{
	if( chatCommand.empty( ) )
		return;

	try
	{ 
		EXECUTE_HANDLER("QueueChatResponse", true, boost::ref(this), chatCommand, user, whisper)
	}
	catch(...) 
	{ 
		return;
	}

	EXECUTE_HANDLER("QueueChatResponse", false, boost::ref(this), chatCommand, user, whisper)

	// if whisper is true send the chat command as a whisper to user, otherwise just queue the chat command

	if( whisper )
		QueueChatCommand( "/w " + user + " " + chatCommand );
	else
		QueueChatCommand( chatCommand );
}

void CBNET :: SendClanInvitation( string accountName )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_CLANINVITATION( accountName ) );
}

void CBNET :: SendClanRemove( string name )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_CLANREMOVE( name ) );
}

void CBNET :: SendClanChangeRank( string accountName, CBNETProtocol :: RankCode rank )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_CLANCHANGERANK( accountName, rank ) );
}

void CBNET :: SendClanMakeChieftain( string accountName )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_CLANMAKECHIEFTAIN( accountName ) );
}

void CBNET :: SendClanSetMOTD( string message )
{
	if( m_LoggedIn )
		m_Socket->PutBytes( m_Protocol->SEND_SID_CLANSETMOTD( message ) );
}

void CBNET :: SendClanAcceptInvite( bool accept )
{
	if( m_LoggedIn ) {
		BYTEARRAY SendBytes;
		
		if( m_LastInviteCreation )
			SendBytes = m_Protocol->SEND_SID_CLANCREATIONINVITATION( accept );
		else
			SendBytes = m_Protocol->SEND_SID_CLANINVITATIONRESPONSE( accept );
		
		if( SendBytes.size( ) != 0 )
			m_Socket->PutBytes( SendBytes );
	}
}

bool CBNET :: IsRootAdmin( string name )
{
	// m_RootAdmin was already transformed to lower case in the constructor

	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	return name == m_RootAdmin;
}

bool CBNET :: IsBannedName( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	// todotodo: optimize this - maybe use a map?

	for( vector<CDBBan *> :: iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
	{
		if( (*i)->GetName( ) == name )
			return true;
	}

	return false;
}

bool CBNET :: IsBannedIP( string ip )
{
	// todotodo: optimize this - maybe use a map?

	for( vector<CDBBan *> :: iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
	{
		if( (*i)->GetIP( ) == ip )
			return true;
	}

	return false;
}

CDBBan *CBNET :: GetBannedInfo( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	// todotodo: optimize this - maybe use a map?

	for( vector<CDBBan *> :: iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
	{
		if( (*i)->GetName( ) == name )
			return *i;
	}

	return NULL;
}

void CBNET :: AddBan( string name, string ip, string gamename, string admin, string reason )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	m_Bans.push_back( new CDBBan( m_Server, name, ip, "N/A", gamename, admin, reason ) );
}

void CBNET :: AddUser( string name, uint32_t access )
{
	string lowerName = name;
	transform( lowerName.begin( ), lowerName.end( ), lowerName.begin( ), (int(*)(int))tolower );
	map<string, CUser *> :: iterator i = m_Users.find( lowerName );

	if( i != m_Users.end( ) )
	{
		i->second->SetAccess( access );
		return;
	}

	m_Users[lowerName] = new CUser( name, access );
	
	// user was added; if in channel, update channel user
	map<string, CUser *> :: iterator channelIndex = m_Channel.find(lowerName);
	
	if( channelIndex != m_Channel.end( ) ) {
		delete (*channelIndex).second;
		m_Channel.erase(channelIndex);
		m_Channel[lowerName] = m_Users[lowerName];
	}
}

void CBNET :: RemoveBan( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );

	for( vector<CDBBan *> :: iterator i = m_Bans.begin( ); i != m_Bans.end( ); )
	{
		if( (*i)->GetName( ) == name )
		{
			i = m_Bans.erase( i );
			return;
		}
		else
			i++;
	}
}

void CBNET :: RemoveUser( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	map<string, CUser *> :: iterator i = m_Users.find( name );

	if( i != m_Users.end( ) ) {
		// user was deleted; if not in channel, delete, otherwise update access
		map<string, CUser *> :: iterator channelIndex = m_Channel.find(name);
	
		if( channelIndex != m_Channel.end( ) ) {
			(*channelIndex).second->SetAccess(0);
		} else {
			delete (*i).second;
		}

		// finish removing from m_Users
		m_Users.erase( i );
	}
}

string CBNET :: FormatTime( uint32_t time )
{
	string date;

	if( time >= 86400 )
	{
		uint32_t days = time / 86400;
		time = time - ( days * 86400 );
		date = UTIL_ToString( days ) + "d ";
	}
	if( time >= 3600 )
	{
		uint32_t hours = time / 3600;
		time = time - ( hours * 3600 );
		date += UTIL_ToString( hours ) + "h ";
	}
	if( time >= 60 )
	{
		uint32_t minutes = time / 60;
		time = time - ( minutes * 60 );
		date += UTIL_ToString( minutes ) + "m ";
	}
	date += UTIL_ToString( time ) + "s";
	return date;
}

string CBNET :: GetClanRank( string name )
{
	for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
	{
		if( (*i)->GetName( ) == name )
			return (*i)->GetRank( );
	}

	return "unknown";
}

bool CBNET :: IsClanMember( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	
	for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
	{
		string memberName = (*i)->GetName( );
		transform( memberName.begin( ), memberName.end( ), memberName.begin( ), (int(*)(int))tolower );
		
		if( memberName == name )
			return true;
	}

	return false;
}

bool CBNET :: IsInChannel( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	map<string, CUser *> :: iterator i = m_Channel.find( name );

	if( i != m_Channel.end( ) )
		return true;
	else
		return false;
}

uint32_t CBNET :: ChannelNameMatch( string name, string &match )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t matches = 0;

	for( map<string, CUser *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); i++ )
	{
		if( (*i).first == name )
		{
			match = (*i).second->GetName( );
			return 1;
		}
		if( (*i).first.find( name ) != string :: npos )
		{
			match = (*i).second->GetName( );
			matches++;
		}
	}

	return matches;
}

uint32_t CBNET :: ClanNameMatch( string name, string &match )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t matches = 0;

	for( vector<CIncomingClanList *> :: iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
	{
		string lowerName = (*i)->GetName( );
		transform( lowerName.begin( ), lowerName.end( ), lowerName.begin( ), (int(*)(int))tolower );

		if( lowerName == name )
		{
			match = (*i)->GetName( );
			return 1;
		}
		if( lowerName.find( name ) != string :: npos )
		{
			match = (*i)->GetName( );
			matches++;
		}
	}

	return matches;
}

CUser *CBNET :: GetUserByName( string name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	map<string, CUser *> :: iterator i;

	i = m_Channel.find( name );
	if( i != m_Channel.end( ) )
		return i->second;

	i = m_Users.find( name );
	if( i != m_Users.end( ) )
		return i->second;

	return NULL;
}

void CBNET :: UpdateSeen( string user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );

	m_CallableUserSeens.push_back( m_ChOP->m_DB->ThreadedUserSetSeen( m_Server, user ) );
}

uint32_t CBNET :: NumPackets( )
{
	return m_OutPackets.size( );
}

int64_t CBNET :: PacketDelayTime( uint32_t PacketSize, uint32_t FrequencyDelayTimes )
{
	// check if at least one packet is waiting to be sent and if we've waited long enough to prevent flooding
	// this formula has changed many times but currently we wait 1 second if the last packet was "small", 3.2 seconds if it was "medium", and 4 seconds if it was "big"

	uint32_t WaitTicks = 0;

	if( PacketSize < 10 )
		WaitTicks = 1300;
	else if( PacketSize < 30 )
		WaitTicks = 3400;
	else if( PacketSize < 50 )
		WaitTicks = 3600;
	else if( PacketSize < 100 )
		WaitTicks = 3900;
	else
		WaitTicks = 5500;

	// add on frequency delay

	WaitTicks += FrequencyDelayTimes * 60;
	
	uint32_t ElapsedTime = GetTicks( ) - m_LastOutPacketTicks;
	int64_t RemainingWait = (int64_t) WaitTicks - (int64_t) ElapsedTime;
	return RemainingWait;
}

int64_t CBNET :: DelayTime( )
{
	return PacketDelayTime( m_LastOutPacketSize, m_FrequencyDelayTimes );
}

int64_t CBNET :: TotalDelayTime( )
{
	// checks unsent packets, in addition to last packet's delay time

	int64_t TotalDelayTime = DelayTime( );
	uint32_t VirtualFrequencyDelayTimes = m_FrequencyDelayTimes;
	
	queue<BYTEARRAY> OutPackets = m_OutPackets;
	
	while( OutPackets.size( ) > 0 )
	{
		TotalDelayTime += PacketDelayTime( OutPackets.front( ).size( ), VirtualFrequencyDelayTimes );
		
		if( VirtualFrequencyDelayTimes >= 100 )
			VirtualFrequencyDelayTimes = 0;
		else
			VirtualFrequencyDelayTimes++;
		
		OutPackets.pop( );
	}
	
	return TotalDelayTime;
}

vector<string> CBNET :: GetChannelNameList( )
{
	vector<string> channelNameList;
	
	for( map<string, CUser *> :: iterator i = m_Channel.begin( ); i != m_Channel.end( ); i++ )
	{
		channelNameList.push_back((*i).first);
	}
	
	return channelNameList;
}

vector<string> CBNET :: GetUserNameList( )
{
	vector<string> userNameList;
	
	for( map<string, CUser *> :: iterator i = m_Users.begin( ); i != m_Users.end( ); i++ )
	{
		userNameList.push_back((*i).first);
	}
	
	return userNameList;
}

CIncomingClanList CBNET :: GetClanMember( uint32_t position )
{
	return *(m_Clans[position]);
}

CIncomingFriendList CBNET :: GetFriend( uint32_t position )
{
	return *(m_Friends[position]);
}

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

void CBNET :: RegisterPythonClass( )
{
	using namespace boost::python;

	void (CBNET::*QueueChatCommand1)(string)				= &CBNET::QueueChatCommand;
	void (CBNET::*QueueChatCommand2)(string, string, bool)  = &CBNET::QueueChatCommand;
	
	class_<std::vector<std::string> >("STD_String")
		.def(vector_indexing_suite<std::vector<std::string> >())
	;

	class_<CBNET>("BNET", no_init)
		.def_readonly("ChOP", &CBNET::m_ChOP)

		.add_property("socket", make_getter(&CBNET::m_Socket, return_value_policy<reference_existing_object>()))
		.add_property("protocol", make_getter(&CBNET::m_Protocol, return_value_policy<reference_existing_object>()) )
		.add_property("BNLSClient", make_getter(&CBNET::m_BNLSClient, return_value_policy<reference_existing_object>()) )
		.def_readonly("packets", &CBNET::m_Packets)
		.add_property("BNCSUtil", make_getter(&CBNET::m_BNCSUtil, return_value_policy<reference_existing_object>()) )
		.def_readonly("m_OutPackets", &CBNET::m_OutPackets)
		.def_readonly("pairedBanCounts", &CBNET::m_PairedBanCounts)
		.def_readonly("pairedBanAdds", &CBNET::m_PairedBanAdds)
		.def_readonly("pairedBanRemoves", &CBNET::m_PairedBanRemoves)
		.def_readonly("pairedGPSChecks", &CBNET::m_PairedGPSChecks)
		.def_readonly("pairedDPSChecks", &CBNET::m_PairedDPSChecks)
		.add_property("callableBanList", make_getter(&CBNET::m_CallableBanList, return_value_policy<reference_existing_object>()) )
		.def_readonly("bans", &CBNET::m_Bans)
		.def_readonly("exiting", &CBNET::m_Exiting)
		.def_readonly("server", &CBNET::m_Server)
		.def_readonly("serverIP", &CBNET::m_ServerIP)
		.def_readonly("serverAlias", &CBNET::m_ServerAlias)
		.def_readonly("BNLSServer", &CBNET::m_BNLSServer)
		.def_readonly("BNLSPort", &CBNET::m_BNLSPort)
		.def_readonly("BNLSWardenCookie", &CBNET::m_BNLSWardenCookie)
		.def_readonly("CDKeyROC", &CBNET::m_CDKeyROC)
		.def_readonly("countryAbbrev", &CBNET::m_CountryAbbrev)
		.def_readonly("country", &CBNET::m_Country)
		.def_readonly("userName", &CBNET::m_UserName)
		.def_readonly("userPassword", &CBNET::m_UserPassword)
		.def_readonly("firstChannel", &CBNET::m_FirstChannel)
		.def_readonly("currentChannel", &CBNET::m_CurrentChannel)
		.def_readonly("rootAdmin", &CBNET::m_RootAdmin)
		.def_readonly("commandTrigger", &CBNET::m_CommandTrigger)
		.def_readonly("War3Version", &CBNET::m_War3Version)
		.def_readonly("EXEVersion", &CBNET::m_EXEVersion)
		.def_readonly("EXEVersionHash", &CBNET::m_EXEVersionHash)
		.def_readonly("passwordHashType", &CBNET::m_PasswordHashType)
		.def_readonly("maxMessageLength", &CBNET::m_MaxMessageLength)
		.def_readonly("lastNullTime", &CBNET::m_LastNullTime)
		.def_readonly("lastOutPacketTicks", &CBNET::m_LastOutPacketTicks)
		.def_readonly("lastOutPacketSize", &CBNET::m_LastOutPacketSize)
		.def_readonly("lastBanRefreshTime", &CBNET::m_LastBanRefreshTime)
		.def_readonly("waitingToConnect", &CBNET::m_WaitingToConnect)
		.def_readonly("loggedIn", &CBNET::m_LoggedIn)
		.def_readonly("inChat", &CBNET::m_InChat)
		
		.def_readonly("channel", &CBNET::m_Channel)
		.def_readonly("users", &CBNET::m_Users)


		.def("getExiting", &CBNET::GetExiting)
		.def("getServer", &CBNET::GetServer)
		.def("getServerAlias", &CBNET::GetServerAlias)
		.def("getCDKeyROC", &CBNET::GetCDKeyROC)
		.def("getUserName", &CBNET::GetUserName)
		.def("getUserPassword", &CBNET::GetUserPassword)
		.def("getFirstChannel", &CBNET::GetFirstChannel)
		.def("getCurrentChannel", &CBNET::GetCurrentChannel)
		.def("getRootAdmin", &CBNET::GetRootAdmin)
		.def("getCommandTrigger", &CBNET::GetCommandTrigger)
		.def("getEXEVersion", &CBNET::GetEXEVersion)
		.def("getEXEVersionHash", &CBNET::GetEXEVersionHash)
		.def("getPasswordHashType", &CBNET::GetPasswordHashType)
		.def("getLoggedIn", &CBNET::GetLoggedIn)
		.def("getInChat", &CBNET::GetInChat)
		.def("getOutPacketsQueued", &CBNET::GetOutPacketsQueued)
		.def("getUniqueName", &CBNET::GetUniqueName)

		.def("sendJoinChannel", &CBNET::SendJoinChannel)
		.def("sendGetFriendsList", &CBNET::SendGetFriendsList)
		.def("sendGetClanList", &CBNET::SendGetClanList)
		.def("queueEnterChat", &CBNET::QueueEnterChat)
		.def("queueChatCommand", QueueChatCommand1)
		.def("queueChatCommand", QueueChatCommand2)
		.def("sendClanInvitation", &CBNET::SendClanInvitation)
		.def("sendClanRemove", &CBNET::SendClanRemove)
		.def("sendClanChangeRank", &CBNET::SendClanChangeRank)
		.def("sendClanSetMOTD", &CBNET::SendClanSetMOTD)

		.def("isRootAdmin", &CBNET::IsRootAdmin)
		.def("isBannedName", &CBNET::IsBannedName)
		.def("isBannedIP", &CBNET::IsBannedIP)
		.def("addUser", &CBNET::AddUser)
		.def("addBan", &CBNET::AddBan)
		.def("removeUser", &CBNET::RemoveUser)
		.def("removeBan", &CBNET::RemoveBan)
		
		.def("formatTime", &CBNET::FormatTime)
		.def("getClanRank", &CBNET::GetClanRank)
		.def("isClanMember", &CBNET::IsClanMember)
		.def("isInChannel", &CBNET::IsInChannel)
		.def("channelNameMatch", &CBNET::ChannelNameMatch)
		.def("clanNameMatch", &CBNET::ClanNameMatch)
		.def("getUserByName", &CBNET::GetUserByName, return_internal_reference<>())
		.def("updateSeen", &CBNET::UpdateSeen)
		.def("numPackets", &CBNET::NumPackets)
		.def("delayTime", &CBNET::DelayTime)
		.def("totalDelayTime", &CBNET::TotalDelayTime)
		
		.def("getUserNameList", &CBNET::GetUserNameList)
		.def("getChannelNameList", &CBNET::GetChannelNameList)
		.def("getNumClanMembers", &CBNET::GetNumClanMembers)
		.def("getNumFriends", &CBNET::GetNumFriends)
		.def("getClanMember", &CBNET::GetClanMember)
		.def("getFriend", &CBNET::GetFriend)
	;
}
