# author = uakf.b
# version = 1.0
# name = accesskick
# fullname = plugins/pychop/accesskick
# description = Automatically kicks users from the channel who do not have a certain access level on the bot.
# help = See description.

# modify settings below

# minimum access to not be automatically kicked
minAccess = 1

# end settings

import host

def init():
	host.registerHandler('UserJoined', onJoin)
	
def deinit():
	host.unregisterHandler(onJoin)

def onJoin(bnet, user, isShow):
	global nextTime

	if user.getAccess() < minAccess:
		bnet.queueChatCommand("/kick " + user.getName())
