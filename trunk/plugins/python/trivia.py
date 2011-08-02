# author = uakf.b
# version = 1.0
# name = trivia
# fullname = plugins/pychop/trivia
# description = Asks trivia questions when enabled.
# help = Use !trivia on to enable trivia, and !trivia off to disable it. !trivia delay X changes the delay after asking a question. !trivia difficulty X changes the difficulty (1-5). !trivia category disables categories, and !trivia category X sets the category to X.

commands = ("trivia", "plugins/pychop/trivia")
trivia_minaccess = 6  # only for commands; playing trivia needs no access :)

trivia_bnet = 0
trivia_enabled = False
trivia_question_delay = 6000  # millisecond delay after answered
trivia_delay = 8000  # millisecond delay between each answer stage
trivia_difficulty = -1  # -1 means no difficulty
trivia_category = -1  # -1 means no category

# these are dynamic during a trivia session

# depending on state next operation should be:
# 0: display the question
# 1: display ???? (# characters in answer)
# 2: display ?a?a (stage2 answer)
# 3: display aa?a (stage3 answer)
# 4: display aaaa (display answer) and -> 0
trivia_state = 0
trivia_lasttime = 0

# list of acceptable answers
trivia_answer = 0

# uncovered list version of trivia_answer[0] (???? -> ?a?a -> etc)
trivia_uncover = 0

# user who answered the question correctly
trivia_answer_user = ""

import host
import time
from collections import deque
import urllib2
import urlparse
import random
from plugindb import PluginDB

trivia_questions = deque([]) # this will be populated with question/answer pairs later

# PluginDB instance
pdb = 0

def dbReady():
	print("[TRIVIA] Loading scores...")
	
	pdb.dbconnect()
	pdb.dbGetScores()
	
	print("[TRIVIA] Found " + str(pdb.dbScoreNum()) + " scores")

def init():
	global pdb

	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('ChatReceived', onTalk)
	host.registerHandler('Update', onUpdate)
	
	pdb = PluginDB()
	pdb.notifyReady(dbReady)
	pdb.setPluginName("trivia")
	
def deinit():
	host.unregisterHandler('ProcessCommand', onCommand)
	host.unregisterHandler('Update', onUpdate)
	host.unregisterHandler('ChatReceived', onTalk)
	pdb.close()

def onTalk(bnet, username, message):
	global trivia_lasttime, trivia_state, trivia_enabled
	
	# are we waiting for an answer?
	if trivia_enabled and trivia_state > 0:
		userAnswer = message.lower()
		
		if userAnswer in trivia_answer:
			# update score
			pdb.dbScoreAdd(username.lower(), 1)
		
			# get new score
			newScore = pdb.dbGetScore(username.lower())
			
			say("The answer was: " + userAnswer + "; user " + username + " got it correct! (points: " + str(newScore) + ")");
			# reset
			trivia_lasttime = gettime()
			trivia_state = 0

def onUpdate(chop) :
	global trivia_lasttime, trivia_answer_user, trivia_state, trivia_enabled, trivia_answer, trivia_uncover
	
	if trivia_enabled:
		if trivia_state == 0:
			if gettime() - trivia_lasttime > trivia_question_delay:
				# it's time to ask a question
				askQuestion()
				# reset some things
				trivia_lasttime = gettime()
				trivia_answer_user = ""
				trivia_state = 1
		elif trivia_state == 1:
			if gettime() - trivia_lasttime > trivia_delay:
				say("Hint: " + "".join(trivia_uncover))
				# reset some things
				trivia_lasttime = gettime()
				trivia_state = 2
		elif trivia_state == 2 or trivia_state == 3:
			if gettime() - trivia_lasttime > trivia_delay:
				uncover()
				say("Hint: " + "".join(trivia_uncover))
				# reset some things
				trivia_lasttime = gettime()
				trivia_state = trivia_state + 1
		elif trivia_state == 4:
			if gettime() - trivia_lasttime > trivia_delay:
				# no one answered; let's show them the answer
				say("The answer was: " + trivia_answer[0]);
				# reset some things
				trivia_lasttime = gettime()
				trivia_state = 0

