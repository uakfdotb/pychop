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
#include "config.h"
#include "language.h"

//
// CLanguage
//

CLanguage :: CLanguage( string nCFGFile )
{
	m_CFG = new CConfig( );
	m_CFG->Read( nCFGFile );
}

CLanguage :: ~CLanguage( )
{
	delete m_CFG;
}

void CLanguage :: Replace( string &Text, string Key, string Value )
{
	// don't allow any infinite loops

	if( Value.find( Key ) != string :: npos )
		return;

	string :: size_type KeyStart = Text.find( Key );

	while( KeyStart != string :: npos )
	{
		Text.replace( KeyStart, Key.size( ), Value );
		KeyStart = Text.find( Key );
	}
}


string CLanguage :: ConnectingToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0001", "lang_0001" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ConnectedToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0002", "lang_0002" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: DisconnectedFromBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0003", "lang_0003" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: LoggedInToBNET( string server )
{
	string Out = m_CFG->GetString( "lang_0004", "lang_0004" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ConnectingToBNETTimedOut( string server )
{
	string Out = m_CFG->GetString( "lang_0005", "lang_0005" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: UpdatingClanList( )
{
	return m_CFG->GetString( "lang_0006", "lang_0006" );
}

string CLanguage :: UpdatingFriendsList( )
{
	return m_CFG->GetString( "lang_0007", "lang_0008" );
}


string CLanguage :: InvitedToClan( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0020", "lang_0020" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: RemovedFromClan( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0021", "lang_0021" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: MOTDChanged( )
{
	return m_CFG->GetString( "lang_0022", "lang_0022" );
}

string CLanguage :: ChangedClanRank( string server, string user, string rank )
{
	string Out = m_CFG->GetString( "lang_0023", "lang_0023" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	Replace( Out, "$RANK$", rank );
	return Out;
}


string CLanguage :: YouDontHaveAccessToThatCommand( )
{
	return m_CFG->GetString( "lang_0030", "lang_0030" );
}

string CLanguage :: UnknownCommand( string command )
{
	string Out = m_CFG->GetString( "lang_0031", "lang_0031" );
	Replace( Out, "$COMMAND$", command );
	return Out;
}

string CLanguage :: WhisperCommandsDisabled( )
{
	return m_CFG->GetString( "lang_0032", "lang_0032" );
}

string CLanguage :: SyntaxError( )
{
	return m_CFG->GetString( "lang_0033", "lang_0033" );
}

string CLanguage :: KickSpam( )
{
	return m_CFG->GetString( "lang_0034", "lang_0034" );
}

string CLanguage :: KickYell( )
{
	return m_CFG->GetString( "lang_0035", "lang_0035" );
}

string CLanguage :: KickPhrase( )
{
	return m_CFG->GetString( "lang_0036", "lang_0036" );
}


string CLanguage :: AddedUser( string server, string user, string access )
{
	string Out = m_CFG->GetString( "lang_0040", "lang_0040" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	Replace( Out, "$ACCESS$", access );
	return Out;
}

string CLanguage :: ErrorAddingUser( string user )
{
	string Out = m_CFG->GetString( "lang_0041", "lang_0041" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: DeletedUser( string server, string user )
{
	string Out = m_CFG->GetString( "lang_0042", "lang_0042" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: ErrorDeletingUser( string user )
{
	string Out = m_CFG->GetString( "lang_0043", "lang_0043" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: YouCantDeleteTheRootAdmin( )
{
	return m_CFG->GetString( "lang_0044", "lang_0044" );
}

string CLanguage :: ThereAreNoUsers( string server )
{
	string Out = m_CFG->GetString( "lang_0045", "lang_0045" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ThereIsUser( string server )
{
	string Out = m_CFG->GetString( "lang_0046", "lang_0046" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ThereAreUsers( string server, string count )
{
	string Out = m_CFG->GetString( "lang_0047", "lang_0047" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$COUNT$", count );
	return Out;
}

string CLanguage :: UserHasAccess( string server, string user, string access )
{
	string Out = m_CFG->GetString( "lang_0048", "lang_0048" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$USER$", user );
	Replace( Out, "$ACCESS$", access );
	return Out;
}

string CLanguage :: UserIsAlreadyBanned( string server, string victim )
{
	string Out = m_CFG->GetString( "lang_0049", "lang_0049" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: BannedUser( string server, string victim )
{
	string Out = m_CFG->GetString( "lang_0050", "lang_0050" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: ErrorBanningUser( string victim )
{
	string Out = m_CFG->GetString( "lang_0051", "lang_0051" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: UserWasBannedOnByBecause( string server, string victim, string date, string admin, string reason )
{
	string Out = m_CFG->GetString( "lang_0052", "lang_0052" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$DATE$", date );
	Replace( Out, "$ADMIN$", admin );
	Replace( Out, "$REASON$", reason );
	return Out;
}

string CLanguage :: UserIsNotBanned( string server, string victim )
{
	string Out = m_CFG->GetString( "lang_0053", "lang_0053" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: ThereAreNoBannedUsers( string server )
{
	string Out = m_CFG->GetString( "lang_0054", "lang_0054" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ThereIsBannedUser( string server )
{
	string Out = m_CFG->GetString( "lang_0055", "lang_0055" );
	Replace( Out, "$SERVER$", server );
	return Out;
}

string CLanguage :: ThereAreBannedUsers( string server, string count )
{
	string Out = m_CFG->GetString( "lang_0056", "lang_0056" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$COUNT$", count );
	return Out;
}

string CLanguage :: UnbannedUser( string server, string victim )
{
	string Out = m_CFG->GetString( "lang_0057", "lang_0057" );
	Replace( Out, "$SERVER$", server );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: ErrorUnbanningUser( string victim )
{
	string Out = m_CFG->GetString( "lang_0058", "lang_0058" );
	Replace( Out, "$VICTIM$", victim );
	return Out;
}

string CLanguage :: HasPlayedGames( string user, string firstgame, string lastgame, string totalgames, string avgloadingtime, string avgstay )
{
	string Out = m_CFG->GetString( "lang_0059", "lang_0059" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$FIRSTGAME$", firstgame );
	Replace( Out, "$LASTGAME$", lastgame );
	Replace( Out, "$TOTALGAMES$", totalgames );
	Replace( Out, "$AVGLOADINGTIME$", avgloadingtime );
	Replace( Out, "$AVGSTAY$", avgstay );
	return Out;
}

string CLanguage :: HasntPlayedGames( string user )
{
	string Out = m_CFG->GetString( "lang_0060", "lang_0060" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: HasPlayedDotAGames( string user, string totalgames, string totalwins, string totallosses, string totalkills, string totaldeaths, string totalcreepkills, string totalcreepdenies, string totalassists, string totalneutralkills, string totaltowerkills, string totalraxkills, string totalcourierkills, string avgkills, string avgdeaths, string avgcreepkills, string avgcreepdenies, string avgassists, string avgneutralkills, string avgraxkills, string avgcourierkills )
{
	string Out = m_CFG->GetString( "lang_0061", "lang_0061" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$TOTALGAMES$", totalgames );
	Replace( Out, "$TOTALWINS$", totalwins );
	Replace( Out, "$TOTALLOSSES$", totallosses );
	Replace( Out, "$TOTALKILLS$", totalkills );
	Replace( Out, "$TOTALDEATHS$", totaldeaths );
	Replace( Out, "$TOTALCREEPKILLS$", totalcreepkills );
	Replace( Out, "$TOTALCREEPDENIES$", totalcreepdenies );
	Replace( Out, "$TOTALASSISTS$", totalassists );
	Replace( Out, "$TOTALNEUTRALKILLS$", totalneutralkills );
	Replace( Out, "$TOTALTOWERKILLS$", totaltowerkills );
	Replace( Out, "$TOTALRAXKILLS$", totalraxkills );
	Replace( Out, "$TOTALCOURIERKILLS$", totalcourierkills );
	Replace( Out, "$AVGKILLS$", avgkills );
	Replace( Out, "$AVGDEATHS$", avgdeaths );
	Replace( Out, "$AVGCREEPKILLS$", avgcreepkills );
	Replace( Out, "$AVGCREEPDENIES$", avgcreepdenies );
	Replace( Out, "$AVGASSISTS$", avgassists );
	Replace( Out, "$AVGNEUTRALKILLS$", avgneutralkills );
	Replace( Out, "$AVGRAXKILLS$", avgraxkills );
	Replace( Out, "$AVGCOURIERKILLS$", avgcourierkills );
	return Out;
}

string CLanguage :: HasntPlayedDotAGames( string user )
{
	string Out = m_CFG->GetString( "lang_0062", "lang_0062" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UserIsHere( string user )
{
	string Out = m_CFG->GetString( "lang_0063", "lang_0063" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UserSeen( string user, string time )
{
	string Out = m_CFG->GetString( "lang_0064", "lang_0064" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$TIME$", time );
	return Out;
}

string CLanguage :: UserNotInClan( string user )
{
	string Out = m_CFG->GetString( "lang_0065", "lang_0065" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: UserNotSeen( string user )
{
	string Out = m_CFG->GetString( "lang_0066", "lang_0066" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: Slap( string text, string user, string victim, string number )
{
	string Out = text;
	Replace( Out, "$USER$", user );
	Replace( Out, "$VICTIM$", victim );
	Replace( Out, "$RAND$", number );
	return Out;
}

string CLanguage :: NoWhispersYet( )
{
	return m_CFG->GetString( "lang_0067", "lang_0067" );
}

string CLanguage :: LastWhisperFrom( string user )
{
	string Out = m_CFG->GetString( "lang_0068", "lang_0068" );
	Replace( Out, "$USER$", user );
	return Out;
}

string CLanguage :: Version( string version )
{
	string Out = m_CFG->GetString( "lang_0069", "lang_0069" );
	Replace( Out, "$VERSION$", version );
	return Out;
}

string CLanguage :: UsersPingIs( string user, string ping )
{
	string Out = m_CFG->GetString( "lang_0070", "lang_0070" );
	Replace( Out, "$USER$", user );
	Replace( Out, "$PING$", ping );
	return Out;
}

string CLanguage :: QuoteAdded( )
{
	return m_CFG->GetString( "lang_0071", "lang_0071" );
}

string CLanguage :: BotOwner( string user )
{
	string Out = m_CFG->GetString( "lang_0072", "lang_0072" );
	Replace( Out, "$USER$", user );
	return Out;
}
