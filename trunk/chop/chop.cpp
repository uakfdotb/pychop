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

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "chop.h"
#include "util.h"
#include "crc32.h"
#include "config.h"
#include "language.h"
#include "socket.h"
#include "db.h"
#include "dbmysql.h"
#include "bnet.h"
#include "bnlsprotocol.h"
#include "bncsutilinterface.h"
#include "user.h"

#include <signal.h>
#include <stdlib.h>

#define __STORMLIB_SELF__
#include <stormlib/StormLib.h>


#ifdef WIN32
 #include <windows.h>
 #include <winsock.h>
#endif

#include <time.h>

#ifndef WIN32
 #include <sys/time.h>
#endif

#ifdef __APPLE__
 #include <mach/mach_time.h>
#endif

time_t gStartTime;
string gLogFile;
CChOP *gChOP = NULL;
CConfig CFG;

uint32_t GetTime( )
{
	return (uint32_t)( ( GetTicks( ) / 1000 ) - gStartTime );
}

uint32_t GetTicks( )
{
#ifdef WIN32
	return GetTickCount( );
#elif __APPLE__
	uint64_t current = mach_absolute_time( );
	static mach_timebase_info_data_t info = { 0, 0 };
	// get timebase info
	if( info.denom == 0 )
		mach_timebase_info( &info );
	uint64_t elapsednano = current * ( info.numer / info.denom );
	// convert ns to ms
	return elapsednano / 1e6;
#else
	uint32_t ticks;
	struct timespec t;
	clock_gettime( CLOCK_MONOTONIC, &t );
	ticks = t.tv_sec * 1000;
	ticks += t.tv_nsec / 1000000;
	return ticks;
#endif
}

CConfig GetCFG( )
{
	return CFG;
}

void SignalCatcher( int s )
{
	// signal( SIGABRT, SignalCatcher );
	// signal( SIGINT, SignalCatcher );

	CONSOLE_Print( "[!!!] caught signal " + UTIL_ToString( s ) + ", shutting down" );

	if( gChOP )
	{
		if( gChOP->m_Exiting )
			exit( 1 );
		else
			gChOP->m_Exiting = true;
	}
	else
		exit( 1 );
}

//
// host module 
//

map< string, vector<boost::python::object> > gHandlersFirst;
map< string, vector<boost::python::object> > gHandlersSecond;

void RegisterHandler( string HandlerName, boost::python::object nFunction, bool nBool = false )
{
	using namespace boost::python;

	if( !PyFunction_Check( nFunction.ptr() ) )
	{
		string Error = "argument 1 must be function, not ";
		Error += nFunction.ptr()->ob_type->tp_name;
		PyErr_SetString( PyExc_TypeError, Error.c_str() );

		throw_error_already_set();
	}

	if( nBool )
		gHandlersFirst[HandlerName].push_back( nFunction );
	else
		gHandlersSecond[HandlerName].push_back( nFunction );
}

void UnregisterHandler( string HandlerName, boost::python::object nFunction, bool nBool = false )
{
	using namespace boost::python;

	if( !PyFunction_Check( nFunction.ptr() ) )
	{
		string Error = "argument 1 must be function, not ";
		Error += nFunction.ptr()->ob_type->tp_name;
		PyErr_SetString( PyExc_TypeError, Error.c_str() );

		throw_error_already_set();
	}

	vector<object>* Functions = nBool ? &gHandlersFirst[HandlerName] : &gHandlersSecond[HandlerName];
	for( vector<object>::iterator i = Functions->begin(); i != Functions->end(); )
	{
		if( *i == nFunction )
			i = Functions->erase(i);
		else
			i++;
	}
}

BOOST_PYTHON_FUNCTION_OVERLOADS(RegisterHandler_Overloads, RegisterHandler, 2, 3);
BOOST_PYTHON_FUNCTION_OVERLOADS(UnregisterHandler_Overloads, UnregisterHandler, 2, 3);

BOOST_PYTHON_MODULE(host)
{
	using namespace boost::python;

	def( "registerHandler", RegisterHandler, RegisterHandler_Overloads() );
	def( "unregisterHandler", UnregisterHandler, UnregisterHandler_Overloads() );
	def( "log", CONSOLE_Print );
	def( "GetTicks", GetTicks );
	def( "config", GetCFG );
}

