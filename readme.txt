
=== ChOP++ Version 0.93
	====================


ChannelOperator++ aka ChOP++ is a deeply modified version of the GHost++ project.
It is developed by Spoof.3D (Michael Höferle) & jampe (Daniel Jampen).
This software is developed and tested on Windows and Linux machines but should support other OS like Mac OSX, too.


Official ChOP++ forums:	http://chop.game-host.org/


=== Introduction

ChOP++ is a channel bot for Blizzard's Battle.Net servers and PvPGN servers.
It is capable to manage clans and semi-automatically moderate channels.
There are also other features like games or full support of GHost++ host bots.

Though to connect to official Battle.Net servers it needs its own CD-Keys like any other client.
Currently you need a Warcraft III: Reign of Chaos key to connect.
Other options might follow by time.


=== Configuration

To get ChOP++ running properly

	!! YOU MUST EDIT "chop.cfg" AND FILL IN ALL REQUIRED INFORMATION !!

All config values have discriptive comments that should help users to do so.
ChOP++ requires some Warcraft III game files to connect.
It will look for them within the bot_war3path set in your config.

These files are the following:
	• "game.dll"
	• "Storm.dll"
	• "war3.exe"

There is no need to forward any ports for this bot to work.


=== Databases setup

For now ChOP++ DOES require a MySQL database or it will fail upon startup.
Support for additional database systems are planned.

All MySQL connection information must be filled in your config for now.
You can use a remote MySQL server if you wish, but this can lead to data loss when queries fail.
To create the needed tables on your MySQL server simply use 'mysql_create_users.sql' that came with the download.
The .sql file can also be used to upgrade your existing GHost++ database without any data loss.


=== Hostbot setup

ChOP++ can interact with GHost++ hostbots.
To combine these you should use the same database for both.
Also you should set 'ghost_connect' to '1' and 'ghost_account' to the account name of your bot.
The account name is needed for some features and to prevent spam of overlapping commands like 'stats'.


=== Users

ChOP++ has a hierachical access system.
To be able to execute any command you need a specific access level defined in 'command.txt' (in 'bot_cfgpath').
These access levels range from 0 (no access) to 10 (full access) and are saved to the database together with some other user data.
One exception to this is the root admin that always has an access of 10.

To grant another users access to yor bot you need to assign access:
	!add [user] [access]
To delete a user from the database use:
	!del [user]

You're best to remove access from a user by setting it to 0 because deleting the user will clear ALL other data like seen, too.


=== Automated scripts

There is a growing list of automated scripts included in ChOP++.
You can enable or disable them individually with the corresponding config value.

• AntiSpam:
 This is a basic anti-spam implementation trying to keep the channel clean.
 If enabled it will cache the last channel messages from all users in the current channel.
 Bot commands are ignored and not saved to the cache.
 The amount of cached messages is set with op_spam_cachesize.
 Now when a single users with access 0 fills up the chat cache he will get kicked.

• AntiYell:
 This is a little script to kick yelling users (uppercase messages) from the channel.
 Bot commands and users with more than 0 access are ignored.

• PhraseKick:
 Kicks users that write any banned phrases from 'phrase.txt' (in 'bot_cfgpath').
 Bot commands and users with more than 0 access are ignored.


=== Warden

Quote from the official GHost++ readme.txt:

"On April 14th, 2009 Blizzard enabled the Warden anti cheat system when connecting to official battle.net servers. This does not affect PVPGN servers and LAN games.
Unfortunately there is currently no portable method to handle the Warden requests sent by battle.net.
To get around this, GHost++ requires an external "Warden server".
The Warden server is a type of BNLS server running on a Windows computer which GHost++ connects to. The Warden server helps GHost++ generate the correct Warden responses.
GHost++ does not send any sensitive information to the BNLS server. It is NOT possible for the BNLS server operator to steal your CD keys or username or password.
However, GHost++ assumes that the BNLS server is sending correct responses to the Warden requests.
You will need to trust the BNLS server author and the operator as it is possible they could forge fake Warden responses which could indicate to battle.net that you are cheating.
This would result in the banning of your CD keys.
It's even possible that Blizzard could update their Warden code in such a way that the Warden server would generate incorrect responses resulting in your CD keys getting banned.
It's entirely at your own risk that you connect to battle.net now that Warden is active (although it was always at your own risk it's just riskier now).
Your CD keys may be banned at any time."

