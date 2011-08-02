# author = uakf.b
# version = 1.0
# name = votekick
# fullname = plugins/pychop/votekick
# description = Allows non-admins to votekick users from the channel.
# help = !votekick <username> to initiate a votekick on <username> or vote yes. !votekick <username> no to vote no. !vk can also be used in place of !votekick.

### begin configuration

commands = ("votekick", "plugins/pychop/votekick", "vk")

# minimum access to initiate a votekick
initAccess = 0

# maximum access to be affected by votekick
affectAccess = 10

# time a votekick lasts in seconds
votekickTime = 60

### end configuration

# dictionary with lowername victim -> [votecount, gettime,votedlist]
votekicks = {}

votekickBnet = 0

import host
import time

def init():
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	
def deinit():
	host.unregisterHandler(onCommand)
	host.unregisterHandler(onUpdate)

def onUpdate(chop):
	removeName = -1

	for lowername, value in votekicks.items():
		votetime = value[1]
		
		if gettime() - votetime > votekickTime * 1000:
			# expire the votekick
			removeName = lowername
			break
	
	if removeName != -1:
		# check if it passed or not
		votekickTuple = votekicks[removeName]
		if votekickTuple[0] >= 4:
			votekickBnet.queueChatCommand("/kick " + removeName + " votekick")
		else:
			votekickBnet.queueChatCommand("A votekick on [" + removeName + "] has expired.")
		del votekicks[removeName]

def gettime():
	return int(round(time.time() * 1000))

def onCommand(bnet, user, command, payload, nType):
	global votekickBnet
	votekickBnet = bnet
	
	if command in commands and payload != "":
		parts = payload.split(" ");
		
		if len(parts) <= 1 or parts[1] == "yes":
			victim = parts[0].lower()
			# init a votekick if needed
			if not victim in votekicks:
				if user.getAccess() >= initAccess:
					votekicks[victim] = [0,gettime(),[]]
				else:
					return
			
			votekickTuple = votekicks[victim]
			
			# make sure not already voted
			if user.getName().lower() in votekickTuple[2]:
				return
			
			votekickTuple[0] = votekickTuple[0] + 1
			votekickTuple[2].append(user.getName().lower())
			bnet.queueChatCommand("Votekick count on [" + victim + "] is now " + str(votekickTuple[0]))
		elif parts[1] == "no":
			victim = parts[2].lower()
			if victim in votekicks:
				votekickTuple = votekicks[victim]
				
				# make sure not already voted
				if user.getName().lower() in votekickTuple[2]:
					return
				
				votekickTuple[0] = votekickTuple[0] - 1
				votekickTuple[2].append(user.getName().lower())
				bnet.queueChatCommand("Votekick count on [" + victim + "] is now " + str(votekickTuple[0]))
