# author = uakf.b
# version = 1.0
# name = pluginman
# fullname = plugins/pychop/pluginman
# description = A plugin management system. Can reload plugins during testing so that the bot does not have to be fully restarted.
# help = Specify plugins to load on startup in the source file. Use the commands "!pluginman load", "!pluginman unload", and "!pluginman reload" to load, unload, and reload plugins, respectively. Note that reloading must be used if you wish to test a new version of a plugin as import is stored in unload.

### begin configuration

# strings to identify this command
commands = ("plugins/pychop/pluginman", "pluginman")

# minimum access to control
controlAccess = 10

# plugins (python module strings) to load on startup
startupLoad = ()

### end configuration

import host

# loaded plugins, string -> module
loadedPlugins = {}

# plugins that were imported at some time, string -> module
importedPlugins = {}

def init():
	host.registerHandler('ProcessCommand', onCommand, True)
	
	for name in startupLoad:
		importedPlugins[parts[1]] = __import__(parts[1], globals(), locals(), [], -1)
		loadedPilugins[parts[1]] = importedPlugins[parts[1]]
		loadedPlugins[parts[1]].init()

def deinit():
	host.unregisterHandler('ProcessCommand', onCommand, True)

def onCommand(bnet, user, command, payload, nType):
	if command in commands and user.getAccess() >= controlAccess:
		parts = payload.split(" ", 1)
		
		if parts[0]=="load":
			print("Loading plugin " + parts[1])
			
			# make sure it's not already loaded
			if parts[1] in loadedPlugins:
				print("Error: already loaded")
				return
			
			# check if the plugin is imported; import if not
			if not parts[1] in importedPlugins:
				importedPlugins[parts[1]] = __import__(parts[1], globals(), locals(), [], -1)
				print("Imported plugin: " + str(importedPlugins[parts[1]]))
				
			loadedPlugins[parts[1]] = importedPlugins[parts[1]]
			loadedPlugins[parts[1]].init()
		elif parts[0]=="unload":
			print("Unloading plugin " + parts[1])
			
			# make sure it's loaded
			if not parts[1] in loadedPlugins:
				print("Error: not loaded")
				return
			
			loadedPlugins[parts[1]].deinit()
			del loadedPlugins[parts[1]]
		elif parts[0]=="reload":
			print("Reloading plugin " + parts[1])
			
			# make sure it has been imported
			if not parts[1] in importedPlugins:
				print("Error: not imported")
				return
			
			if parts[1] in loadedPlugins:
				loadedPlugins[parts[1]].deinit()
				del loadedPlugins[parts[1]]
			
			reload(importedPlugins[parts[1]])
			
			loadedPlugins[parts[1]] = importedPlugins[parts[1]]
			loadedPlugins[parts[1]].init()
		
		# stop executing so we don't run anything we just inited
		return False
	
	return True
			