BOOST_PYTHON_MODULE(BNLSProtocol)
{
	using namespace boost::python;
	enum_<CBNLSProtocol::Protocol>("protocol")
		.value("BNLS_NULL", CBNLSProtocol::BNLS_NULL)
		.value("BNLS_CDKEY", CBNLSProtocol::BNLS_CDKEY)
		.value("BNLS_LOGONCHALLENGE", CBNLSProtocol::BNLS_LOGONCHALLENGE)
		.value("BNLS_LOGONPROOF", CBNLSProtocol::BNLS_LOGONPROOF)
		.value("BNLS_CREATEACCOUNT", CBNLSProtocol::BNLS_CREATEACCOUNT)
		.value("BNLS_CHANGECHALLENGE", CBNLSProtocol::BNLS_CHANGECHALLENGE)
		.value("BNLS_CHANGEPROOF", CBNLSProtocol::BNLS_CHANGEPROOF)
		.value("BNLS_UPGRADECHALLENGE", CBNLSProtocol::BNLS_UPGRADECHALLENGE)
		.value("BNLS_UPGRADEPROOF", CBNLSProtocol::BNLS_UPGRADEPROOF)
		.value("BNLS_VERSIONCHECK", CBNLSProtocol::BNLS_VERSIONCHECK)
		.value("BNLS_CONFIRMLOGON", CBNLSProtocol::BNLS_CONFIRMLOGON)
		.value("BNLS_HASHDATA", CBNLSProtocol::BNLS_HASHDATA)
		.value("BNLS_CDKEY_EX", CBNLSProtocol::BNLS_CDKEY_EX)
		.value("BNLS_CHOOSENLSREVISION", CBNLSProtocol::BNLS_CHOOSENLSREVISION)
		.value("BNLS_AUTHORIZE", CBNLSProtocol::BNLS_AUTHORIZE)
		.value("BNLS_AUTHORIZEPROOF", CBNLSProtocol::BNLS_AUTHORIZEPROOF)
		.value("BNLS_REQUESTVERSIONBYTE", CBNLSProtocol::BNLS_REQUESTVERSIONBYTE)
		.value("BNLS_VERIFYSERVER", CBNLSProtocol::BNLS_VERIFYSERVER)
		.value("BNLS_RESERVESERVERSLOTS", CBNLSProtocol::BNLS_RESERVESERVERSLOTS)
		.value("BNLS_SERVERLOGONCHALLENGE", CBNLSProtocol::BNLS_SERVERLOGONCHALLENGE)
		.value("BNLS_SERVERLOGONPROOF", CBNLSProtocol::BNLS_SERVERLOGONPROOF)
		.value("BNLS_RESERVED0", CBNLSProtocol::BNLS_RESERVED0)
		.value("BNLS_RESERVED1", CBNLSProtocol::BNLS_RESERVED1)
		.value("BNLS_RESERVED2", CBNLSProtocol::BNLS_RESERVED2)
		.value("BNLS_VERSIONCHECKEX", CBNLSProtocol::BNLS_VERSIONCHECKEX)
		.value("BNLS_RESERVED3", CBNLSProtocol::BNLS_RESERVED3)
		.value("BNLS_VERSIONCHECKEX2", CBNLSProtocol::BNLS_VERSIONCHECKEX2)
		.value("BNLS_WARDEN", CBNLSProtocol::BNLS_WARDEN)
	;
}

