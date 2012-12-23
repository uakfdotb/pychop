# author = uakf.b
# version = 1.0
# name = copycat
# fullname = plugins/pychop/copycat
# description = Copies everything that users say in the channel.
# help = !copycat to toggle copycat.
# config = access|Access needed to control plugin

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
	global controlAccess
	
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('ChatReceivedExtended', onTalk) # extended to distinguish between local chat and whispers
	
	# configuration
	config = host.config()
	controlAccess = config.getInt("p_copycat_access", controlAccess)
	
def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)
	host.unregisterHandler('ChatReceivedExtended', onTalk)

def onTalk(bnet, username, message, isWhisper):
	if not isWhisper and copycatEnabled:
		bnet.queueChatCommand(message)

def onCommand(bnet, user, command, payload, nType):
	global copycatEnabled
	
	if command in commands and user.getAccess() >= controlAccess and bnet.getOutPacketsQueued() < 5:
		copycatEnabled = not copycatEnabled
