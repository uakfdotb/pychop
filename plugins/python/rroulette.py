# author = uakf.b
# version = 1.0
# name = rroulette
# fullname = plugins/pychop/rroulette
# description = A game that randomly kicks players from the channel. Hope that you do not get kicked!
# help = "!rroulette start" starts a game; first, there is a starting period during which players are identified. To enter the game, players type "!rroulette join". "!rroulette channel" during the starting period will add all channel members to the player list. After the starting period, the game starts and every 10 seconds one player is kicked from the channel. Last few players remaining win.

### begin configuration

# commands that rroulette will trigger on
commands = ("rroulette", "plugins/pychop/rroulette")

# access needed to control rroulette
rouletteAccess = 10

# milliseconds time for starting period
timeStarting = 45000

# milliseconds time in between kicks
timeKick = 10000

# number of winners that do not get kicked
numWinners = 1

### end cofiguration

# roulette state
# 0: disabled
# 1: starting period
# 2: in game (lastTime is reset without change in state until len(players) < numWinners
rouletteState = 0

# last event time
lastTime = 0

# list of players still in the game (kicked players are removed)
roulettePlayers = []

# bnet to use for chatting
rouletteBnet = 0

import host
import time
import random

def init():
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	
def deinit():
	host.unregisterHandler(onCommand)
	host.unregisterHandler(onUpdate)

def onUpdate(chop) :
	global lastTime, rouletteState
	
	if rouletteState != 0:
		if rouletteState == 1:
			if gettime() - lastTime > timeStarting:
				# begin the round
				rouletteBnet.queueChatCommand("rroulette: the round has started; a player will be kicked shortly...")
				
				lastTime = gettime()
				rouletteState = 2
		elif rouletteState == 2:
			if gettime() - lastTime > timeKick:
				# kick a random player
				if rouletteBnet.getOutPacketsQueued() < 5:
					playerIndex = random.randint(0, len(roulettePLayers) - 1)
					rouletteBnet.queueChatCommand("/kick " + roulettePlayers[playerIndex] + " rroulette")
					del roulettePlayers[playerIndex]
				
				# see if game is over
				if len(roulettePlayers) <= numWinners:
					rouletteBnet.queueChatCommand("The round is over; the remaining players win")
				
				# reset
				rouletteState = 0

def gettime():
	return int(round(time.time() * 1000))

def onCommand(bnet, user, command, payload, nType):
	global roulettePlayers, rouletteState, lastTime, rouletteBnet
	
	rouletteBnet = bnet
	
	if command in commands and (user.getAccess() >= rouletteAccess or bnet.getOutPacketsQueued() < 3):
		parts = payload.split(None);
		
		if user.getAccess() >= rouletteAccess:
			if parts[0] == "start":
				rouletteState = 1
				lastTime = gettime()
				bnet.queueChatCommand("rroulette: starting game; type \\rroulette join to join the game!")
			elif parts[0] == "channel" and rouletteState == 1:
				channelUsers = bnet.channel
				for cuser in channelUsers:
					if not cuser.getName().lower() in roulettePlayers:
						roulettePlayers.append(cuser.getName().lower())
						print("[RROULETTE] Added user " + cuser.getName())
		if parts[0] == "join" and rouletteState == 1:
			if not user.getName().lower() in roulettePlayers:
				roulettePlayers.append(user.getName().lower())
				print("[RROULETTE] Added user " + user.getName())
