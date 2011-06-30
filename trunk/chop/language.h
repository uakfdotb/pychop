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

#ifndef LANGUAGE_H
#define LANGUAGE_H

//
// CLanguage
//

class CLanguage
{
private:
	CConfig *m_CFG;

public:
	CLanguage( string nCFGFile );
	~CLanguage( );

	void Replace( string &Text, string Key, string Value );


	// BNET
	string ConnectingToBNET( string server );
	string ConnectedToBNET( string server );
	string DisconnectedFromBNET( string server );
	string LoggedInToBNET( string server );
	string ConnectingToBNETTimedOut( string server );
	string UpdatingClanList( );
	string UpdatingFriendsList( );

	// clan
	string InvitedToClan( string server, string user );
	string RemovedFromClan( string server, string user );
	string MOTDChanged( );
	string ChangedClanRank( string server, string user, string rank );

	// UAC
	string YouDontHaveAccessToThatCommand( );
	string WhisperCommandsDisabled( );
	string SyntaxError( );
	string UnknownCommand( string command );
	string KickSpam( );
	string KickYell( );
	string KickPhrase( );

	// commands
	string AddedUser( string server, string user, string access );
	string ErrorAddingUser( string user );
	string DeletedUser( string server, string user );
	string ErrorDeletingUser( string user );
	string YouCantDeleteTheRootAdmin( );
	string ThereAreNoUsers( string server );
	string ThereIsUser( string server );
	string ThereAreUsers( string server, string count );
	string UserHasAccess( string server, string user, string access );
	string UserIsAlreadyBanned( string server, string victim );
	string BannedUser( string server, string victim );
	string ErrorBanningUser( string victim );
	string UserWasBannedOnByBecause( string server, string victim, string date, string admin, string reason );
	string UserIsNotBanned( string server, string victim );
	string ThereAreNoBannedUsers( string server );
	string ThereIsBannedUser( string server );
	string ThereAreBannedUsers( string server, string count );
	string UnbannedUser( string server, string victim );
	string ErrorUnbanningUser( string victim );
	string HasPlayedGames( string user, string firstgame, string lastgame, string totalgames, string avgloadingtime, string avgstay );
	string HasntPlayedGames( string user );
	string HasPlayedDotAGames( string user, string totalgames, string totalwins, string totallosses, string totalkills, string totaldeaths, string totalcreepkills, string totalcreepdenies, string totalassists, string totalneutralkills, string totaltowerkills, string totalraxkills, string totalcourierkills, string avgkills, string avgdeaths, string avgcreepkills, string avgcreepdenies, string avgassists, string avgneutralkills, string avgraxkills, string avgcourierkills );
	string HasntPlayedDotAGames( string user );
	string UserIsHere( string user );
	string UserSeen( string user, string time );
	string UserNotInClan( string user );
	string UserNotSeen( string user );
	string Slap( string text, string user, string victim, string number );
	string NoWhispersYet( );
	string LastWhisperFrom( string user );
	string Version( string version );
	string UsersPingIs( string user, string ping );
	string QuoteAdded( );
	string BotOwner( string user );
};

#endif
