# author = uakf.b
# version = 1.0
# name = inviteme
# fullname = plugins/pychop/inviteme
# description = Allows users to invite themselves into a clan (must be shaman).
# help = Use !inviteme to invite yourself into the clan.

### begin configuration

# strings to identify this command
commands = ("plugins/pychop/inviteme", "inviteme")

# flood control - no usage within the last floodDelay ms to use
floodDelay = 10000

### end configuration

import host
import time

# last time used
lastInvite = 0

def init():
	host.registerHandler('ProcessCommand', onCommand)

def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)

def onCommand(bnet, user, command, payload, nType):
	global lastInvite
	
	if command in commands and gettime() - lastInvite > floodDelay:
		lastInvite = gettime()
		bnet.sendClanInvitation(user.getName())
		print("Invited " + user.getName() + " to the clan")

def gettime():
	return int(round(time.time() * 1000))
