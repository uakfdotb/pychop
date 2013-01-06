# author = uakf.b
# version = 1.0
# name = announce
# fullname = plugins/pychop/announce
# description = Announces a message at a set time interval.
# help = Use !announce <x> <string> to say <string> every <x> seconds or !announce to disable announce message.
# config = access|Access needed to control plugin

### begin configuration

commands = ("announce", "plugins/pychop/announce")

# minimum access to control announce
announceAccess = 10

### end configuration

announceEnabled = False
announceMessage = ""
announceInterval = 60
announceBnet = 0
announceTime = 0

import host
import time

def init():
	global announceAccess, announceMessage, announceEnabled, announceInterval
	
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	
	# configuration
	config = host.config()
	announceAccess = config.getInt("p_announce_access", announceAccess)
	
	announceMessage = config.getString("p_announce_message", announceMessage)
	
	if announceMessage:
		announceEnabled = True
		announceInterval = config.getInt("p_announce_interval", announceInterval)
	
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
