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
#include "bnetprotocol.h"

CBNETProtocol :: CBNETProtocol( )
{
	unsigned char ClientToken[] = { 220, 1, 203, 7 };
	m_ClientToken = UTIL_CreateByteArray( ClientToken, 4 );
}

CBNETProtocol :: ~CBNETProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

bool CBNETProtocol :: RECEIVE_SID_NULL( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_NULL" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length

	return ValidateLength( data );
}

bool CBNETProtocol :: RECEIVE_SID_ENTERCHAT( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_ENTERCHAT" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// null terminated string	-> UniqueName

	if( ValidateLength( data ) && data.size( ) >= 5 )
	{
		m_UniqueName = UTIL_ExtractCString( data, 4 );
		return true;
	}

	return false;
}

CIncomingChatEvent *CBNETProtocol :: RECEIVE_SID_CHATEVENT( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CHATEVENT" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> EventID
	// 4 bytes					-> UserFlags
	// 4 bytes					-> Ping
	// 4 bytes					-> IPAddress [defunct]
	// 4 bytes					-> AccountNumber [defunct]
	// 4 bytes					-> RegistrationAuthority [defunct]
	// null terminated string	-> User
	// null terminated string	-> Message

	if( ValidateLength( data ) && data.size( ) >= 29 )
	{
		BYTEARRAY EventID = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );
		BYTEARRAY UserFlag = BYTEARRAY( data.begin() + 8, data.begin( ) + 12);
		BYTEARRAY Ping = BYTEARRAY( data.begin( ) + 12, data.begin( ) + 16 );
		BYTEARRAY User = UTIL_ExtractCString( data, 28 );
		BYTEARRAY Message = UTIL_ExtractCString( data, User.size( ) + 29 );
		
		//DEBUG_Print( "EventID: " + UTIL_ToString( UTIL_ByteArrayToUInt32( EventID, false ) ) + " User: " + string( User.begin( ), User.end( ) ) + " Flag: " + UTIL_ToString( UTIL_ByteArrayToUInt32( BYTEARRAY( data.begin() + 8, data.begin( ) + 12) , false) ) );
		switch( UTIL_ByteArrayToUInt32( EventID, false ) )
		{
		case CBNETProtocol :: EID_SHOWUSER:
		case CBNETProtocol :: EID_JOIN:
		case CBNETProtocol :: EID_LEAVE:
		case CBNETProtocol :: EID_WHISPER:
		case CBNETProtocol :: EID_TALK:
		case CBNETProtocol :: EID_BROADCAST:
		case CBNETProtocol :: EID_CHANNEL:
		case CBNETProtocol :: EID_USERFLAGS:
		case CBNETProtocol :: EID_WHISPERSENT:
		case CBNETProtocol :: EID_CHANNELFULL:
		case CBNETProtocol :: EID_CHANNELDOESNOTEXIST:
		case CBNETProtocol :: EID_CHANNELRESTRICTED:
		case CBNETProtocol :: EID_INFO:
		case CBNETProtocol :: EID_ERROR:
		case CBNETProtocol :: EID_EMOTE:
			return new CIncomingChatEvent(	(CBNETProtocol :: IncomingChatEvent)UTIL_ByteArrayToUInt32( EventID, false ),
												( UTIL_ByteArrayToUInt32( UserFlag, false ) == FLAG_OP ),
												UTIL_ByteArrayToUInt32( Ping, false ),
												string( User.begin( ), User.end( ) ),
												string( Message.begin( ), Message.end( ) ) );
		}

	}
	return NULL;
}

bool CBNETProtocol :: RECEIVE_SID_CHECKAD( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CHECKAD" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length

	return ValidateLength( data );
}

bool CBNETProtocol :: RECEIVE_SID_STARTADVEX3( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_STARTADVEX3" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		BYTEARRAY Status = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );

		if( UTIL_ByteArrayToUInt32( Status, false ) == 0 )
			return true;
	}

	return false;
}

BYTEARRAY CBNETProtocol :: RECEIVE_SID_PING( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_PING" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Ping

	if( ValidateLength( data ) && data.size( ) >= 8 )
		return BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );

	return BYTEARRAY( );
}

