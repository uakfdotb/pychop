# author = uakf.b
# version = 1.0
# name = security
# fullname = plugins/pychop/security
# description = Bans users with '@' in their name and no bot access.
# help = See description. You are recommended to use plugin manager to enable and disable the security block.

# begin settings

# access above which users won't be considered for banning
minAccess = 1

# maximum size of ban queue
banQueue = 1000

# end settings

import host
import time
import random

banQueue = []
banBnet = 0 # bnet to ban on

def init():
	host.registerHandler('UserJoined', onJoin)
	host.registerHandler('UserLeft', onLeave)
	host.registerHandler('Update', onUpdate)
	
def deinit():
	host.unregisterHandler('UserJoined', onJoin)
	host.unregisterHandler('UserLeft', onLeave)
	host.unregisterHandler('Update', onUpdate)

def onJoin(bnet, user, isShow):
	global banBnet, banQueue
	banBnet = bnet

	if user.getAccess() < minAccess and user.getName().find("@") != -1:
		banQueue.append(user.getName().lower())
		print("Security: queue [" + user.getName() + "]")

def onLeave(bnet, user):
	global banQueue
	
	user = user.lower()

	if user in banQueue:
		banQueue.remove(user)
		print("Security: unqueue [" + user + "]")

def onUpdate(chop):
	global banBnet, banQueue

	if banBnet != 0 and banBnet.getOutPacketsQueued() <= 3 and len(banQueue) > 0:
		pos = random.randrange(len(banQueue))
		elem = banQueue[pos]
		banQueue[pos] = banQueue[-1]
		del banQueue[-1]

		banBnet.queueChatCommand("/ban " + elem)

def gettime():
        return int(round(time.time() * 1000))
