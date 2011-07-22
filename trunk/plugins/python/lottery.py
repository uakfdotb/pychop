# author = uakf.b
# version = 1.0
# name = lottery
# fullname = plugins/pychop/lottery
# description = A lottery game where channel users can bet on a number.
# help = !lottery start and !lottery stop will start or stop the lottery. After lottery is started, after a delay of ten seconds the first round will begin. When that round ends, the next round will begin in ten seconds. Each round lasts five minutes and begins with an announcement that the round has started with the round parameters. !lottery bet <x> <y> will bet <y> coins on <x>. !lottery in will give you 10 coins if you have 0. A message containing the winner (if there is one) will be displayed after a round ends.

### begin configuration

# commands that lottery will trigger on
commands = ("lottery", "plugins/pychop/lottery")

# access needed to control lottery (start/stop)
lotteryAccess = 10

# seconds before a round starts
lotteryDelay = 10

# seconds a round lastts
roundTime = 5 * 60

# minimum number for lottery
lotteryMin = 0

# maximum number for lottery
lotteryMax = 100

# whether to enable lottery by default
lotteryEnabled = False

### end cofiguration

# lottery state
# 0: delaying
# 1: in a round
lotteryState = 0

lastTime = 0

# dictionary playername -> (guess,betamount,)
lotteryPlayers = {}

# PluginDB instance
pdb = 0

# bnet to use for chatting
lotteryBnet = 0

# the number that they're guessing
lotterySecret = 0

import host
import time
import random
from plugindb import PluginDB

def dbReady():
	print("[LOTTERY] Loading scores...")
	
	pdb.dbconnect()
	pdb.dbGetScores()
	
	print("[LOTTERY] Found " + str(pdb.dbScoreNum()) + " scores")

def init():
	global pdb

	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	
	pdb = PluginDB()
	pdb.notifyReady(dbReady)
	pdb.setPluginName("lottery")
	
def deinit():
	host.unregisterHandler(onCommand)
	host.unregisterHandler(onUpdate)
	pdb.close()

def onUpdate(chop) :
	global lotteryState, lotterySecret, lotteryTime
	
	if lotteryEnabled:
		if lotteryState == 0:
			if gettime() - lastTime > lotteryDelay:
				# start a round
				lotteryPlayers.clear()
				lotterySecret = random.randint(lotteryMin, lotteryMax)
				
				lotteryBnet.queueChatCommand("Lottery: a round has started; secret is between " + str(lotteryMin) + " and " + str(lotteryMax))
				
				lotteryTime = gettime()
				lotteryState = 1
		elif lotteryState == 1:
			if gettime() - lastTime > roundTime:
				# see if anyone won
				winnerList = []
				
				for name,playerTuple in lotteryPlayers:
					guess = playerTuple[0]
					bet = playerTuple[1]
					if guess == lotterySecret:
						winnerList.append(name)
						# name is already lowercase; add 4*bet to repay him and give him winner's pot (3*bet)
						pdb.dbScoreAdd(name, 4 * bet)
				
				displayText = "Lottery: secret was " + str(lotterySecret) + ". "
				if len(winnerList) == 0:
					displayText += "There were no winners."
				else:
					displayText += "Winners:"
					for name in winnerList:
						displayText += " " + name
				
				lotteryBnet.queueChatCommand(displayText)
				
				# reset
				lotteryTime = gettime()
				lotteryState = 0

def gettime():
	return int(round(time.time() * 1000))

def onCommand(bnet, user, command, payload, nType):
	global lotteryState, lotteryEnabled, lotteryTime, lotteryBnet, lotteryPlayers
	
	lotteryBnet = bnet
	
	if command in commands and (user.getAccess() >= 5 or bnet.getOutPacketsQueued() < 3):
		parts = payload.split(None);
		
		if user.getAccess() >= lotteryAccess:
			if parts[0] == "start":
				lotteryEnabled = True
				lotteryState = 0
				lotteryTime = gettime()
			elif parts[0] == "stop":
				lotteryEnabled = False
		if parts[0] == "top":
			# display top 5
			bnet.queueChatCommand(pdb.dbScoreTopStr(5))
		elif parts[0] == "coins":
			lowername = user.getName().lower()
			
			if len(parts) == 2:
				lowername = parts[1].lower()

			bnet.queueChatCommand(lowername + " coins: " + str(pdb.dbGetScore(lowername)))
		elif parts[0] == "in":
			lowername = user.getName().lower()
			
			currentScore = pdb.dbGetScore(lowername)
			if currentScore <= 0:
				pdb.dbScoreAdd(lowername, 10 - currentScore)
		elif parts[0] == "bet" and len(parts) >= 3:
			selection = int(parts[1])
			bet = int(parts[2])
			
			lowername = user.getName().lower()
			
			# ensure user has enough coins
			if pdb.dbGetScore(lowername) - bet < 0:
				bnet.queueChatCommand(lowername + ": you do not have enough coins for that bet")
				return
			
			# ensure selection is inside range
			if lotteryMin < selection or lotteryMax > selection:
				bnet.queueChatCommand(lowername + ": selection is out of bounds")
				return
			
			lotteryPlayers[lowername] = (selection,bet,)
			bnet.queueChatCommand(lowername + ": bet" + bet + " on " + selection)