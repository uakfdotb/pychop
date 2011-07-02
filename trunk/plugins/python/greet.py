# author = uakf.b
# version = 1.0
# name = greet
# fullname = plugins/pychop/greet
# description = Greets users who join the channel.
# help = Rejoin the channel!

# modify settings below

greetMessage = "Welcome!"

# minimum user access that will be greeted
minAccess = 0

# maximum user access will be greeted (if greet is for help information, admins probably don't need it)
maxAccess = 10

# end settings

import host

def init():
	host.registerHandler('UserJoined', onJoin)
	
def deinit():
	host.unregisterHandler(onJoin)

def onJoin(bnet, user, isShow):
	# need correct access; also only show this when a user actually joins, not just when we join
	if user.getAccess() >= minAccess and user.getAccess() <= maxAccess and not isShow:
		bnet.queueChatCommand(greetMessage, user.getName(), True)
