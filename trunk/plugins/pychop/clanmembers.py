# author = uakf.b
# version = 1.0
# name = clanmembers
# fullname = plugins/pychop/clanmembers
# description = Prints a list of clan members to console and displays number of clan members.
# help = !clanmembers will print a list of clan members to console and display the number of clan members.

### begin configuration

# commands to trigger on
commands = ["clanmembers", "plugins/pychop/clanmembers"]

# access needed to use
commandAccess = 10

### end configuration

import host

def init():
	host.registerHandler('ProcessCommand', onCommand)
	
def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)

def onCommand(bnet, user, command, payload, nType):
	
	if command in commands and user.getAccess() >= commandAccess:
		# print clan members
		numClanMembers = bnet.getNumClanMembers()
		
		for i in range(numClanMembers):
			member = bnet.getClanMember(i)
			print('Name: ' + member.getName() + '; rank: ' + member.getRank() + '; status: ' + member.getStatus())
		
		bnet.queueChatCommand('# clan members: ' + str(numClanMembers))
