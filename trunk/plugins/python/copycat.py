# author = uakf.b
# version = 1.0
# name = copycat
# fullname = plugins/pychop/copycat
# description = Copies everything that users say in the channel.
# help = !copycat to toggle copycat.

# modify settings below

# commands to trigger on
commands = ["copycat", "plugins/pychop/copycat"]

# minimum access to control
controlAccess = 10

# end settings

# whether or not copycat is enabled
copycatEnabled = False

import host

def init():
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('ChatReceivedExtended', onTalk) # extended to distinguish between local chat and whispers
	
def deinit():
	host.unregisterHandler(onCommand)
	host.unregisterHandler(onTalk)

def onTalk(bnet, username, message, isWhisper):
	if not isWhisper and copycatEnabled:
		bnet.queueChatCommand(message)

def onCommand(bnet, user, command, payload, nType):
	global copycatEnabled
	
	if command in commands and user.getAccess() >= controlAccess:
		copycatEnabled = not copycatEnabled