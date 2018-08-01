import serial
import json
import time
import random

from pubnub.enums import PNStatusCategory
from pubnub.callbacks import SubscribeCallback
from pubnub.pnconfiguration import PNConfiguration
from pubnub.pubnub import PubNub, SubscribeListener


# establish PubNub communication object
pnconfig = PNConfiguration()
pnconfig.subscribe_key = 'YOUR SUB KEY HERE'
pnconfig.publish_key = 'YOUR PUB KEY HERE'
pnconfig.ssl = False

pubnub = PubNub(pnconfig)

# create the tweet request channel listener

def publishGammaTweet():
	print("gamma tweet request publish function")
	# tweet = 'this is only a TEST! canary CPM: {:d}'.format(random.randint(0,500))
	# tweet = 'this is only a TEST! canary CPM: {:f}'.format(cpm[1])
	tweetLine1 = '*!*!* THIS IS ONLY A TEST *!*!*\n'
	tweetLine2 = 'The gamma canary Alert System\n'
	tweetLine3 = 'CPM: {:d} at lat {:04.5f}, lon {:04.5f}\n'.format(int(cpm[1]),currentLocation[0],currentLocation[1])
	tweetLine4 = 'Local Time(EST): ' + time.asctime( time.localtime(time.time()) ) + "\n"
	tweet = tweetLine1 + tweetLine2 + tweetLine3 + tweetLine4
	msg = { "tweet" : tweet }
	print(msg)
	pubnub.publish().channel('gamma-canary-tweet').message(msg).async(publish_callback)

def publishSMSMessage(phoneNumber):
        smsLine1 = 'ALERT! '
        smsLine2 = 'CPM: {:d} LAT: {:04.5f}, LON: {:04.5f}\n'.format(int(cpm[1]),currentLocation[0],currentLocation[1])
        smsMsg = smsLine1 + smsLine2
        sms = { "to" : phoneNumber, "body" : smsMsg }

        print(sms)
        pubnub.publish().channel('cpm-text-alert').message(sms).async(publish_callback)

class myPubNubListener(SubscribeCallback):
    def status(self, pubnub, status):
        print("status changed: %s" % status)

    def message(self, pubnub, message):
        print("new message: %s" % message.message)
        print(str(message.message))
        print(message.message['request'])

        if (message.message['request'] == 'tweet'):
                publishGammaTweet()
        elif (message.message['request'] == 'sms'):
                publishSMSMessage(message.message['number'])

    def presence(self, pubnub, presence):
        pass

myPubNubListener = myPubNubListener()
pubnub.add_listener(myPubNubListener)

# subscribe to our pubnub channel
pubnub.subscribe().channels('canaryTweet').execute()
# myPubNubListener.wait_for_connect()
# pubnub publish callback listener - aids in debugging pubnub connectivty/messaging

def publish_callback(result, status):
	if  not  status.is_error():
		pass
	else:
		print("Error %s" %str(status.error_data.exception))
		print("Error category %d" %status.category)
		pass

# subscribe call back listenr function to execute on recieving a mag on channel

def callback(message, channel):
    print('[' + channel + ']: ' + str(message))

# Serial "gateway" connection from this RPi to MAX32620FTHR/LoRa board
# RPi serial pins:
# MAXFTHR serial  pins:
ser = serial.Serial('/dev/serial0',9600)

# LoRa Network node & data type 'tables'
txSourceDict = { '0':'LoRaGateway', '1':'canary001', '2':'canary002' }
txDataTypes = { '@':'GPS', 'A':'CPM', 'B':'FuelGauge' }

# LoRa Message Header Variables
sourceID = ''
sourceName = ''
txDataType = ''

# Data Parameters
# GPS/Location Data

currentLocation = [0.000000, 0.000000] 	# [longitude,latitude]
previousLocation = [0.000000, 0.000000]	# [longitude,latitude]

currentLatitude 	= 0.000000
currentLongitude 	= 0.000000
previousLatitude 	= 0.000000
previousLongitude 	= 0.000000

# CPM (Counts Per Minute) Data
cpm = [0,0] # cpm[0] = previous, cpm[1] = current
currentCPM 	= 0
previousCPM 	= 0

# Fuel Gauge Data
currentFuel = [0.000000, 0.000000, 0.00]
previousFuel = [0.000000, 0.000000, 0.00]

currentVoltage = 0.000000
currentCurrent = 0.000000
currentTemperature = 0.00
previousVoltage = 0.000000
previousCurrent = 0.000000
previousTemperature = 0.00

print " **** LoRa Pi PubNub Gateway v0.01 **** "
print txSourceDict.keys() # debug purposes
print txSourceDict['0']   # debug purposes

# ****************************************************************** #
#  LoRa message parsing functions
# ****************************************************************** #

