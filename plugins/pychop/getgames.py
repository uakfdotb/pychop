# author = uakf.b
# version = 1.0
# name = getgames
# fullname = plugins/pychop/getgames
# description = An example python plugin to retrieve a list of games.
# help = Use !getgames to get the list of games (no access required).

commands = ("getgames", "plugins/pychop/getgames")

import host
import MySQLdb
from plugindb import PluginDB

pdb = 0
	
def init():
	global pdb

	host.registerHandler('ProcessCommand', onCommand, True)
	
	pdb = PluginDB()
	print("[GETGAMES] Connecting to database...")
	pdb.dbconnect()
	
def deinit():
	host.unregisterHandler('ProcessCommand', onCommand, True)

def onCommand(bnet, user, command, payload, nType):
	if command in commands and bnet.getOutPacketsQueued() < 4:
		pdb.execute("SELECT gamename, slotstaken, slotstotal FROM gamelist");
		result_set = pdb.getCursor().fetchall()
		result_string = "Current games: "
		
		num_games = 0
		
		for row in result_set:
			if row[0] != "":
				if payload == "" or payload.lower() in str(row[0]).lower():
					result_string += str(row[0]) + " (" + str(row[1]) + "/" + str(row[2]) + "), "
					num_games += 1
		
		if num_games > 0:
			result_string = result_string[:-2]
		else:
			result_string += "none"
		
		bnet.queueChatCommand(result_string, user.getName(), nType == 1)
		return False
		
	return True
