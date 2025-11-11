#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include "ExampleFunctions.h"
#include <DHT.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_BMP085.h>

// Firebase Credentials
#define FIREBASE_API_KEY "AIzaSyCdFstFC2m8LqA5x8Bkj8xEWN2Mii0l0-s"
#define FIREBASE_DATABASE_URL "https://iot-rtdb-15a22-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define FIREBASE_USER_EMAIL "example@gmail.com"
#define FIREBASE_USER_PASSWORD "password"

// WiFi Credentials
#define WIFI_SSID "home24"
#define WIFI_PASSWORD "temp4321"

// User functions
void processData(AsyncResult &aResult);

// Authentication
UserAuth user_auth(FIREBASE_API_KEY, FIREBASE_USER_EMAIL, FIREBASE_USER_PASSWORD);

SSL_CLIENT ssl_client;

// Firebase components
FirebaseApp app;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;
AsyncResult dbResult;


#define DHTPIN 4     // You can change this to any suitable GPIO pin
#define DHTTYPE DHT11  // Define the type of DHT sensor (DHT11 in this case)
DHT dht(DHTPIN, DHTTYPE);  // Initialize DHT object

Adafruit_MPU6050 mpu;
sensors_event_t a, g, tempmpu;

BH1750 lightMeter;

Adafruit_BMP085 bmp;

float tempdht,humid,light,accelx,gyrox,press;

void setup() {
  Serial.begin(115200);

  // Initialize the sensors
  dht.begin();
  mpu.begin(0x69);
  lightMeter.begin();
  bmp.begin();

  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  Serial.println("Attempting to connect to primary WiFi: " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to WiFi: " + String(WIFI_SSID));

  // Configure SSL client
  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);

  // Initialize Firebase
  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(FIREBASE_DATABASE_URL);

}

void loop() {

  app.loop();

  // Read dht
  tempdht = dht.readTemperature();
  humid = dht.readHumidity();

  // Read mpu6050
  mpu.getEvent(&a, &g, &tempmpu);
  accelx = a.acceleration.x;
  gyrox = g.gyro.x;

  // Read BH1750
  light = lightMeter.readLightLevel();

  // Read BMP180
  press = bmp.readPressure();

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Humidity: ");
  Serial.print(humid);
  Serial.println(" %");

  Serial.print("DHT Temperature: ");
  Serial.print(tempdht);
  Serial.println(" degC");

  Serial.print("Light: ");
  Serial.print(light);
  Serial.println(" lx");

  Serial.print("Pressure = ");
  Serial.print(press);
  Serial.println(" Pa");

  // Push sensor data to Firebase
  if(app.ready()){
    Database.set<float>(aClient, "/Temperature", tempdht);
    Database.set<float>(aClient, "/Humidity", humid);
    Database.set<float>(aClient, "/AccelerationX", accelx);
    Database.set<float>(aClient, "/GyroscopeX", gyrox);
    Database.set<float>(aClient, "/Light", light);
    Database.set<float>(aClient, "/Pressure", press);
  }
  

}

void processData(AsyncResult &aResult){
  if (!aResult.isResult())
    return;

  if (aResult.isEvent()){
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());
  }

  if (aResult.isDebug()){
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError()){
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available()){
    RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();

  }
}
