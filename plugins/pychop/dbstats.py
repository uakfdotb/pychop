# author = uakf.b
# version = 1.0
# name = dbstats
# fullname = plugins/pychop/dbstats
# description = Displays some statistics about your MySQL database.
# help = Type !dbstats and see what happens.

# modify settings below

# commands to trigger on
commands = ["dbstats", "plugins/pychop/dbstats"]

# end settings

# list of users
userList = []

# number joins recorded
numJoins = 0

# number leaves recorded
numLeaves = 0

# number messages recorded
numMessages = 0

# number whispers received
numWhispers = 0

import host
import MySQLdb
from plugindb import PluginDB

pdb = 0

def init():
	global pdb
	
	host.registerHandler('ProcessCommand', onCommand)
	
	pdb = PluginDB()
	pdb.dbconnect()

def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)

def onCommand(bnet, user, command, payload, nType):
	whisper = nType == 1
	
	if command in commands and bnet.getOutPacketsQueued() < 4:
		pdb.execute("SELECT COUNT(DISTINCT plugin) FROM plugindb")
		result = pdb.getCursor().fetchone()
		numPlugins = result[0]
		
		pdb.execute("SELECT COUNT(*) FROM plugindb")
		result = pdb.getCursor().fetchone()
		numPDB = result[0]
		
		pdb.execute("SELECT COUNT(*) FROM users")
		result = pdb.getCursor().fetchone()
		numUsers = result[0]
		
		pdb.execute("SELECT COUNT(*) FROM admins")
		result = pdb.getCursor().fetchone()
		numAdmins = result[0]
		
		pdb.execute("SELECT COUNT(*) FROM bans")
		result = pdb.getCursor().fetchone()
		numBans = result[0]
		
		pdb.execute("SELECT COUNT(*) FROM users WHERE access>'1'")
		result = pdb.getCursor().fetchone()
		numAccessUsers = result[0]
		
		bnet.queueChatCommand("#plugins: " + str(numPlugins) + "; #pdbrows: " + str(numPDB) + "; #users: " + str(numUsers) + "; #admins: " + str(numAdmins) + "; #bans: " + str(numBans) + "; #users with access: " + str(numAccessUsers), user.getName(), whisper)
