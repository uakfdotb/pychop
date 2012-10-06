# author = uakf.b
# version = 1.0
# name = announce
# fullname = plugins/pychop/announce
# description = Announces a message at a set time interval.
# help = Use !announce <x> <string> to say <string> every <x> seconds or !announce to disable announce message.

### begin configuration

commands = ("announce", "plugins/pychop/announce")

# minimum access to control announce
announceAccess = 10

### end configuration

announceEnabled = False
announceMessage = ""
announceInterval = 1
announceBnet = 0
announceTime = 0

import host
import time

def init():
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	
def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)
	host.unregisterHandler('Update', onUpdate)

def onUpdate(chop) :
	global announceTime
	
	if announceEnabled and gettime() - announceTime > announceInterval * 1000 and announceBnet != 0:
		# announce
		announceBnet.queueChatCommand(announceMessage)
		announceTime = gettime()

def gettime():
	return int(round(time.time() * 1000))

def onCommand(bnet, user, command, payload, nType):
	global announceEnabled, announceMessage, announceInterval, announceBnet
	announceBnet = bnet
	
	if command in commands and user.getAccess() >= announceAccess:
		parts = payload.split(" ", 1);
		
		if len(parts) < 2:
			announceEnabled = False
			bnet.queueChatCommand("Announce message disabled")
		else:
			announceInterval = int(parts[0])
			announceMessage = parts[1]
			announceEnabled = True
