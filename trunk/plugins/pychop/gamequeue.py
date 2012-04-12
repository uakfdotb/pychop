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

### end settings

# plugin db instance if gqGamelist is enabled
pdb = 0

# last time that we tried to host a game
lastTime = 0

# list of maps
mapList = []

# dictionary from username to loaded map
userMaps = {}

# bnet to use
gqBnet = 0

from collections import deque

# queue containing tuples (username, command, mapname, gamename)
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
	global mapList
	
	print("[GAMEQUEUE] Refreshing internal map list...")
	mapList = os.listdir(gqMapPath)

def onUpdate(chop):
	global lastTime

	if gettime() - lastTime > 3000 and hostQueue and gqBnet != 0:
		lastTime = gettime()

		# first thing is to check if we have a bot available
		# find all bots in channel
		potentialBots = dict(gqBots) # create a copy so we don't modify original
		channelUsers = gqBnet.getChannelNameList()
		
		for key in potentialBots.keys():
			if not potentialBots[key][0].lower() in channelUsers:
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
				mapname = firstEntry[2]
				gamename = firstEntry[3]

				# select a bot at random from the remaining list
				randIndex = random.choice(potentialBots.keys())
				botName = potentialBots[randIndex][0]
				botTrigger = potentialBots[randIndex][1]
				
				if command == "priv" and not gqGamelist:
					command = "pub"
				
				targetString = command + "by " + username + " " + gamename
				
				gqBnet.queueChatCommand("/w " + botName + " " + botTrigger + "map " + mapname)
				gqBnet.queueChatCommand("/w " + botName + " " + botTrigger + targetString)
				gqBnet.queueChatCommand("/w " + username + " Your game [" + gamename + "] should now be hosted on [" + botName + "]!")


def onCommand(bnet, user, command, payload, nType):
	global gqBnet
	gqBnet = bnet

	lowername = user.getName().lower()
	
	if user.getAccess() >= gqAccess:
		if command == "priv" or command == "pub":
			if lowername in userMaps.keys():
				mapname = userMaps[lowername]
				gamename = payload
				
				# make sure this user hasn't hosted already
				duplicate = False
				for entry in hostQueue:
					if entry[0] == lowername:
						duplicate = True
						break
				
				if not duplicate:
					hostQueue.append((lowername, command, mapname, gamename,))
				else:
					bnet.queueChatCommand("Error: you have a game in queue already; use !unhost to unqueue that game first")
			else:
				bnet.queueChatCommand("Error: you do not have any map file loaded!")
		elif command == "unhost":
			foundEntry = 0
			for entry in hostQueue:
				if entry[0] == lowername:
					foundEntry = entry
					break
			
			if foundEntry != 0:
				hostQueue.remove(foundEntry)
		elif command == "map":
			if payload != "":
				payload = payload.lower() # case insensitive search
				lastMatch = ""
				foundMatches = ""
				countMatches = 0
				
				for fname in mapList:
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
					bnet.queueChatCommand("No maps found with that name.")
				elif countMatches == 1:
					bnet.queueChatCommand("Loading map file [" + lastMatch + "].")
					userMaps[lowername] = lastMatch
				else:
					bnet.queueChatCommand("Maps found: " + foundMatches)
			else:
				if lowername in userMaps.keys():
					bnet.queueChatCommand("Your currently loaded map file is [" + userMaps[lowername] + "].")
				else:
					bnet.queueChatCommand("You currently do not have any map file loaded.")
		elif command == "gamequeue" and payload == "refresh":
			refreshMaps()
			bnet.queueChatCommand("Refreshed internal maps list")
		elif user.getAccess() == 10 and command == "gamequeue" and payload == "print":
			print("[GAMEQUEUE] Printing loaded maps")
			
			for fname in mapList:
				print("[GAMEQUEUE] " + fname)
		elif user.getAccess() == 10 and command == "gamequeue" and payload == "queue":
			print("[GAMEQUEUE] Printing queue")
			
			for entry in hostQueue:
				print("[GAMEQUEUE] " + hostQueue[0] + "  " + hostQueue[1] + "  " + hostQueue[2] + "  " + hostQueue[3])

def gettime():
	return int(round(time.time() * 1000))
