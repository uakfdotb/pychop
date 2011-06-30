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
#include "db.h"

//
// CChOPDB
//

CChOPDB :: CChOPDB( CConfig *CFG )
{
	m_HasError = false;
}

CChOPDB :: ~CChOPDB( )
{

}

void CChOPDB :: RecoverCallable( CBaseCallable *callable )
{

}

bool CChOPDB :: Begin( )
{
	return true;
}

bool CChOPDB :: Commit( )
{
	return true;
}

uint32_t CChOPDB :: BanCount( string server )
{
	return 0;
}

CDBBan *CChOPDB :: BanCheck( string server, string user, string ip )
{
	return NULL;
}

bool CChOPDB :: BanAdd( string server, string user, string ip, string gamename, string admin, string reason )
{
	return false;
}

bool CChOPDB :: BanRemove( string server, string user )
{
	return false;
}

bool CChOPDB :: BanRemove( string user )
{
	return false;
}

vector<CDBBan *> CChOPDB :: BanList( string server )
{
	return vector<CDBBan *>( );
}

uint32_t CChOPDB :: GamePlayerCount( string name )
{
	return 0;
}

CDBGamePlayerSummary *CChOPDB :: GamePlayerSummaryCheck( string name )
{
	return NULL;
}

uint32_t CChOPDB :: DotAPlayerCount( string name )
{
	return 0;
}

CDBDotAPlayerSummary *CChOPDB :: DotAPlayerSummaryCheck( string name )
{
	return NULL;
}

string CChOPDB :: FromCheck( uint32_t ip )
{
	return "??";
}

bool CChOPDB :: FromAdd( uint32_t ip1, uint32_t ip2, string country )
{
	return false;
}

uint32_t CChOPDB :: UserCount( string server )
{
	return 0;
}

bool CChOPDB :: UserAdd( string server, string name, uint32_t access )
{
	return false;
}

bool CChOPDB :: UserDelete( string server, string name )
{
	return false;
}

vector<CUser *> CChOPDB :: UserList( string server )
{
	return vector<CUser *>( );
}

bool CChOPDB :: UserSetSeen( string server, string name )
{
	return false;
}

uint32_t CChOPDB :: UserSeen( string server, string name )
{
	return 0;
}

void CChOPDB :: CreateThread( CBaseCallable *callable )
{
	callable->SetReady( true );
}

CCallableBanCount *CChOPDB :: ThreadedBanCount( string server )
{
	return NULL;
}

CCallableBanCheck *CChOPDB :: ThreadedBanCheck( string server, string user, string ip )
{
	return NULL;
}

CCallableBanAdd *CChOPDB :: ThreadedBanAdd( string server, string user, string ip, string gamename, string admin, string reason )
{
	return NULL;
}

CCallableBanRemove *CChOPDB :: ThreadedBanRemove( string server, string user )
{
	return NULL;
}

CCallableBanRemove *CChOPDB :: ThreadedBanRemove( string user )
{
	return NULL;
}

CCallableBanList *CChOPDB :: ThreadedBanList( string server )
{
	return NULL;
}

CCallableGamePlayerSummaryCheck *CChOPDB :: ThreadedGamePlayerSummaryCheck( string name )
{
	return NULL;
}

CCallableDotAPlayerSummaryCheck *CChOPDB :: ThreadedDotAPlayerSummaryCheck( string name )
{
	return NULL;
}

CCallableScoreCheck *CChOPDB :: ThreadedScoreCheck( string category, string name, string server )
{
	return NULL;
}

CCallableUserCount *CChOPDB :: ThreadedUserCount( string server )
{
	return NULL;
}

CCallableUserAdd *CChOPDB :: ThreadedUserAdd( string server, string name, uint32_t access )
{
	return NULL;
}

CCallableUserDelete *CChOPDB :: ThreadedUserDelete( string server, string name )
{
	return NULL;
}

CCallableUserList *CChOPDB :: ThreadedUserList( string server )
{
	return NULL;
}

CCallableUserSetSeen *CChOPDB :: ThreadedUserSetSeen( string server, string name )
{
	return NULL;
}

CCallableUserSeen *CChOPDB :: ThreadedUserSeen( string server, string name )
{
	return NULL;
}

//
// Callables
//

void CBaseCallable :: Init( )
{
	m_StartTicks = GetTicks( );
}

void CBaseCallable :: Close( )
{
	m_EndTicks = GetTicks( );
	m_Ready = true;
}

CCallableBanCount :: ~CCallableBanCount( )
{

}

CCallableBanCheck :: ~CCallableBanCheck( )
{
	delete m_Result;
}

CCallableBanAdd :: ~CCallableBanAdd( )
{

}

CCallableBanRemove :: ~CCallableBanRemove( )
{

}

CCallableBanList :: ~CCallableBanList( )
{
	// don't delete anything in m_Result here, it's the caller's responsibility
}

CCallableGamePlayerSummaryCheck :: ~CCallableGamePlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableDotAPlayerSummaryCheck :: ~CCallableDotAPlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableScoreCheck :: ~CCallableScoreCheck( )
{

}

CCallableUserCount :: ~CCallableUserCount( )
{

}

CCallableUserAdd :: ~CCallableUserAdd( )
{

}

CCallableUserDelete :: ~CCallableUserDelete( )
{

}

CCallableUserList :: ~CCallableUserList( )
{

}

CCallableUserSetSeen :: ~CCallableUserSetSeen( )
{

}

CCallableUserSeen :: ~CCallableUserSeen( )
{

}

//
// CDBBan
//

