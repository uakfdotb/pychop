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

#ifndef BNET_H
#define BNET_H

#include "bnetprotocol.h"

//
// CBNET
//

class CTCPClient;
class CCommandPacket;
class CBNCSUtilInterface;
class CBNETProtocol;
class CBNLSClient;
class CIncomingFriendList;
class CIncomingClanList;
class CIncomingChatEvent;
class CCallableBanCount;
class CCallableBanAdd;
class CCallableBanRemove;
class CCallableBanList;
class CCallableGamePlayerSummaryCheck;
class CCallableDotAPlayerSummaryCheck;
class CCallableUserCount;
class CCallableUserAdd;
class CCallableUserDelete;
class CCallableUserList;
class CCallableUserSeen;
class CCallableUserSetSeen;
class CDBBan;

typedef pair<string,CCallableBanCount *> PairedBanCount;
typedef pair<string,CCallableBanAdd *> PairedBanAdd;
typedef pair<string,CCallableBanRemove *> PairedBanRemove;
typedef pair<string,CCallableGamePlayerSummaryCheck *> PairedGPSCheck;
typedef pair<string,CCallableDotAPlayerSummaryCheck *> PairedDPSCheck;
typedef pair<string,CCallableUserCount *> PairedUserCount;
typedef pair<string,CCallableUserAdd *> PairedUserAdd;
typedef pair<string,CCallableUserDelete *> PairedUserDelete;
typedef pair<string,CCallableUserSeen *> PairedUserSeen;

class CBNET
{
public:
	CChOP *m_ChOP;

private:
	CTCPClient *m_Socket;								// the connection to battle.net
	CBNETProtocol *m_Protocol;							// battle.net protocol
	CBNLSClient *m_BNLSClient;							// the BNLS client (for external warden handling)
	queue<CCommandPacket *> m_Packets;					// queue of incoming packets
	CBNCSUtilInterface *m_BNCSUtil;						// the interface to the bncsutil library (used for logging into battle.net)
	queue<BYTEARRAY> m_OutPackets;						// queue of outgoing packets to be sent (to prevent getting kicked for flooding)
	vector<CIncomingFriendList *> m_Friends;			// vector of friends
	vector<CIncomingClanList *> m_Clans;				// vector of clan members
	vector<PairedBanCount> m_PairedBanCounts;			// vector of paired threaded database ban counts in progress
	vector<PairedBanAdd> m_PairedBanAdds;				// vector of paired threaded database ban adds in progress
	vector<PairedBanRemove> m_PairedBanRemoves;			// vector of paired threaded database ban removes in progress
	vector<PairedGPSCheck> m_PairedGPSChecks;			// vector of paired threaded database game player summary checks in progress
	vector<PairedDPSCheck> m_PairedDPSChecks;			// vector of paired threaded database DotA player summary checks in progress
	vector<PairedUserCount> m_PairedUserCounts;			// vector of paired threaded database user counts in progress
	vector<PairedUserAdd> m_PairedUserAdds;				// vector of paired threaded database user adds in progress
	vector<PairedUserDelete> m_PairedUserDeletes;		// vector of paired threaded database user deletes in progress
	vector<PairedUserSeen> m_PairedUserSeens;			// vector of paired threaded database user seens in progress
	vector<CCallableUserSetSeen *> m_CallableUserSeens;	// vector of threaded database user set seens in progress
	CCallableBanList *m_CallableBanList;				// threaded database ban list in progress
	CCallableUserList *m_CallableUserList;				// threaded database user list in progress
	vector<CDBBan *> m_Bans;							// vector of cached bans
	bool m_Exiting;										// set to true and this class will be deleted next update
	string m_Server;									// battle.net server to connect to
	string m_ServerIP;									// battle.net server to connect to (the IP address so we don't have to resolve it every time we connect)
	string m_ServerAlias;								// battle.bet server alias (short name e.g. "USEast")
	string m_BNLSServer;								// BNLS server to connect to (for warden handling)
	uint16_t m_BNLSPort;								// BNLS port
	uint32_t m_BNLSWardenCookie;						// BNLS warden cookie
	string m_CDKeyROC;									// ROC CD key
	string m_CountryAbbrev;								// country abbreviation
	string m_Country;									// country
	string m_UserName;									// battle.net username
	string m_UserPassword;								// battle.net password
	string m_FirstChannel;								// the first chat channel to join upon entering chat (note: we hijack this to store the last channel when entering a game)
	string m_CurrentChannel;							// the current chat channel
	string m_RootAdmin;									// the root admin
	char m_CommandTrigger;								// the character prefix to identify commands
	unsigned char m_War3Version;						// custom warcraft 3 version for PvPGN users
	BYTEARRAY m_EXEVersion;								// custom exe version for PvPGN users
	BYTEARRAY m_EXEVersionHash;							// custom exe version hash for PvPGN users
	string m_PasswordHashType;							// password hash type for PvPGN users
	uint32_t m_MaxMessageLength;						// maximum message length for PvPGN users
	uint32_t m_NextConnectTime;							// GetTime when we should try connecting to battle.net next (after we get disconnected)
	uint32_t m_LastNullTime;							// GetTime when the last null packet was sent for detecting disconnects
	uint32_t m_LastOutPacketTicks;						// GetTicks when the last packet was sent for the m_OutPackets queue
	uint32_t m_LastOutPacketSize;
	uint32_t m_FrequencyDelayTimes;						// times when we sent quickly after WaitTicks was over
	uint32_t m_LastBanRefreshTime;						// GetTime when the ban list was last refreshed from the database
	uint32_t m_LastUserRefreshTime;						// GetTime when the user list was last refreshed from the database
	bool m_WaitingToConnect;							// if we're waiting to reconnect to battle.net after being disconnected
	bool m_LoggedIn;									// if we've logged into battle.net or not
	bool m_InChat;										// if we've entered chat or not (but we're not necessarily in a chat channel yet)
	bool m_LastInviteCreation;						// whether the last invite received was for a clan creation (else, it was for invitation response)

