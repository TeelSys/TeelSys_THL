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

# Set to your Adafruit IO key.
ADAFRUIT_IO_KEY = '<value here>'

# Email Settings
SMTP_SERVER = '<value here>'
SMTP_PORT = 587
SMTP_USERNAME = '<value here>'
SMTP_PASSWORD = '<value here>' #CAUTION: This is stored in plain text!

# Separate email addresses by commas
ALERT_EMAIL_ADDRESSES = ["<value here>"]
ALERT_EMAIL_SUBJECT = 'Sensor Read Failed'

# Create an instance of the REST client.
aio = Client(ADAFRUIT_IO_KEY)

LED_OFF = 0x00
LED_ON = 0x01
LED_FLASH = 0x02
TEMPERATURE = 0x04
HUMIDITY = 0x08
LIGHT = 0x0C
MODEL = 0x10
VERSION = 0x18

POWER_RESET_GPIO_PIN = 7
SECONDS_TO_WAIT_FOR_RESET = 5

DEVICE_WARM_1 = 0x10
DEVICE_WARM_2 = 0x11
DEVICE_WARM_3 = 0x12
DEVICE_ROOM_1 = 0x20
DEVICE_ROOM_2 = 0x21
DEVICE_ROOM_3 = 0x22
DEVICE_COLD_1 = 0x30
DEVICE_COLD_2 = 0x31
DEVICE_COLD_3 = 0x32

LED_0 = 11
LED_1 = 13
LED_2 = 15
LED_3 = 22
LED_4 = 18
LED_5 = 16
LED_6 = 12
LED_7 = 10
LED_8 = 8

leds = [LED_0, LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7, LED_8]

PROGRAM_BUS_FAST = "/home/pi/i2c_fast"
PROGRAM_BUS_SLOW = "/home/pi/i2c_slow"
PROGRAM_FILE = "/home/pi/getdevval"
DATA_FILE_ROOM = "/home/pi/data_room.txt"
LOG_FILE_ROOM = "/home/pi/log_room.txt"
MODEL_VALUE = "TS000001"

readError = False

def ChangeI2cBaudRate(program):
	p = subprocess.Popen(['sudo', program], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	output, err = p.communicate()
	
	if(err):
		writeLog(LOG_FILE_ROOM, "Error", "Failed to set I2C baud rate (" + err + ")")
	else:
		writeLog(LOG_FILE_ROOM, "Information", "Set I2C baud rate (" + program + ")")

def FlashLed(pin):
	GPIO.output(pin, True)
	time.sleep(1)
	GPIO.output(pin, False)
	
def getFloatValue(device, command):
	p = subprocess.Popen([PROGRAM_FILE, str(device), str(command)], stdout=subprocess.PIPE)
	output, err = p.communicate()
	if math.isnan(float(output)):
		readError=True
		return float('0.0')
	else:
		return float(output)
		
def getString(device, command):
	p = subprocess.Popen([PROGRAM_FILE, str(device), str(command)], stdout=subprocess.PIPE)
	output, err = p.communicate()
	return output

def resetI2CBus():
	GPIO.output(POWER_RESET_GPIO_PIN, False) ## Turn off GPIO pin 7
	time.sleep(SECONDS_TO_WAIT_FOR_RESET) # delays for 5 seconds
	GPIO.output(POWER_RESET_GPIO_PIN, True) ## Turn on GPIO pin 7
	time.sleep(SECONDS_TO_WAIT_FOR_RESET) # delays for 5 seconds
	
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
try:	
	GPIO.setmode(GPIO.BOARD) ## Use board pin numbering
	
	# for led in leds:
	#	GPIO.setup(led, GPIO.OUT)
	#	GPIO.output(led, False)
		
	#GPIO.setup(POWER_RESET_GPIO_PIN, GPIO.OUT) ## Setup GPIO Pin 7 to OUT
	#GPIO.output(POWER_RESET_GPIO_PIN, True) ## Turn on GPIO pin 7
	#time.sleep(SECONDS_TO_WAIT_FOR_RESET) # delays for 5 seconds
	
	GPIO.setup(LED_3, GPIO.OUT)
	GPIO.output(LED_3, True)
	
	ChangeI2cBaudRate(PROGRAM_BUS_SLOW)
	time.sleep(SECONDS_TO_WAIT_FOR_RESET)
	
	temperature_room = getFloatValue(DEVICE_WARM_1, TEMPERATURE)
	humidity_room = getFloatValue(DEVICE_WARM_1, HUMIDITY)
	light_room = getFloatValue(DEVICE_WARM_1, LIGHT)
	model = getString(DEVICE_WARM_1, MODEL)
	
	if(model != MODEL_VALUE):
		readError=True
	
	
	if(readError==False):
		writeResults(DATA_FILE_ROOM, temperature_room, humidity_room, light_room)
		aio.send('temperature-room', temperature_room)
		aio.send('humidity-room', humidity_room)
		aio.send('light-room', light_room)
		writeLog(LOG_FILE_ROOM, "Information", "Sent Data to Adafruit.IO")
	else:
		writeLog(LOG_FILE_ROOM, "Error", "I2C Bus Communication Error")
		# Reset I2C bus
		#resetI2CBus()
		# writeLog(LOG_FILE_ROOM, "Information", "I2C Bus Reset")
		sendAlertEmail("Error reading from I2C sensor")
	
	ChangeI2cBaudRate(PROGRAM_BUS_FAST)
	
finally:
	GPIO.output(LED_3, False)
	GPIO.cleanup()
	writeLog(LOG_FILE_ROOM, "Information", "Completed")
	if(readError==False):
		exit(0)
	else:
		exit(1)