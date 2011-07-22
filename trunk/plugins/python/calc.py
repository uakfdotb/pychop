# author = uakf.b
# version = 1.0
# name = calc
# fullname = plugins/pychop/calc
# description = A basic calculator.
# help = Use !calc [expression] to evaluate an expression. Every element of the expression (parentheses, numbers, operators, etc) must be separated by a space.

import host
import random
import math

# string to output before result
before = "plugins/pychop/calc: "

# strings to identify the calculator command
commands = ("plugins/pychop/calc", "math", "calc", "calculate")

# strings to identify the set command (sets a variable to a number or another variable)
set_commands = ("set")

# maximum number of variables to store
varmax = 20

# preset variables
variables = {"ans":0.0, "pi":math.pi, "e":math.e}

### end configuration

#functions and operations
def add(x, y):
	return x + y

def subtract(x, y):
	return x - y

def multiply(x, y):
	return x * y

def divide(x, y):
	return x / y

def power(x, y):
	return x**y

#all functions take one operand
functions = {"cos":math.cos, "sin":math.sin, "tan":math.tan, "sqrt":math.sqrt, "log":math.log,
	"acos":math.acos, "asin":math.asin, "atan":math.atan, "toradians":math.radians, "todegrees":math.degrees,
	"factorial":math.factorial, "log10":math.log10}
# all operations take two operands
operations = {"+":add, "-":subtract, "*":multiply, "/":divide, "^":power}
operator_precedence = {"^":3, "*":2, "/":2, "+":1, "-":1}

def init():
	host.registerHandler('ProcessCommand', onCommand)

def deinit():
	host.unregisterHandler(onCommand)

def onCommand(bnet, user, command, payload, nType):
	whisper = nType == 1

	if command in commands and bnet.getOutPacketsQueued() < 3:
		args = payload.split(None)
		bnet.queueChatCommand(before + calc(args), user.getName(), whisper)
	elif command in set_commands and bnet.getOutPacketsQueued() < 3:
		args = payload.split(None)
		varset(args[0], args[1])

def varset(s1, s2):
	global variables
	
	if len(variables) >= varmax:
		return
	
	if isnum(s2):
		variables[s1] = float(s2)
	elif s2 == "clear":
		del variables[s1]
	else:
		variables[s1] = variables[s2]

def calc(args):
	"""
	This function uses a simple algorithm to evaluate postfix expressions that is detailed at http://en.wikipedia.org/wiki/Reverse_Polish_notation.
	"""
	
	#first we convert infix -> postfix
	args = convert(args)
	#now evaluate the postfix expression
	stack = []
	
	for token in args:
		if isnum(token):
			stack.append(token)
		elif token in functions:
			x = float(stack.pop())
			stack.append(functions[token](x))
		elif token in operations:
			y = float(stack.pop())
			x = float(stack.pop())
			stack.append(operations[token](x, y))
	
	global variables
	variables['ans'] = stack[-1]
	return str(stack[-1])

def convert(args):
	"""
	More details on this algorithm (Dijkstra's shunting-yard algorithm) can be found at http://en.wikipedia.org/wiki/Shunting-yard_algorithm.
	"""
	
	li = []
	stack = []
	
	for token in args:
		if isnum(token):
			li.append(token)
		elif token in functions:
			stack.append(token)
		elif token == ",":
			while len(stack) >= 1:
				tmp = stack[-1]
				if tmp == "(":
					break
				else:
					li.append(stack.pop())
		elif token in operations:
			while len(stack) >= 1:
				tmp = stack[-1]
				if tmp not in operations or operator_precedence[tmp] < operator_precedence[token]:
					break
				else:
					li.append(stack.pop())
			stack.append(token)
		elif token == "(":
			stack.append(token)
		elif token == ")":
			while len(stack) >= 1:
				tmp = stack.pop()
				if tmp == "(":
					break
				else:
					li.append(tmp)
			if len(stack) >= 1 and stack[-1] in functions:
				li.append(stack.pop())
		else:
			li.append(variables[token])
	
	while len(stack) >= 1:
		li.append(stack.pop())
	return li

def isnum(s):
	"""
	Returns true if the input string can be converted to a float.
	"""
	try:
		float(s)
		return True
	except ValueError:
		return False
