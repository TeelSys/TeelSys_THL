/*
	* gcc -Wall -pthread -o getdevval_bb getdevval_bb.c -lpigpio -lrt -lm
	* gcc -Wall -pthread -o getdevval_bb getdevval_bb.c -lpigpiod_if2 -lrt
	* ./getdevval_bb 16 0
	*
	* sudo pigpiod
*/

// #include <linux/i2c-dev.h>
// #include <pigpio.h>

/*
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pigpiod_if2.h>

#define CMD_GET_TEMPERATURE 1
#define CMD_GET_HUMIDITY 2
#define CMD_SET_LIGHT 3
#define CMD_SET_LED_ON 4
#define CMD_SET_LED_OFF 5
#define CMD_SET_LED_FLASH 6
#define CMD_GET_MODEL 250
#define CMD_GET_VERSION 251
#define CMD_GET_HELLO_WORLD 254

// The GPIO pins to use for the 'External' I2C bus
#define SDA_PIN 2
#define SDL_PIN 3
#define BAUD 5000

// Function Headers
char receiveChar(unsigned char deviceAddress);
float receiveFloat(unsigned char deviceAddress);
int receiveInt(unsigned char deviceAddress);
void receiveString(unsigned char deviceAddress, char *buf, int bufSize);
int sendCommand(unsigned char deviceAddress, unsigned char cmdCode);

// Unions
union u_tag {
	char b[4];
	float fval;
} fdata;

// Global Variables
int startHandle;
unsigned int openHandle;
int pi;

// Main program
int main(int argc, char** argv) {
	int device_id;
	int command;
	
	if (argc != 3) {
    fprintf(stderr, "Usage getdevval <device id> <command>\n");
    exit(1);
  }
  
  if (0 == sscanf(argv[1], "%d", &device_id)) {
    fprintf(stderr, "Invalid device id \"%s\"\n", argv[1]);
    exit(1);
  }
  
  if(0 == sscanf(argv[2], "%d", &command)) {
    fprintf(stderr, "Invalid command \"%s\"\n", argv[2]);
    exit(1);
  }
  
  // Initialize/setup i2c
  int pi = pigpio_start(NULL, NULL);
  if(pi != 0) {
    fprintf(stderr, "Failed to connect to pigpiod, is it running?\nTo start it, run the following command.\n\tsudo pigpiod\n");
    exit(1);
  }
  
  // Open bit banging I2C on standard I2C pins
  int h = bb_i2c_open(pi, SDA_PIN, SDL_PIN, BAUD);
  if (h) {
  	switch(h) {
  		case PI_BAD_USER_GPIO:
  			fprintf(stderr, "Failed to open bit bang port (PI_BAD_USER_GPIO)\n");
  			break;
  		case PI_BAD_I2C_BAUD:
  			fprintf(stderr, "Failed to open bit bang port (PI_BAD_I2C_BAUD)\n");
  			break;
  		case PI_GPIO_IN_USE:
  			fprintf(stderr, "Failed to open bit bang port (PI_GPIO_IN_USE)\n");
  			break;
  		case -2011:	// pigif_unconnected_pi
  			fprintf(stderr, "Failed to open bit bang port (pigif_unconnected_pi)\n");
  			break;
  		default:
  			fprintf(stderr, "Failed to open bit bang port (%d)\n", h);
  			break;
  	}
    exit(1);
  }
  
	sendCommand(device_id, command);
	
	char buf[9];
	int bufSize = sizeof(buf)/sizeof(buf[0]);
	int ledState = 0;
	float temperature = 0.0;
	float percentHumidity = 0.0;
	float lightLevel = 0.0;
	
  switch(command) {
  	case 0x00:	// LED OFF
  		break;
  	case 0x01:	// LED ON
  		break;
  	case 0x02:	// LED FLASH
  		break;
  	case 0x03:	// LED STATE
		  ledState = receiveInt(device_id);
		  printf("%d", ledState);
  		break;
  	case 0x04:	// TEMPERATURE
		  temperature = receiveFloat(device_id);
		  printf("%1.2f", temperature);
  		break;
  	case 0x08:	// HUMIDITY
		  percentHumidity = receiveFloat(device_id);
		  printf("%1.2f", percentHumidity);
  		break;
  	case 0x0C:	// LIGHT LEVEL
		  lightLevel = receiveFloat(device_id);
		  printf("%1.2f", lightLevel);
  		break;
  	case 0x10:	// MODEL
  		receiveString(device_id, buf, bufSize);
  		printf("%s", buf);
  		break;
  	case 0x18:	// VERSION
  		receiveString(device_id, buf, bufSize);
  		printf("%s", buf);
  		break;
  	default:
  		break;
  }
  
  // We want to stop the pigpio connection when the program exits
  bb_i2c_close(pi, SDA_PIN);
  pigpio_stop(pi);
  	
  exit(0);
}

char receiveChar(unsigned char deviceAddress) {
	char ReadBuf[256];
	char CmdBuf[] = {4, deviceAddress,  // Chip address
                       2, 6, 1, 3,    // Write command to sensor
                       0 // EOL
                       };
                       
	int bytesRead = bb_i2c_zip(pi, SDA_PIN, CmdBuf, sizeof(CmdBuf), ReadBuf, sizeof(ReadBuf));
	
	if(bytesRead < 0) {
		fprintf(stderr, "Failed to read from device (Device=%d / Error Code=%d\n", deviceAddress, bytesRead);
	}
	
  char retVal = 0x00;
  
  if (bytesRead == 1) {
  	retVal=ReadBuf[0];
  }
  
	usleep(10000);
  return retVal;
}

float receiveFloat(unsigned char deviceAddress) {	
	fdata.b[3] = 0;
	fdata.b[2] = 0;
	fdata.b[1] = 0;
	fdata.b[0] = 0;
	
	fdata.b[3] = receiveChar(deviceAddress);
	fdata.b[2] = receiveChar(deviceAddress);
	fdata.b[1] = receiveChar(deviceAddress);
	fdata.b[0] = receiveChar(deviceAddress);
	
	return fdata.fval;
}

int receiveInt(unsigned char deviceAddress) {
  return (int)receiveChar(deviceAddress);
}

void receiveString(unsigned char deviceAddress, char *buf, int bufSize) {
  int charCount=0;
  
  for(charCount=0; charCount<bufSize-1; charCount++) {
		buf[charCount] = receiveChar(deviceAddress);
	}
	buf[bufSize-1] = 0x00;
}

int sendCommand(unsigned char deviceAddress, unsigned char cmdCode) {
	/*
			unsigned SDA: 0-31 (as used in a prior call to bbI2COpen)
			char	*inBuf: pointer to the concatenated I2C commands, see below
			unsigned inLen: size of command buffer
			char	*outBuf: pointer to buffer to hold returned data
			unsigned outLen: size of output buffer
	*/
	
	char ReadBuf[1];
	char CmdBuf[] = {4, deviceAddress,  // Chip address
                       2, 7, 1, cmdCode, 3,    // Write command to sensor
                       0 // EOL
                       };
                       
	int bytesRead = bb_i2c_zip(pi, SDA_PIN, CmdBuf, sizeof(CmdBuf), ReadBuf, sizeof(ReadBuf));
	
	if(bytesRead < 0) {
		fprintf(stderr, "Failed to send command (Device=%d / Command=%d / Error Code=%d\n", deviceAddress, cmdCode, bytesRead);
	}
	
	usleep(10000);
	return 0;
}