#include <NewPing.h>
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <WebServer.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TimeLib.h>


#define TRIGGER_PIN 21
#define ECHO_PIN 13
#define MAX_DISTANCE 400

/**
    Definition of all the global variable
*/

WebServer server(80);
WiFiMulti WiFiMulti;
WebSocketsServer webSocket = WebSocketsServer(81);
int distance = 0;
String value;
String message;
int nbOfCollision = 0;
bool messageUpdated = false;
int totalNbOfCollisions = 0;
String id;
String name;
String phoneNumber;
String location;
String timeOfPreviousCollision;

const int numReadings = 5; // Number of readings to average
int readings[numReadings]; // Array to store the readings
int indexPing = 0; // Index for the current reading
int total = 0; // Running total of the readings

unsigned long previousCollisionTime = 0;
unsigned long totalCollisionTime = 0;
float averageTimeBetweenCollisions = 0;
unsigned long currentMillis = 0;
float average = 0;

/**
    Definition of all the html template to use
*/

char html_template[] PROGMEM = R"=====(
  <html>
    <head>
      <style>
        body {
          font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
          background-color: #1f1f1f;
          color: #fff;
          margin: 0;
          padding: 20px;
        }
        h1 {
          color: #f0f0f0;
          text-align: center;
          margin-bottom: 30px;
          font-size: 36px;
        }
        .container {
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
        }
        .distance {
          font-size: 80px;
          margin-bottom: 20px;
          color: #4af7ff;
          text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.3);
        }
        .additional-message {
          font-size: 24px;
          color: #ff3b3b;
          text-align: center;
          font-style: italic;
        }
        .submit-btn {
          display: inline-block;
          padding: 12px 24px;
          margin-top: 40px;
          text-decoration: none;
          color: #fff;
          background-color: #4af7ff;
          border: 2px solid #4af7ff;
          border-radius: 6px;
          font-size: 18px;
          transition: all 0.3s ease-in-out;
        }
        .submit-btn:hover {
          background-color: #307e8a;
          border-color: #307e8a;
        }
      </style>
    </head>
    <body>
      <h1>Real Time Distance</h1>
      <div class="container">
        <div class="distance" id="distance_container">Ready for Action!</div>
        <div class="additional-message" id="additional_message_container">Awaiting Command...</div>
      </div>
      <a href='/collisionHistory' class="submit-btn">View Collision Log</a>
      <script>
        const socket = new WebSocket("ws://" + location.host + ":81");
        socket.onopen = function(e) { console.log("[socket] socket.onopen "); };
        socket.onerror = function(e) { console.log("[socket] socket.onerror "); };
        socket.onmessage = function(e) {
          try {
            const data = JSON.parse(e.data);
            const distance = data.distance;
            const message = data.additionalMessage;
            document.getElementById("distance_container").innerHTML = "Distance: " + distance + " cm";
            document.getElementById("additional_message_container").innerHTML = message;
          } catch (error) {
            console.error("Error parsing JSON:", error);
          }
        };
      </script>
    </body>
  </html>
)=====";


  char html_userBoard[] PROGMEM = R"=====(
  <html>
    <head>
      <title>User Board</title>
      <style>
        body {
          font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
          background-color: #1f1f1f;
          color: #fff;
          margin: 0;
          padding: 20px;
        }
        h1 {
          color: #f0f0f0;
          text-align: center;
          margin-bottom: 30px;
          font-size: 36px;
        }
        .container {
          text-align: center;
          margin-top: 20px;
        }
        .button {
          display: inline-block;
          padding: 12px 24px;
          margin-bottom: 10px;
          text-decoration: none;
          color: #fff;
          background-color: #4af7ff;
          border: 2px solid #4af7ff;
          border-radius: 6px;
          font-size: 18px;
          transition: all 0.3s ease-in-out;
        }
        .button:hover {
          background-color: #307e8a;
          border-color: #307e8a;
        }
        footer {
          position: absolute;
          bottom: 10px;
          width: 100%;
          text-align: center;
          color: #888;
        }
      </style>
    </head>
    <body>
      <h1>Welcome to the User Board</h1>
      <div class="container">
        <a href='/distancePage' class="button">Real-time Distance to Object</a>
        <a href='/collisionHistory' class="button">Collision History</a>
        <a href='/collisionStats' class="button">Statistics about collisions</a>
      </div>
      <footer>
        <p>&copy; 2023 User Board</p>
      </footer>
    </body>
  </html>
)=====";