bool CBNETProtocol :: RECEIVE_SID_LOGONRESPONSE( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_LOGONRESPONSE" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		BYTEARRAY Status = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );

		if( UTIL_ByteArrayToUInt32( Status, false ) == 1 )
			return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_INFO( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_INFO" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> LogonType
	// 4 bytes					-> ServerToken
	// 4 bytes					-> ???
	// 8 bytes					-> MPQFileTime
	// null terminated string	-> IX86VerFileName
	// null terminated string	-> ValueStringFormula

	if( ValidateLength( data ) && data.size( ) >= 25 )
	{
		m_LogonType = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );
		m_ServerToken = BYTEARRAY( data.begin( ) + 8, data.begin( ) + 12 );
		m_MPQFileTime = BYTEARRAY( data.begin( ) + 16, data.begin( ) + 24 );
		m_IX86VerFileName = UTIL_ExtractCString( data, 24 );
		m_ValueStringFormula = UTIL_ExtractCString( data, m_IX86VerFileName.size( ) + 25 );
		return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_CHECK( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_CHECK" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> KeyState
	// null terminated string	-> KeyStateDescription

	if( ValidateLength( data ) && data.size( ) >= 9 )
	{
		m_KeyState = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );
		m_KeyStateDescription = UTIL_ExtractCString( data, 8 );

		if( UTIL_ByteArrayToUInt32( m_KeyState, false ) == KR_GOOD )
			return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_ACCOUNTLOGON( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_ACCOUNTLOGON" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status
	// if( Status == 0 )
	//		32 bytes			-> Salt
	//		32 bytes			-> ServerPublicKey

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		BYTEARRAY status = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );

		if( UTIL_ByteArrayToUInt32( status, false ) == 0 && data.size( ) >= 72 )
		{
			m_Salt = BYTEARRAY( data.begin( ) + 8, data.begin( ) + 40 );
			m_ServerPublicKey = BYTEARRAY( data.begin( ) + 40, data.begin( ) + 72 );
			return true;
		}
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_ACCOUNTLOGONPROOF" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		uint32_t Status = UTIL_ByteArrayToUInt32( BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 ), false );

		if( Status == 0 || Status == 0xE )
			return true;
	}

	return false;
}

BYTEARRAY CBNETProtocol :: RECEIVE_SID_WARDEN( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_WARDEN" );
	// DEBUG_PRINT( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// n bytes					-> Data

	if( ValidateLength( data ) && data.size( ) >= 4 )
		return BYTEARRAY( data.begin( ) + 4, data.end( ) );

	return BYTEARRAY( );
}

vector<CIncomingFriendList *> CBNETProtocol :: RECEIVE_SID_FRIENDSLIST( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_FRIENDSLIST" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 1 byte					-> Total
	// for( 1 .. Total )
	//		null term string	-> Account
	//		1 byte				-> Status
	//		1 byte				-> Area
	//		4 bytes				-> ???
	//		null term string	-> Location

	vector<CIncomingFriendList *> Friends;

	if( ValidateLength( data ) && data.size( ) >= 5 )
	{
		unsigned int i = 5;
		unsigned char Total = data[4];

		while( Total > 0 )
		{
			Total--;

			if( data.size( ) < i + 1 )
				break;

			BYTEARRAY Account = UTIL_ExtractCString( data, i );
			i += Account.size( ) + 1;

			if( data.size( ) < i + 7 )
				break;

			unsigned char Status = data[i];
			unsigned char Area = data[i + 1];
			i += 6;
			BYTEARRAY Location = UTIL_ExtractCString( data, i );
			i += Location.size( ) + 1;
			Friends.push_back( new CIncomingFriendList(	string( Account.begin( ), Account.end( ) ),
														Status,
														Area,
														string( Location.begin( ), Location.end( ) ) ) );
		}
	}

	return Friends;
}

vector<CIncomingClanList *> CBNETProtocol :: RECEIVE_SID_CLANMEMBERLIST( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CLANMEMBERLIST" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> ???
	// 1 byte					-> Total
	// for( 1 .. Total )
	//		null term string	-> Name
	//		1 byte				-> Rank
	//		1 byte				-> Status
	//		null term string	-> Location

	vector<CIncomingClanList *> ClanList;

	if( ValidateLength( data ) && data.size( ) >= 9 )
	{
		unsigned int i = 9;
		unsigned char Total = data[8];

		while( Total > 0 )
		{
			Total--;

			if( data.size( ) < i + 1 )
				break;

			BYTEARRAY Name = UTIL_ExtractCString( data, i );
			i += Name.size( ) + 1;

			if( data.size( ) < i + 3 )
				break;

			unsigned char Rank = data[i];
			unsigned char Status = data[i + 1];
			i += 2;

			// in the original VB source the location string is read but discarded, so that's what I do here

			BYTEARRAY Location = UTIL_ExtractCString( data, i );
			i += Location.size( ) + 1;
			ClanList.push_back( new CIncomingClanList(	string( Name.begin( ), Name.end( ) ),
														Rank,
														Status ) );
		}
	}

	return ClanList;
}

CIncomingClanList *CBNETProtocol :: RECEIVE_SID_CLANMEMBERSTATUSCHANGE( BYTEARRAY data )
{
	// DEBUG_Print( "RECEIVED SID_CLANMEMBERSTATUSCHANGE" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// null terminated string	-> Name
	// 1 byte					-> Rank
	// 1 byte					-> Status
	// null terminated string	-> Location

	if( ValidateLength( data ) && data.size( ) >= 5 )
	{
		BYTEARRAY Name = UTIL_ExtractCString( data, 4 );

		if( data.size( ) >= Name.size( ) + 7 )
		{
			unsigned char Rank = data[Name.size( ) + 5];
			unsigned char Status = data[Name.size( ) + 6];

			// in the original VB source the location string is read but discarded, so that's what I do here

			BYTEARRAY Location = UTIL_ExtractCString( data, Name.size( ) + 7 );
			return new CIncomingClanList(	string( Name.begin( ), Name.end( ) ),
											Rank,
											Status );
		}
	}

	return NULL;
}

////////////////////
// SEND FUNCTIONS //
////////////////////

BYTEARRAY CBNETProtocol :: SEND_PROTOCOL_INITIALIZE_SELECTOR( )
{
	BYTEARRAY packet;
	packet.push_back( 1 );
	// DEBUG_Print( "SENT PROTOCOL_INITIALIZE_SELECTOR" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_NULL( )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_NULL );				// SID_NULL
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_NULL" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_ENTERCHAT( )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_ENTERCHAT );			// SID_ENTERCHAT
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// Account Name is NULL on Warcraft III/The Frozen Throne
	packet.push_back( 0 );						// Stat String is NULL on CDKEY'd products
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_ENTERCHAT" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_JOINCHANNEL( string channel )
{
	unsigned char NoCreateJoin[]	= { 2, 0, 0, 0 };
	unsigned char FirstJoin[]		= { 1, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );				// BNET header constant
	packet.push_back( SID_JOINCHANNEL );					// SID_JOINCHANNEL
	packet.push_back( 0 );									// packet length will be assigned later
	packet.push_back( 0 );									// packet length will be assigned later

	if( channel.size( ) > 0 )
		UTIL_AppendByteArray( packet, NoCreateJoin, 4 );	// flags for no create join
	else
		UTIL_AppendByteArray( packet, FirstJoin, 4 );		// flags for first join

	UTIL_AppendByteArrayFast( packet, channel );
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_JOINCHANNEL" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CHATCOMMAND( string command )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );		// BNET header constant
	packet.push_back( SID_CHATCOMMAND );			// SID_CHATCOMMAND
	packet.push_back( 0 );							// packet length will be assigned later
	packet.push_back( 0 );							// packet length will be assigned later
	UTIL_AppendByteArrayFast( packet, command );	// Message
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_CHATCOMMAND" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CHECKAD( )
{
	unsigned char Zeros[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CHECKAD );			// SID_CHECKAD
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	UTIL_AppendByteArray( packet, Zeros, 4 );	// Platform ID
	UTIL_AppendByteArray( packet, Zeros, 4 );	// Product ID
	UTIL_AppendByteArray( packet, Zeros, 4 );	// ID of last displayed banner
	UTIL_AppendByteArray( packet, Zeros, 4 );	// Current time
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_CHECKAD" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_PING( BYTEARRAY pingValue )
{
	BYTEARRAY packet;

	if( pingValue.size( ) == 4 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );		// BNET header constant
		packet.push_back( SID_PING );					// SID_PING
		packet.push_back( 0 );							// packet length will be assigned later
		packet.push_back( 0 );							// packet length will be assigned later
		UTIL_AppendByteArrayFast( packet, pingValue );	// Ping Value
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_PING" );

	// DEBUG_Print( "SENT SID_PING" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_LOGONRESPONSE( BYTEARRAY clientToken, BYTEARRAY serverToken, BYTEARRAY passwordHash, string accountName )
{
	// todotodo: check that the passed BYTEARRAY sizes are correct (don't know what they should be right now so I can't do this today)

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_LOGONRESPONSE );				// SID_LOGONRESPONSE
	packet.push_back( 0 );								// packet length will be assigned later
	packet.push_back( 0 );								// packet length will be assigned later
	UTIL_AppendByteArrayFast( packet, clientToken );	// Client Token
	UTIL_AppendByteArrayFast( packet, serverToken );	// Server Token
	UTIL_AppendByteArrayFast( packet, passwordHash );	// Password Hash
	UTIL_AppendByteArrayFast( packet, accountName );	// Account Name
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_LOGONRESPONSE" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_NETGAMEPORT( uint16_t serverPort )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_NETGAMEPORT );				// SID_NETGAMEPORT
	packet.push_back( 0 );								// packet length will be assigned later
	packet.push_back( 0 );								// packet length will be assigned later
	UTIL_AppendByteArray( packet, serverPort, false );	// local game server port
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_NETGAMEPORT" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_AUTH_INFO( unsigned char ver, string countryAbbrev, string country )
{
	unsigned char ProtocolID[]		= {   0,   0,   0,   0 };
	unsigned char PlatformID[]		= {  54,  56,  88,  73 };	// "IX86"
	unsigned char ProductID[]		= {  51,  82,  65,  87 };	// "WAR3"
	unsigned char Version[]			= { ver,   0,   0,   0 };
	unsigned char Language[]		= {  83,  85, 110, 101 };	// "enUS"
	unsigned char LocalIP[]			= { 127,   0,   0,   1 };
	unsigned char TimeZoneBias[]	= {  44,   1,   0,   0 };	// 300 minutes (GMT -0500)
	unsigned char LocaleID[]		= {   9,   4,   0,   0 };	// 0x0409 English (United States)
	unsigned char LanguageID[]		= {   9,   4,   0,   0 };	// 0x0409 English (United States)

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_AUTH_INFO );					// SID_AUTH_INFO
	packet.push_back( 0 );								// packet length will be assigned later
	packet.push_back( 0 );								// packet length will be assigned later
	UTIL_AppendByteArray( packet, ProtocolID, 4 );		// Protocol ID
	UTIL_AppendByteArray( packet, PlatformID, 4 );		// Platform ID
	UTIL_AppendByteArray( packet, ProductID, 4 );		// Product ID
	UTIL_AppendByteArray( packet, Version, 4 );			// Version
	UTIL_AppendByteArray( packet, Language, 4 );		// Language
	UTIL_AppendByteArray( packet, LocalIP, 4 );			// Local IP for NAT compatibility
	UTIL_AppendByteArray( packet, TimeZoneBias, 4 );	// Time Zone Bias
	UTIL_AppendByteArray( packet, LocaleID, 4 );		// Locale ID
	UTIL_AppendByteArray( packet, LanguageID, 4 );		// Language ID
	UTIL_AppendByteArrayFast( packet, countryAbbrev );	// Country Abbreviation
	UTIL_AppendByteArrayFast( packet, country );		// Country
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_AUTH_INFO" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_AUTH_CHECK( BYTEARRAY clientToken, BYTEARRAY exeVersion, BYTEARRAY exeVersionHash, BYTEARRAY keyInfoROC, string exeInfo, string keyOwnerName )
{
	unsigned char NumKeys[]		= { 1, 0, 0, 0 };	// 2
	unsigned char UsingSpawn[]	= { 0, 0, 0, 0 };	// false

	BYTEARRAY packet;

	if( clientToken.size( ) == 4 && exeVersion.size( ) == 4 && exeVersionHash.size( ) == 4 && keyInfoROC.size( ) == 36 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
		packet.push_back( SID_AUTH_CHECK );					// SID_AUTH_CHECK
		packet.push_back( 0 );								// packet length will be assigned later
		packet.push_back( 0 );								// packet length will be assigned later
		UTIL_AppendByteArrayFast( packet, clientToken );	// Client Token
		UTIL_AppendByteArrayFast( packet, exeVersion );		// EXE Version
		UTIL_AppendByteArrayFast( packet, exeVersionHash );	// EXE Version Hash
		UTIL_AppendByteArray( packet, NumKeys, 4 );			// number of keys in this packet
		UTIL_AppendByteArray( packet, UsingSpawn, 4 );		// boolean Using Spawn (32 bit)
		UTIL_AppendByteArrayFast( packet, keyInfoROC );		// ROC Key Info
		UTIL_AppendByteArrayFast( packet, exeInfo );		// EXE Info
		UTIL_AppendByteArrayFast( packet, keyOwnerName );	// CD Key Owner Name
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_CHECK" );

	// DEBUG_Print( "SENT SID_AUTH_CHECK" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_AUTH_ACCOUNTLOGON( BYTEARRAY clientPublicKey, string accountName )
{
	BYTEARRAY packet;

	if( clientPublicKey.size( ) == 32 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );				// BNET header constant
		packet.push_back( SID_AUTH_ACCOUNTLOGON );				// SID_AUTH_ACCOUNTLOGON
		packet.push_back( 0 );									// packet length will be assigned later
		packet.push_back( 0 );									// packet length will be assigned later
		UTIL_AppendByteArrayFast( packet, clientPublicKey );	// Client Key
		UTIL_AppendByteArrayFast( packet, accountName );		// Account Name
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_ACCOUNTLOGON" );

	// DEBUG_Print( "SENT SID_AUTH_ACCOUNTLOGON" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY clientPasswordProof )
{
	BYTEARRAY packet;

	if( clientPasswordProof.size( ) == 20 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );					// BNET header constant
		packet.push_back( SID_AUTH_ACCOUNTLOGONPROOF );				// SID_AUTH_ACCOUNTLOGONPROOF
		packet.push_back( 0 );										// packet length will be assigned later
		packet.push_back( 0 );										// packet length will be assigned later
		UTIL_AppendByteArrayFast( packet, clientPasswordProof );	// Client Password Proof
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_ACCOUNTLOGON" );

	// DEBUG_Print( "SENT SID_AUTH_ACCOUNTLOGONPROOF" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_WARDEN( BYTEARRAY wardenResponse )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_WARDEN );						// SID_WARDEN
	packet.push_back( 0 );								// packet length will be assigned later
	packet.push_back( 0 );								// packet length will be assigned later
	UTIL_AppendByteArrayFast( packet, wardenResponse );	// warden response
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_WARDEN" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_FRIENDSLIST( )
{
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_FRIENDSLIST );		// SID_FRIENDSLIST
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_FRIENDSLIST" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANMEMBERLIST( )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANMEMBERLIST );		// SID_CLANMEMBERLIST
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );	// cookie
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_CLANMEMBERLIST" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANINVITATION( string name )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };
	
	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANINVITATION );		// SID_CLANINVITATION
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );	// cookie
	UTIL_AppendByteArray( packet, name );
	AssignLength( packet );
	// DEBUG_Print( "SEND SID_CLANINVITATION" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANREMOVE( string name )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANREMOVE );			// SID_CLANREMOVE
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );	// cookie
	UTIL_AppendByteArray( packet, name );
	AssignLength( packet );
	// DEBUG_Print( "SEND SID_CLANREMOVE" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANCHANGERANK( string name, CBNETProtocol :: RankCode rank )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANCHANGERANK );		// SID_CLANCHANGERANK
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );	// cookie
	UTIL_AppendByteArray( packet, name );
	packet.push_back( rank );
	AssignLength( packet );
	// DEBUG_Print( "SEND SID_CLANCHANGERANK" );
	// DEBUG_Print( packet );
	return packet;
}

BYTEARRAY CBNETProtocol :: SEND_SID_CLANSETMOTD( string message )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };

	BYTEARRAY packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANSETMOTD );		// SID_CLANSETMOTD
	packet.push_back( 0 );						// packet length will be assigned later
	packet.push_back( 0 );						// packet length will be assigned later
	UTIL_AppendByteArray( packet, Cookie, 4 );	// cookie
	UTIL_AppendByteArray( packet, message );
	AssignLength( packet );
	// DEBUG_Print( "SEND SID_CLANMOTD" );
	// DEBUG_Print( packet );
	return packet;
}

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CBNETProtocol :: AssignLength( BYTEARRAY &content )
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	BYTEARRAY LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateByteArray( (uint16_t)content.size( ), false );
		content[2] = LengthBytes[0];
		content[3] = LengthBytes[1];
		return true;
	}

	return false;
}