def uncover():
	global trivia_uncover
	# calculate how many to uncover; note that we might uncover same one twice
	# this formula ensures that we don't uncover too many
	num_uncover = int(round(.33 * len(trivia_uncover)))
	
	if len(trivia_uncover) <= 2:
		num_uncover = 0
	elif len(trivia_uncover) < 4:
		num_uncover = 1
	
	for i in range(num_uncover):
		index = random.randint(0, len(trivia_uncover) - 1)
		trivia_uncover[index] = trivia_answer[0][index]

def askQuestion():
	global trivia_answer, trivia_uncover, trivia_questions
	
	# make sure there are questions available
	if len(trivia_questions) == 0:
		addQuestions()
	
	# pair will now contain (question, answer)
	pair = trivia_questions.popleft()
	trivia_answer = pair[1]
	
	# generate trivia_uncover
	trivia_uncover = []
	for i in range(len(trivia_answer[0])):
		if trivia_answer[0][i] != " ":
			trivia_uncover.append("?")
		else:
			trivia_uncover.append(trivia_answer[0][i])
	
	say(pair[0] + " (category: " + pair[2] + ")")

def say(message):
	trivia_bnet.queueChatCommand(message)

def addQuestions():
	global trivia_enabled, trivia_questions
	targetURL = "http://snapnjacks.com/getq.php?client=plugins/pychop/trivia"
	
	# quote custom parameters to replace with %XX
	if trivia_difficulty != -1:
		targetURL += "&dif=" + urllib2.quote(str(trivia_difficulty))
	
	if trivia_category != -1:
		targetURL += "&ctg=" + urllib2.quote(trivia_category)
	
	print("[TRIVIA] Reading questions from " + targetURL)
	
	try:
		content = urllib2.urlopen(targetURL).read()
	except Exception as E:
		print("[TRIVIA] Error: unable to read " + targetURL + ":" + str(E))
		trivia_questions.append(("Unable to load questions! You won't be able to answer this question D:", ["error"]))
		
		# disable trivia
		trivia_enabled = False
		
		return
	
	questionSplit = content.split("**")
	
	for questionString in questionSplit:
		parts = questionString.split("|")
		
		if(len(parts) < 2):
			continue
		
		unformattedAnswers = parts[1].split("/")
		answers = []
		
		for x in unformattedAnswers:
			# remove whitespace and convert to lowercase
			answers.append(x.lower().strip())
		
		# parts[0] is question, parts[6] is category
		trivia_questions.append((parts[0], answers, parts[6]))
		print("[TRIVIA] Appended question: " + parts[0] + "; storing " + str(len(trivia_questions)) + " questions now")

def gettime():
	return int(round(time.time() * 1000))

def onCommand(bnet, user, command, payload, nType):
	global trivia_enabled, trivia_bnet, trivia_state, trivia_category, trivia_difficulty, trivia_questions
	
	if command in commands:
		parts = payload.split(" ");
		
		if user.getAccess() >= trivia_minaccess:
			if parts[0] == "on":
				trivia_enabled = True
				trivia_bnet = bnet
				trivia_state = 0
			
				print("[TRIVIA] Enabled with category=" + str(trivia_category) + " and diff=" + str(trivia_difficulty))
			elif parts[0] == "off":
				trivia_enabled = False
			elif parts[0] == "delay" and len(parts) >= 2:
				trivia_delay = parts[1]
			elif parts[0] == "category":
				if len(parts) >= 2:
					trivia_category = parts[1]
				else:
					trivia_category = -1
			
				# clear questions since we changed the type
				trivia_questions = deque([])
			elif parts[0] == "difficulty" and len(parts) >= 2:
				if len(parts) >= 2:
					trivia_difficulty = parts[1]
				else:
					trivia_difficulty = -1

				# clear questions since we changed the type
				trivia_questions = deque([])

		if parts[0] == "top" and bnet.getOutPacketsQueued() < 3:
			# display top 5
			bnet.queueChatCommand(pdb.dbScoreTopStr(5))
		elif parts[0] == "score" and bnet.getOutPacketsQueued() < 3:
			lowername = user.getName().lower()
			
			if len(parts) == 2:
				lowername = parts[1].lower()

			bnet.queueChatCommand(lowername + " points: " + str(pdb.dbGetScore(lowername)))
