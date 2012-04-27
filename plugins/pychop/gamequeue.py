# author = uakf.b
# version = 1.0
# name = gamequeue
# fullname = plugins/pychop/gamequeue
# description = If you have multiple GHost instances, this is great.

### modify settings below

# dictionary mapping from botid to (username, trigger)
gqBots = {}
gqBots[1] = ('yourfirstbot', '!',)
gqBots[2] = ('yoursecondbot', '-',)

# access needed to create games
gqAccess = 0

# whether or not to check gamelist patch (if false, !priv will be disabled)
gqGamelist = True

# path to maps
gqMapPath = "/home/ghost/maps"

# path to map configuration files
gqCfgPath = "/home/ghost/mapcfgs"

### end settings

# plugin db instance if gqGamelist is enabled
pdb = 0

# last time that we tried to host a game
lastTime = 0

# list of maps
mapList = []

# list of map configs
cfgList = []

# dictionary from username to tuple ("map" or "cfg", loaded map or cfg name)
userMaps = {}

# bnet to use
gqBnet = 0

# the last time that each bot was used
gqBotTime = {}

from collections import deque

# queue containing tuples (username, command, maptype (load or map), mapname, gamename)
hostQueue = deque()

import host
import MySQLdb
import random
import os
import time
from plugindb import PluginDB

def init():
	global pdb
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)

	if gqGamelist:
		pdb = PluginDB()
		pdb.dbconnect()
	
	refreshMaps()

def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)
	host.unregisterHandler('Update', onUpdate)

def refreshMaps():
	global mapList, cfgList
	
	print("[GAMEQUEUE] Refreshing internal map list...")
	mapList = os.listdir(gqMapPath)
	cfgList = os.listdir(gqCfgPath)

def onUpdate(chop):
	global lastTime, gqBotTime

	if gettime() - lastTime > 3000 and hostQueue and gqBnet != 0:
		lastTime = gettime()

		# first thing is to check if we have a bot available
		# find all bots in channel
		potentialBots = dict(gqBots) # create a copy so we don't modify original
		channelUsers = gqBnet.getChannelNameList()
		
		for key in potentialBots.keys():
			if not potentialBots[key][0].lower() in channelUsers:
				del potentialBots[key]
		
		# remove bots that have been used too recently
		for key in potentialBots.keys():
			if gettime() - gqBotTime.get(key, 0) < 10000:
				del potentialBots[key]
		
		if gqGamelist:
			# now, delete remaining bots that have a game in gamelist
			pdb.execute("SELECT gamename, botid FROM gamelist");
			
			result_set = pdb.getCursor().fetchall()
			
			for row in result_set:
				botid = int(row[1])
				
				if row[0] != "" and botid in potentialBots.keys():
					del potentialBots[botid]
			
		if len(potentialBots) > 0:
			firstEntry = hostQueue.popleft()
			username = firstEntry[0]
			command = firstEntry[1] # either pub or priv
			maptype = firstEntry[2]
			mapname = firstEntry[3]
			gamename = firstEntry[4]
			
			if gqGamelist:
				# make sure user doesn't already have game
				pdb.execute("SELECT COUNT(*) FROM gamelist WHERE ownername = %s OR creatorname = %s", (username.lower(), username.lower()))
				row = pdb.getCursor().fetchone()
				
				if row[0] > 0:
					gqBnet.queueChatCommand("/w " + username + " You already have a game in lobby!")
					return

			# select a bot at random from the remaining list
			randIndex = random.choice(potentialBots.keys())
			botName = potentialBots[randIndex][0]
			botTrigger = potentialBots[randIndex][1]
			
			# update the time that this bot was used
			gqBotTime[randIndex] = gettime()
			
			if command == "priv" and not gqGamelist:
				command = "pub"
			
			targetString = command + "by " + username + " " + gamename
			
			gqBnet.queueChatCommand("/w " + botName + " " + botTrigger + maptype + " " + mapname) # !load or !map the map
			gqBnet.queueChatCommand("/w " + botName + " " + botTrigger + targetString)
			gqBnet.queueChatCommand("/w " + username + " Your game [" + gamename + "] should now be hosted on [" + botName + "]!")


