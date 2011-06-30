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

#ifndef CHOP_H
#define CHOP_H

// standard integer sizes for 64 bit compatibility

#ifdef WIN32
 #include "ms_stdint.h"
#else
 #include <stdint.h>
#endif

// STL

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

using namespace std;

typedef vector<unsigned char> BYTEARRAY;

//python

#include <boost/python.hpp>
extern map< string, vector<boost::python::object> > gHandlersFirst;
extern map< string, vector<boost::python::object> > gHandlersSecond;

#define EXECUTE_HANDLER(HandlerName, throw_if_not_all_succeeded, ...)											\
	{																											\
		bool AllSucceeded = true;																				\
		vector<boost::python::object>* Functions = throw_if_not_all_succeeded ?									\
											&gHandlersFirst[ HandlerName ] : &gHandlersSecond[ HandlerName ];	\
																												\
		if( !Functions->empty() )																				\
		{																										\
			for( vector<boost::python::object>::iterator i = Functions->begin(); i != Functions->end(); i++ )	\
			{																									\
				try																								\
				{																								\
					bool Val = boost::python::extract<bool>( (*i)( __VA_ARGS__ ) );								\
																												\
					if( !Val )																				\
						AllSucceeded = false;																	\
				}																								\
				catch(...)																						\
				{																								\
					AllSucceeded = false;																		\
					PyErr_Print( );																				\
				}																								\
			}																									\
																												\
			if( !AllSucceeded && throw_if_not_all_succeeded )													\
				throw 5;																						\
		}																										\
	}	



// time

uint32_t GetTime( );		// seconds
uint32_t GetTicks( );		// milliseconds

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif

// network

#undef FD_SETSIZE
#define FD_SETSIZE 512

// output

void CONSOLE_Print( string message );
void DEBUG_Print( string message );
void DEBUG_Print( BYTEARRAY b );

//
// CChOP
//

class CUDPSocket;
class CCRC32;
class CBNET;
class CChOPDB;
class CBaseCallable;
class CLanguage;
class CConfig;

class CChOP
{
public:
	CCRC32 *m_CRC;							// for calculating CRC's
	vector<CBNET *> m_BNETs;				// all our battle.net connections (there can be more than one)
	CChOPDB *m_DB;							// database
	vector<CBaseCallable *> m_Callables;	// vector of orphaned callables waiting to die
	CLanguage *m_Language;					// language
	bool m_Exiting;							// set to true to force chop to shutdown next update (used by SignalCatcher)
	string m_Version;						// ChOP++ version string
	string m_LanguageFile;					// config value: language file
	string m_Warcraft3Path;					// config value: Warcraft 3 path
	char m_CommandTrigger;					// config value: the command trigger inside games


	string m_GHostServerAccount;			// name of the GHost++ account
	string m_CFGPath;						// path to txt files

	uint32_t m_SpamCacheSize;				// size of spam cache
	uint32_t m_MaxChatMsg;					// maximum chat messages of a user per 2s
	uint32_t m_UpTime;						// time when chop started
	
	bool m_SeenAllUsers;					// log seen for all users or just clan members?

	bool m_UseGHost;						// connect to GHost++ bot ?
	bool m_WhisperAllowed;					// allow commands via whisper ?
	bool m_AntiSpam;						// activate AntiSpam ?
	bool m_AntiYell;						// activate AntiYell ?
	bool m_PhraseKick;						// activate PhraseKick ?

	vector<string> m_Phrases;				// vector of words for PhraseKick
	vector<string> m_SlapPositive;			// vector of slap messages against the victim
	vector<string> m_SlapNegative;			// vector of slap messages against the user
	vector<string> m_Ask8Ball;				// vector of ask8ball answers
	vector<string> m_Quotes;				// vector of quotes

#ifdef WIN32
	string CmdInput;
#endif

	CChOP( CConfig *CFG );
	~CChOP( );

	// processing functions

	bool Update( long usecBlock );
	void UpdatePhrases( );
	void UpdateQuotes( );
	void UpdateSlap( );
	void UpdateAsk8Ball( );

	// events

	void EventBNETConnecting( CBNET *bnet );
	void EventBNETConnected( CBNET *bnet );
	void EventBNETDisconnected( CBNET *bnet );
	void EventBNETLoggedIn( CBNET *bnet );
	void EventBNETConnectTimedOut( CBNET *bnet );
	
	static void RegisterPythonClass( );
};

#endif
