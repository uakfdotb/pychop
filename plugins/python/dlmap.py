# plugins/pychop/getmap, ported from pyghost++/getmap

#Maximum filesize to DL in kB (default 10MB)
max_filesize = 10240

#allowed file extensions
allowed_exts = ('.w3x', '.w3m')

#minimum access to use
min_access = 5

#path to maps from pychop++ working direcotry
mappath = "../ghost/maps/"

import os.path
import host
import re
import random

#getdota.com web address
DOTA_ADDR = "http://www.getdota.com"

#a list of all the mirrors for downloading dota maps
DOTA_HOSTS = {
'getdota' : 'http://static.getdota.com/maps/eng/'
}

import urllib2
import urlparse

has_httplib2 = False

try:
		import httplib2.httplib2 as hl2
		http_con = hl2.Http(".cache")
		has_httplib2 = True
except ImportError:
		print('[GETMAP] You should get httplib2!')

class DownloadError(Exception):
		'''To indicate errors with downloading a map'''
		pass
		
def get_latest_dota_version():
		'''Returns the latest DotA version from getdota.com'''
		f = urllib2.urlopen(DOTA_ADDR)
		for line in f:
				if line.strip().lower().startswith('<div class="header">latest map:'):
						f.close()
						return (line.split('version">')[1]).split('<')[0]
		raise DownloadError("Couldn't get Latest DotA verson")

def download_map ( url_string, output_folder='' ):
		'''Downloads a map, checking the size and extension
		 
		 Keyword Arguments:
		 
		 url_string --the url to download (direct link to map)
		 output_folder --the location to save the map to
		'''
		#get the unquoted filename to save, and the extension
		filename = os.path.basename(urlparse.urlparse(urllib2.unquote(url_string))[2])
		extension = os.path.splitext(filename)[1]
		
		if extension not in allowed_exts:
				raise DownloadError('File is not a valid map to download')

		if has_httplib2:
				response, content = http_con.request(url_string)
				if response.status != 200:
						raise DownloadError('{0} - Error downloading {1}'.format(url_string,response.status))
		else:
				try:
						content = urllib2.urlopen(url_string).read()
				except:
						raise DownloadError('Error downloading {0}'.format(url_string))
		
		if len(content) > max_filesize * 1024:
				raise DownloadError('File too large to download')
		
		local_file = os.path.join(output_folder, filename)
		
		if not os.path.exists(local_file):
			with open(local_file, 'wb') as local_map:
					local_map.write(content)
		else:
			raise DownloadError('File already exists');
		
		return filename

def download_dota ( ver, output_folder='', mirror='' ):
		'''Downloads a dota map given only the version, defaults
		   to picking a random mirror
		   
		   Keyword Arguments:
		   
		   ver --the version to download (matching (\d\.\d{2,2}[a-z]?))
		   mirror --the key for the DOTA_HOSTS dictionary, decides where to dl from
		 '''
		
		if not re.match(r'^\d\.\d\d[a-z]?$',ver):
		 raise ValueError('{0} is not a valid version string (i.e. 6.66b)'.format(ver))
		 
		map_name = urllib2.quote('DotA v{0}.w3x'.format(ver))
		
		if mirror:
				host_url = DOTA_HOSTS[mirror]
		else:
				host_url = DOTA_HOSTS[random.choice(list(DOTA_HOSTS.keys()))]
				print('[GETMAP]Downloading from mirror ' + host_url)
		
		try:
				#call download map with the constructed url and map_name
				return download_map (urlparse.urljoin(host_url,map_name), output_folder)
		except DownloadError:
				for host, url in DOTA_HOSTS.iteritems():
						try:
								filename = download_map (urlparse.urljoin(url, map_name), output_folder)
								if os.path.isfile(os.path.join(output_folder, map_name)):
										return filename
						except Exception as E:
								print('[GETMAP]Failed downloading from {0}: {1}'.format(host,str(E)))
								pass
								
def get_map(reply, payload):
		'''Parses the command and calls download_dota, or download_map
		
		Keyword arguments:
		reply --a function used to reply to the user
		payload --the arguments with which !getmap was called
		'''
		if not payload:
				reply('Downloads a map, given a url (or a version for dota) Usage is :')
				reply('getmap <url OR dota [ver]>')
				
		else:
				# !getmap dota 6.66b
				if 'dota' in payload.lower():
						if re.match('^dota\s+\d.\d\d[a-z]?\s?\w*',payload.lower()):
								args = payload.split()
								ver = args[1]
						#!getmap dota
						else:
								ver = get_latest_dota_version()
						
						map = 'DotA v{0}.w3x'.format(ver)
						
						#check if we already have the map
						if os.path.isfile(os.path.join(mappath, map)):
								reply('Already have map for version {0}'.format(ver))
						else:
								reply('Downloading {0}'.format(map))
								mapname = download_dota(ver, mappath)
								if mapname:
										reply('Download complete!'.format(ver))
								else:
										reply("Error Downloading that map, maybe the servers don't have it?")
				else:
						args = payload.split()
						
						try:
								reply('Downloading...')
								downloadedname = download_map( payload, mappath )
								reply('Successfully downloaded map {0}'.format(downloadedname))
						except:
								reply( 'Error downloading map' )
								raise
								
def init():
		'''Initializes this plugin, by registering a bunch of handlers'''
		import host
		host.registerHandler("ProcessCommand", onCommand, True)

def deinit():
		'''Unregisters all handlers registered in init()'''
		host.unregisterHandler(onCommand)

def reply(obj, user, whisper=False):
		'''A high-order function that defines how a command function
		   should talk back to the user, returns functions that take one
		  argument, a string, and respond appropriately
		'''
		return lambda message: obj.queueChatCommand(message, user, whisper)
		
#needs to be defined after the functions
command_map={'getmap' : get_map, 'dlmap' : get_map}
		
def onCommand(bnet, user, command, payload, nType):
		'''Decides what to do with a chat message (calls the appropriate function
				if it is a command
		Keyword Arguments:
		bnet --a bnet instance
		user --the user who sent the message
		message --the actual message
		whisper --whether to respond with a whisper or not

		'''
		whisper = nType == 1
		
		if command in command_map and user.getAccess() > min_access:
				command_map[command](reply(bnet, user.getName(), whisper), payload)
				return False
		
		return True
