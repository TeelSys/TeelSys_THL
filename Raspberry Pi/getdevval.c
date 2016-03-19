#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#define CMD_GET_TEMPERATURE 1
#define CMD_GET_HUMIDITY 2
#define CMD_SET_LIGHT 3
#define CMD_SET_LED_ON 4
#define CMD_SET_LED_OFF 5
#define CMD_SET_LED_FLASH 6
#define CMD_GET_MODEL 250
#define CMD_GET_VERSION 251
#define CMD_GET_HELLO_WORLD 254

// The PiWeather board i2c address
#define ADDRESS 0x23

// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
char *devName = "/dev/i2c-0";
int file;
int devices[128];
int sensorDevices[128];

union u_tag {
	char b[4];
	float fval;
} fdata;

float computeHeatIndex(float temperature, float percentHumidity, int isFahrenheit);
float convertCtoF(float c);
float convertFtoC(float f);
void displayConnectedI2cDevices();
void dumpDeviceInfo(int deviceAddress);
void findAllI2cDevices();
void findI2cBus();
void findSensors();
float receiveFloat();
int receiveInt();
void receiveString(char *str, int bufSize);
int sendCommand(int deviceAddress, int cmdCode);

int main(int argc, char** argv) {
	int device_id;
	int command;
	
	if (argc != 3) {
    printf("Usage getdevval <device id> <command>\n");
    exit(1);
  }
  
  if (0 == sscanf(argv[1], "%d", &device_id)) {
    fprintf(stderr, "Invalid device id \"%s\"\n", argv[2]);
    exit(1);
  }
  
  if (0 == sscanf(argv[2], "%d", &command)) {
    fprintf(stderr, "Invalid command \"%s\"\n", argv[3]);
    exit(1);
  }
  
	// Look for the I2C bus device

  // printf("I2C: Connecting\n");
	findI2cBus();
  
  // Find Devices
  //findAllI2cDevices();
  
  // Display devices found (Simlar to i2cdetect -y 0)
  //displayConnectedI2cDevices();
  
  //dumpDeviceInfo(0x23);
  
  
	char buf[9];
	int bufSize = sizeof(buf)/sizeof(buf[0]);
	
  switch(command) {
  	case 0x00:	// LED OFF
		  sendCommand(device_id, command);
  		break;
  	case 0x01:	// LED ON
		  sendCommand(device_id, command);
  		break;
  	case 0x02:	// LED FLASH
		  sendCommand(device_id, command);
  		break;
  	case 0x03:	// LED STATE
		  sendCommand(device_id, command);
		  int ledState = receiveInt();
		  printf("%d", ledState);
  		break;
  	case 0x04:	// TEMPERATURE
		  sendCommand(device_id, command);
		  float temperature = receiveFloat();
		  printf("%1.2f", temperature);
  		break;
  	case 0x08:	// HUMIDITY
		  sendCommand(device_id, command);
		  float percentHumidity = receiveFloat();
		  printf("%1.2f", percentHumidity);
  		break;
  	case 0x0C:	// LIGHT LEVEL
		  sendCommand(device_id, command);
		  float lightLevel = receiveFloat();
		  printf("%1.2f", lightLevel);
  		break;
  	case 0x10:	// MODEL
		  sendCommand(device_id, command);
  		receiveString(buf, bufSize);
  		printf("%s", buf);
  		break;
  	case 0x18:	// VERSION
		  sendCommand(device_id, command);
  		receiveString(buf, bufSize);
  		printf("%s", buf);
  		break;
  	default:
  		break;
  }

  close(file);
  return (EXIT_SUCCESS);
}

