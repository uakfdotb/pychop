# author = uakf.b
# version = 1.0
# name = getgames
# fullname = plugins/pychop/getgames
# description = An example python plugin to retrieve a list of games.
# help = Use !getgames to get the list of games (no access required).

commands = ("getgames", "plugins/pychop/getgames")

import host
import MySQLdb

conn = MySQLdb.connect(host = "localhost", user = "user", passwd = "pass", db = "ghost")
cursor = conn.cursor()

def init():
	host.registerHandler('ProcessCommand', onCommand)
	
def deinit():
	host.unregisterHandler(onCommand)

def onCommand(bnet, user, command, payload, nType):
	if command in commands:
		cursor.execute("SELECT gamename, slotstaken, slotstotal FROM gamelist");
		result_set = cursor.fetchall()
		result_string = "Current games: "
		
		for row in result_set:
			if row[0] != "":
				result_string += row[0] + " (" + row[1] + "/" + row[2] + "), "
		
		if len(result_string) > 0:
			result_string = result_string[:-2]
		else:
			result_string += "none"
		
		bnet.queueChatCommand(result_string, user.getName(), nType == 1)
