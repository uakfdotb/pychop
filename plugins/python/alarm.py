# author = uakf.b
# version = 1.0
# name = gettime
# fullname = plugins/pychop/gettime
# description = Your own alarm clock. Alarm messages are stored via PluginDB so that if the bot is restarted, alarms do not have to be set again. If you do wish to reset all alarms, clearing the database is also supported.
# help = Use !alarm set <minutes> <message> to alarm at a specific time (for example, "!alarm set 90 hello, world" will print hello, world in 90 minutes. "!alarm list" lists currently stored alarms. "!alarm clear" clears the database.

### begin configuration

# strings to identify this command
commands = ("plugins/pychop/alarm", "alarm")

# minimum access to control
controlAccess = 10

### end configuration

import host
import time

# PluginDB instance
pdb = 0

# MySQL cursor instance
cursor = 0

alarms = []

# bnet instance to send alarms too
alarm_bnet = 0

def dbReady():
	global cursor
	
	print("[ALARM] Connecting to database...")
	cursor = pdb.dbconnect()
	
	print("[ALARM] Getting list of alarms...")
	dbAlarmList = pdb.dbGetAll()
	
	# alarms stored as key=time, value=message
	for dbAlarm in dbAlarmList:
		alarms.append((dbAlarm[0], dbAlarm[1],))

def init():
	global pdb

	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	
	pdb = PluginDB()
	pdb.notifyReady(dbReady)
	pdb.setPluginName("alarm")

def deinit():
	host.unregisterHandler(onCommand)
	host.unregisterHandler(onUpdate)

def onCommand(bnet, user, command, payload, nType):
	global alarm_bnet
	
	alarm_bnet = bet
	whisper = nType == 1

	if command in commands and user.getAccess() > controlAccess:
		# might as well split with max arraylength=3
		parts = payload.split(" ", 2)
		
		if parts[0]=="set":
			# get time from the minutes
			time = int(parts[1]) * 60 * 1000 + gettime()
			alarms.append((time, parts[2],))
			# add to database
			pdb.dbAdd(time, parts[2])
		elif parts[0]=="count":
			bnet.queueChatCommand("Alarms stored: " + str(len(alarms)), user.getName(), whisper)
		elif parts[0]=="print":
			for alarm in alarms:
				print(str(alarm[0]) + "|" + alarm[1])

def onUpdate(chop):
	global alarms
	
	to_del = 0

	for i in range(len(alarms)):
		alarm = alarms[i]
		
		if gettime() > alarm[0]:
			# it's time to print it...
			if alarm_bnet != 0:
				alarm_bnet.queueChatCommand(alarm[1])
				to_del = i
	
	delete alarms[to_del]

def gettime():
	return int(round(time.time() * 1000))