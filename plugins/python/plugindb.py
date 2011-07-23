# name = plugindb
# fullname = pychop/plugindb
# description = Used by other plguins to connect to the database.

import MySQLdb
import host
import operator

class PluginDB:
	'''Interface between other plugins and a MySQL database. Can either be used simply to get a connection or to write key/value pairs to the plugindb table. Also supports automatic score management.'''
	
	# these are not settings; they will be automatically retrieved!
	dbHost = "localhost"
	dbUser = "root"
	dbPassword = "password"
	dbName = "ghost"
	dbPort = 3306

	conn = 0
	cursor = 0

	readyCallback = 0
	
	# used in automatic score management
	scoreTuple = 0
	
	# used for interaction with plugindb table
	pluginName = "default"

	def __init__(self):
		def startupHandler(config):
			self.onStartup(config)

		host.registerHandler('StartUp', startupHandler)
	
	def close():
		host.unregisterHandler(onStartup)

	# callback will be called when onStartup completes
	def notifyReady(self, callback):
		self.readyCallback = callback

	def onStartup(self, config):
		self.dbHost = config.getString("db_server", self.dbHost)
		self.dbUser = config.getString("db_user", self.dbUser)
		self.dbPassword = config.getString("db_password", self.dbPassword)
		self.dbName = config.getString("db_database", self.dbName)
		self.dbPort = config.getInt("db_port", self.dbPort)
	
		if self.readyCallback != 0:
			self.readyCallback()

	def dbconnect(self):
		self.conn = MySQLdb.connect(host = self.dbHost, user = self.dbUser, passwd = self.dbPassword, db = self.dbName, port = self.dbPort)
		self.cursor = self.conn.cursor()
	
		# make sure the plugin db exists
		self.cursor.execute("CREATE TABLE IF NOT EXISTS plugindb (id INT NOT NULL PRIMARY KEY AUTO_INCREMENT, plugin VARCHAR(16), k VARCHAR(128), val VARCHAR(128))")
	
		return self.cursor

	def setPluginName(self, name):
		self.pluginName = name

	def escape(self, nStr):
		return self.conn.escape_string(nStr)

	# from plugin table, retrieve (key's value (string), id) or return -1
	def dbGet(self, name, key):
		self.cursor.execute("SELECT val,id FROM plugindb WHERE plugin=%s AND k=%s", (name,key,))
		result = self.cursor.fetchone()
	
		if result == None:
			return -1
		else:
			return (str(result[0]), int(result[1]),)

	def dbSet(self, key, value):
		self.cursor.execute("UPDATE plugindb SET val=%s WHERE plugin=%s AND k=%s", (value,self.pluginName,key,))

	def dbFastSet(self, i, value):
		self.cursor.execute("UPDATE plugindb SET val=%s WHERE id=%s", (value,i,))

	def dbAdd(self, key, value):
		self.cursor.execute("INSERT INTO plugindb (plugin,k,val) VALUES(%s, %s, %s)", (self.pluginName,key,value,))
		return self.cursor.lastrowid

	def dbRemove(self, key):
		self.cursor.execute("DELETE FROM plugindb WHERE plugin=%s AND k=%s", (self.pluginName,key,))

	# returns array with tuples (key, value, dbID)
	def dbGetAll(self):
		self.cursor.execute("SELECT k,val,id FROM plugindb WHERE plugin=%s", (self.pluginName,))
		result_set = self.cursor.fetchall()
		result_list = []
	
		for row in result_set:
			result_list.append((str(row[0]), str(row[1]), int(row[2]),))
	
		return result_list

	def dbClear(self):
		self.cursor.execute("DELETE FROM plugindb WHERE plugin=%s", (self.pluginName,))

	# gets tuple(dictionary of key->score, dictionary of userid->score)
	# score functions later will use this tuple (scoreTuple)
	def dbGetScores(self):
		dbList = self.dbGetAll()
	
		scores = {}
		userids = {}
		self.scoreTuple = (scores, userids,)
	
		# element = (key, value, id)
		for element in dbList:
			scores[element[0]] = int(element[1])
			userids[element[0]] = int(element[2])
	
		return self.scoreTuple

	def dbScoreAdd(self, key, amount):
		scores = self.scoreTuple[0]
		userids = self.scoreTuple[1]
	
		# update score
		if not key in scores:
			scores[key] = 0
	
		scores[key] = scores[key] + amount
	
		# fast set if we have the DB ID, add otherwise
		if key in userids:
			self.dbFastSet(userids[key], scores[key])
		else:
			rowid = self.dbAdd(key, scores[key])
			userids[key] = rowid

	def dbGetScore(self, key):
		scores = self.scoreTuple[0]
	
		if key in scores:
			return scores[key]
		else:
			return 0

	def dbScoreTop(self):
		# sort scores and return
		scores = self.scoreTuple[0]
		sortedScores = sorted(scores.items(), key=operator.itemgetter(1), reverse=True)
		return sortedScores

	def dbScoreTopStr(self, num):
		sortedScores = self.dbScoreTop()
	
		response = ""
		maxIndex = min(num, len(sortedScores))
		for i in range(maxIndex):
			response += str(i + 1) + ": " + sortedScores[i][0] + " with " + str(sortedScores[i][1]) + "; "
		
		return response
	
	def dbScoreNum(self):
		# return the number of scores stored
		return len(self.scoreTuple[0])
	
	
