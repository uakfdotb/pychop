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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h> 
#include <math.h>
#include <cstdlib>
#include <iostream>
#include <fstream>

string CBNET :: ProcessCommand( CUser *User, string command, string payload, uint32_t type )
{
	try	
	{ 
		EXECUTE_HANDLER("ProcessCommand", true, boost::ref(this), boost::ref(User), command, payload, type)
	}
	catch(...) 
	{ 
		return "";
	}
	
	bool Console = ( type == 0 );
	bool Whisper = ( type == 1 );
	bool Chat = ( type == 2 );
	uint32_t CommandAccess;
	uint32_t Access = User->GetAccess( );
	string UserName = User->GetName( );

	// are commands in whisperes interpreted ? (can be set in chop.cfg)
	if( Whisper && !m_ChOP->m_WhisperAllowed && Access > 0 )
		return m_ChOP->m_Language->WhisperCommandsDisabled( );
	
	//put at beginning or someone else might return something...
	EXECUTE_HANDLER("ProcessCommand", false, boost::ref(this), boost::ref(User), command, payload, type)
	
	int i = m_CFG.GetInt( command, -1 );

	if( i > 10 )
	{
		// fix invalid access level
		CommandAccess = 10;
	}
	else if( i < 0 )
	{
		// unknown command, ignore
		return "";
	}
	else
		CommandAccess = i;

	// does the user have enough access?
	if( Access < CommandAccess )
	{
		// only return a message if the user has access at all
		if( Access > 0 )
			return m_ChOP->m_Language->YouDontHaveAccessToThatCommand( );
		else
			return "";
	}

	// Command system
	//
	//	User:		CUser class of the triggering user
	//	UserName:	Name of the triggering user
	//	Access:		Access of the triggering user
	//	Console:	true if the input came from the console
	//	Whisper:	true if the input came from a whisper
	//	Chat:		true if the input came from ordinary chat
	//	command:	the command
	//	payload:	the payload
	//
	//	always use a return at the end of a command
	//	use 'return "";' to not output any chat message
	//
	//	you can use the bools to limit commands to specific sources:
	//		'Console', 'Whisper' and 'Chat'

	// user / access commands

	if( command == "add" )
	{
		if( payload.empty( ) )
			return "";

		stringstream ss;
		string name;
		uint32_t newAccess;

		ss << payload;
		ss >> name;
		ss >> newAccess;

		if( newAccess > 10 )
			newAccess = 10;

		if( ss.fail( ) || name == m_RootAdmin )
			return m_ChOP->m_Language->ErrorAddingUser( name );

		// grab access of the targetted user
		uint32_t access = 0;
		CUser *user = GetUserByName( name );

		if( user )
		{
			access = user->GetAccess( );
			name = user->GetName( );
		}

		// only allow users with access < 10 to add other users with less access
		if( Access < 10 && ( Access <= access || Access <= newAccess ) )
			return m_ChOP->m_Language->ErrorAddingUser( name );

		// check for potential abuse
		if( name.size( ) < 16 && name[0] != '/' )
			m_PairedUserAdds.push_back( PairedUserAdd( Whisper ? UserName : string( ), m_ChOP->m_DB->ThreadedUserAdd( m_Server, name, newAccess ) ) );

		return "";
	}

	if( command == "del" )
	{
		if( payload.empty( ) )
			return "";

		// prevent deleting the root admin as he is not in the database
		if( payload == m_RootAdmin )
			return m_ChOP->m_Language->YouCantDeleteTheRootAdmin( );

		CUser *user = GetUserByName( payload );
		if( user )
		{
			if( Access < 10 && Access <= user->GetAccess( ) )
				return m_ChOP->m_Language->ErrorDeletingUser( payload );
		}

		// check for potential abuse
		if( payload.size( ) < 16 && payload[0] != '/' )
			m_PairedUserDeletes.push_back( PairedUserDelete( Whisper ? UserName : string( ), m_ChOP->m_DB->ThreadedUserDelete( m_Server, payload ) ) );

		return "";
	}

	if( command == "count" )
	{
		m_PairedUserCounts.push_back( PairedUserCount( Whisper ? UserName : string( ), m_ChOP->m_DB->ThreadedUserCount( m_Server ) ) );

		return "";
	}

	if( command == "access" || command == "whoami" || command == "whois" )
	{
		if( command == "whois" && payload.empty( ) )
			return "";

		string AccessUser = UserName;

		if( !payload.empty( ) && command != "whoami" )
			AccessUser = payload;

		CUser *user = GetUserByName( AccessUser );
		uint32_t access = 0;

		if( user )
		{
			AccessUser = user->GetName( );
			access = user->GetAccess( );
		}

		return m_ChOP->m_Language->UserHasAccess( m_Server, AccessUser, UTIL_ToString( access ) );
	}


	// ban commands
	
	if( command == "addban" || command == "ban" )
	{
		if( payload.empty( ) || !m_IsOperator )
			return "";

		stringstream ss;
		string name, reason, tmp, victim;

		// extract the victim and reason: "ban victim reason can be longer"
		//	-> victim: "victim", reason: "reason can be longer"

		ss << payload;
		ss >> name;
		while( ss >> tmp )
			reason += " " + tmp;
		if( ChannelNameMatch( name, victim ) != 1 )
			return "";

		QueueChatCommand( "/ban " + victim + " " + reason );
		return "";
	}

	if( command == "delban" || command == "unban" )
	{
		if( payload.empty( ) || !m_IsOperator )
			return "";

		QueueChatCommand( "/unban " + payload );
		return "";
	}

	if( command == "checkban" )
	{
		if( payload.empty( ) )
			return "";

		CDBBan *Ban = IsBannedName( payload );

		if( Ban )
			return m_ChOP->m_Language->UserWasBannedOnByBecause( m_Server, payload, Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) );
		else
			return m_ChOP->m_Language->UserIsNotBanned( m_Server, payload );
	}

	if( command == "countbans" )
	{
		m_PairedBanCounts.push_back( PairedBanCount( Whisper ? UserName : string( ), m_ChOP->m_DB->ThreadedBanCount( m_Server ) ) );

		return "";
	}


	// channel commands

	if( command == "channel" )
	{
		if( payload.empty( ) )
			return "";

		QueueChatCommand( "/join " + payload );
		return "";
	}

	if( command == "say" )
	{
		if( payload.empty( ) )
			return "";

		if( payload[0] == '/' )
		{
			uint32_t access;
			int i = m_CFG.GetInt( "saycommand" , -1 );

			// check for invalid number or command not found
			if( i > 10 || i < 0 )
				access = 10;
			else
				access = i;

			// does the user have enough access to perform BNet commands ?
			if( Access < access )
			{
				// only return a message if the user has access at all
				if( Access > 0 )
					return m_ChOP->m_Language->YouDontHaveAccessToThatCommand( );
				else
					return "";
			}
		}

		QueueChatCommand( payload );
		return "";
	}

	if( command == "kick" )
	{
		if( payload.empty( ) || !m_IsOperator )
			return "";

		// extract the victim and reason: "kick victim reason can be longer"
		//	-> victim: "victim", reason: "reason can be longer"

		stringstream ss;
		string name, reason, tmp, victim;

		ss << payload;
		ss >> name;
		while( ss >> tmp )
			reason += " " + tmp;

		if( ChannelNameMatch( name, victim ) != 1 )
			return "";

		QueueChatCommand( "/kick " + victim + reason );
		return "";
	}

	if( command == "designate" )
	{
		if( payload.empty( ) || !m_IsOperator )
			return "";

		string name;

		if( ChannelNameMatch( payload, name ) != 1 )
			return "";

		QueueChatCommand( "/designate " + name );
		QueueChatCommand( "/rejoin" );
		return "";
	}

	if( command == "rejoin" )
	{
		QueueChatCommand( "/rejoin" );
		return "";
	}


	// clan commands

	if( command == "invite" )
	{
		if( payload.empty( ) || m_ClanRank < CBNETProtocol :: CLAN_SHAMAN )
			return "";

		string name;
		if( ChannelNameMatch( payload, name ) > 1 )
			return "";

		if( name.empty( ) )
			name = payload;

		SendClanInvitation( name );
		SendGetClanList( );
		CONSOLE_Print( "[Clan] " + name + " was invited by " + UserName + " to join the clan" );

		return m_ChOP->m_Language->InvitedToClan( m_Server, name );
	}

	if( command == "remove" )
	{
		if( payload.empty( ) || m_ClanRank < CBNETProtocol :: CLAN_SHAMAN )
			return "";

		string name;
		if( ClanNameMatch( payload, name ) != 1 )
			return "";

		SendClanRemove( name );
		SendGetClanList( );
		CONSOLE_Print( "[Clan] " + name + " was removed from the clan by " + UserName );

		return m_ChOP->m_Language->RemovedFromClan( m_Server, name );
	}

	if( command == "motd" )
	{
		if( payload.empty( ) || m_ClanRank < CBNETProtocol :: CLAN_SHAMAN )
			return "";

		SendClanSetMOTD( payload );
		CONSOLE_Print( "[Clan] " + UserName + " changed the MOTD to " + payload );

		return m_ChOP->m_Language->MOTDChanged( );
	}

	if( command == "peon" )
	{
		if( payload.empty( ) || m_ClanRank < CBNETProtocol :: CLAN_SHAMAN )
			return "";

		string name;
		if( ClanNameMatch( payload, name ) != 1 )
			return "";

		SendClanChangeRank( name, CBNETProtocol :: CLAN_PEON );
		SendGetClanList( );
		CONSOLE_Print( "[ChOP++] " + UserName + " made " + name + " a peon" );

		return m_ChOP->m_Language->ChangedClanRank( m_Server, name, "peon" );
	}

	if( command == "grunt" )
	{
		if( payload.empty( ) || m_ClanRank < CBNETProtocol :: CLAN_SHAMAN )
			return "";

		string name;
		if( ClanNameMatch( payload, name ) != 1 )
			return "";

		SendClanChangeRank( name, CBNETProtocol :: CLAN_GRUNT );
		SendGetClanList( );
		CONSOLE_Print( "[ChOP++] " + UserName + " made " + name + " a grunt" );

		return m_ChOP->m_Language->ChangedClanRank( m_Server, name, "grunt" );
	}

	if( command == "shaman" )
	{
		if( payload.empty( ) || m_ClanRank < CBNETProtocol :: CLAN_CHIEF )
			return "";

		string name;
		if( ClanNameMatch( payload, name ) != 1 )
			return "";

		SendClanChangeRank( name, CBNETProtocol :: CLAN_SHAMAN );
		SendGetClanList( );
		CONSOLE_Print( "[ChOP++] " + UserName + " made " + name + " a shaman" );

		return m_ChOP->m_Language->ChangedClanRank( m_Server, name, "shaman" );
	}


	// management commands

	if( command == "dbstatus" )
	{
		return m_ChOP->m_DB->GetStatus( );
	}

	if( command == "wardenstatus" )
	{
		if( m_BNLSClient )
			return "WARDEN STATUS --- " + UTIL_ToString( m_BNLSClient->GetTotalWardenIn( ) ) + " requests received, " + UTIL_ToString( m_BNLSClient->GetTotalWardenOut( ) ) + " responses sent";
		else
			return "WARDEN STATUS --- Not connected to BNLS server.";
	}

	if( command == "lastwhisper" || command == "lw" )
	{
		if( m_LastWhisper.empty( ) )
			return m_ChOP->m_Language->NoWhispersYet( );

		return m_ChOP->m_Language->LastWhisperFrom( m_LastWhisper );
	}

	if( command == "getclan" )
	{
		SendGetClanList( );
		return m_ChOP->m_Language->UpdatingClanList( );
	}

	if( command == "getfriends" )
	{
		SendGetFriendsList( );
		return m_ChOP->m_Language->UpdatingFriendsList( );
	}

	if( command == "exit" || command == "quit" )
	{
		m_Exiting = true;

		return "";
	}

	if( command == "uptime" )
	{
		return "Uptime: " + FormatTime( GetTime( ) - m_ChOP->m_UpTime );
	}

	if( command == "reload" )
	{
		m_ChOP->UpdateAsk8Ball( );
		m_ChOP->UpdatePhrases( );
		m_ChOP->UpdateQuotes( );
		m_ChOP->UpdateSlap( );
	}


	// misc commands

	if( command == "seen" )
	{
		if( payload.empty( ) )
			return "";

		string name;
		if( ClanNameMatch( payload, name ) > 1 )
			return "";

		if( name.empty( ) ) {
			//target is not in clan; seen anyway?
			
			if(m_ChOP->m_SeenAllUsers) {
				name = payload;
			} else {
				return m_ChOP->m_Language->UserNotInClan( payload );
			}
		}

		if( m_Channel.find( name ) != m_Channel.end( ) )
			return m_ChOP->m_Language->UserIsHere( name );

		if( name.size( ) < 16 && name[0] != '/' )
			m_PairedUserSeens.push_back( PairedUserSeen( Whisper ? UserName : string( ), m_ChOP->m_DB->ThreadedUserSeen( m_Server, name ) ) );

		return "";
	}

	if( command == "version" )
	{
		return m_ChOP->m_Language->Version( m_ChOP->m_Version );
	}

	if( command == "ping" )
	{
		string PingUser;
		string name = UserName;

		if( !payload.empty( ) )
			name = payload;

		// we only know the ping of users in the channel
		// so exit if target is not in the channel

		if( ChannelNameMatch( name, PingUser ) != 1 )
			return "";

		CUser *Result = GetUserByName( PingUser );

        if(Result) {
            return m_ChOP->m_Language->UsersPingIs( PingUser, UTIL_ToString( Result->GetPing( ) ) );
        }
    }

	if( command == "owner" )
		return m_ChOP->m_Language->BotOwner( m_RootAdmin );


	// stats commands

	if( command == "stats" )
	{
		if( m_GHostIsInChannel )
			return "";

		string StatsUser = UserName;

		if( !payload.empty( ) )
			StatsUser = payload;

		// check for potential abuse

		if( StatsUser.size( ) < 16 && StatsUser[0] != '/' )
			m_PairedGPSChecks.push_back( PairedGPSCheck( Whisper ? UserName : string( ), m_ChOP->m_DB->ThreadedGamePlayerSummaryCheck( StatsUser ) ) );

		return "";
	}

	if( command == "statsdota" )
	{
		if( m_GHostIsInChannel )
			return "";

		string StatsUser = UserName;

		if( !payload.empty( ) )
			StatsUser = payload;

		// check for potential abuse

		if( StatsUser.size( ) < 16 && StatsUser[0] != '/' )
			m_PairedDPSChecks.push_back( PairedDPSCheck( Whisper ? UserName : string( ), m_ChOP->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser ) ) );

		return "";
	}


	// fun commands

	if( command == "slap" )
	{
		string victim, chop, ghost;
		uint32_t i, r;

		victim = payload;
		transform( victim.begin( ), victim.end( ), victim.begin( ), (int(*)(int))tolower );
		chop = m_UserName;
		transform( chop.begin( ), chop.end( ), chop.begin( ), (int(*)(int))tolower );
		ghost = m_ChOP->m_GHostServerAccount;
		transform( ghost.begin( ), ghost.end( ), ghost.begin( ), (int(*)(int))tolower );
		i = rand( );
		r = rand( );

		if( victim == chop || victim == ghost || rand( ) % 10 < 2 )
		{
			i %= m_ChOP->m_SlapNegative.size( );
			return m_ChOP->m_Language->Slap( m_ChOP->m_SlapNegative[i], UserName, payload, UTIL_ToString( r ) );
		}
		i %= m_ChOP->m_SlapPositive.size( );

		return m_ChOP->m_Language->Slap( m_ChOP->m_SlapPositive[i], UserName, payload, UTIL_ToString( r ) );
	}

	if( command == "ask8ball" )
	{
		uint32_t i = rand( ) % m_ChOP->m_Ask8Ball.size( );

		return m_ChOP->m_Ask8Ball[i];
    }

	if( command == "addquote" )
	{
		string path = m_ChOP->m_CFGPath + "quote.txt";
		ofstream file( path.c_str( ), ios :: app );

		if( file.fail( ) )
			return "";

		file << payload << endl;
		file.close( );
		m_ChOP->UpdateQuotes( );

		return m_ChOP->m_Language->QuoteAdded( );
	}

	if( command == "quote" )
	{
		uint32_t i;

		i = rand( ) % m_ChOP->m_Quotes.size( );

		QueueChatCommand( "/me " + m_ChOP->m_Quotes[i] );
		return "";
	}
	
	return "";
}
