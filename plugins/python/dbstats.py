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

cursor = 0
pdb = 0

def dbReady():
	global cursor
	
	print("[DBSTATS] Connecting to database...")
	cursor = pdb.dbconnect()

def init():
	global pdb
	
	host.registerHandler('ProcessCommand', onCommand)
	
	pdb = PluginDB()
	pdb.notifyReady(dbReady)

def deinit():
	host.unregisterHandler(onCommand)

def onCommand(bnet, user, command, payload, nType):
	whisper = nType == 1
	
	if command in commands and bnet.getOutPacketsQueued() < 4:
		cursor.execute("SELECT COUNT(DISTINCT plugin) FROM plugindb")
		result = cursor.fetchone()
		numPlugins = result[0]
		
		cursor.execute("SELECT COUNT(*) FROM plugindb")
		result = cursor.fetchone()
		numPDB = result[0]
		
		cursor.execute("SELECT COUNT(*) FROM users")
		result = cursor.fetchone()
		numUsers = result[0]
		
		cursor.execute("SELECT COUNT(*) FROM admins")
		result = cursor.fetchone()
		numAdmins = result[0]
		
		cursor.execute("SELECT COUNT(*) FROM bans")
		result = cursor.fetchone()
		numBans = result[0]
		
		cursor.execute("SELECT COUNT(*) FROM users WHERE access>'1'")
		result = cursor.fetchone()
		numAccessUsers = result[0]
		
		bnet.queueChatCommand("#plugins: " + str(numPlugins) + "; #pdbrows: " + str(numPDB) + "; #users: " + str(numUsers) + "; #admins: " + str(numAdmins) + "; #bans: " + str(numBans) + "; #users with access: " + str(numAccessUsers), user.getName(), whisper)