bool CBNETProtocol :: ValidateLength( BYTEARRAY &content )
{
	// verify that bytes 3 and 4 (indices 2 and 3) of the content array describe the length

	uint16_t Length;
	BYTEARRAY LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[2] );
		LengthBytes.push_back( content[3] );
		Length = UTIL_ByteArrayToUInt16( LengthBytes, false );

		if( Length == content.size( ) )
			return true;
	}

	return false;
}

//
// CIncomingGameHost
//

CIncomingGameHost :: CIncomingGameHost( BYTEARRAY &nIP, uint16_t nPort, string nGameName, BYTEARRAY &nHostCounter )
{
	m_IP = nIP;
	m_Port = nPort;
	m_GameName = nGameName;
	m_HostCounter = nHostCounter;
}

CIncomingGameHost :: ~CIncomingGameHost( )
{

}

string CIncomingGameHost :: GetIPString( )
{
	string Result;

	if( m_IP.size( ) >= 4 )
	{
		for( unsigned int i = 0; i < 4; i++ )
		{
			Result += UTIL_ToString( (unsigned int)m_IP[i] );

			if( i < 3 )
				Result += ".";
		}
	}

	return Result;
}

//
// CIncomingChatEvent
//

CIncomingChatEvent :: CIncomingChatEvent( CBNETProtocol :: IncomingChatEvent nChatEvent, bool nOperator, uint32_t nPing, string nUser, string nMessage )
{
	m_ChatEvent = nChatEvent;
	m_Operator = nOperator;
	m_Ping = nPing;
	m_User = nUser;
	m_Message = nMessage;
}