const char html_collisionHistory[] PROGMEM = R"=====(
<html>
<head>
    <title>Collision History</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #1f1f1f;
            color: #fff;
            margin: 0;
            padding: 20px;
            text-align: center;
        }

        h1 {
            color: #f0f0f0;
            font-size: 36px;
            margin-bottom: 20px;
        }

        .container {
            display: flex;
            justify-content: center;
            align-items: center;
            flex-direction: column;
            margin-top: 50px;
        }

        .collisions {
            background-color: #2c2c2c;
            border-radius: 8px;
            box-shadow: 0px 0px 10px rgba(255, 255, 255, 0.1);
            padding: 20px;
            margin-bottom: 20px;
            width: 80%;
            max-width: 600px;
        }

        .number {
            font-size: 24px;
            color: #fff;
            margin-bottom: 10px;
        }

        .message {
            font-size: 18px;
            color: #ff6565;
        }

        .back-button {
            display: inline-block;
            padding: 12px 24px;
            margin-top: 30px;
            text-decoration: none;
            color: #fff;
            background-color: #4af7ff;
            border: 2px solid #4af7ff;
            border-radius: 6px;
            font-size: 18px;
            transition: all 0.3s ease-in-out;
        }

        .back-button:hover {
            background-color: #307e8a;
            border-color: #307e8a;
        }

        footer {
            position: absolute;
            bottom: 10px;
            width: 100%;
            text-align: center;
            color: #888;
        }
    </style>
</head>
<body>
    <h1>User's Collision History</h1>
    <div class="container">
        <div class="collisions">
            <p class="number">Number of collisions since usage:</p>
            <p class="message">XX</p>
        </div>
        <a href="/" class="back-button">Go Back</a>
    </div>
    <footer>
        <p>&copy; 2023 Analytical Page</p>
    </footer>
</body>
</html>
)=====";


const char html_collisionStats[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Statistical Analysis</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: #1f1f1f;
      color: #fff;
      margin: 0;
      padding: 20px;
      text-align: center;
    }
    .analysis-container {
      border: 1px solid #3a3a3a;
      background-color: #2c2c2c;
      padding: 20px;
      margin-bottom: 20px;
      border-radius: 8px;
      box-shadow: 0px 0px 10px rgba(255, 255, 255, 0.1);
    }
    h2 {
      margin-bottom: 10px;
      color: #f0f0f0;
    }
    .user-data {
      margin-bottom: 20px;
    }
    .user-field {
      margin-bottom: 10px;
      color: #ccc;
    }
    .last-collision {
      margin-bottom: 20px;
    }
    .collision-field {
      margin-bottom: 10px;
      color: #ccc;
    }
  </style>
</head>
<body>

  <div class="analysis-container">
    <h2>User Data Analysis</h2>
    <div class="user-data">
      <div class="user-field">
        <strong>User ID:</strong> INSERT_USER_ID
      </div>
      <div class="user-field">
        <strong>Name:</strong> INSERT_USER_NAME
      </div>
      <div class="user-field">
        <strong>Phone Number:</strong> INSERT_USER_PHONE_NUMBER
      </div>
    </div>
  </div>

  <div class="analysis-container">
    <h2>Last Collision Data Analysis</h2>
    <div class="last-collision">
      <div class="collision-field">
        <strong>Total number of Collisions:</strong> INSERT_COLLISION_COUNT
      </div>
      <div class="collision-field">
        <strong>Time of Previous Collision:</strong> INSERT_COLLISION_TIME
      </div>
      <div class="collision-field">
        <strong>Average Time Between Collisions:</strong> INSERT_AVG_TIME_BETWEEN
      </div>
      <div class="collision-field">
        <strong>Location of Last Collision:</strong> INSERT_COLLISION_LOCATION
      </div>
    </div>
  </div>

