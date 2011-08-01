# author = uakf.b
# version = 1.0
# name = matchmake
# fullname = plugins/pychop/matchmake
# description = A plugin to randomly matchmake.
# help = "!matchmake start <number team 1> <number team 2>" or "!matchmake startn <numteams> <number team _1> <number team _2> ... <number team _numteams>" to start match making. Then, "!matchmake in" will include a player in the matchmaking (assuming they have sufficient access). "!matchmake clear" will stop matchmaking.

### begin configuration

# strings to identify this command
commands = ("plugins/pychop/matchmake", "matchmake")

# maximum amount of teams to allow
maxteams = 4

# maximum amount of players per team to allow
# default is 11 is 11v1 is maximum possible: 12 player game does not need matchmaking
maxplayers = 11

# minimum access to control
controlAccess = 10

# minimum access to join matchmaking
joinAccess = 0

### end configuration

matchmakingEnabled = False
# list of players; permuted and then outputted based on team numbers
matchmakingList = []

# number of players to wait for
matchmakingPlayers = 0

matchmakingTeams = 0
# team id -> number players per team
matchmakingTeamList = []

import host
import random

def init():
	host.registerHandler('ProcessCommand', onCommand)

def deinit():
	host.unregisterHandler(onCommand)

def onCommand(bnet, user, command, payload, nType):
	global matchmakingEnabled, matchmakingList, matchmakingTeams, matchmakingTeamList

	if command in commands and user.getAccess() >= joinAccess:
		args = payload.split(None)
		if (args[0] == "start" or args[0]=="startt") and user.getAccess() >= controlAccess:
			if not matchmakingEnabled:
				del matchmakingList[:]
				del matchmakingTeamList[:]
				
				if args[0] == "start":
					matchmakingTeams = 2
					matchmakingTeamList.append(int(args[1]))
					matchmakingTeamList.append(int(args[2]))
				else:
					matchmakingTeams = int(args[1])
					
					for i in range(matchmakingTeams):
						matchmakingTeamList.append(args[i + 2])
				
				matchmakingPlayers = checkTeams()
				if not matchmakingPlayers:
					bnet.queueChatCommand("Matchmaking failed: too many players on a team or too many teams.")
				else:
					matchmakingEnabled = True
					bnet.queueChatCommand("Matchmaking started with " + str(matchmakingTeams) + " teams and " + str(matchmakingPlayers) + " players.")
		elif args[0] == "in":
			if matchmakingEnabled and not user.getName().lower() in matchmakingList:
				matchmakingList.append(user.getName().lower())
				
				# check if we have the number needed
				if len(matchmakingList) >= matchmakingPlayers:
					random.shuffle(matchmakingList)
					chatString = "T1: "
					teamCounter = 0
					teamPlayers = 0
					
					for name in matchmakingList:
						chatString += name + " "
						
						teamPlayers = teamPlayers + 1;
						# check if we have filled this team
						if teamPlayers >= matchmakingTeamList[teamCounter]:
							teamCounter = teamCounter + 1
							chatString += "  T2: "
					
					bnet.queueChatCommand(chatString)
					# reset
					matchmakingEnabled = False
				else:
					bnet.queueChatCommand("Matchmaking needs " + str(matchmakingPlayers - len(matchmakingList)) + " more players.")
		elif args[0] == "clear":
			matchmakingEnabled = False
			bnet.queueChatCommand("Matchmaking cleared")

def checkTeams():
	if len(matchmakingTeamList) > maxteams and len(matchmakingTeamList) > 1:
		return False
	
	numplayers = 0
	
	for num in matchmakingTeamList:
		if num > maxplayers:
			return False
		else:
			numplayers = numplayers + num
	
	return numplayers