CIncomingChatEvent :: ~CIncomingChatEvent( )
{

}

//
// CIncomingFriendList
//

CIncomingFriendList :: CIncomingFriendList( string nAccount, unsigned char nStatus, unsigned char nArea, string nLocation )
{
	m_Account = nAccount;
	m_Status = nStatus;
	m_Area = nArea;
	m_Location = nLocation;
}

CIncomingFriendList :: ~CIncomingFriendList( )
{

}

string CIncomingFriendList :: GetDescription( )
{
	string Description;
	Description += GetAccount( ) + "\n";
	Description += ExtractStatus( GetStatus( ) ) + "\n";
	Description += ExtractArea( GetArea( ) ) + "\n";
	Description += ExtractLocation( GetLocation( ) ) + "\n\n";
	return Description;
}

string CIncomingFriendList :: ExtractStatus( unsigned char status )
{
	string Result;

	if( status & 1 )
		Result += "<Mutual>";

	if( status & 2 )
		Result += "<DND>";

	if( status & 4 )
		Result += "<Away>";

	if( Result.empty( ) )
		Result = "<None>";

	return Result;
}

string CIncomingFriendList :: ExtractArea( unsigned char area )
{
	switch( area )
	{
	case 0: return "<Offline>";
	case 1: return "<No Channel>";
	case 2: return "<In Channel>";
	case 3: return "<Public Game>";
	case 4: return "<Private Game>";
	case 5: return "<Private Game>";
	}

	return "<Unknown>";
}