</body>
</html>
)=====";


/**
    Definition of the Websocket implementation
*/

#define USE_SERIAL Serial1

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		USE_SERIAL.printf("%02X ", *src);
		src++;
	}
	USE_SERIAL.printf("\n");
}



void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
    }

}


/**
    Definition of all web methods to return the html template
*/


void handleRoot(){
  server.send(200, "text/html", html_userBoard);
}

void distancePage(){
  server.send(200, "text/html", html_template);
}

char html_with_collision[sizeof(html_collisionHistory) + 10];

void collisionHistory(){
  String collisionCount = String(nbOfCollision);

  snprintf(html_with_collision, sizeof(html_with_collision), "%s", html_collisionHistory);
  char* placeholder = strstr(html_with_collision, "XX"); // Find the placeholder
   if (placeholder != NULL) {
        snprintf(placeholder, sizeof(html_with_collision) - (placeholder - html_with_collision), "%s", collisionCount.c_str());
    }
  server.send(200, "text/html", html_with_collision);
}

void collisionStats(){
  String webpage = String(html_collisionStats);

  // Replace placeholders with actual data
  webpage.replace("INSERT_USER_ID", id);
  webpage.replace("INSERT_USER_NAME", name);
  webpage.replace("INSERT_USER_PHONE_NUMBER", phoneNumber);
  webpage.replace("INSERT_COLLISION_COUNT", (String) totalNbOfCollisions);
  webpage.replace("INSERT_COLLISION_TIME", (String) previousCollisionTime);
  webpage.replace("INSERT_AVG_TIME_BETWEEN", (String) averageTimeBetweenCollisions);
  webpage.replace("INSERT_COLLISION_LOCATION", location);

  server.send(200, "text/html", webpage);
}
/**
    Definition of the wifi methods
*/


// Replace with your network credentials
const char* ssid = "VIDEOTRON3303";
const char* password = "4W9YN7JCUMFHP";

const int buzzer = 12;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

void initWiFi() {

 
  WiFiMulti.addAP(ssid,password);
  Serial.print("Connecting to WiFi ..");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("Connected to wifi: ");
  Serial.println(WiFi.localIP());
}

/**
    Method to read the distance from sensor
*/

void readFromSensor(){
  delay(150);
  distance = sonar.ping_cm(); // Take a new reading 
 
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println("cm");
}

/**
   Methods to send real time distance and warning through the WebSocket
*/

void sendDataToWeb(String theMessage){
   
   StaticJsonDocument<100> doc;
    value = (String) distance + " cm from the object";
    doc["distance"] = value;
    Serial.println(theMessage);
    doc["additionalMessage"] = theMessage;
    String jsonString;
    serializeJson(doc, jsonString);
    webSocket.broadcastTXT(jsonString);
}

/**
   Methods to send collision data to Flask API server to a database
*/
void sendDataToServer(){
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("inside send data");
    HTTPClient http;
    DynamicJsonDocument doc(512);
    JsonObject collisions = doc.createNestedObject("collisions");
    collisions["numberOfCollisions"] = totalNbOfCollisions + nbOfCollision;
    collisions["timeOfPreviousCollision"] = "2023-11-18T12:00:00";
    collisions["averageTimeBetweenCollisions"] = averageTimeBetweenCollisions;
    collisions["locationOfLastCollision"] = "MontrealQuebec";

        // Serialize JSON to a string
  String jsonData;
  serializeJson(doc, jsonData);

    http.begin("http://192.168.0.180:5000/api/data");
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonData);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Error in sending POST request: ");
    Serial.println(httpResponseCode);
  String error = http.errorToString(httpResponseCode);
  Serial.println(error);
  }

  http.end();
}
  }

