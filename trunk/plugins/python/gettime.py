# author = uakf.b
# version = 1.0
# name = gettime
# fullname = plugins/pychop/gettime
# description = A plugin to check the server time
# help = Use !time to retrieve the current server time

# string to output before time
before_time = "the current time is: "

# strings to identify this command
commands = ("plugins/pychop/gettime", "gettime", "servertime", "time")

### end configuration

import host
import time

def init():
    host.registerHandler('onCommand', onCommand)

def deinit():
    host.unregisterHandler(onCommand)

def onWhisper(ghost, bnet, user, message):
    onMessage(ghost, bnet, user, message, True);

def onMessage(ghost, bnet, user, message, whisper=False):
    if message[0] == bnet.commandTrigger[0]:
        args = message[1:].split(None)
        if args[0] in commands:
            bnet.queueChatCommand(before_time + gettime(), user, whisper)

def onCommand(bnet, user, command, payload, nType):
	whisper = nType == 1

    if command in commands:
        bnet.queueChatCommand(before_time + gettime(), user.getName(), whisper)

def gettime():
    # time.time() returns the seconds since the epoch
    # time.localtime() generates a tuple with year, month, week day, and other information
    # time.asctime() translates the tuple into a 24 character string; ex: "Sun Jun 20 23:21:05 1993"
    return time.asctime(time.localtime(time.time()))
