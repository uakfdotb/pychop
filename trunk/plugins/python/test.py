# author = uakf.b
# version = 1.0
# name = test
# fullname = plugins/pychop/test
# description = Test of the plugin system, also an example plugin.

import host

def init():
    host.registerHandler('ProcessCommand', onCommand)
    
def deinit():
    host.unregisterHandler(onCommand)

def onCommand(bnet, user, command, payload, nType):
    if command == "test":
        print("received test command from " + user.getName() + " (" + str(user.getAccess()) + ") with payload " + payload)