BOOST_PYTHON_MODULE(BNETProtocol)
{
	using namespace boost::python;

	enum_<CBNETProtocol::Protocol>("protocol")
		.value("SID_NULL", CBNETProtocol::SID_NULL)
		.value("SID_ENTERCHAT", CBNETProtocol::SID_ENTERCHAT)
		.value("SID_JOINCHANNEL", CBNETProtocol::SID_JOINCHANNEL)
		.value("SID_CHATCOMMAND", CBNETProtocol::SID_CHATCOMMAND)
		.value("SID_CHATEVENT", CBNETProtocol::SID_CHATEVENT)
		.value("SID_CHECKAD", CBNETProtocol::SID_CHECKAD)
		.value("SID_DISPLAYAD", CBNETProtocol::SID_DISPLAYAD)
		.value("SID_PING", CBNETProtocol::SID_PING)
		.value("SID_LOGONRESPONSE", CBNETProtocol::SID_LOGONRESPONSE)
		.value("SID_NETGAMEPORT", CBNETProtocol::SID_NETGAMEPORT)
		.value("SID_AUTH_INFO", CBNETProtocol::SID_AUTH_INFO)
		.value("SID_AUTH_CHECK", CBNETProtocol::SID_AUTH_CHECK)
		.value("SID_AUTH_ACCOUNTLOGON", CBNETProtocol::SID_AUTH_ACCOUNTLOGON)
		.value("SID_AUTH_ACCOUNTLOGONPROOF", CBNETProtocol::SID_AUTH_ACCOUNTLOGONPROOF)
		.value("SID_WARDEN", CBNETProtocol::SID_WARDEN)
		.value("SID_FRIENDSLIST", CBNETProtocol::SID_FRIENDSLIST)
		.value("SID_FRIENDSUPDATE", CBNETProtocol::SID_FRIENDSUPDATE)
		.value("SID_CLANINVITATION", CBNETProtocol::SID_CLANINVITATION)
		.value("SID_CLANREMOVE", CBNETProtocol::SID_CLANREMOVE)
		.value("SID_CLANCHANGERANK", CBNETProtocol::SID_CLANCHANGERANK)
		.value("SID_CLANSETMOTD", CBNETProtocol::SID_CLANSETMOTD)
		.value("SID_CLANMEMBERLIST", CBNETProtocol::SID_CLANMEMBERLIST)
		.value("SID_CLANMEMBERSTATUSCHANGE", CBNETProtocol::SID_CLANMEMBERSTATUSCHANGE)
	;

	enum_<CBNETProtocol::KeyResult>("keyResult")
		.value("KR_GOOD", CBNETProtocol::KR_GOOD)
		.value("KR_OLD_GAME_VERSION", CBNETProtocol::KR_OLD_GAME_VERSION)
		.value("KR_INVALID_VERSION", CBNETProtocol::KR_INVALID_VERSION)
		.value("KR_ROC_KEY_IN_USE", CBNETProtocol::KR_ROC_KEY_IN_USE)
		.value("KR_TFT_KEY_IN_USE", CBNETProtocol::KR_TFT_KEY_IN_USE)
	;

	enum_<CBNETProtocol::IncomingChatEvent>("incomingChatEvent")
		.value("EID_SHOWUSER", CBNETProtocol::EID_SHOWUSER)
		.value("EID_JOIN", CBNETProtocol::EID_JOIN)
		.value("EID_LEAVE", CBNETProtocol::EID_LEAVE)
		.value("EID_WHISPER", CBNETProtocol::EID_WHISPER)
		.value("EID_TALK", CBNETProtocol::EID_TALK)
		.value("EID_BROADCAST", CBNETProtocol::EID_BROADCAST)
		.value("EID_CHANNEL", CBNETProtocol::EID_CHANNEL)
		.value("EID_USERFLAGS", CBNETProtocol::EID_USERFLAGS)
		.value("EID_WHISPERSENT", CBNETProtocol::EID_WHISPERSENT)
		.value("EID_CHANNELFULL", CBNETProtocol::EID_CHANNELFULL)
		.value("EID_CHANNELDOESNOTEXIST", CBNETProtocol::EID_CHANNELDOESNOTEXIST)
		.value("EID_CHANNELRESTRICTED", CBNETProtocol::EID_CHANNELRESTRICTED)
		.value("EID_INFO", CBNETProtocol::EID_INFO)
		.value("EID_ERROR", CBNETProtocol::EID_ERROR)
		.value("EID_EMOTE", CBNETProtocol::EID_EMOTE)
		.value("FLAG_OP", CBNETProtocol::FLAG_OP)
	;
	
	enum_<CBNETProtocol::RankCode>("rankCode")
		.value("CLAN_RECRUIT", CBNETProtocol::CLAN_RECRUIT)
		.value("CLAN_PEON", CBNETProtocol::CLAN_PEON)
		.value("CLAN_GRUNT", CBNETProtocol::CLAN_GRUNT)
		.value("CLAN_SHAMAN", CBNETProtocol::CLAN_SHAMAN)
		.value("CLAN_CHIEF", CBNETProtocol::CLAN_CHIEF)
	;
}

//
// main
//

