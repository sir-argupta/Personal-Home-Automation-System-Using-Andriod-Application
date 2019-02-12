 
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"
#include <FirebaseArduino.h>

#define FIREBASE_HOST "homevision-696a8.firebaseio.com"                     //Your Firebase Project URL goes here without "http:" , "\" and "/"
#define FIREBASE_AUTH "7sinwzauHRQECIrbjOMAy2dGnn983R4cxnSv3zF2"       //Your Firebase Database Secret goes here


#define Relay1            D6
int val1;
#define Relay2            D5
int val2;
#define Relay3            D2
int val3;
#define Relay4            D1
int val4;
//DHT11 for reading temperature and humidity value
#define DHTPIN            D7

//GAS Sensor Code
#define gaspin            A0
 float gasvalue;
int sensorThres=400;
 #define buzzer           D8
/************ Necessary declaration for DHT11 ******************/
#define DHTTYPE           DHT11     // DHT 11 

DHT dht(DHTPIN, DHTTYPE);
uint32_t delayMS;

#define WLAN_SSID       "yurekablack"             // Your SSID
#define WLAN_PASS       "little27"        // Your password

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "visionhome0123"            // Replace it with your username
#define AIO_KEY         "1544060eb31d4fe5a44d4b7f13193ba2"   // Replace with your Project Auth Key

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/


// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe Light1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME"/feeds/Relay1"); // FeedName
Adafruit_MQTT_Subscribe Light2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay2");
Adafruit_MQTT_Subscribe Light3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay3");
Adafruit_MQTT_Subscribe Light4 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Relay4");

Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");

void MQTT_connect();

void setup() {
  Serial.begin(9600);

  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(gaspin, INPUT);

  digitalWrite(Relay1,HIGH);
  digitalWrite(Relay2,HIGH);
  digitalWrite(Relay3,HIGH);
  digitalWrite(Relay4,HIGH);

  
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  
  //Setting up DHT sensor
  dht.begin();
 

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&Light1);
  mqtt.subscribe(&Light3);
  mqtt.subscribe(&Light2);
  mqtt.subscribe(&Light4);

  Firebase.begin(FIREBASE_HOST,FIREBASE_AUTH);
  Firebase.setInt("S1",0);                     //Here the varialbe"S1","S2","S3" and "S4" needs to be the one which is used in our Firebase and MIT App Inventor
  Firebase.setInt("S2",0); 
  Firebase.setInt("S3",0); 
  Firebase.setInt("S4",0); 
}

void firebasereconnect()
{
  Serial.println("Trying to reconnect");
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  }

uint32_t x = 0;
void loop() {
 
  MQTT_connect();
  

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &Light1) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light1.lastread);
      int Light1_State = atoi((char *)Light1.lastread);
      digitalWrite(Relay1, Light1_State);
      
    }
    if (subscription == &Light2) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light2.lastread);
      int Light2_State = atoi((char *)Light2.lastread);
      digitalWrite(Relay2, Light2_State);
    }
    if (subscription == &Light3) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light3.lastread);
      int Light3_State = atoi((char *)Light3.lastread);
      digitalWrite(Relay3, Light3_State);
    }
    if (subscription == &Light4) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light4.lastread);
      int Light4_State = atoi((char *)Light4.lastread);
      digitalWrite(Relay4, Light4_State);
      
    }
  }

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  if (! Humidity.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  if (! Temperature.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  gasvalue = analogRead(gaspin);
  Serial.print("The Gas Value is: ");
  Serial.println(gasvalue);
  
 if (Firebase.failed())
      {
      Serial.print("setting number failed:");
      Serial.println(Firebase.error());
      firebasereconnect();
      return;
      }
      
  val1=Firebase.getString("S1").toInt();                                        //Reading the value of the varialble Status from the firebase
  
  if(val1==1)                                                             // If, the Status is 1, turn on the Relay1
     {
      digitalWrite(Relay1,LOW);
      Serial.println("light 1 OFF");
    }
    else if(val1==0)                                                      // If, the Status is 0, turn Off the Relay1
    {                                      
      digitalWrite(Relay1,HIGH);
      Serial.println("light 1 ON");
    }

  val2=Firebase.getString("S2").toInt();                                        //Reading the value of the varialble Status from the firebase
  
  if(val2==1)                                                             // If, the Status is 1, turn on the Relay2
     {
      digitalWrite(Relay2,LOW);
      Serial.println("light 2 OFF");
    }
    else if(val2==0)                                                      // If, the Status is 0, turn Off the Relay2
    {                                      
      digitalWrite(Relay2,HIGH);
      Serial.println("light 2 ON");
    }

   val3=Firebase.getString("S3").toInt();                                        //Reading the value of the varialble Status from the firebase
  
  if(val3==1)                                                             // If, the Status is 1, turn on the Relay3
     {
      digitalWrite(Relay3,LOW);
      Serial.println("light 3 OFF");
    }
    else if(val3==0)                                                      // If, the Status is 0, turn Off the Relay3
    {                                      
      digitalWrite(Relay3,HIGH);
      Serial.println("light 3 ON");
    }

   val4=Firebase.getString("S4").toInt();                                        //Reading the value of the varialble Status from the firebase
  
  if(val4==1)                                                             // If, the Status is 1, turn on the Relay4
     {
      digitalWrite(Relay4,LOW);
      Serial.println("light 4 OFF");
    }
    else if(val4==0)                                                      // If, the Status is 0, turn Off the Relay4
    {                                      
      digitalWrite(Relay4,HIGH);
      Serial.println("light 4 ON");
    }    

 Firebase.setFloat ("Temp",t);
  Firebase.setFloat ("Humidity",h);
  Firebase.setFloat ("Gas",gasvalue);
  if (gasvalue > sensorThres ){
  digitalWrite(buzzer,HIGH);
}
  
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
  
}