/**
   Methods to receive collision data from Flask API server from a database
*/
void getDataFromServer(){
   if (WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    http.begin("http://192.168.0.180:5000/api/data");
    int httpResponseCode = http.GET();

    if(httpResponseCode > 0){
      if (httpResponseCode == HTTP_CODE_OK){
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        deserializeJson(doc,payload);

        // extract the user data
        JsonObject userData = doc["user"];
          id = userData["id"].as<String>();
          Serial.println("User id: ");
          Serial.println(id);
          name = userData["name"].as<String>();
          Serial.println("User name: ");
          Serial.println(name);
          phoneNumber = userData["phoneNumber"].as<String>();
          Serial.println("User phone: ");
          Serial.println(phoneNumber);
        
        // extract last collision data
        JsonObject lastCollision = doc["last_collision"];
        totalNbOfCollisions = lastCollision["numberOfCollisions"].as<int>();
        Serial.println("Total collision: ");
        Serial.println(totalNbOfCollisions);
        timeOfPreviousCollision = lastCollision["timeOfPreviousCollision"].as<String>();
        Serial.print("Time of previous ocllision: ");
        Serial.println(timeOfPreviousCollision);
        average = lastCollision["averageTimeBetweenCollisions"].as<float>();
        Serial.print("average: ");
        Serial.println(averageTimeBetweenCollisions);
        location = lastCollision["locationOfLastCollision"].as<String>();
        Serial.println("location: ");
        Serial.println(location);

      }
    } else {
       Serial.print("Error in sending GET request: ");
    Serial.println(httpResponseCode);
  String error = http.errorToString(httpResponseCode);
  Serial.println(error);
    }
   }
}
/**
   Methods to output a buzzer sound depending on the distance and send the data through websocket
*/
void outputWarning(){
  if(distance < 75 && distance >= 40){
    sendDataToWeb("Object detected!");
    tone(buzzer, 1000);
    delay(100);
    noTone(buzzer);
    delay(100);
    tone(buzzer,1000);
    delay(100);
    noTone(buzzer);
    delay(1000);
    
  }
  else if(distance < 40 && distance >=20){
    sendDataToWeb("Low warning!");
    tone(buzzer,1000);
    delay(500);
    noTone(buzzer);
    delay(500);
  }
  else if(distance < 20 && distance >= 10){
    sendDataToWeb("High warning!");
    tone(buzzer,1000);
    delay(250);
    noTone(buzzer);
    delay(250);
  }
  else if(distance < 10 && distance >= 4){
    sendDataToWeb("Caution imminent collision!");
    tone(buzzer,1000);
    delay(100);
    noTone(buzzer);
    delay(100);
  }
  // when collision detected, calculate the times, average, collisions, and send data to db, fetch new data from db
  else if(distance < 4 && distance >= 0){
    nbOfCollision++;
    currentMillis = millis();
    unsigned long timeSincePreviousCollision = currentMillis - previousCollisionTime;

    previousCollisionTime = currentMillis;
    totalCollisionTime += timeSincePreviousCollision;
    averageTimeBetweenCollisions = (float)totalCollisionTime / totalNbOfCollisions;

    sendDataToWeb("Collision detected!");
    tone(buzzer,100);
    delay(1000);
    noTone(buzzer);
    delay(500);
    sendDataToServer();
    getDataFromServer();
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  initWiFi();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

    server.on("/", handleRoot);
    server.on("/distancePage", distancePage);
    server.on("/collisionHistory", collisionHistory);
    server.on("/collisionStats", collisionStats);
    //here the list of headers to be recorded
    const char * headerkeys[] = {"User-Agent", "Cookie"} ;
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
    //ask server to track these headers
    server.collectHeaders(headerkeys, headerkeyssize);
    server.begin();
    Serial.println("HTTP server started");
    // initialize data from database
    getDataFromServer();

}

void loop() {
 webSocket.loop();
 server.handleClient();
  readFromSensor();
  delay(50); // Add a short delay for stability
  outputWarning();
  delay(100); // Another delay after processing
 





 
}