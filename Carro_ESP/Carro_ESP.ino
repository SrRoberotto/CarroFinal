//#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
// #include "AsyncSonarLib.h"
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>

//Define a "classe" motor
struct MOTOR_PINS
{
  int pinEn;  //PWM
  int pinCMD1;
  int pinCMD2;    
};

//Define a "classe" sensores
struct SENSOR_PINS
{
  int pinTrg;  
  int pinEcho;
  float distanceCm;    
};


//Vetor com os motores e a pinagem configurada
std::vector<MOTOR_PINS> motorPins = 
{
  {4, 16, 17}, //RIGHT_MOTOR Pins (EnA, CMD1, CMD2)
  {21, 18, 19}, //LEFT_MOTOR  Pins (EnB, CMD3, CMD4)
};

//Vetor com os motores e a pinagem configurada
std::vector<SENSOR_PINS> sensorPins = 
{
  {21, 16, 0}, //RIGHT_MOTOR Pins (EnA, CMD1, CMD2)
  {21, 18, 0}, //LEFT_MOTOR  Pins (EnB, CMD3, CMD4)
};


#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define STOP 0

#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1

#define FORWARD 1
#define BACKWARD -1

#define SOFT_AP 1
#define WIFI_CLIENT 2

#define SOUND_SPEED 0.034
#define TEMPERATURE_CORRECTION 28

const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int PWMSpeedChannel = 4;

const int ledPin = 2;

const char* ssid_AP     = "MyWiFiCar";
const char* password_AP = "12345678";
const char* ssid_CLIENT = "Sem nome";
const char* password_CLIENT = "x1x1x1m4VEIA";
const int   wifi_Type = WIFI_CLIENT;

AsyncWebServer server(80);  
AsyncWebSocket wsCarInput("/CarInput");

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <body align="center" style="background-color:white">
    <H1>SPYCar</H1>
  </body>    
</html>
)HTMLHOMEPAGE";


void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinCMD1, HIGH);
    digitalWrite(motorPins[motorNumber].pinCMD2, LOW);    
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinCMD1, LOW);
    digitalWrite(motorPins[motorNumber].pinCMD2, HIGH);     
  }
  else
  {
    digitalWrite(motorPins[motorNumber].pinCMD1, LOW);
    digitalWrite(motorPins[motorNumber].pinCMD2, LOW);       
  }
}

void moveCar(int inputValue)
{
  Serial.printf("Got value as %d\n", inputValue);  
  switch(inputValue)
  {

    case UP:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);                  
      break;
  
    case DOWN:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case LEFT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      //rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case RIGHT:
      //rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD); 
      break;
 
    case STOP:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;
  
    default:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;
  }
}

void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      moveCar(0);   
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str()); 
        int valueInt = atoi(value.c_str());     
        if (key == "MoveCar")
        {
          moveCar(valueInt);        
        }
        else if (key == "Speed")
        {
          ledcWrite(PWMSpeedChannel, valueInt);
        }             
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}


void setUpMotorPinModes()
{
  //Set up PWM
  ledcSetup(PWMSpeedChannel, PWMFreq, PWMResolution);
  pinMode(ledPin, OUTPUT);
      
  for (int i = 0; i < motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinEn, OUTPUT);    
    pinMode(motorPins[i].pinCMD1, OUTPUT);
    pinMode(motorPins[i].pinCMD2, OUTPUT);  
    /* Attach the PWM Channel to the motor enb Pin */
    ledcAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
  }
  moveCar(STOP);

}

void setupWIFI(int type){
    // Connect to Wi-Fi
  if (type == WIFI_CLIENT){
    WiFi.begin(ssid_CLIENT, password_CLIENT);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi... ");
      Serial.println(ssid_CLIENT);
    }

    Serial.println("connected to wifi ...");
    Serial.println(WiFi.localIP());
  } 
  if (type == SOFT_AP){
    WiFi.softAP(ssid_AP, password_AP);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }
}

void setup(void) 
{
  setUpMotorPinModes();
  Serial.begin(115200);

  setupWIFI(wifi_Type);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
  
  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();
  Serial.print("HTTP server started with max queued: ");
  Serial.println(WS_MAX_QUEUED_MESSAGES);
  Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());

}


void loop() 
{
  wsCarInput.cleanupClients();
}

