# pychop alpha #

pychop (Python CHannel OPerator) is a fork of the now ended ChOP++ project. The original project was developed by Spoof.3D (Michael Höferle) & jampe (Daniel Jampen), but pychop is currently being developed by uakf.b. pychop has currently only been tested on Linux, but should also work on Windows and OS X.

Additional core features include: numerous stability and bug patches, support for commands sent through standard input, and Python plugin support.

The official pychop website is at http://code.google.com/p/pychop/. The issues tab there should be used for support, feature request, and bug/crash reports.

## Introduction ##

pychop is an open source C++ channel bot for Blizzard's battle.net servers and private PvPGN servers. It is divided into the core and the plugins. The current goal of this project is to make a relatively small, highly stable and efficient core that allows the installation of powerful plugins. Plugins then support additional features.

Currently, the core is capable of, among other things, managing clans and automatically moderating channels. Plugins include trivia, greetings, and a map downloader.

To connect to the official battle.net servers, you need a Blizzard game cd key. So far, pychop only supports Warcraft III: Reign of Chaos keys. Other options should follow by time.

## Installation ##

To install pychop, simple unzip the ZIP download. Alternatively, you can checkout the latest SVN revision:

svn checkout http://pychop.googlecode.com/svn/trunk/ pychop

The SVN includes all plugins, while plugins are separated in the main ZIP download. pychop has several requirements:

  * Python 2.7
  * game.dll, storm.dll, and war3.exe from Warcraft III (see below)
  * libboost
  * libbncutil
  * StormLib

## Configuration ##

chop.cfg must be modified in order for pychop to properly work. All config values have descriptive comments that should guide you.

pychop, like GHost++, requires some Warcraft III game files to connect. It will search for these in the bot\_war3path set in chop.cfg.

These files are: game.dll, storm.dll, and war3.exe. There is no need to forward any ports for this bot to work.

## Database setup ##

pychop requires a MySQL database or it will fail upon startup. Support for additional database systems is not planned.

All MySQL connection information must be filled in chop.cfg. To create the needed tables on your MySQL server simply use 'mysql\_create\_users.sql'. Both 'mysql\_create\_users.sql' and 'mysql\_upgrade\_ghost.sql' can be used to upgrade your existing GHost++ database without any data loss.

## Hostbot setup ##

pychop can interact with GHost++ hostbots. To combine these you should use the same database for both. Also you should set 'ghost\_connect' to '1' and 'ghost\_account' to the account name of your bot. The account name is needed for some features and to prevent spam of overlapping commands like 'stats'.

A major revision of the hostbot system from ChOP++ is planned.

## Users ##

pychop has a hierachical access system. To be able to execute any core command you need a specific access level defined in 'command.txt' (in 'bot\_cfgpath'). Plugin command access levels are individually setup.

Access levels range from 0 (default access) to 10 (full access) and are saved to the database together with some other user data. Both the root admin and the console user always have an access of 10.

To grant other users access:
> !add [user](user.md) [access](access.md)
To delete a user from the database use:
> !del [user](user.md)

To remove access from a user, it is recommended to set the access to 0: deleting the user will clear other data.

## Automated scripts: core ##

pychop includes three scripts that can be individually enabled or disabled with the corresponding config value. No new scripts will be added now that the plugin system has been implemented.

AntiSpam: a basic anti-spam implementation trying to keep the channel clean. If enabled it will cache the last channel messages from all users in the current channel (bot commands are ignored and not saved to the cache; this may be revised). The amount of cached messages is set with op\_spam\_cachesize. When a single user with access 0 fills up the chat cache, that user will be kicked.

AntiYell: a script to kick yelling users (uppercase messages) from the channel. Bot commands and users with more than 0 access are ignored.

PhraseKick: kicks users that write any banned phrases from 'phrase.txt' (in 'bot\_cfgpath'). Bot commands and users with more than 0 access are ignored.

## External scripts: plugins ##

Several official plugins are included in the SVN repository and in the plugins download. These are:

afk: keeps track of AFK users. Support for kicking these users is planned.

chanstats: basic channel statistics. Currently displays the number of unique users who have entered the channel, the number of messages sent in the channel, and the number of joins/leaves recorded in the channel (these should be about the same).

getgames: if you have the MySQL gamelist modification installed (will be posted soon...), then you can use the getgames plugin to display the list of games. getgames relies on plugindb.

greet: automatically sends a message when a user joins the channel.

plugindb: a library that takes database configuration values from pychop and allows plugins to access it. Also supports access to a plugindb table where key/value pairs can be recorded, and automatic score management.

