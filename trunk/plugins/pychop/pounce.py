# author = uakf.b
# version = 1.0
# name = pounce
# fullname = plugins/pychop/pounce
# description = Display text in channel as soon as user joins.
# help = Type !pounce <username> <message> to add a pounce. Type !pounce <username> to remove a pounce.
# config = access|Access needed to use the plugin

# modify settings below

# commands to trigger on
commands = ["pounce", "plugins/pychop/pounce"]

# access needed to use pounce
pounceAccess = 10

# end settings

import host
import random

# dictionary: user -> message
pounceList = {}

def init():
	global pounceAccess
	host.registerHandler('UserJoined', onJoin)
	host.registerHandler('ProcessCommand', onCommand)
	
	# configuration
	config = host.config()
	pounceAccess = config.getInt("p_pounce_access", pounceAccess)
	
def deinit():
	host.unregisterHandler('UserJoined', onJoin)
	host.unregisterHandler('ProcessCommand', onCommand)

def onJoin(bnet, user, isShow):
	if user.getName().lower() in pounceList:
		bnet.queueChatCommand(pounceList[user.getName().lower()])
		del pounceList[user.getName().lower()]

def onCommand(bnet, user, command, payload, nType):
	if command in commands and user.getAccess() >= pounceAccess:
		parts = payload.split(" ", 1)
		targetName = parts[0].lower()
		
		message = -1
		if len(parts) >= 2:
			message = parts[1]
		
		if targetName in pounceList.iterkeys():
			del pounceList[targetName]
			if bnet.getOutPacketsQueued() < 5:
				bnet.queueChatCommand("pounce: removed user " + targetName)
			else:
				print("[POUNCE] Removed user " + targetName)
		else:
			if message != -1:
				pounceList[targetName] = message
				if bnet.getOutPacketsQueued() < 5:
					bnet.queueChatCommand("pounce: added user " + targetName)
				else:
					print("[POUNCE] Added user " + targetName)
