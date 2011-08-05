# author = uakf.b
# version = 1.0
# name = snipe
# fullname = plugins/pychop/snipe
# description = Kicks a user after they join the channel.
# help = Type !snipe <username> to add the user to the snipe list. Type !snipe <username> again to remove them.

# modify settings below

# commands to trigger on
commands = ["snipe", "plugins/pychop/snipe"]

# access needed to use snipe
snipeAccess = 10

# end settings

import host
import random

# list of users to be sniped
snipeList = []

def init():
	host.registerHandler('UserJoined', onJoin)
	host.registerHandler('ProcessCommand', onCommand)
	
def deinit():
	host.unregisterHandler('UserJoined', onJoin)
	host.unregisterHandler('ProcessCommand', onCommand)

def onJoin(bnet, user, isShow):
	if user.getName().lower() in snipeList:
		bnet.queueChatCommand("/kick " + user.getName() + " sniped")

def onCommand(bnet, user, command, payload, nType):
	if command in commands and user.getAccess() >= snipeAccess:
		targetName = payload.lower()
		
		if targetName in snipeList:
			snipeList.remove(targetName)
			if bnet.getOutPacketsQueued() < 5:
				bnet.queueChatCommand("snipe: removed user " + targetName)
			else:
				print("[SNIPE] Removed user " + targetName)
		else:
			snipeList.append(targetName)
			if bnet.getOutPacketsQueued() < 5:
				bnet.queueChatCommand("snipe: added user " + targetName)
			else:
				print("[SNIPE] Added user " + targetName)