randspeed: a simple game. pychop will display a string to the channel, and the first user to type it out wins. Stats are stored via plugindb.

test: a plugin to see if you correctly installed pychop.  Type !test and see if it works.

trivia: a powerful trivia handler. Trivia questions are read from the snapnjacks.com trivia question database. Category and difficulty can be specified both in the python file and through battle.net. Hints are given, and stats are stored via plugindb.

To submit a plugin, post it on the codelain.com forums in the pychop topic.

## Plugin management ##

To install a plugin, copy the python (for example: plugin.py) file to /pychop/plugins/python/. Then, edit /pychop/plugins/python/init.py so that the plugin is imported and initiated. If the target plugin is called target.py, add:

import target
target.init()

This can be repeated for loading multiple plugins. Note that plugins can do whatever they want, and you should only install plugins from sources you trust (i.e., if that source were to give you an executable file, you should be willing to use it). You can always, of course, look over the plugin's source and make sure it doesn't do anything nasty.

## Warden ##

Warden is currently not active.

## Commands: core ##

All commands are accessible via battle.net chat. These can be sent through the channel or whispered directly to the bot (the latter option can be disabled in chop.cfg). Alternatively, you can send them through standard input (through the terminal).

=== user / access
  * add [name](name.md) [access](access.md)	- sets the access for user [name](name.md) to [access](access.md)
  * del [name](name.md)			- delete ALL data about user [name](name.md) from the database
  * count					- displays the total number of users in the database
  * access (name)			- displays the access of user (name), leave blank to see your own access

=== ban
  * addban [name](name.md)			- bans user [name](name.md) from the channel and adds him to the database (ban from GHost++ bots with same database), accepts partial names (channel)
  * ban [name](name.md)			- alias to addban
  * delban [name](name.md)			- unbans user [name](name.md) from the channel and the database
  * unban	[name](name.md)			- alias to delban
  * checkban [name](name.md)		- checks if user [name](name.md) is banned in the database
  * countbans				- displays the total number of bans in the database

=== channel
  * channel [channel](channel.md)		- joins channel [channel](channel.md)
  * say [text](text.md)			- repeats [text](text.md), can be used to execute BNet commands
  * saycommand			- NOT A COMMAND, command.txt config value to control 'say'ing of BNet commands
  * kick [user](user.md)			- kicks [user](user.md) from the channel, accepts partial names (channel)
  * designate [name](name.md)		- designates user [name](name.md) and rejoins the channel
  * rejoin				- rejoins the current channel

=== clan (require ChOP++ to be shaman or chieftain)
  * motd [text](text.md)			- set the clan motd to [text](text.md)
  * invite [name](name.md)			- invites user [name](name.md) to join the clan, accepts partial names (channel)
  * remove [name](name.md)			- throws user [name](name.md) out of the clan, accepts partial names (clan)
  * peon [name](name.md)			- change user [name](name.md)'s clan rank to peon, accepts partial names (clan)
  * grunt [name](name.md)			- change user [name](name.md)'s clan rank to grunt, accepts partial names (clan)
  * shaman [name](name.md)			- change user [name](name.md)'s clan rank to shaman, accepts partial names (clan), requires chieftain

=== managemant
  * dbstatus				- show database status information
  * wardenstatus			- show warden status information
  * lastwhisper			- displays name of the last user that whispered
  * lw					- alias to lastwhisper
  * getclan				- refresh the internal copy of the clan member list
  * getfriends			- refresh the internal copy of the friends list
  * exit					- shuts ChOP++ down
  * quit					- alias to exit
  * uptime				- displays how much time passed since ChOP++ startup
  * reload				- reloads all textfiles in the bot\_cfgpath

=== misc
  * seen [name](name.md)			- displays when user [name](name.md) was last seen in the channel (clan members only), accepts partial names (clan)
  * version				- displays version information
  * ping (name)			- displays the ping of user (name), leave blank to see your own
  * owner					- displays the owner of the bot (root admin)

=== stats
  * stats (name)			- displays basic player statistics of (name), leave blank to display your own
  * statsdota (name)		- displays dota player statistics of (name), leave blank to display your own

=== fun
  * slap [name](name.md)			- makes a random joke about [name](name.md), can also hit yourself! (jokes read from 'slap\_pos.txt' & 'slap\_neg.txt')
  * ask8ball [question](question.md)	- gives random answers to [question](question.md) (answers read from 'ask8ball.txt')
  * addquote [quote](quote.md)		- adds a quote to 'quote.txt'
  * quote					- displays a random quote from 'quote.txt'