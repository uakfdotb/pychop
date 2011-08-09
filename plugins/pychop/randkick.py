# author = uakf.b
# version = 1.0
# name = randkick
# fullname = plugins/pychop/randkick
# description = Randomly kicks users from the channel when the !randkick command is used. Anyone in the channel is at risk!
# help = Type !randkick, and a random person will be kicked.

# modify settings below

# commands to trigger on
commands = ["randkick", "plugins/pychop/randkick"]

# access needed to use randkick
randkickAccess = 10

# end settings

import host
import random

def init():
	host.registerHandler('ProcessCommand', onCommand)
	
def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)

def onCommand(bnet, user, command, payload, nType):
	if command in commands and bnet.getOutPacketsQueued() < 5 and user.getAccess() >= randkickAccess:
		# select a random user
		randIndex = random.randint(0, len(bnet.channel) - 1)
		randUser = bnet.channel[randIndex]
		# kick
		bnet.queueChatCommand("/kick " + str(randUser) + " randkick")