The same is for ChOP++ and any other bot connecting to official Battle.Net.
So you need to set up BNLS for ChOP++ to stay connected to official Blizzard servers.
There are 3 needed config values:
	• bnet_bnls_server = <BNLS server address>
	• bnet_bnls_port = <BNLS server port>
	• bnet_bnls_wardencookie = <number> (default: 0)

The 'bnet_bnls_wardencookie' is used to identify bots connecting to the same BNLS server.
So when you use multiple bots (ChOP++, GHost++ and any other bot using BNLS) connecting to the same BNLS server you need to set a unique cookie for each of them.
That means that you set the cookie to 0 for the first bot, to 1 for the second bot and so on.
If you set the same cookie twice on different bots that connect to the same BNLS server from the same connection it will not work properly.


=== Commands

All commands are accessible via Battle.Net chat.
You can write them in the channel or whisper the bot.
Though whispered commands can be disabled with the config value 'bot_whisperallowed'

= user / access
• add [name] [access]			- sets the access for user [name] to [access]
• del [name]				- delete ALL data about user [name] from the database
• count					- displays the total number of users in the database
• access (name)				- displays the access of user (name), leave blank to see your own access

= ban
• addban [name]				- bans user [name] from the channel and adds him to the database (ban from GHost++ bots with same database), accepts partial names (channel)
• ban [name]				- alias to addban
• delban [name]				- unbans user [name] from the channel and the database
• unban	[name]				- alias to delban
• checkban [name]			- checks if user [name] is banned in the database
• countbans				- displays the total number of bans in the database

= channel
• channel [channel]			- joins channel [channel]
• say [text]				- repeats [text], can be used to execute BNet commands
• saycommand				- NOT A COMMAND, command.txt config value to control 'say'ing of BNet commands
• kick [user]				- kicks [user] from the channel, accepts partial names (channel)
• designate [name]			- designates user [name] and rejoins the channel
• rejoin				- rejoins the current channel

= clan (require ChOP++ to be shaman or chieftain)
• motd [text]				- set the clan motd to [text]
• invite [name]				- invites user [name] to join the clan, accepts partial names (channel)
• remove [name]				- throws user [name] out of the clan, accepts partial names (clan)
• peon [name]				- change user [name]'s clan rank to peon, accepts partial names (clan)
• grunt [name]				- change user [name]'s clan rank to grunt, accepts partial names (clan)
• shaman [name]				- change user [name]'s clan rank to shaman, accepts partial names (clan), requires chieftain

= managemant
• dbstatus				- show database status information
• wardenstatus				- show warden status information
• lastwhisper				- displays name of the last user that whispered
• lw					- alias to lastwhisper
• getclan				- refresh the internal copy of the clan member list
• getfriends				- refresh the internal copy of the friends list
• exit					- shuts ChOP++ down
• quit					- alias to exit
• uptime				- displays how much time passed since ChOP++ startup
• reload				- reloads all textfiles in the bot_cfgpath

= misc
• seen [name]				- displays when user [name] was last seen in the channel (clan members only), accepts partial names (clan)
• version				- displays version information
• ping (name)				- displays the ping of user (name), leave blank to see your own
• owner					- displays the owner of the bot (root admin)

= stats
• stats (name)				- displays basic player statistics of (name), leave blank to display your own
• statsdota (name)			- displays dota player statistics of (name), leave blank to display your own

= fun
• slap [name]				- makes a random joke about [name], can also hit yourself! (jokes read from 'slap_pos.txt' & 'slap_neg.txt')
• ask8ball [question]			- gives random answers to [question] (answers read from 'ask8ball.txt')
• addquote [quote]			- adds a quote to 'quote.txt'
• quote					- displays a random quote from 'quote.txt'
