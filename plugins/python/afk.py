# author = uakf.b
# version = 1.0
# name = afk
# fullname = plugins/pychop/afk
# description = Stores an internal list of AFK users based on idle time and whether they said "afk". To be registered as AFK, a user only needs to say "afk" and nothing aftewards. To be unregistered, a user simply has to say something. Users who have not talked for a set number of minutes will also be registered as AFK.
# help = Use !afk print to print the list of AFK users to the console. Use !afk clear to clear the list of AFK users and reset the AFK times for all channel users.

# modify settings below

# time in milliseconds to wait until registering user as AFK
afkTime = 10 * 60 * 1000

# access needed to print AFK or reset AFK userlist
afkAccess = 10

# messages that will register a user as AFK (must be the entire message, not just a word, because a user might say "is he afk?" or something)
afkMessages = ["afk"]

# commands to trigger on
commands = ["afk", "plugins/pychop/afk"]

# whether or not to kick afk users
afkKick = False

# end settings

# dictionary maps username -> last user event (sent message, joined channel); last user event is -1 if this user has been registered as AFK
userAfkTime = {}

# lists AFK users
afkUserList = []

# next time a user might go AFK (minimum event time in userAfkTime)
nextTime = 0

# BNET to use for kicking
afkBnet = 0

import host
import time

def init():
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('ChatReceivedExtended', onTalk) # extended to distinguish between local chat and whispers
	host.registerHandler('Update', onUpdate)
	host.registerHandler('UserLeft', onLeave)
	host.registerHandler('UserJoined', onJoin)
	
def deinit():
	host.unregisterHandler(onCommand)
	host.unregisterHandler(onUpdate)
	host.unregisterHandler(onTalk)
	host.unregisterHandler(onLeave)
	host.unregisterHandler(onJoin)

def onTalk(bnet, username, message, isWhisper):
	global nextTime, userAfkTime, afkUserList

	if isWhisper:
		return

	lowername = username.lower()

	# first make sure message isn't in the afk list
	if message.lower() in afkMessages:
		# add them to afk user list if not already there
		if not lowername in afkUserList:
			afkUserList.append(lowername)
			userAfkTime[lowername] = gettime()
	else:
		if lowername in afkUserList:
			# clear this user from the afk list
			afkUserList.remove(lowername)
	
		# update their AFK time
		userAfkTime[lowername] = gettime()
	
		# reset nextTime if needed (should never be needed since this user is already in channel)
		if nextTime == 0:
			nextTime = gettime() + afkTime

def onLeave(bnet, username):
	global userAfkTime, afkUserList

	lowername = username.lower()
	
	if lowername in afkUserList:
		# this user is leaving, clear from afk list
		afkUserList.remove(lowername)
	
	# also clear from userAfkTime
	print("leaving user " + username)
	del userAfkTime[lowername]
	print("deleted?: " + str(lowername in userAfkTime))

def onJoin(bnet, user, isShow):
	global nextTime

	lowername = user.getName().lower()
	
	# update their AFK time
	userAfkTime[lowername] = gettime()
	
	# reset nextTime if needed (in the event that this is first user to enter channel)
	if nextTime == 0:
		nextTime = gettime() + afkTime

def onUpdate(chop) :
	global nextTime, userAfkTime, afkUserList
	
	if nextTime != 0 and gettime() >= nextTime:
		# first, go through dictionary and register users as AFK
		for user in userAfkTime.keys():
			time = userAfkTime[user]
			
			if time == -1:
				continue
			elif gettime() - time > afkTime:
				# this user is now afk...
				if afkKick:
					afkBnet.queueChatCommand("/kick " + user)
					del userAfkTime[user]
				else:
					userAfkTime[user] = -1
					afkUserList.append(user)
		
		# second, generate a new nexttime value
		nextTime = 0
		
		for time in userAfkTime.values():
			if time != -1 and (nextTime == 0 or time + afkTime < nextTime):
				nextTime = time + afkTime

def gettime():
	return int(round(time.time() * 1000))

def onCommand(bnet, user, command, payload, nType):
	global afkUserList, userAfkTime, afkBnet
	
	afkBnet = bnet
	
	if command in commands and user.getAccess() > afkAccess:
		if payload == "print":
			print("[AFK] Printing AFK times (-1 means AFK listed)...")
			
			for k,v in userAfkTime.iteritems():
				vDifference = gettime() - v

				if v == -1:
					vDifference = -1

				print("[AFK] " + str(k) + " has been afk for " + str(vDifference / 1000) + " seconds")
		elif payload == "clear":
			print("[AFK] Clearing AFK users and times...")
			
			afkUserList = []
			for key in userAfkTime:
				userAfkTime[key] = gettime()