int main( int argc, char **argv )
{
	string CFGFile = "chop.cfg";

	if( argc > 1 && argv[1] )
		CFGFile = argv[1];

	// read config file

	CFG.Read( CFGFile );
	gLogFile = CFG.GetString( "bot_log", string( ) );

	// print something for logging purposes

	CONSOLE_Print( "[PYCHOP] starting up" );

	// catch SIGABRT and SIGINT

	signal( SIGABRT, SignalCatcher );
	signal( SIGINT, SignalCatcher );

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

	// initialize the start time

	gStartTime = 0;
	gStartTime = GetTime( );

#ifdef WIN32
	// initialize winsock

	CONSOLE_Print( "[PYCHOP] starting winsock" );
	WSADATA wsadata;

	if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
	{
		CONSOLE_Print( "[PYCHOP] error starting winsock" );
		return 1;
	}
#endif

    // initialize rand
    srand ( time(NULL) );

	// register the builtin modules

	if( PyImport_AppendInittab("host", inithost) == -1 )
		throw std::runtime_error( "Failed to add host to the interpreter's builtin modules" );
	if( PyImport_AppendInittab("BNLSProtocol", initBNLSProtocol) == -1 )
		throw std::runtime_error( "Failed to add host to the interpreter's builtin modules" );
	if( PyImport_AppendInittab("BNETProtocol", initBNETProtocol) == -1 )
		throw std::runtime_error( "Failed to add host to the interpreter's builtin modules" );

#ifdef WIN32
	Py_SetPythonHome(".\\python\\");
#endif

	Py_Initialize( );

	boost::python::object global( boost::python::import("__main__").attr("__dict__") );
	boost::python::exec("import sys, host												\n"
						"																\n"
						"class Logger:													\n"	
						"	def __init__(self, name):									\n"
						"		self.name = name										\n"
						"		self.buffer = ''										\n"
						"																\n"
						"	def write(self, string):									\n"
						"		if len(string) == 0: return								\n"
						"		self.buffer += string									\n"
						"																\n"
						"		if string[-1] == '\\n':									\n"
						"			host.log('[PYTHON] ' + self.buffer[:-1])			\n"
						"			self.buffer = ''									\n"
						"																\n"
						"	def flush(self): pass										\n"
						"	def close(self): pass										\n"
						"																\n"
						"#forwarding all python 'prints' to the c++ log ( host.log )	\n"
						"sys.stdout = Logger('stdout')									\n"
						"sys.stderr = Logger('stderr')									\n",
						global, global);
						
	string PluginPath = CFG.GetString( "bot_pluginpath", "." );
	string AppendCode = "import sys\nsys.path.append('" + PluginPath + "')";

	try
	{
		boost::python::exec(AppendCode.c_str(), global, global);
	}
	catch(...)
	{
		PyErr_Print();
		throw;
	}

	CChOP::RegisterPythonClass( );
	CBNLSProtocol::RegisterPythonClass( );
	CBNETProtocol::RegisterPythonClass( );
	CBNET::RegisterPythonClass( );
	CConfig::RegisterPythonClass( );
	CUser::RegisterPythonClass( );
	CIncomingClanList::RegisterPythonClass( );
	CIncomingFriendList::RegisterPythonClass( );
	CIncomingChatEvent::RegisterPythonClass( );
	CIncomingGameHost::RegisterPythonClass( );
	CIncomingProfile::RegisterPythonClass( );

    CONSOLE_Print("[PYTHON] Importing plugins module");

	try
	{
		boost::python::object module = boost::python::import("plugins.pychop");
	}
	catch(...)
	{
		PyErr_Print( );
		throw;
	}

	EXECUTE_HANDLER("StartUp", false, boost::ref(CFG))
	EXECUTE_HANDLER("StartUp", true, boost::ref(CFG))

	// initialize chop

	gChOP = new CChOP( &CFG );
 
	EXECUTE_HANDLER("ChOPStarted", false, boost::ref(gChOP))
	try	
	{ 
		EXECUTE_HANDLER("ChOPStarted", true, boost::ref(gChOP)) 
	}
	catch(...) 
	{ 

	}

	while( 1 )
	{
		// block for 10ms on all sockets - if you intend to perform any timed actions more frequently you should change this
		// that said it's likely we'll loop more often than this due to there being data waiting on one of the sockets but there aren't any guarantees

		if( gChOP->Update( 10000 ) )
			break;
	}

	// shutdown chop

	CONSOLE_Print( "[PYCHOP] shutting down" );
	delete gChOP;
	gChOP = NULL;
	
	EXECUTE_HANDLER("ShutDown", false)
	try	
	{ 
		EXECUTE_HANDLER("ShutDown", true) 
	}
	catch(...) 
	{ 

	}

#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print( "[PYCHOP] shutting down winsock" );
	WSACleanup( );
#endif

	return 0;
}

void CONSOLE_Print( string message )
{
	cout << message << endl;

	// logging

	if( !gLogFile.empty( ) )
	{
		ofstream Log;
		Log.open( gLogFile.c_str( ), ios :: app );

		if( !Log.fail( ) )
		{
			time_t Now = time( NULL );
			string Time = asctime( localtime( &Now ) );

			// erase the newline
			
			Time.erase( Time.size( ) - 1 );
			Log << "[" << Time << "] " << message << endl;
			Log.close( );
		}
	}
}