CDBBan :: CDBBan( string nServer, string nName, string nIP, string nDate, string nGameName, string nAdmin, string nReason )
{
	m_Server = nServer;
	m_Name = nName;
	m_IP = nIP;
	m_Date = nDate;
	m_GameName = nGameName;
	m_Admin = nAdmin;
	m_Reason = nReason;
}

CDBBan :: ~CDBBan( )
{

}

//
// CDBGame
//

CDBGame :: CDBGame( uint32_t nID, string nServer, string nMap, string nDateTime, string nGameName, string nOwnerName, uint32_t nDuration )
{
	m_ID = nID;
	m_Server = nServer;
	m_Map = nMap;
	m_DateTime = nDateTime;
	m_GameName = nGameName;
	m_OwnerName = nOwnerName;
	m_Duration = nDuration;
}

CDBGame :: ~CDBGame( )
{

}

//
// CDBGamePlayer
//

CDBGamePlayer :: CDBGamePlayer( uint32_t nID, uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Name = nName;
	m_IP = nIP;
	m_Spoofed = nSpoofed;
	m_SpoofedRealm = nSpoofedRealm;
	m_Reserved = nReserved;
	m_LoadingTime = nLoadingTime;
	m_Left = nLeft;
	m_LeftReason = nLeftReason;
	m_Team = nTeam;
	m_Colour = nColour;
}

CDBGamePlayer :: ~CDBGamePlayer( )
{

}

//
// CDBGamePlayerSummary
//

CDBGamePlayerSummary :: CDBGamePlayerSummary( string nServer, string nName, string nFirstGameDateTime, string nLastGameDateTime, uint32_t nTotalGames, uint32_t nMinLoadingTime, uint32_t nAvgLoadingTime, uint32_t nMaxLoadingTime, uint32_t nMinLeftPercent, uint32_t nAvgLeftPercent, uint32_t nMaxLeftPercent, uint32_t nMinDuration, uint32_t nAvgDuration, uint32_t nMaxDuration )
{
	m_Server = nServer;
	m_Name = nName;
	m_FirstGameDateTime = nFirstGameDateTime;
	m_LastGameDateTime = nLastGameDateTime;
	m_TotalGames = nTotalGames;
	m_MinLoadingTime = nMinLoadingTime;
	m_AvgLoadingTime = nAvgLoadingTime;
	m_MaxLoadingTime = nMaxLoadingTime;
	m_MinLeftPercent = nMinLeftPercent;
	m_AvgLeftPercent = nAvgLeftPercent;
	m_MaxLeftPercent = nMaxLeftPercent;
	m_MinDuration = nMinDuration;
	m_AvgDuration = nAvgDuration;
	m_MaxDuration = nMaxDuration;
}

CDBGamePlayerSummary :: ~CDBGamePlayerSummary( )
{

}

//
// CDBDotAGame
//

CDBDotAGame :: CDBDotAGame( uint32_t nID, uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Winner = nWinner;
	m_Min = nMin;
	m_Sec = nSec;
}

CDBDotAGame :: ~CDBDotAGame( )
{

}

//
// CDBDotAPlayer
//

CDBDotAPlayer :: CDBDotAPlayer( )
{
	m_ID = 0;
	m_GameID = 0;
	m_Colour = 0;
	m_Kills = 0;
	m_Deaths = 0;
	m_CreepKills = 0;
	m_CreepDenies = 0;
	m_Assists = 0;
	m_Gold = 0;
	m_NeutralKills = 0;
	m_NewColour = 0;
	m_TowerKills = 0;
	m_RaxKills = 0;
	m_CourierKills = 0;
}

CDBDotAPlayer :: CDBDotAPlayer( uint32_t nID, uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Colour = nColour;
	m_Kills = nKills;
	m_Deaths = nDeaths;
	m_CreepKills = nCreepKills;
	m_CreepDenies = nCreepDenies;
	m_Assists = nAssists;
	m_Gold = nGold;
	m_NeutralKills = nNeutralKills;
	m_Items[0] = nItem1;
	m_Items[1] = nItem2;
	m_Items[2] = nItem3;
	m_Items[3] = nItem4;
	m_Items[4] = nItem5;
	m_Items[5] = nItem6;
	m_Hero = nHero;
	m_NewColour = nNewColour;
	m_TowerKills = nTowerKills;
	m_RaxKills = nRaxKills;
	m_CourierKills = nCourierKills;
}

CDBDotAPlayer :: ~CDBDotAPlayer( )
{

}

string CDBDotAPlayer :: GetItem( unsigned int i )
{
	if( i < 6 )
		return m_Items[i];

	return string( );
}

void CDBDotAPlayer :: SetItem( unsigned int i, string item )
{
	if( i < 6 )
		m_Items[i] = item;
}

//
// CDBDotAPlayerSummary
//

CDBDotAPlayerSummary :: CDBDotAPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalAssists, uint32_t nTotalNeutralKills, uint32_t nTotalTowerKills, uint32_t nTotalRaxKills, uint32_t nTotalCourierKills )
{
	m_Server = nServer;
	m_Name = nName;
	m_TotalGames = nTotalGames;
	m_TotalWins = nTotalWins;
	m_TotalLosses = nTotalLosses;
	m_TotalKills = nTotalKills;
	m_TotalDeaths = nTotalDeaths;
	m_TotalCreepKills = nTotalCreepKills;
	m_TotalCreepDenies = nTotalCreepDenies;
	m_TotalAssists = nTotalAssists;
	m_TotalNeutralKills = nTotalNeutralKills;
	m_TotalTowerKills = nTotalTowerKills;
	m_TotalRaxKills = nTotalRaxKills;
	m_TotalCourierKills = nTotalCourierKills;
}

CDBDotAPlayerSummary :: ~CDBDotAPlayerSummary( )
{

}
