/*
   Copyright [2008]  Spoof.3D (Michael H�ferle) & jampe (Daniel Jampen)

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

#ifndef CONFIG_H
#define CONFIG_H

//
// CConfig
//

class CConfig
{
private:
	map<string, string> m_CFG;

public:
	CConfig( );
	~CConfig( );

	void Read( string file );
	bool Exists( string key );
	int GetInt( string key, int x );
	string GetString( string key, string x );
	void Set( string key, string x );
	bool GetBool( string key, bool x );
	
	static void RegisterPythonClass( );
};

#endif