string CONSOLE_Read( )
{
	string line;
	getline(cin, line);
	return line;
}

void DEBUG_Print( string message )
{
	cout << message << endl;
}

void DEBUG_Print( BYTEARRAY b )
{
	cout << "{ ";

	for( unsigned int i = 0; i < b.size( ); i++ )
		cout << hex << (int)b[i] << " ";

	cout << "}" << endl;
}

//
// CChOP
//

CChOP :: CChOP( CConfig *CFG )
{
	m_CRC = new CCRC32( );
	m_CRC->Initialize( );
	
	m_ConsoleUser = new CUser("console", 10);
	
	CONSOLE_Print( "[PYCHOP] opening primary database" );

	m_DB = new CChOPDBMySQL( CFG );

	m_Exiting = false;
	m_Version = "a1";

	m_LanguageFile = CFG->GetString( "bot_language", "language.cfg" );
	m_Language = new CLanguage( m_LanguageFile );
	m_Warcraft3Path = CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" );
	m_CFGPath = CFG->GetString( "bot_cfgpath", "cfg\\" );
	m_WhisperAllowed = CFG->GetBool( "bot_whisperallowed", true );
	
	m_SeenAllUsers = CFG->GetBool( "bot_seenallusers", true );
	m_Follow = CFG->GetInt( "bot_follow", 1 );
	m_DisablePublic = CFG->GetBool( "bot_disablepublic", false );
	m_DisplayNoAccess = CFG->GetBool( "bot_displaynoaccess", false );
	m_BanlistChannel = CFG->GetBool( "bot_banlistchannel", false );

	m_AntiSpam = CFG->GetBool( "op_antispam", false );
	m_AntiYell = CFG->GetBool( "op_antiyell", false );
	m_PhraseKick = CFG->GetBool( "op_phrasekick", false );
	m_SpamCacheSize = CFG->GetInt( "op_spamcachesize", 4 );

	// load the battle.net connections
	// we're just loading the config data and creating the CBNET classes here, the connections are established later (in the Update function)

	for( uint32_t i = 1; i < 10; i++ )
	{
		string Prefix;

		if( i == 1 )
			Prefix = "bnet_";
		else
			Prefix = "bnet" + UTIL_ToString( i ) + "_";

		string Server = CFG->GetString( Prefix + "server", string( ) );
		string ServerAlias = CFG->GetString( Prefix + "serveralias", string( ) );
		string CDKeyROC = CFG->GetString( Prefix + "cdkey_roc", string( ) );
		string CountryAbbrev = CFG->GetString( Prefix + "countryabbrev", "USA" );
		string Country = CFG->GetString( Prefix + "country", "United States" );
		string UserName = CFG->GetString( Prefix + "username", string( ) );
		string UserPassword = CFG->GetString( Prefix + "password", string( ) );
		string FirstChannel = CFG->GetString( Prefix + "firstchannel", "The Void" );
		string RootAdmin = CFG->GetString( Prefix + "rootadmin", string( ) );
		string BNETCommandTrigger = CFG->GetString( Prefix + "commandtrigger", "!" );

		if( BNETCommandTrigger.empty( ) )
			BNETCommandTrigger = "!";

		string BNLSServer = CFG->GetString( Prefix + "bnls_server", string( ) );
		int BNLSPort = CFG->GetInt( Prefix + "bnls_port", 9367 );
		int BNLSWardenCookie = CFG->GetInt( Prefix + "bnls_wardencookie", 0 );
		unsigned char War3Version = CFG->GetInt( Prefix + "custom_war3version", 23 );
		BYTEARRAY EXEVersion = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversion", string( ) ), 4 );
		BYTEARRAY EXEVersionHash = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversionhash", string( ) ), 4 );
		string PasswordHashType = CFG->GetString( Prefix + "custom_passwordhashtype", string( ) );
		uint32_t MaxMessageLength = CFG->GetInt( Prefix + "custom_maxmessagelength", 200 );

		if( Server.empty( ) )
			break;

		if( CDKeyROC.empty( ) )
		{
			CONSOLE_Print( "[PYCHOP] missing " + Prefix + "cdkeyroc, skipping this battle.net connection" );
			continue;
		}

		if( UserName.empty( ) )
		{
			CONSOLE_Print( "[PYCHOP] missing " + Prefix + "username, skipping this battle.net connection" );
			continue;
		}

		if( UserPassword.empty( ) )
		{
			CONSOLE_Print( "[PYCHOP] missing " + Prefix + "password, skipping this battle.net connection" );
			continue;
		}

		CONSOLE_Print( "[PYCHOP] found battle.net connection #" + UTIL_ToString( i ) + " for server [" + Server + "]" );
		m_BNETs.push_back( new CBNET( this, Server, ServerAlias, BNLSServer, (uint16_t)BNLSPort, (uint32_t)BNLSWardenCookie, CDKeyROC, CountryAbbrev, Country, UserName, UserPassword, FirstChannel, RootAdmin, BNETCommandTrigger[0], War3Version, EXEVersion, EXEVersionHash, PasswordHashType, MaxMessageLength ) );
	}

	if( m_BNETs.empty( ) )
		CONSOLE_Print( "[PYCHOP] warning - no battle.net connections found in config file" );

	CONSOLE_Print( "[PYCHOP] pychop " + m_Version );

	UpdateQuotes( );
	UpdateSlap( );
	UpdateAsk8Ball( );
	UpdatePhrases( );

	m_UpTime = GetTime( );
	
	// initialize the input thread
	inputThread = new boost::thread(&CChOP::inputLoop, this);
}

