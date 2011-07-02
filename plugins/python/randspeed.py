# author = uakf.b
# version = 1.0
# name = randspeed
# fullname = plugins/pychop/randspeed
# description = Randomly displays letters every now and then (or on command). First player to respond wins!
# help = Use !randspeed now to display letters now, or !randspeed on and !randspeed off to display them every now and then. !randspeed score USERNAME to get USERNAME's score, !randspeed top to get top scores.

# modify settings below

# commands to trigger on
commands = ["randspeed", "plugins/pychop/randspeed"]

# number of random letters to print
stringLength = 8

# minimum interval to wait (milliseconds)
#  default is 5 seconds
minInterval = 5 * 1000

# maximum interval to wait (milliseconds)
#  default is two minutes
maxInterval = 20 * 1000

# access needed to run now, on, and off
controlAccess = 5

# possible characters in string
characterSet = "abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

# end settings

# next time to give a randspeed, or 0 if disabled
randTime = 0

# the current random string
randString = ""

# bnet to send rands to
randBnet = 0

# tuple with scores from plugindb
scoreTuple = 0

import host
import plugindb
import time
import random

def dbReady():
	global scoreTuple

	print("[RANDSPEED] Loading scores...")
	
	plugindb.dbconnect()
	scoreTuple = plugindb.dbGetScores("randspeed")
	
	print("[RANDSPEED] Found " + str(len(scoreTuple[0])) + " scores")

def init():
	global scores

	host.registerHandler('ChatReceived', onTalk)
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	plugindb.init()
	
	plugindb.notifyReady(dbReady)
	
def deinit():
	host.unregisterHandler(onTalk)
	host.unregisterHandler(onCommand)
	host.unregisterHandler(onUpdate)
	plugindb.deinit()
	del scores

def onTalk(bnet, username, message):
	global randString, scores, userids

	if randString != "" and message == randString:
		# update score
		plugindb.dbScoreAdd(scoreTuple, "randspeed", username.lower(), 1)
		
		# get new score
		newScore = plugindb.dbGetScore(scoreTuple, username.lower())
		
		bnet.queueChatCommand("randspeed: " + username + " got it! (points: " + str(newScore) + ")")
		randString = ""
		
		# if this came from a timer, reset timer
		if randTime > 0:
			setNext()

def onUpdate(chop):
	global randTime

	# make sure game is not already in progress and time is ripe
	if randString == "" and randTime > 0 and gettime() >= randTime:
		doRand()

def doRand():
	global randString
	
	randString = ""
	for i in range(stringLength):
		randIndex = random.randint(0, len(characterSet) - 1)
		randString += characterSet[randIndex]
	
	randBnet.queueChatCommand("Type: " + randString)

def setNext():
	global randTime
	
	rand = random.randint(minInterval, maxInterval)
	randTime = gettime() + rand

def onCommand(bnet, user, command, payload, nType):
	global randTime, randBnet
	randBnet = bnet

	whisper = nType == 1
	
	if command in commands:
		parts = payload.split(" ", 1)
		
		if user.getAccess() > controlAccess and parts[0] == "on":
			setNext()
		elif user.getAccess() > controlAccess and parts[0] == "off":
			randTime = 0
		elif user.getAccess() > controlAccess and parts[0] == "now":
			doRand()
		elif parts[0] == "top":
			bnet.queueChatCommand(plugindb.dbScoreTop(scoreTuple))
		elif parts[0] == "score":
			lowername = parts[1].lower()
			bnet.queueChatCommand(lowername + " points: " + plugindb.dbGetScore(scoreTuple, lowername))

def gettime():
	return int(round(time.time() * 1000))
