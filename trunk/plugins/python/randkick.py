# author = uakf.b
# version = 1.0
# name = randkick
# fullname = plugins/pychop/randkick
# description = Randomly kicks users from the channel when the !randkick command is used. Anyone in the channel is at risk!
# help = Type !randkick, and a random person will be kicked.

# modify settings below

# commands to trigger on
commands = ["randkick", "plugins/pychop/randkick"]

# end settings

import host
import random

# list of users in channel
channelList = []

def init():
	host.registerHandler('UserJoined', onJoin)
	host.registerHandler('UserLeft', onLeave)
	host.registerHandler('ProcessCommand', onCommand)
	
def deinit():
	host.unregisterHandler(onJoin)
	host.unregisterHandler(onLeft)
	host.unregisterHandler(onCommand)

def onJoin(bnet, user, isShow):
	if not user.getName() in channelList:
		channelList.append(user.getName())

def onLeave(bnet, username):
	if user.getName() in channelList:
		channelList.remove(user.getName())

def onCommand(bnet, user, command, payload, nType):

	if command in commands and bnet.getOutPacketsQueued() < 5:
		# select a random user
		randIndex = random.randint(0, len(channelList) - 1)
		randUser = channelList[randIndex]
		# kick
		bnet.queueChatCommand("/kick " + str(randUser) + " randkick")