CChOP :: ~CChOP( )
{
	delete m_CRC;

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		delete *i;

	delete m_DB;

	// warning: we don't delete any entries of m_Callables here because we can't be guaranteed that the associated threads have terminated
	// this is fine if the program is currently exiting because the OS will clean up after us
	// but if you try to recreate the CChOP object within a single session you will probably leak resources!

	if( !m_Callables.empty( ) )
		CONSOLE_Print( "[PYCHOP] warning - " + UTIL_ToString( m_Callables.size( ) ) + " orphaned callables were leaked (this is not an error)" );

	delete m_Language;
}

bool CChOP :: Update( long usecBlock )
{
	// notify plugins of update
	
	EXECUTE_HANDLER("Update", false, boost::ref(this))

	// todotodo: do we really want to shutdown if there's a database error? is there any way to recover from this?

	if( m_DB->HasError( ) )
	{
		CONSOLE_Print( "[PYCHOP] database error - " + m_DB->GetError( ) );
		return true;
	}

	// update callables

	for( vector<CBaseCallable *> :: iterator i = m_Callables.begin( ); i != m_Callables.end( ); )
	{
		if( (*i)->GetReady( ) )
		{
			m_DB->RecoverCallable( *i );
			delete *i;
			i = m_Callables.erase( i );
		}
		else
			i++;
	}

	unsigned int NumFDs = 0;

	// take every socket we own and throw it in one giant select statement so we can block on all sockets

	int nfds = 0;
	fd_set fd;
	fd_set send_fd;
	FD_ZERO( &fd );
	FD_ZERO( &send_fd );

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		NumFDs += (*i)->SetFD( &fd, &send_fd, &nfds );

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = usecBlock;

	struct timeval send_tv;
	send_tv.tv_sec = 0;
	send_tv.tv_usec = 0;

#ifdef WIN32
	select( 1, &fd, NULL, NULL, &tv );
	select( 1, NULL, &send_fd, NULL, &send_tv );
#else
	select( nfds + 1, &fd, NULL, NULL, &tv );
	select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
#endif

	if( NumFDs == 0 )
	{
		// we don't have any sockets (i.e. we aren't connected to battle.net maybe due to a lost connection and there aren't any games running)
		// select will return immediately and we'll chew up the CPU if we let it loop so just sleep for 50ms to kill some time

		MILLISLEEP( 50 );
	}

	bool AdminExit = false;
	bool BNETExit = false;

	// update battle.net connections

	for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		if( (*i)->Update( &fd, &send_fd ) )
			BNETExit = true;
	}

	// execute input message
	boost::mutex::scoped_lock lock( m_InputMutex );
	if( !m_InputMessage.empty( ) ) {
		for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( m_InputMessage[0] == (*i)->GetCommandTrigger() )
			{
				string Reply, Command, Payload;
				string :: size_type PayloadStart = m_InputMessage.find( " " );
				uint32_t Type = 0; // type 0 indicates console origin

				if( PayloadStart != string :: npos )
				{
					Command = m_InputMessage.substr( 1, PayloadStart - 1 );
					Payload = m_InputMessage.substr( PayloadStart + 1 );
				}
				else
					Command = m_InputMessage.substr( 1 );

				string response = (*i)->ProcessCommand(m_ConsoleUser, Command, Payload, Type);
			
				if( !response.empty() ) {
					CONSOLE_Print(response);
				}
			} else {
					(*i)->QueueChatCommand(m_InputMessage);
			}
		}
		
		m_InputMessage = "";
	}
	
	lock.unlock( );

	return m_Exiting || AdminExit || BNETExit;
}

