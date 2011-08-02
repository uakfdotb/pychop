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

# number whispers received
numWhispers = 0

import host

def init():
	host.registerHandler('ChatReceivedExtended', onTalk)
	host.registerHandler('UserLeft', onLeave)
	host.registerHandler('UserJoined', onJoin)
	host.registerHandler('ProcessCommand', onCommand)
	
def deinit():
	host.unregisterHandler('ChatReceivedExtended', onTalk)
	host.unregisterHandler('UserLeft', onLeave)
	host.unregisterHandler('UserJoined', onJoin)
	host.unregisterHandler('ProcessCommand', onCommand)

def onTalk(bnet, username, message, isWhisper):
	global numMessages, numWhispers
	
	if isWhisper:
		numWhispers = numWhispers + 1
	else:
		numMessages = numMessages + 1

def onLeave(bnet, username):
	global numLeaves
	numLeaves = numLeaves + 1

def onJoin(bnet, user, isShow):
	global numJoins
	numJoins = numJoins + 1
	
	# see if we can add to userlist
	if not user.getName().lower() in userList:
		userList.append(user.getName().lower())

def onCommand(bnet, user, command, payload, nType):
	whisper = nType == 1
	
	if command in commands and bnet.getOutPacketsQueued() < 3:
		message = "Unique users: " + str(len(userList));
		message += ";  num joins: " + str(numJoins);
		message += ";  num leaves: " + str(numLeaves);
		message += ";  num messages: " + str(numMessages);
		message += ";  num whispers: " + str(numWhispers);
		bnet.queueChatCommand(message, user.getName(), whisper)