float computeHeatIndex(float temperature, float percentHumidity, int isFahrenheit) {
  // Using both Rothfusz and Steadman's equations
  // http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
  float hi;

  if (isFahrenheit==0)
    temperature = convertCtoF(temperature);

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

  if (hi > 79) {
    hi = -42.379 +
             2.04901523 * temperature +
            10.14333127 * percentHumidity +
            -0.22475541 * temperature*percentHumidity +
            -0.00683783 * pow(temperature, 2) +
            -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature*pow(percentHumidity, 2) +
            -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  return isFahrenheit ? hi : convertFtoC(hi);
}

float convertCtoF(float c) {
  return c * (9.0/5.0) + 32;
}

float convertFtoC(float f) {
  return (f - 32) * (5.0/9.0);
}

void displayConnectedI2cDevices() {
	int idx=0;
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f");
	for(idx=0; idx<=0x7F; idx++) {
		if(idx%16==0) {
			printf("\n%d0:",idx/16);
		}
		if(idx>0x07 && idx<0x78) {
			if(devices[idx]>0) {
				if(devices[idx]==-9) {
					printf(" UU");
				}
				else {
					printf(" %02x", idx);
				}
			}
			else {
				printf(" --");
			}
		}
		else {
				printf("   ");
		}
  }
  printf("\n");
}

void dumpDeviceInfo(int deviceAddress) {
	int i=0;
	
	sendCommand(deviceAddress, 0x03);
	
	for(i=0x03; i<0x20; i++) {
		int val = receiveInt();
		
		printf("0x%02x: 0x%02x (%d)\t%c\n", i, val, val, val);
	}
}

void findAllI2cDevices() {
	int idx=0;
  for(idx=0; idx<=0x7F; idx++) {
  	int device=0;
  	
  	if(idx>0x07 && idx<0x78) {
	  	if (ioctl(file, I2C_SLAVE, idx) < 0) {
	  		if(errno == EBUSY) {
	  			device = -9;
	  		}
	  		else {
		  		device = -1;
		  	}
	  	}
	  	else {
	  		char buf[1];
	  		if(read(file, buf, 1) == 1 && buf[0] >= 0) {
	  			device = idx;
	  		}
	  	}
  	}
  	
  	devices[idx] = device;
  }
}

void findI2cBus() {
	if ((file = open(devName, O_RDWR)) < 0) {
  	devName = "/dev/i2c-1";
  	if ((file = open(devName, O_RDWR)) < 0) {
	    //fprintf(stderr, "I2C: Failed to access %d\n", devName);
	    exit(1);
	  }
  }
  
  //printf("Found I2C bus at %s\n", devName);
}

void findSensors() {
	char *sensorType="TeelSys Data and Light Sensor";
	char buf[256];
	int idx=0;
	int sensorIdx=0;
	// sensorDevices
	// devices
	
	// Clear the sensorDevices array
	for(idx=0; idx<128; idx++) {
		sensorDevices[idx] = 0;
	}
	
  for(idx=0x08; idx<=0x78; idx++) {
  	int device=0;
  	
  	if(devices[idx]==idx) {
  		if(sendCommand(0x22, CMD_GET_MODEL)==1) {
  			int bufSize = sizeof(buf)/sizeof(buf[0]);
  			receiveString(buf, bufSize);
  			if(strlen(sensorType)==strlen(buf) && strcmp(sensorType, buf)==0) {
  				sensorDevices[sensorIdx]=devices[idx];
  				sensorIdx++;
  				printf("Found Sensor at: 0x%02x\n", devices[idx]);
  			}
  		}
  	}
  }
}

void receiveString(char *buf, int bufSize) {
  int charCount=0;
  
  for(charCount=0; charCount<bufSize-1; charCount++) {
		buf[charCount] = receiveChar();
	}
	buf[bufSize-1] = 0x00;
}

int receiveChar() {
  char buf[1];
  char retVal = 0x00;
  
  if (read(file, buf, 1) == 1) {
  	retVal=buf[0];
  }
  
	usleep(10000);
  return retVal;
}

float receiveFloat() {	
	fdata.b[3] = 0;
	fdata.b[2] = 0;
	fdata.b[1] = 0;
	fdata.b[0] = 0;
	
	fdata.b[3] = receiveChar();
	fdata.b[2] = receiveChar();
	fdata.b[1] = receiveChar();
	fdata.b[0] = receiveChar();
	
	return fdata.fval;
	//return (float)fdata.b[3];
}

int receiveInt() {
  return (int)receiveChar();
}

int sendCommand(int deviceAddress, int cmdCode) {
	int retVal = 0;
	unsigned char cmd[16];
	cmd[0] = cmdCode;
	
	if (ioctl(file, I2C_SLAVE, deviceAddress) < 0) {
    //fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", deviceAddress);
    exit(1);
  }
  
  if (write(file, cmd, 1) == 1) {
  	// As we are not talking to direct hardware but a microcontroller we
    // need to wait a short while so that it can respond.
    //
    // 1ms seems to be enough but it depends on what workload it has
    usleep(10000);
    retVal = 1;
  }
  
  return retVal;
}
