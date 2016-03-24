#!/usr/bin/env python

# Import Adafruit IO REST client.
from Adafruit_IO import Client
import subprocess
import math
import os.path
import datetime
import RPi.GPIO as GPIO ## Import GPIO library
import time
import smtplib
import ConfigParser

# *** CONSTANTS ***
LED_OFF = 0x00
LED_ON = 0x01
LED_FLASH = 0x02
TEMPERATURE = 0x04
HUMIDITY = 0x08
LIGHT = 0x0C
MODEL = 0x10
VERSION = 0x18

# *** GLOBAL VARIABLES ***
# Set to your Adafruit IO key.
ADAFRUIT_IO_KEY = ""

# Global Settings
SMTP_SERVER = ""
SMTP_PORT = 25
SMTP_USERNAME = ""
SMTP_PASSWORD = ""
ALERT_EMAIL_ADDRESSES = []
ALERT_EMAIL_SUBJECT = ""
PROGRAM_FILE = ""

readError = False

globalLogFile = "/home/pi/thl_log.txt"

Config = ConfigParser.ConfigParser()
# SensorSections = []


# *** FUNCTIONS ***

def CreateSectionIfNotExist(section):
	if not Config.has_section(section):
		Config.add_section(section)

def GetOptionCreateIfNotExist(section, option, defaultvalue):
	CreateSectionIfNotExist(section)
	if not Config.has_option(section, option):
		Config.add_option(section, option, defaultvalue)
	
	return Config.get(section, option)
	
def GetBoolOptionCreateIfNotExist(section, option, defaultvalue):
	CreateSectionIfNotExist(section)
	if not Config.has_option(section, option):
		Config.add_option(section, option, defaultvalue)
	
	return Config.getboolean(section, option)
	
def GetIntOptionCreateIfNotExist(section, option, defaultvalue):
	CreateSectionIfNotExist(section)
	if not Config.has_option(section, option):
		Config.add_option(section, option, defaultvalue)
	
	return Config.getint(section, option)

def LoadProgramConfiguration():
	global SMTP_SERVER, SMTP_PORT, SMTP_USERNAME, SMTP_PASSWORD, ALERT_EMAIL_ADDRESSES, ALERT_EMAIL_SUBJECT, ADAFRUIT_IO_KEY, PROGRAM_FILE
	# Email
	SMTP_SERVER = GetOptionCreateIfNotExist("Email", "SmtpServer", "smtp-relay.gmail.com")
	SMTP_PORT = GetIntOptionCreateIfNotExist("Email", "SmtpPort", 25)
	SMTP_USERNAME = GetOptionCreateIfNotExist("Email", "SmtpUserName", "")
	SMTP_PASSWORD = GetOptionCreateIfNotExist("Email", "SmtpPassword", "")
	ALERT_EMAIL_ADDRESSES = GetOptionCreateIfNotExist("Email", "EmailAddresses", "someone@gmil.com").split(';')
	ALERT_EMAIL_SUBJECT = GetOptionCreateIfNotExist("Email", "AlertSubject", "Sensor Read Failed")
	# AdafruitIO
	ADAFRUIT_IO_KEY = GetOptionCreateIfNotExist("AdafruitIO", "AdafruitIoKey", "")
	# Programs
	PROGRAM_FILE = GetOptionCreateIfNotExist("Programs", "SensorData", "/home/pi/getdevval")
	
def WriteProgramConfiguration():
	writeLog(globalLogFile, "DEBUG", "SMTP_SERVER = " + SMTP_SERVER)
	writeLog(globalLogFile, "DEBUG", "SMTP_PORT = " + str(SMTP_PORT))
	writeLog(globalLogFile, "DEBUG", "SMTP_USERNAME = " + SMTP_USERNAME)
	writeLog(globalLogFile, "DEBUG", "SMTP_PASSWORD = " + SMTP_PASSWORD)
	writeLog(globalLogFile, "DEBUG", "ALERT_EMAIL_ADDRESSES = " + ';'.join(ALERT_EMAIL_ADDRESSES))
#	writeLog(globalLogFile, "DEBUG", "ALERT_EMAIL_ADDRESSES = " + ALERT_EMAIL_ADDRESSES)
	writeLog(globalLogFile, "DEBUG", "ALERT_EMAIL_SUBJECT = " + ALERT_EMAIL_SUBJECT)
	writeLog(globalLogFile, "DEBUG", "ADAFRUIT_IO_KEY = " + ADAFRUIT_IO_KEY)
	writeLog(globalLogFile, "DEBUG", "PROGRAM_FILE = " + PROGRAM_FILE)
	
def getFloatValue(device, command):
	p = subprocess.Popen([PROGRAM_FILE, str(device), str(command)], stdout=subprocess.PIPE)
	output, err = p.communicate()
	if math.isnan(float(output)):
		readError=True
		return float('0.0')
	else:
		return float(output)

def GetSensorSections():
	allSections = Config.sections()
	sensorSectionsList = []
	
	for section in allSections:
		if(section.startswith("Sensor")):
			sensorSectionsList.append(section)
	
	return sensorSectionsList
		
def getString(device, command):
	p = subprocess.Popen([PROGRAM_FILE, str(device), str(command)], stdout=subprocess.PIPE)
	output, err = p.communicate()
	return output
	