string CIncomingFriendList :: ExtractLocation( string location )
{
	string Result;

	if( location.substr( 0, 4 ) == "PX3W" )
		Result = location.substr( 4 );

	if( Result.empty( ) )
		Result = ".";

	return Result;
}

//
// CIncomingClanList
//

CIncomingClanList :: CIncomingClanList( string nName, unsigned char nRank, unsigned char nStatus )
{
	m_Name = nName;
	m_Rank = nRank;
	m_Status = nStatus;
}

CIncomingClanList :: ~CIncomingClanList( )
{

}

string CIncomingClanList :: GetRank( )
{
	switch( m_Rank )
	{
	case 0: return "Recruit";
	case 1: return "Peon";
	case 2: return "Grunt";
	case 3: return "Shaman";
	case 4: return "Chieftain";
	}

	return "Rank Unknown";
}

string CIncomingClanList :: GetStatus( )
{
	if( m_Status == 0 )
		return "Offline";
	else
		return "Online";
}

string CIncomingClanList :: GetDescription( )
{
	string Description;
	Description += GetName( ) + "\n";
	Description += GetStatus( ) + "\n";
	Description += GetRank( ) + "\n\n";
	return Description;
}

#include <boost/python.hpp>

void CBNETProtocol :: RegisterPythonClass( )
{
	using namespace boost::python;

	class_<CBNETProtocol>("CNETProtocol", no_init)
		.def_readonly("clientToken", &CBNETProtocol::m_ClientToken)
		.def_readonly("logonType", &CBNETProtocol::m_LogonType)
		.def_readonly("serverToken", &CBNETProtocol::m_ServerToken)
		.def_readonly("MPQFileTime", &CBNETProtocol::m_MPQFileTime)
		.def_readonly("IX86VerFileName", &CBNETProtocol::m_IX86VerFileName)
		.def_readonly("valueStringFormula", &CBNETProtocol::m_ValueStringFormula)
		.def_readonly("keyState", &CBNETProtocol::m_KeyState)
		.def_readonly("keyStateDescription", &CBNETProtocol::m_KeyStateDescription)
		.def_readonly("salt", &CBNETProtocol::m_Salt)
		.def_readonly("serverPublicKey", &CBNETProtocol::m_ServerPublicKey)
		.def_readonly("uniqueName", &CBNETProtocol::m_UniqueName)

		.def("getClientToken", &CBNETProtocol::GetClientToken)
		.def("getLogonType", &CBNETProtocol::GetLogonType)
		.def("getServerToken", &CBNETProtocol::GetServerToken)
		.def("getMPQFileTime", &CBNETProtocol::GetMPQFileTime)
		.def("getIX86VerFileName", &CBNETProtocol::GetIX86VerFileName)
		.def("getIX86VerFileNameString", &CBNETProtocol::GetIX86VerFileNameString)
		.def("getValueStringFormula", &CBNETProtocol::GetValueStringFormula)
		.def("getValueStringFormulaString", &CBNETProtocol::GetValueStringFormulaString)
		.def("getKeyState", &CBNETProtocol::GetKeyState)
		.def("getKeyStateDescription", &CBNETProtocol::GetKeyStateDescription)
		.def("getSalt", &CBNETProtocol::GetSalt)
		.def("getServerPublicKey", &CBNETProtocol::GetServerPublicKey)
		.def("getUniqueName", &CBNETProtocol::GetUniqueName)

		.def("RECEIVE_SID_NULL", &CBNETProtocol::RECEIVE_SID_NULL)
		.def("RECEIVE_SID_ENTERCHAT", &CBNETProtocol::RECEIVE_SID_ENTERCHAT)
		.def("RECEIVE_SID_CHATEVENT", &CBNETProtocol::RECEIVE_SID_CHATEVENT, return_internal_reference<>())
		.def("RECEIVE_SID_CHECKAD", &CBNETProtocol::RECEIVE_SID_CHECKAD)
		.def("RECEIVE_SID_STARTADVEX3", &CBNETProtocol::RECEIVE_SID_STARTADVEX3)
		.def("RECEIVE_SID_PING", &CBNETProtocol::RECEIVE_SID_PING)
		.def("RECEIVE_SID_LOGONRESPONSE", &CBNETProtocol::RECEIVE_SID_LOGONRESPONSE)
		.def("RECEIVE_SID_AUTH_INFO", &CBNETProtocol::RECEIVE_SID_AUTH_INFO)
		.def("RECEIVE_SID_AUTH_CHECK", &CBNETProtocol::RECEIVE_SID_AUTH_CHECK)
		.def("RECEIVE_SID_AUTH_ACCOUNTLOGON", &CBNETProtocol::RECEIVE_SID_AUTH_ACCOUNTLOGON)
		.def("RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF", &CBNETProtocol::RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF)
		.def("RECEIVE_SID_WARDEN", &CBNETProtocol::RECEIVE_SID_WARDEN)
		.def("RECEIVE_SID_FRIENDSLIST", &CBNETProtocol::RECEIVE_SID_FRIENDSLIST)
		.def("RECEIVE_SID_CLANMEMBERLIST", &CBNETProtocol::RECEIVE_SID_CLANMEMBERLIST)
		.def("RECEIVE_SID_CLANMEMBERSTATUSCHANGE", &CBNETProtocol::RECEIVE_SID_CLANMEMBERSTATUSCHANGE, return_internal_reference<>())

		.def("SEND_PROTOCOL_INITIALIZE_SELECTOR", &CBNETProtocol::SEND_PROTOCOL_INITIALIZE_SELECTOR)
		.def("SEND_SID_NULL", &CBNETProtocol::SEND_SID_NULL)
		.def("SEND_SID_ENTERCHAT", &CBNETProtocol::SEND_SID_ENTERCHAT)
		.def("SEND_SID_JOINCHANNEL", &CBNETProtocol::SEND_SID_JOINCHANNEL)
		.def("SEND_SID_CHATCOMMAND", &CBNETProtocol::SEND_SID_CHATCOMMAND)
		.def("SEND_SID_CHECKAD", &CBNETProtocol::SEND_SID_CHECKAD)
		.def("SEND_SID_PING", &CBNETProtocol::SEND_SID_PING)
		.def("SEND_SID_LOGONRESPONSE", &CBNETProtocol::SEND_SID_LOGONRESPONSE)
		.def("SEND_SID_NETGAMEPORT", &CBNETProtocol::SEND_SID_NETGAMEPORT)
		.def("SEND_SID_AUTH_INFO", &CBNETProtocol::SEND_SID_AUTH_INFO)
		.def("SEND_SID_AUTH_CHECK", &CBNETProtocol::SEND_SID_AUTH_CHECK)
		.def("SEND_SID_AUTH_ACCOUNTLOGON", &CBNETProtocol::SEND_SID_AUTH_ACCOUNTLOGON)
		.def("SEND_SID_AUTH_ACCOUNTLOGONPROOF", &CBNETProtocol::SEND_SID_AUTH_ACCOUNTLOGONPROOF)
		.def("SEND_SID_WARDEN", &CBNETProtocol::SEND_SID_WARDEN)
		.def("SEND_SID_FRIENDSLIST", &CBNETProtocol::SEND_SID_FRIENDSLIST)
		.def("SEND_SID_CLANINVITATION", &CBNETProtocol::SEND_SID_CLANMEMBERLIST)
		.def("SEND_SID_CLANREMOVE", &CBNETProtocol::SEND_SID_CLANINVITATION)
		.def("SEND_SID_CLANREMOVE", &CBNETProtocol::SEND_SID_CLANREMOVE)
		.def("SEND_SID_CLANSETMOTD", &CBNETProtocol::SEND_SID_CLANSETMOTD)
		.def("SEND_SID_CLANCHANGERANK", &CBNETProtocol::SEND_SID_CLANCHANGERANK)

		.def("assignLength", &CBNETProtocol::AssignLength)
		.def("validateLength", &CBNETProtocol::ValidateLength)
	;
}

