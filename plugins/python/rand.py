# author = uakf.b
# version = 1.0
# name = gettime
# fullname = plugins/pychop/gettime
# description = A plugin to generate random numbers.
# help = Use !rand to get a random number, !rand <x> to generate an integer up to <x>, or !rand <x> <y> to generate a random integer from <x> to <y>.

# string to output before random result
before_rand = "rand: "

# strings to identify this command
commands = ("plugins/rand", "rand")

### end configuration

import host
import random

def init():
	host.registerHandler('ProcessCommand', onCommand)

def deinit():
	host.unregisterHandler(onCommand)

def onCommand(bnet, user, command, payload, nType):
	whisper = nType == 1

	if command in commands and bnet.getOutPacketsQueued() < 3:
		args = payload.split(None)
		bnet.queueChatCommand(before_rand + rand(args), user.getName(), whisper)

def rand(args):
	if len(args) == 0:
		return str(random.random())
	elif len(args) == 1:
		try:
			return str(random.randint(0, int(args[0])))
		except ValueError:
			return "usage: !rand [integer max]"
	elif len(args) == 2:
		try:
			return str(random.randint(int(args[0]), int(args[1])))
		except ValueError:
			return "usage: !rand [integer min] [integer max]"
	else:
		return "error: too many arguments"