def sendAlertEmail(emailMessage):
	headers = ["From: Bread Mold Test <" + SMTP_USERNAME + ">",
           "Subject: " + ALERT_EMAIL_SUBJECT,
           "To: " + ';'.join(map(str, ALERT_EMAIL_ADDRESSES)),
           "MIME-Version: 1.0",
           "Content-Type: text/html"]
	headers = "\r\n".join(headers)
	
	session = smtplib.SMTP(SMTP_SERVER, SMTP_PORT)
	
	session.ehlo()
	session.starttls()
	session.ehlo
	
	session.login(SMTP_USERNAME, SMTP_PASSWORD)
	
	session.sendmail(SMTP_USERNAME, ALERT_EMAIL_ADDRESSES, headers + "\r\n\r\n" + emailMessage)
	session.quit()

def sendCommand(device, command):
	p = subprocess.Popen([PROGRAM_FILE, str(device), str(command)], stdout=subprocess.PIPE)
	output, err = p.communicate()

def timeStamp(fmt='%Y-%m-%d %H:%M:%S'):
    return datetime.datetime.now().strftime(fmt)
	
def writeLog(filename, messageType, message):
	fmt = "{s1:s}\t{s2:s}\t{s3:s}\n"
	
	if not os.path.isfile(filename):
		writeLogFileHeader(filename, 'DateTime\tMessageType\tMessage\n')
	with open(filename, 'a') as f:
		f.write(fmt.format(s1=timeStamp(), s2=messageType, s3=message))

def writeLogFileHeader(filename, header):
	with open(filename, 'w') as f:
		f.write(header)
	
def writeResults(filename, temperature, humidity, light):
	fmt = "{s1:s}\t{f1:0.2f}\t{f2:0.2f}\t{f3:0.2f}\n"
	
	if not os.path.isfile(filename):
		writeResultsFileHeader(filename, 'DateTime\tTemperature\tHumidity\tLightLevel\n')
	with open(filename, 'a') as f:
		f.write(fmt.format(s1=timeStamp(), f1=temperature, f2=humidity, f3=light))

def writeResultsFileHeader(filename, header):
	with open(filename, 'w') as f:
		f.write(header)
		

# Main
writeLog(globalLogFile, "Information", "Started")
GPIO.setmode(GPIO.BOARD) ## Use board pin numbering
Config.read("/home/pi/thl.config")
LoadProgramConfiguration()
# WriteProgramConfiguration()
SensorSections = GetSensorSections()
writeLog(globalLogFile, "DEBUG", "Found Sensor Sections: " + ", ".join(SensorSections))

# Create an instance of the REST client.
aio = Client(ADAFRUIT_IO_KEY)

for section in SensorSections:
	# Get Config Settings
	i2cAddress = GetIntOptionCreateIfNotExist(section, "I2CAddress", 0x08)
	sensorName = GetOptionCreateIfNotExist(section, "SensorName", section)
	temperatureFeed = GetOptionCreateIfNotExist(section, "AIOFeed_Temperature", section + "-temperature")
	humidityFeed = GetOptionCreateIfNotExist(section, "AIOFeed_Humidity", section + "-humidity")
	lightFeed = GetOptionCreateIfNotExist(section, "AIOFeed_Light", section + "-light")
	dataFile = GetOptionCreateIfNotExist(section, "DataFile", section + "_data.text")
	logFile = GetOptionCreateIfNotExist(section, "LogFile", section + "_log.txt")
	modelNumber = GetOptionCreateIfNotExist(section, "ModelNumber", "TS000000")
	ledPin = GetIntOptionCreateIfNotExist(section, "LED_Pin", 11)
	isActive = GetBoolOptionCreateIfNotExist(section, "IsActive", False)
	
	if(isActive):
		try:
			GPIO.setup(ledPin, GPIO.OUT)
			GPIO.output(ledPin, True)
			
			temperature = 0
			humidity = 0
			light = 0
			model = ""
			temperature = getFloatValue(i2cAddress, TEMPERATURE)
			humidity = getFloatValue(i2cAddress, HUMIDITY)
			light = getFloatValue(i2cAddress, LIGHT)
			model = getString(i2cAddress, MODEL)
			
			if(model != modelNumber):
				readError=True
			
			if(readError==False):
				writeResults(dataFile, temperature, humidity, light)
				aio.send(temperatureFeed, temperature)
				aio.send(humidityFeed, humidity)
				aio.send(lightFeed, light)
				writeLog(logFile, "Information", "Sent Data to Adafruit.IO for " + sensorName)
			else:
				writeLog(globalLogFile, "Error", "I2C Bus Communication Error for sensor " + sensorName)
				writeLog(logFile, "Error", "I2C Bus Communication Error")
				sendAlertEmail("Error reading from I2C sensor: " + sensorName)
		except Exception,e:
			writeLog(logFile, "Error", "Some Error Occurred " + str(e))
			
			exc_type, exc_value, exc_traceback = sys.exc_info()
			lines = traceback.format_exception(exc_type, exc_value, exc_traceback)
			writeLog(logFile, "Error", ''.join('!! ' + line for line in lines))
		finally:
			GPIO.output(ledPin, False)
			
GPIO.cleanup()
writeLog(globalLogFile, "Information", "Completed")
if(readError==False):
	exit(0)
else:
	exit(1)
	
