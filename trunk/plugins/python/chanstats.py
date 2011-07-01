# author = uakf.b
# version = 1.0
# name = chanstats
# fullname = plugins/pychop/chanstats
# description = Displays some statistics on the channel.
# help = Type !chanstats and see what happens.

# modify settings below

# commands to trigger on
commands = ["chanstats", "plugins/pychop/chanstats"]

# end settings

# list of users
userList = []

# number joins recorded
numJoins = 0

# number leaves recorded
numLeaves = 0

# number messages recorded
numMessages = 0

import host

def init():
	host.registerHandler('ChatReceived', onTalk)
	host.registerHandler('UserLeft', onLeave)
	host.registerHandler('UserJoined', onJoin)
	host.registerHandler('ProcessCommand', onCommand)
	
def deinit():
	host.unregisterHandler(onTalk)
	host.unregisterHandler(onLeave)
	host.unregisterHandler(onJoin)
	host.unregisterHandler(onCommand)

def onTalk(bnet, username, message):
	global numMessages
	numMessages = numMessages + 1

def onLeave(bnet, username):
	global numLeaves
	numLeaves = numLeaves + 1

def onJoin(bnet, user):
	global numJoins
	numJoins = numJoins + 1
	
	# see if we can add to userlist
	if not user.getName().lower() in userList:
		userList.append(user.getName().lower())

def onCommand(bnet, user, command, payload, nType):
	whisper = nType == 1
	
	if command in commands:
		message = "Unique users: " + str(len(userList));
		message += ";  num joins: " + str(numJoins);
		message += ";  num leaves: " + str(numLeaves);
		message += ";  num messages: " + str(numMessages);
		bnet.queueChatCommand(message, user.getName(), whisper)
