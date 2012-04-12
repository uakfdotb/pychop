# author = uakf.b
# version = 1.0
# name = queue
# fullname = plugins/pychop/queue
# description = Queues messages through MySQL.

master = True
port = 7000

import host
import socket
import struct
import time

# the main socket
#  for master, this is the socket to listen for connections on
#  for slave, this is the only client socket
mainSocket = 0

# list of connections for server (master)
connections = []

# buffer for receiving a packet
packet_buf = ''

# time of last attempted socket creation
lastSocketCreate = 0

# these are used in searching for connection with minimum delay
best_connection = 0
best_delay = -1

def connect():
	global mainSocket, lastSocketCreate
	
	print("[QUEUE] Creating socket...")
	lastSocketCreate = gettime()
	
	try:
		mainSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	
		if master:
			mainSocket.bind(('localhost', port))
			mainSocket.listen(5)
		else:
			mainSocket.connect(('localhost', port))
		
		mainSocket.settimeout(0)
	except socket.error, msg:
		print("[QUEUE] Socket creation failed: " + str(msg))
		disconnect()
	else:
		print("[QUEUE] Socket created successfully!")

def disconnect():
	global mainSocket
	
	try:
		mainSocket.close()
	except:
		pass
	
	mainSocket = 0

def init():
	connect()

	if master:
		host.registerHandler('QueueChatCommand', onQueue, True)
	
	host.registerHandler('Update', onUpdate)
	
def deinit():
	if master:
		host.unregisterHandler('QueueChatCommand', onQueue, True)
	
	host.unregisterHandler('Update', onUpdate)
	disconnect()

def tryConnection(connection, message):
	global best_connection, best_delay

	socketError = False

	try:
		if message == 0: # send request for client's delay
			connection.sendall(struct.pack('>IB', 5, 0))
		elif message == 1: # read client's delay response
			data = connection.recv(8)
			
			if len(data) == 0:
				# client has terminated connection
				print("[QUEUE] Client terminated connection")
				socketError = True
			else:
				# receive a signed long, representing needed delay
				# negative delay measures time since no delay
				data = struct.unpack('>q', data)[0]
				
				# check whether or not this client has the minimum delay
				if best_delay == -1 or data < best_delay:
					best_delay = data
					best_connection = connection
		else: # send message to client
			connection.sendall(struct.pack('>IB', 5 + len(message), 1))
			connection.sendall(message)
	except socket.error, msg:
		print("[QUEUE] Socket error: " + str(msg))
		socketError = True
	except socket.timeout:
		print("[QUEUE] Socket timed out")
		socketError = True
	
	if socketError:
		# socket has encountered error, close connection and return false so that connection is removed from list
		
		try:
			connection.close()
		except:
			pass
		
		return False
	
	return True

# this function is only accessed if we are the master
def onQueue(bnet, command):
	global best_connection, best_delay
	
	# first check to see if we can reply directly, if our queue is empty (or if no connections to use)
	remainingWait = bnet.delayTime()
	
	if remainingWait <= 0 or len(connections) == 0:
		return True

	# try to contact each connection and get the one with minimum remainingWait, or any with remainingWait <= 0
	best_connection = 0
	best_delay = -1
	
	connections[:] = [connection for connection in connections if tryConnection(connection, 0)]
	connections[:] = [connection for connection in connections if tryConnection(connection, 1)]
	
	if best_connection != 0:
		print("[QUEUE] Forwarding to " + str(connection.getpeername()))
		tryConnection(best_connection, command)
	
	return False

def onUpdate(chop):
	global mainSocket, packet_buf
	
	if mainSocket == 0:
		# create socket if it doesn't exist and we haven't created it in a while
		
		if gettime() - lastSocketCreate > 5000:
			connect()
	else:
		try:
			if master:
				# accept new connections
				
				try:
					conn, addr = mainSocket.accept()
					conn.settimeout(1) # one second timeout so we don't block forever
					connections.append(conn)
					
					print("[QUEUE] Accepted connection from " + str(addr))
				except socket.timeout:
					pass
				except socket.error: # happens because of timeout if no connections to accept
					pass
			else:
				# first, read data
				
				try:
					data = mainSocket.recv(512)
					
					if data == "": # make sure we are still connected
						print("[QUEUE] Disconnected from server!")
						disconnect()
						return
					
					packet_buf += data
				except socket.timeout:
					pass
				except socket.error, msg: # happens because of timeout if no data available
					pass
				
				# now, see if we have enough data for a complete packet
				if len(packet_buf) >= 5:
					packet_info = struct.unpack('>IB', packet_buf[:5])
					packet_len = packet_info[0]
					packet_type = packet_info[1]
					
					if len(packet_buf) >= packet_len:
						packet = packet_buf[5:packet_len]
						packet_buf = packet_buf[packet_len:]
						bnet = chop.BNETs[0]
						
						if packet_type == 0: # server is requesting to get our delay
							mainSocket.sendall(struct.pack('>q', bnet.delayTime()))
						elif packet_type == 1: # server is telling us to send a message
							print("[QUEUE] Accepting message from server")
							message = packet[0:]
							bnet.queueChatCommand(message)
						else:
							print("[QUEUE] Server sent unknown packet type: " + str(packet[0]))
		except socket.error, msg:
			print("[QUEUE] Socket error: " + str(msg))
			disconnect()

def gettime():
	return int(round(time.time() * 1000))
