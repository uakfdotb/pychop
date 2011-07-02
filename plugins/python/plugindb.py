# name = plugindb
# fullname = pychop/plugindb
# description = Used by other plguins to connect to the database.

import MySQLdb
import host
import operator

# these are not settings; they will be automatically retrieved!
dbHost = "localhost"
dbUser = "root"
dbPassword = "password"
dbName = "ghost"
dbPort = 3306

conn = 0
cursor = 0

readyCallback = 0

def init():
	host.registerHandler('StartUp', onStartup)
	
def deinit():
	host.unregisterHandler(onStartup)

# callback will be called when onStartup completes
def notifyReady(callback):
	global readyCallback
	readyCallback = callback

def onStartup(config):
	global dbHost, dbUser, dbPassword, dbName, dbPort
	
	dbHost = config.getString("db_server", dbHost)
	dbUser = config.getString("db_user", dbUser)
	dbPassword = config.getString("db_password", dbPassword)
	dbName = config.getString("db_database", dbName)
	dbPort = config.getInt("db_port", dbPort)
	
	if readyCallback != 0:
		readyCallback()

def dbconnect():
	global conn, cursor
	
	conn = MySQLdb.connect(host = dbHost, user = dbUser, passwd = dbPassword, db = dbName, port = dbPort)
	cursor = conn.cursor()
	
	# make sure the plugin db exists
	cursor.execute("CREATE TABLE IF NOT EXISTS plugindb (id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, plugin VARCHAR(16), k VARCHAR(128), val VARCHAR(128))")
	
	return cursor

def escape(nStr):
	return conn.escape_string(nStr)

# from plugin table, retrieve (key's value (string), id) or return -1
def dbGet(name, key):
	cursor.execute("SELECT val,id FROM plugindb WHERE plugin=%s AND k=%s", (name,key,))
	result = cursor.fetchone()
	
	if result == None:
		return -1
	else:
		return (str(result[0]), int(result[1]),)

def dbSet(name, key, value):
	cursor.execute("UPDATE plugindb SET val=%s WHERE plugin=%s AND k=%s", (value,name,key,))

def dbFastSet(i, value):
	cursor.execute("UPDATE plugindb SET val=%s WHERE id=%s", (value,i,))

def dbAdd(name, key, value):
	cursor.execute("INSERT INTO plugindb (plugin,k,val) VALUES(%s, %s, %s)", (name,key,value,))
	return cursor.lastrowid

def dbGetAll(name):
	cursor.execute("SELECT k,val,id FROM plugindb WHERE plugin=%s", (name,))
	result_set = cursor.fetchall()
	result_list = []
	
	for row in result_set:
		result_list.append((str(row[0]), str(row[1]), int(row[2]),))
	
	return result_list

# gets tuple(dictionary of key->score, dictionary of userid->score)
# score functions later will use this tuple (scoreTuple)
def dbGetScores(name):
	dbList = dbGetAll(name)
	
	scores = {}
	userids = {}
	scoreTuple = (scores, userids,)
	
	# element = (key, value, id)
	for element in dbList:
		scores[element[0]] = int(element[1])
		userids[element[0]] = int(element[2])
	
	return scoreTuple

def dbScoreAdd(scoreTuple, name, key, amount):
	scores = scoreTuple[0]
	userids = scoreTuple[1]
	
	# update score
	if not key in scores:
		scores[key] = 0
	
	scores[key] = scores[key] + amount
	
	# fast set if we have the DB ID, add otherwise
	if key in userids:
		dbFastSet(userids[key], scores[key])
	else:
		rowid = dbAdd(name, key, scores[key])
		userids[key] = rowid

def dbGetScore(scoreTuple, key):
	scores = scoreTuple[0]
	
	if key in scores:
		return scores[key]
	else:
		return 0

def dbScoreTop(scoreTuple):
	# sort the scores first
	scores = scoreTuple[0]
	sortedScores = sorted(scores.items(), key=operator.itemgetter(1))
	
	response = ""
	maxIndex = min(5, len(sortedScores))
	for i in range(maxIndex):
		response += str(i + 1) + ": " + sortedScores[i][0] + " with " + str(sortedScores[i][1]) + "; "
		
	return response
