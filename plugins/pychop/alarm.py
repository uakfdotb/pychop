# author = uakf.b
# version = 1.0
# name = gettime
# fullname = plugins/pychop/gettime
# description = Your own alarm clock. Alarm messages are stored via PluginDB so that if the bot is restarted, alarms do not have to be set again. If you do wish to reset all alarms, clearing the database is also supported.
# help = Use !alarm set <minutes> <message> to alarm at a specific time (for example, "!alarm set 90 hello, world" will print hello, world in 90 minutes. "!alarm print" lists currently stored alarms. "!alarm clear" clears the database.
# config = access|Access needed to control

### begin configuration

# strings to identify this command
commands = ("plugins/pychop/alarm", "alarm")

# minimum access to control
controlAccess = 10

### end configuration

import host
import time
from plugindb import PluginDB

# PluginDB instance
pdb = 0

alarms = []

# bnet instance to send alarms too
alarm_bnet = 0

def dbList():
	print("[ALARM] Getting list of alarms...")
	dbAlarmList = pdb.dbGetAll()
	
	# alarms stored as key=time, value=message
	for dbAlarm in dbAlarmList:
		alarms.append((int(dbAlarm[0]), dbAlarm[1],))
	
	print("[ALARM] Found " + str(len(alarms)) + " alarms")

def init():
	global pdb, controlAccess

	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	
	pdb = PluginDB()
	pdb.setPluginName("alarm")
	pdb.dbconnect()
	dbList()
	
	# configuration
	config = host.config()
	controlAccess = config.getInt("p_alarm_access", controlAccess)

def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)
	host.unregisterHandler('Update', onUpdate)

def onCommand(bnet, user, command, payload, nType):
	global alarm_bnet
	
	alarm_bnet = bnet
	whisper = nType == 1

	if command in commands and user.getAccess() >= controlAccess:
		# might as well split with max arraylength=3
		parts = payload.split(" ", 2)
		
		if parts[0]=="set":
			# get time from the minutes
			time = int(parts[1]) * 60 * 1000 + gettime()
			alarms.append((time, parts[2],))
			# add to database
			pdb.dbAdd(time, parts[2])
		elif parts[0]=="count" and bnet.getOutPacketsQueued() < 10:
			bnet.queueChatCommand("Alarms stored: " + str(len(alarms)), user.getName(), whisper)
		elif parts[0]=="print":
			for alarm in alarms:
				print(str(alarm[0]) + "|" + alarm[1])
		elif parts[0]=="clear":
			while len(alarms) > 0:
				pdb.dbRemove(alarms[0][0])
				del alarms[0]

def onUpdate(chop):
	global alarms
	
	to_del = -1

	for i in range(len(alarms)):
		alarm = alarms[i]
		
		if gettime() > alarm[0]:
			print("[ALARM] Attempting to output: " + alarm[1])
			# it's time to print it...
			if alarm_bnet != 0 and alarm_bnet.getOutPacketsQueued() < 10:
				alarm_bnet.queueChatCommand(alarm[1])
				to_del = i
				break
			else:
				print("[ALARM] Failed: too many queued or bnet not set yet")
	
	if to_del >= 0:
		pdb.dbRemove(alarms[to_del][0])
		del alarms[to_del]

def gettime():
	return int(round(time.time() * 1000))