void CChOP :: inputLoop( )
{
	while( true )
	{
		string Message = CONSOLE_Read();
		boost::mutex::scoped_lock lock( m_InputMutex );
		m_InputMessage = Message;
		lock.unlock( );
	}
}

void CChOP :: UpdatePhrases( )
{
	string line, filestr = m_CFGPath + "phrase.txt";
	ifstream file( filestr.c_str( ) );

	if( !file.fail( ) )
	{
		m_Phrases.clear( );

		while( !file.eof( ) )
		{
			getline( file, line );
			line.erase( remove( line.begin( ), line.end( ), '\r' ), line.end( ) );
			line.erase( remove( line.begin( ), line.end( ), '\n' ), line.end( ) );

			if( !line.empty( ) )
			{
				if( find( m_Phrases.begin( ), m_Phrases.end( ), line ) != m_Phrases.end( ) )
					CONSOLE_Print( "[ChOP++] duplicate phrase: " + line );
				else
					m_Phrases.push_back( line );
			}
		}
		CONSOLE_Print( "[ChOP++] updated phrases (" + UTIL_ToString( m_Phrases.size( ) ) + ")" );
	}
	else
		CONSOLE_Print( "[ChOP++] warning - could not open phrase file '" + filestr + "'" );
	file.close( );
	file.clear( );
}

void CChOP :: UpdateQuotes( )
{
	string line, filestr = m_CFGPath + "quote.txt";
	ifstream file( filestr.c_str( ) );
	
	if( !file.fail( ) )
	{
		m_Quotes.clear( );

		while( !file.eof( ) )
		{
			getline( file, line );
			line.erase( remove( line.begin( ), line.end( ), '\r' ), line.end( ) );
			line.erase( remove( line.begin( ), line.end( ), '\n' ), line.end( ) );

            if( !line.empty( ) )
				m_Quotes.push_back( line );
		}
		CONSOLE_Print( "[ChOP++] updated quotes (" + UTIL_ToString( m_Quotes.size( ) ) + ")" );
	}
	else
		CONSOLE_Print( "[ChOP++] warning - could not open quote file '" + filestr + "'" );
	file.close( );
	file.clear( );
}

void CChOP :: UpdateSlap( )
{
	// positive
	string line, filestr = m_CFGPath + "slap_pos.txt";
	ifstream file_pos( filestr.c_str( ) );

	if( !file_pos.fail( ) )
	{
		m_SlapPositive.clear( );

		while( !file_pos.eof( ) )
		{
			getline( file_pos, line );
			line.erase( remove( line.begin( ), line.end( ), '\r' ), line.end( ) );
			line.erase( remove( line.begin( ), line.end( ), '\n' ), line.end( ) );

			if( !line.empty( ) )
				m_SlapPositive.push_back( line );
		}
		CONSOLE_Print( "[ChOP++] updated positive slap stirngs (" + UTIL_ToString( m_SlapPositive.size( ) ) + ")" );
	}
	else
		CONSOLE_Print( "[ChOP++] warning - could not open slap file '" + filestr + "'" );
	file_pos.close( );
	file_pos.clear( );

	// negative
	filestr = m_CFGPath + "slap_neg.txt";
	ifstream file_neg( filestr.c_str( ) );

	if( !file_neg.fail( ) )
	{
		m_SlapNegative.clear( );

		while( !file_neg.eof( ) )
		{
			getline( file_neg, line );
			line.erase( remove( line.begin( ), line.end( ), '\r' ), line.end( ) );
			line.erase( remove( line.begin( ), line.end( ), '\n' ), line.end( ) );

			if( !line.empty( ) )
				m_SlapNegative.push_back( line );
		}
		CONSOLE_Print( "[ChOP++] updated negative slap strings (" + UTIL_ToString( m_SlapNegative.size( ) ) + ")" );
	}
	else
		CONSOLE_Print( "[ChOP++] warning - could not open slap file '" + filestr + "'" );
	file_neg.close( );
	file_neg.clear( );
}

