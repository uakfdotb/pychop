# author = uakf.b
# version = 1.0
# name = trivia
# fullname = plugins/pychop/trivia
# description = Asks trivia questions when enabled.
# help = Use !trivia on to enable trivia, and !trivia off to disable it. !trivia delay X changes the delay after asking a question. !trivia difficulty X changes the difficulty (1-5). !trivia category disables categories, and !trivia category X sets the category to X.

commands = ("trivia", "plugins/pychop/trivia")

trivia_bnet = 0
trivia_enabled = False
trivia_question_delay = 3000  # millisecond delay after answered
trivia_delay = 10000
trivia_difficulty = 1
trivia_category = -1  # -1 means no category

trivia_state = 0  # next operations: 0 = ask question, 1 = get answer or show answer
trivia_lasttime = 0

trivia_answer = 0
trivia_answer_user = ""  # user who answered the question correctly

import host
import datetime

def init():
	host.registerHandler('ProcessCommand', onCommand)
	host.registerHandler('Update', onUpdate)
	
def deinit():
	host.unregisterHandler(onCommand)
	host.unregisterHandler(onUpdate)

def onUpdate(chop) :
	if trivia_enabled:
		if trivia_state == 0:
			if gettime() - trivia_lasttime > trivia_question_delay:
				

def gettime():
	return datetime.now().microsecond / 10

def onCommand(bnet, user, command, payload, nType):
	if command == "trivia":
		parts = payload.split(" ");
		
		if parts[0] == "on":
			global trivia_enabled, trivia_bnet, trivia_state
			trivia_enabled = True
			trivia_bnet = bnet
			trivia_state = 0
		elif parts[0] == "off":
			global trivia_enabled
			trivia_enabled = False
		elif parts[0] == "delay" and len(parts) >= 2:
			global trivia_delay
			trivia_delay = parts[1]
		elif parts[0] == "category":
			global trivia_category
			
			if len(parts) >= 2:
				trivia_category = parts[1]
			else:
				trivia_category = -1
		elif parts[0] == "difficulty" and len(parts) >= 2:
			global trivia_difficulty
			trivia_difficulty = parts[1]