void CIncomingGameHost :: RegisterPythonClass( )
{
	using namespace boost::python;

	class_<CIncomingGameHost>("IncomingGameHost", no_init)
		.def("getIP", &CIncomingGameHost::GetIP)
		.def("getIPString", &CIncomingGameHost::GetIPString)
		.def("getPort", &CIncomingGameHost::GetPort)
		.def("getGameName", &CIncomingGameHost::GetGameName)
		.def("getHostCounter", &CIncomingGameHost::GetHostCounter)
	;
}

void CIncomingChatEvent :: RegisterPythonClass( )
{
	using namespace boost::python;

	class_<CIncomingChatEvent>("IncomingChatEvent", no_init)
		.def("getOperator", &CIncomingChatEvent::GetOperator)
		.def("getChatEvent", &CIncomingChatEvent::GetChatEvent)
		.def("getPing", &CIncomingChatEvent::GetPing)
		.def("getUser", &CIncomingChatEvent::GetUser)
		.def("getMessage", &CIncomingChatEvent::GetMessage)
	;
}

void CIncomingFriendList :: RegisterPythonClass( )
{
	using namespace boost::python;

	class_<CIncomingFriendList>("IncomingFriendList", no_init)
		.def("getAccount", &CIncomingFriendList::GetAccount)
		.def("getStatus", &CIncomingFriendList::GetStatus)
		.def("getArea", &CIncomingFriendList::GetArea)
		.def("getLocation", &CIncomingFriendList::GetLocation)
		.def("getDescription", &CIncomingFriendList::GetDescription)
	;
}

void CIncomingClanList :: RegisterPythonClass( )
{
	using namespace boost::python;

	class_<CIncomingClanList>("IncomingClanList", no_init)
		.def("getName", &CIncomingClanList::GetName)
		.def("getRank", &CIncomingClanList::GetRank)
		.def("getStatus", &CIncomingClanList::GetStatus)
		.def("getDescription", &CIncomingClanList::GetDescription)
	;
}
