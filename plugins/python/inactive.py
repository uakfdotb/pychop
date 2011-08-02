# author = uakf.b
# version = 1.0
# name = inactive
# fullname = plugins/pychop/inactive
# description = Identifies users who have been inactive for a certain period of time. Inactivity is based on presence in channel.
# help = Use !inactive <time in hours> to get a list of inactive users.

commands = ("inactive")
commandAccess = 10

import host
import MySQLdb
from plugindb import PluginDB
import time

cursor = 0
pdb = 0

def dbReady():
	global cursor
	
	print("[INACTIVE] Connecting to database...")
	cursor = pdb.dbconnect()
	
def init():
	global pdb

	host.registerHandler('ProcessCommand', onCommand, True)
	
	pdb = PluginDB()
	pdb.notifyReady(dbReady)
	
def deinit():
	host.unregisterHandler('ProcessCommand', onCommand, True)
	plugindb.deinit()

def onCommand(bnet, user, command, payload, nType):
	if command in commands and bnet.getOutPacketsQueued() < 3 and user.getAccess() >= commandAccess:
		# change hours to milliseconds
		timeMillis = int(payload) * 60 * 60 * 1000;
		# now identify gettime() at timeMillis in the past
		timeTarget = host.GetTicks() - timeMillis;
		
		cursor.execute("SELECT name FROM users WHERE seen < " + str(timeTarget));
		result_set = cursor.fetchall()
		result_string = "Inactive users: "
		
		for row in result_set:
			rowUsername = str(row[0])
			if bnet.isClanMember(rowUsername):
				result_string += str(rowUsername) + ", "
		
		# delete last two characters, don't care if there aren't any inactive users
		result_string = result_string[:-2]
		
		bnet.queueChatCommand(result_string, user.getName(), nType == 1)
		return False

	return True
