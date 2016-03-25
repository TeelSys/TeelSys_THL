// Uses DHT from Rob instead of Adafruit
// http://playground.arduino.cc/Main/DHTLib
// http://playground.arduino.cc/Main/DHT11Lib


#include <TinyWireS.h>
#include <dht11.h>

#define SLAVE_ADDRESS 0x10

#define PIN_DHT 4
#define PIN_PHOTORESISTOR A3
#define PIN_LED 1

unsigned long previousMillis = 0;
#define interval 2500

#define bufferSz 32
byte dataBuffer[bufferSz] = { 32 };
uint8_t bufferIdx = 0;
boolean firstByteRead = false;

dht11 DHT11;

// Union used to convert float to byte array
union u_tag {
  byte b[4];
  float fval;
} fdata;

void setup() {
  pinMode(PIN_DHT, INPUT);
  pinMode(PIN_PHOTORESISTOR, INPUT);
  pinMode(PIN_LED, OUTPUT);

  digitalWrite(PIN_LED, HIGH);

  // initialize i2c as slave
  TinyWireS.begin(SLAVE_ADDRESS);

  // define callbacks for i2c communication
  TinyWireS.onReceive(receiveData);
  TinyWireS.onRequest(sendData);

  // Initialize dataBuffer
  for (int i = 0; i < bufferSz; i++) {
    dataBuffer[i] = 0xFF;
  }
  // Set LED to blink on each loop
  dataBuffer[3] = 2;
  // Load Model Info
  // T  S  0  0  0  0  0  1
  // 54 53 30 30 30 30 30 31
  String storeText = F("TS000001");
  bufferIdx = 0x10;
  for (int i = 0; i < storeText.length(); i++) {
    dataBuffer[bufferIdx] = storeText[i];
    bufferIdx++;
  }
  // Load Version Info
  // 0  0  0  0  0  0  0  3
  // 30 30 30 30 30 30 30 33
  storeText = F("00000003");
  bufferIdx = 0x18;
  for (int i = 0; i < storeText.length(); i++) {
    dataBuffer[bufferIdx] = storeText[i];
    bufferIdx++;
  }
}

void loop() {
  TinyWireS_stop_check();
  
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (dataBuffer[3] == 2) {
      digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    }

    ReadDHT();
    ReadLightLevel();
  }
}

// callback for received data
void receiveData(uint8_t byteCount) {
  if (byteCount != 1)
  {
    // Sanity-check
    return;
  }

  while (TinyWireS.available()) {
    bufferIdx = TinyWireS.receive();
    firstByteRead = false;

    SetLedStatus();
  }
}

// callback for sending data
void sendData() {
  if (firstByteRead) {
    bufferIdx++;
  }

  firstByteRead = true;

  if(bufferIdx < 0 || bufferIdx >= bufferSz) {
    TinyWireS.send(0xFF);
    return;
  }

  TinyWireS.send(dataBuffer[bufferIdx]);
}

void ReadDHT() {
  int chk = DHT11.read(PIN_DHT);

  if(!chk==DHTLIB_OK) {
    return;
  }
  
  float humidity = (float)DHT11.humidity;
  float temperatureCelsius = (float)DHT11.temperature;
  
  SaveFloatToBuffer(0x04, temperatureCelsius);
  SaveFloatToBuffer(0x08, humidity);
}

void ReadLightLevel() {
  int photocellReading = analogRead(PIN_PHOTORESISTOR);
  float lightReading = ((float)photocellReading / 1023.0) * 100.0;
  SaveFloatToBuffer(0x0C, lightReading);
}

void SaveFloatToBuffer(uint8_t bufIdx, float val) { 
  dataBuffer[bufIdx] = 0;
  dataBuffer[bufIdx + 1] = 0;
  dataBuffer[bufIdx + 2] = 0;
  dataBuffer[bufIdx + 3] = 0;
  
  fdata.fval = val;

  dataBuffer[bufIdx] = fdata.b[3];
  dataBuffer[bufIdx + 1] = fdata.b[2];
  dataBuffer[bufIdx + 2] = fdata.b[1];
  dataBuffer[bufIdx + 3] = fdata.b[0];
  
  //dataBuffer[bufIdx] = (int)val;
}

void SetLedStatus() {
  if (bufferIdx > 2)
    return;

  dataBuffer[3] = 2;
  if (bufferIdx < 2) {
    digitalWrite(PIN_LED, bufferIdx);
    dataBuffer[3] = 0;
    if (digitalRead(PIN_LED) == HIGH) {
      dataBuffer[3] = 1;
    }
  }
}