void CChOP :: UpdateAsk8Ball( )
{
	string line, filestr = m_CFGPath + "ask8ball.txt";
	ifstream file( filestr.c_str( ) );

	if( !file.fail( ) )
	{
		m_Ask8Ball.clear( );

		while( !file.eof( ) )
		{
			getline( file, line );
			line.erase( remove( line.begin( ), line.end( ), '\r' ), line.end( ) );
			line.erase( remove( line.begin( ), line.end( ), '\n' ), line.end( ) );

			if( !line.empty( ) );
				m_Ask8Ball.push_back( line );
		}
		CONSOLE_Print( "[ChOP++] updated ask8ball strings (" + UTIL_ToString( m_Ask8Ball.size( ) ) + ")" );
	}
	else
		CONSOLE_Print( "[ChOP++] warning - could not open ask8ball file '" + filestr + "'" );
	file.close( );
	file.clear( );
}

void CChOP :: EventBNETConnecting( CBNET *bnet )
{
	try	
	{ 
		EXECUTE_HANDLER("BNETConnecting", true, boost::ref(this), boost::ref(bnet)) 
	}
	catch(...) 
	{ 
		return;
	}
	
	EXECUTE_HANDLER("BNETConnecting", false, boost::ref(this), boost::ref(bnet))
}

void CChOP :: EventBNETConnected( CBNET *bnet )
{
	try	
	{ 
		EXECUTE_HANDLER("BNETConnected", true, boost::ref(this), boost::ref(bnet)) 
	}
	catch(...) 
	{ 
		return;
	}
	
	EXECUTE_HANDLER("BNETConnected", false, boost::ref(this), boost::ref(bnet))
}

void CChOP :: EventBNETDisconnected( CBNET *bnet )
{
	try	
	{ 
		EXECUTE_HANDLER("BNETDisconnected", true, boost::ref(this), boost::ref(bnet)) 
	}
	catch(...) 
	{ 
		return;
	}
	
	EXECUTE_HANDLER("BNETDisconnected", false, boost::ref(this), boost::ref(bnet))
}

void CChOP :: EventBNETLoggedIn( CBNET *bnet )
{
	try	
	{ 
		EXECUTE_HANDLER("BNETLoggedIn", true, boost::ref(this), boost::ref(bnet)) 
	}
	catch(...) 
	{ 
		return;
	}
	
	EXECUTE_HANDLER("BNETLoggedIn", false, boost::ref(this), boost::ref(bnet))
}

void CChOP :: EventBNETConnectTimedOut( CBNET *bnet )
{
	try	
	{ 
		EXECUTE_HANDLER("BNETConnectTimedOut", true, boost::ref(this), boost::ref(bnet)) 
	}
	catch(...) 
	{ 
		return;
	}
	
	EXECUTE_HANDLER("BNETConnectTimedOut", false, boost::ref(this), boost::ref(bnet))
}

void CChOP :: RegisterPythonClass( )
{
	using namespace boost::python;

	
	class_<std::vector<CBNET *> >("VECT_BNET")
		.def(vector_indexing_suite<std::vector<CBNET *> >())
		;

	class_<CChOP>("CChOP", no_init)
		.def_readwrite("CRC", &CChOP::m_CRC)
		.def_readwrite("BNETs", &CChOP::m_BNETs)
		.def_readwrite("DB", &CChOP::m_DB)
		.def_readwrite("Callables", &CChOP::m_Callables)
		.def_readwrite("Language", &CChOP::m_Language)
		.def_readwrite("Exiting", &CChOP::m_Exiting)
		.def_readwrite("Version", &CChOP::m_Version)
		.def_readwrite("LanguageFile", &CChOP::m_LanguageFile)
		.def_readwrite("Warcraft3Path", &CChOP::m_Warcraft3Path)
		.def_readwrite("CFGPath", &CChOP::m_CFGPath)
		.def_readwrite("SpamCacheSize", &CChOP::m_SpamCacheSize)
		.def_readwrite("MaxChatMsg", &CChOP::m_MaxChatMsg)
		.def_readwrite("UpTime", &CChOP::m_UpTime)
		.def_readwrite("AntiSpam", &CChOP::m_AntiSpam)
		.def_readwrite("AntiYell", &CChOP::m_AntiYell)
		.def_readwrite("PhraseKick", &CChOP::m_PhraseKick)
		.def_readwrite("Phrases", &CChOP::m_Phrases)
		.def_readwrite("SlapPositive", &CChOP::m_SlapPositive)
		.def_readwrite("SlapNegative", &CChOP::m_SlapNegative)
		.def_readwrite("Ask8Ball", &CChOP::m_Ask8Ball)
		.def_readwrite("Quotes", &CChOP::m_Quotes)
		
		.add_property("ConsoleUser", make_getter(&CChOP::m_ConsoleUser, return_value_policy<reference_existing_object>()))
	;
}
