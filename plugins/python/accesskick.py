# author = uakf.b
# version = 1.0
# name = accesskick
# fullname = plugins/pychop/accesskick
# description = Automatically kicks users from the channel who do not have a certain access level on the bot.
# help = See description. Use !accesskick to toggle enabled.

# modify settings below

# minimum access to not be automatically kicked
minAccess = 1

# whether to enable by default
accessEnabled = False

# whether to ban instead of kick
accessBan = False

# end settings

import host

def init():
	host.registerHandler('UserJoined', onJoin)
	host.registerHandler('ProcessCommand', onCommand)
	
def deinit():
	host.unregisterHandler('UserJoined', onJoin)
	host.unregisterHandler('ProcessCommand', onCommand)

def onJoin(bnet, user, isShow):
	if user.getAccess() < minAccess and accessEnabled and bnet.getOutPacketsQueued() < 5:
		if not accessBan:
			bnet.queueChatCommand("/kick " + user.getName())
		else:
			bnet.queueChatCommand("/ban " + user.getName())

def onCommand(bnet, user, command, payload, nType):
	global accessEnabled

	if command == "accesskick":
		accessEnabled = not accessEnabled
		
		if accessEnabled:
			print("accesskick is now enabled!")
		else:
			print("accesskick has been disabled!")