def onCommand(bnet, user, command, payload, nType):
	global gqBnet
	gqBnet = bnet
	whisper = nType == 1

	lowername = user.getName().lower()
	
	if user.getAccess() >= gqAccess:
		if command == "priv" or command == "pub":
			if lowername in userMaps.keys():
				mapinfo = userMaps[lowername]
				gamename = payload
				
				# make sure this user hasn't hosted already
				duplicate = False
				for entry in hostQueue:
					if entry[0] == lowername:
						duplicate = True
						break
				
				if not duplicate:
					hostQueue.append((lowername, command, mapinfo[0], mapinfo[1], gamename,))
					bnet.queueChatCommand("Your game has been queued (your position: " + str(len(hostQueue)) + ")", user.getName(), whisper)
				else:
					bnet.queueChatCommand("Error: you have a game in queue already; use !unhost to unqueue that game first", user.getName(), whisper)
			else:
				bnet.queueChatCommand("Error: you do not have any map file loaded!", user.getName(), whisper)
		elif command == "unhost":
			foundEntry = 0
			for entry in hostQueue:
				if entry[0] == lowername:
					foundEntry = entry
					break
			
			if foundEntry != 0:
				hostQueue.remove(foundEntry)
		elif command == "map" or command == "load":
			if payload != "":
				payload = payload.lower() # case insensitive search
				lastMatch = ""
				foundMatches = ""
				countMatches = 0
				
				targetList = mapList
				if command == "load":
					targetList = cfgList
				
				for fname in targetList:
					fname_lower = fname.lower()
					
					# extract stem for exact stem match
					if fname.find(".") != -1:
						stem = fname_lower.rsplit(".", 1)[1]
						
						if payload == fname_lower or payload == stem:
							countMatches = 1
							lastMatch = fname
							break # stop iterating if we have an exact match
						elif payload in fname_lower:
							countMatches = countMatches + 1
							lastMatch = fname
							
							if foundMatches == "":
								foundMatches = fname
							else:
								foundMatches += ", " + fname
				
				if countMatches == 0:
					if command == "map":
						bnet.queueChatCommand("No maps found with that name.", user.getName(), whisper)
					else:
						bnet.queueChatCommand("No map configuration found with that name. Use !map for normal map files.", user.getName(), whisper)
				elif countMatches == 1:
					bnet.queueChatCommand("Loading map file [" + lastMatch + "].", user.getName(), whisper)
					userMaps[lowername] = (command, lastMatch,)
				else:
					bnet.queueChatCommand("Maps found: " + foundMatches, user.getName(), whisper)
			else:
				if lowername in userMaps.keys():
					bnet.queueChatCommand("Your currently loaded map file is [" + userMaps[lowername][0] + "].", user.getName(), whisper)
				else:
					bnet.queueChatCommand("You currently do not have any map file loaded.", user.getName(), whisper)
		elif command == "gamequeue" and payload == "refresh":
			refreshMaps()
			bnet.queueChatCommand("Refreshed internal maps list", user.getName(), whisper)
		elif user.getAccess() == 10 and command == "gamequeue" and payload == "print":
			print("[GAMEQUEUE] Printing loaded maps")
			for fname in mapList:
				print("[GAMEQUEUE] " + fname)
			
			print("[GAMEQUEUE] Printing loaded map configs")
			for fname in cfgList:
				print("[GAMEQUEUE] " + fname)
		elif user.getAccess() == 10 and command == "gamequeue" and payload == "queue":
			print("[GAMEQUEUE] Printing queue")
			
			for entry in hostQueue:
				print("[GAMEQUEUE] " + hostQueue[0] + "  " + hostQueue[1] + "  " + hostQueue[2] + "  " + hostQueue[3] + "  " + hostQueue[4])

def gettime():
	return int(round(time.time() * 1000))