def parseGPS(gpsMsg,prevGPS,currGPS):

	print "parse gps: " + str(gpsMsg) # debug stmt
	prevGPS[0] = currGPS[0]
	prevGPS[1] = currGPS[1]
	try:
		currGPS[0] = float(gpsMsg[2])	# longitude
		currGPS[1] = float(gpsMsg[3])	# latitude
		print "GPS: " + currGPS[0]+ "," + currGPS[1]
	except Exception as e:
		print "error parsing GPS data: "
		print e

def parseCPM(cpmMsg, cpmData):

	print "parse cpm: " + str(cpmMsg) # debug stmt
	cpmData[0] = cpmData[1]
	try:
		cpmData[1] = float(cpmMsg[2])
	except Exception as e:
		print "error parsing cpm data "
		print e

def parseFuelGauge(fuelMsg, prevFuel, currFuel):

	print "parse fuel: " + str(fuelMsg) 	# debug stmt
	prevFuel[0] = currFuel[0]		# state of charge(SOC)
	prevFuel[1] = currFuel[1]		# time to empty(TTE)
	prevFuel[2] = currFuel[2]		# temperature

	try:
		currFuel[0] = float(fuelMsg[2]) # state of charge(SOC)
		currFuel[1] = float(fuelMsg[3]) # time to empty(TTE)
		currFuel[2] = float(fuelMsg[4]) # temperature
	except Exception as e:
		print "error parsing fuel"
		print e

def parseBatteryLow(msg):
        # this is an "emergency" situation just msg out the problem
        for x in range(0,3):

                msgLine0 = str(x)
                msgLine1 = ' BAT LOW! '
                msgLine2 = 'LAST LAT: {:04.5f}, LON: {:04.5f}\n'.format(currentLocation[0],currentLocation[1])
                emergencyMsg = msgLine0 + msgLine1 + msgLine2

                sms = { "to" : "1231231234", "body" : emergencyMsg }
                tweetMsg = { "tweet" : emergencyMsg }

                pubnub.publish().channel('gamma-canary-tweet').message(tweetMsg).async(publish_callback)
                pubnub.publish().channel('cpm-text-alert').message(sms).async(publish_callback)

        print(emergencyMsg)

# ****************************************************************** #
# End LoRa Parsing functions here
# ****************************************************************** #

# LoRa RPi PubNub Gateway Loop Begins here
while 1:
	# LoRa message format:
	# source id**type**data0**...**dataN**

	LoRaMsg = ser.readline()
	print "raw message: " + LoRaMsg 	# debug purposes
	msg = LoRaMsg.split('**',5)
	print msg
	try:
		# print "source id: " + msg[0] # debug purposes
		# print txSourceDict[msg[0]]   # debug purposes
		if txSourceDict.has_key(msg[0]):
			sourceID = msg[0]
			sourceName = txSourceDict[msg[0]]
			print "[ID,NAME]: " + "[" + sourceID + "," + sourceName + "]" # debug statement
	except Exception as e:
		print "source id not recognized"
		print e

	# LoRa Message parsing begins here:

	try:
		if txDataTypes.has_key(msg[1]):
			txDataType = txDataTypes[msg[1]]
			print "data type: " + txDataType # debug statement
			if txDataType == 'GPS':
				parseGPS(msg,previousLocation,currentLocation)
			elif txDataType == 'CPM':
				parseCPM(msg,cpm)
			elif txDataType == 'FuelGauge':
				parseFuelGauge(msg,previousFuel,currentFuel)
			elif txDataType == 'BatteryLow':
                parseBatteryLow(msg)
	except Exception as e:
		print "unknown data type here"
		print e


	print				# debug stmt
	print "Current Data:"		# debug stmt
	print str(currentLocation) 	# debug stmt
	print str(cpm)			# debug stmt
	print str(currentFuel)		# debug stmt

	# format messages for PubNub publishing
	latlngPN = {'latlng':currentLocation}
	cpmPN = {'CPM':cpm[1]}
	batSOC_PN = {'SOC':currentFuel[0]}
	batTTE_PN = {'TTE':currentFuel[1]}
	batTemp_PN = {'Temp':currentFuel[2]}

	print

	pubnub.publish().channel('myMap').message([latlngPN]).async(publish_callback)
	pubnub.publish().channel('radData').message({'eon': cpmPN}).async(publish_callback)
	pubnub.publish().channel('batterySOC').message({'eon': batSOC_PN}).async(publish_callback)
	pubnub.publish().channel('batteryTTE').message({'eon': batTTE_PN}).async(publish_callback)
	pubnub.publish().channel('batteryTemp').message({'eon': batTemp_PN}).async(publish_callback)

	# pubnub.publish().channel('radData').message({'eon': dataDict}).async(publish_callback)

# end new code here





©