	string m_LastWhisper;								// user who recently whispered
	uint32_t m_LastWarnTime;							// GetTime when warns were updated
	vector<string> m_SpamCache;							// vector of recent channel messages
	map<string, CUser *> m_Users;						// map of cached users
	map<string, CUser *> m_Channel;						// map of users in channel
	CConfig m_CFG;										// config file for commands
	bool m_IsOperator;									// If bot has operator flag in current channel
	uint32_t m_ClanRank;									// ChannelBot's clan rank

public:
	CBNET( CChOP *nChOP, string nServer, string nServerAlias, string nBNLSServer, uint16_t nBNLSPort, uint32_t nBNLSWardenCookie, string nCDKeyROC, string nCountryAbbrev, string nCountry, string nUserName, string nUserPassword, string nFirstChannel, string nRootAdmin, char nCommandTrigger, unsigned char nWar3Version, BYTEARRAY nEXEVersion, BYTEARRAY nEXEVersionHash, string nPasswordHashType, uint32_t nMaxMessageLength );
	~CBNET( );

	bool GetExiting( )					{ return m_Exiting; }
	string GetServer( )					{ return m_Server; }
	string GetServerAlias( )			{ return m_ServerAlias; }
	string GetCDKeyROC( )				{ return m_CDKeyROC; }
	string GetUserName( )				{ return m_UserName; }
	string GetUserPassword( )			{ return m_UserPassword; }
	string GetFirstChannel( )			{ return m_FirstChannel; }
	string GetCurrentChannel( )			{ return m_CurrentChannel; }
	string GetRootAdmin( )				{ return m_RootAdmin; }
	char GetCommandTrigger( )			{ return m_CommandTrigger; }
	BYTEARRAY GetEXEVersion( )			{ return m_EXEVersion; }
	BYTEARRAY GetEXEVersionHash( )		{ return m_EXEVersionHash; }
	string GetPasswordHashType( )		{ return m_PasswordHashType; }
	bool GetLoggedIn( )					{ return m_LoggedIn; }
	bool GetInChat( )					{ return m_InChat; }
	uint32_t GetOutPacketsQueued( )		{ return m_OutPackets.size( ); }
	BYTEARRAY GetUniqueName( );

	// processing functions

	unsigned int SetFD( void *fd, void *send_fd, int *nfds );
	bool Update( void *fd, void *send_fd );
	void ExtractPackets( );
	void ProcessPackets( );
	void ProcessChatEvent( CIncomingChatEvent *chatEvent );
	void ProcessCorePlugins( CUser *user, string Message );
	string ProcessCommand( CUser *user, string command, string payload, uint32_t type );

	// functions to send packets to battle.net

	void SendJoinChannel( string channel );
	void SendGetFriendsList( );
	void SendGetClanList( );
	void QueueEnterChat( );
	void QueueChatCommand( string chatCommand );
	void QueueChatCommand( string chatCommand, string user, bool whisper );
	void SendClanInvitation( string name );
	void SendClanRemove( string name );
	void SendClanChangeRank( string name, CBNETProtocol :: RankCode rank );
	void SendClanMakeChieftain( string name );
	void SendClanSetMOTD( string message );
	void SendClanAcceptInvite( bool accept );
	void SendProfile( string name );

	// other functions

	bool IsRootAdmin( string name );
	bool IsBannedName( string name );
	bool IsBannedIP( string ip );
	CDBBan *GetBannedInfo( string name );
	void AddBan( string name, string ip, string gamename, string admin, string reason );
	void AddUser( string name, uint32_t access );
	void RemoveBan( string name );
	void RemoveUser( string name );

	string FormatTime( uint32_t time );
	string GetClanRank( string name );
	bool IsClanMember( string name );
	bool IsInChannel( string name );
	uint32_t ChannelNameMatch( string name, string &match );
	uint32_t ClanNameMatch( string name, string &match );
	CUser *GetUserByName( string name );
	void UpdateSeen( string user );
	uint32_t NumPackets( ); //get number of outgoing packets in queue
	int64_t PacketDelayTime( uint32_t PacketSize, uint32_t FrequencyDelayTimes );
	int64_t DelayTime( );
	int64_t TotalDelayTime( );
	
	uint32_t GetNumClanMembers( ) { return m_Clans.size( ); }
	uint32_t GetNumFriends( ) { return m_Friends.size( ); }
	
	vector<string> GetChannelNameList( );
	vector<string> GetUserNameList( );
	CIncomingClanList GetClanMember( uint32_t position );
	CIncomingFriendList GetFriend( uint32_t position );
	
	static void RegisterPythonClass();
};

#endif
