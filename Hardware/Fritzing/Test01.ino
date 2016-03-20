#include "DHT.h"

#define PIN_DHT 4
#define PIN_PHOTORESISTOR A3
#define PIN_LED 1

#define debugCode 1

#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)




float humidity = 0;
float temperatureCelsius = 0;
float temperatureFahrenheit = 0;
float heatIndexCelsius = 0;
float heatIndexFahrenheit = 0;
float lightLevelPercent = 0;

int photocellReading = 0;


// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(PIN_DHT, DHTTYPE);

void setup() {
  pinMode(PIN_DHT, INPUT);
  pinMode(PIN_PHOTORESISTOR, INPUT);
  pinMode(PIN_LED, OUTPUT);

  digitalWrite(PIN_LED, LOW);

  if (debugCode) {
    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB
    }
    Serial.println("DHTxx test!");
  }
  dht.begin();
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);
  digitalWrite(PIN_LED, HIGH);
  ReadDHT();
  ReadLightLevel();
  if (debugCode) {
    PrintDebug();
  }
  digitalWrite(PIN_LED, LOW);
}

void ReadDHT() {
  humidity = 0;
  temperatureCelsius = 0;
  temperatureFahrenheit = 0;
  heatIndexCelsius = 0;
  heatIndexFahrenheit = 0;

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  temperatureCelsius = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  temperatureFahrenheit = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperatureCelsius) || isnan(temperatureFahrenheit)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  heatIndexFahrenheit = dht.computeHeatIndex(temperatureFahrenheit, humidity);
  // Compute heat index in Celsius (isFahreheit = false)
  heatIndexCelsius = dht.computeHeatIndex(temperatureCelsius, humidity, false);
}

void ReadLightLevel() {
  photocellReading = analogRead(PIN_PHOTORESISTOR);
  lightLevelPercent = ((float)photocellReading / 1023.0) * 100.0;
}

void PrintDebug() {
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperatureCelsius);
  Serial.print(" *C ");
  Serial.print(temperatureFahrenheit);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(heatIndexCelsius);
  Serial.print(" *C ");
  Serial.print(heatIndexFahrenheit);
  Serial.print(" *F\t");
  Serial.print("Light Level: ");
  Serial.print(photocellReading);     // the raw analog reading
  Serial.print("\t");
  Serial.print(lightLevelPercent);     // the raw analog reading
  Serial.println(" %");
}
