# author = uakf.b
# version = 1.0
# name = inactive
# fullname = plugins/pychop/inactive
# description = Identifies users who have been inactive for a certain period of time. Inactivity is based on presence in channel.
# help = Use !inactive <time in hours> to get a list of inactive users.
# config = access|Access needed to use the command.

commands = ("inactive")
commandAccess = 10

import host
import MySQLdb
from plugindb import PluginDB
import time

pdb = 0
	
def init():
	global pdb, commandAccess

	host.registerHandler('ProcessCommand', onCommand, True)
	
	pdb = PluginDB()
	pdb.dbconnect()
	
	# configuration
	config = host.config()
	commandAccess = config.getInt("p_inactive_access", commandAccess)
	
def deinit():
	host.unregisterHandler('ProcessCommand', onCommand, True)
	plugindb.deinit()

def onCommand(bnet, user, command, payload, nType):
	if command in commands and bnet.getOutPacketsQueued() < 3 and user.getAccess() >= commandAccess:
		# change hours to milliseconds
		timeMillis = int(payload) * 60 * 60 * 1000;
		# now identify gettime() at timeMillis in the past
		timeTarget = host.GetTicks() - timeMillis;
		
		pdb.execute("SELECT name, seen FROM users");
		result_set = pdb.getCursor().fetchall()
		result_string = "Inactive users: "
		
		# this list will be used for determining clan members who have never been seen
		user_set = []
		
		# first, display users who are in list but not active
		for row in result_set:
			rowUsername = str(row[0]).lower()
			user_set.append(rowUsername)
			
			rowSeen = 0
			if row[1] != None:
				rowSeen = int(row[1])
			
			if rowSeen < timeTarget and bnet.isClanMember(rowUsername):
				result_string += str(rowUsername) + ", "
		
		# now search for never seen clan members
		numClanMembers = bnet.getNumClanMembers()
		for i in range(numClanMembers):
			member = bnet.getClanMember(i)
			if not member.getName().lower() in user_set:
				result_string += str(member.getName()) + ", "
		
		# delete last two characters, don't care if there aren't any inactive users
		result_string = result_string[:-2]
		
		bnet.queueChatCommand(result_string, user.getName(), nType == 1)
		return False

	return True
