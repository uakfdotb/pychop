# author = uakf.b
# version = 1.0
# name = greet
# fullname = plugins/pychop/greet
# description = Greets users who join the channel.
# help = Rejoin the channel! Also, use !greet <message> to set the greet message. Use !greet to disable greeting.
# config = access|Access needed to set the greet message, minaccess|Minimum access needed to be greeted, maxaccess|Maximum access needed to be greeted

# modify settings below

greetMessage = "Welcome!"

# minimum user access that will be greeted
minAccess = 0

# maximum user access will be greeted (if greet is for help information, admins probably don't need it)
maxAccess = 10

# access needed to set the greet message
greetAccess = 10

# end settings

import host
import time

def init():
	global minAccess, maxAccess, greetAccess
	
	host.registerHandler('UserJoined', onJoin)
	host.registerHandler('ProcessCommand', onCommand)
	
	# configuration
	config = host.config()
	greetAccess = config.getInt("p_greet_access", greetAccess)
	minAccess = config.getInt("p_greet_minaccess", minAccess)
	maxAccess = config.getInt("p_greet_maxaccess", maxAccess)
	
def deinit():
	host.unregisterHandler('UserJoined', onJoin)
	host.unregisterHandler('ProcessCommand', onCommand)

def onJoin(bnet, user, isShow):
	# need correct access; also only show this when a user actually joins, not just when we join
	if user.getAccess() >= minAccess and user.getAccess() <= maxAccess and not isShow and greetMessage != "" and bnet.getOutPacketsQueued() < 3:
		personalMessage = greetMessage
		personalMessage = personalMessage.replace("#n", user.getName())
		personalMessage = personalMessage.replace("#t", time.asctime(time.localtime(time.time())));
		personalMessage = personalMessage.replace("#d", time.strftime("%B %d, %Y", time.localtime(time.time())));
		bnet.queueChatCommand(personalMessage, user.getName(), True)

def onCommand(bnet, user, command, payload, nType):
	global greetMessage

	if command == "greet" and user.getAccess() > greetAccess:
		greetMessage = payload
