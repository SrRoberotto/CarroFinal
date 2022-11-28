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
// struct SENSOR_PINS
// {
//   int pinTrg;  
//   int pinEcho;
//   float distanceCm;    
// };


//Vetor com os motores e a pinagem configurada
std::vector<MOTOR_PINS> motorPins = 
{
  {4, 16, 17}, //RIGHT_MOTOR Pins (EnA, CMD1, CMD2)
  {21, 18, 19}, //LEFT_MOTOR  Pins (EnB, CMD3, CMD4)
};

//Vetor com os motores e a pinagem configurada
// std::vector<SENSOR_PINS> sensorPins = 
// {
//   {21, 16, 0}, //RIGHT_MOTOR Pins (EnA, CMD1, CMD2)
//   {21, 18, 0}, //LEFT_MOTOR  Pins (EnB, CMD3, CMD4)
// };


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

// #define SOUND_SPEED 0.034
// #define TEMPERATURE_CORRECTION 28

const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int PWMSpeedChannel = 4;

const int ledPin = 1;

const char* ssid_AP     = "SpyCarWifi";
const char* password_AP = "12345678";
const char* ssid_CLIENT = "Sem nome";
const char* password_CLIENT = "x1x1x1m4VEIA";
// const int   wifi_Type = SOFT_AP;
const int   wifi_Type = WIFI_CLIENT; 

// Set static IP
IPAddress AP_LOCAL_IP(10, 0, 1, 125);
IPAddress AP_GATEWAY_IP(10, 0, 1, 1);
IPAddress AP_NETWORK_MASK(255, 255, 0, 0);

int wifiClientsCount = 0;

  
AsyncWebServer server(80);  
AsyncWebSocket wsCarInput("/CarInput");

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
    <script>
      var webSocketCarInputUrl = "ws:\/\/10.0.1.125/CarInput";
      var webSocketCameraUrl   = "ws:\/\/10.0.1.168/Camera";
      var webSocketCamInputUrl = "ws:\/\/10.0.1.168/CamInput";     
    </script>
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
    .arrows {
      font-size:30px;
      color:red;
    }
    td.button {
      background-color:black;
      border-radius:25%;
      box-shadow: 5px 5px #888888;
    }
    td.button:active {
      transform: translate(5px,5px);
      box-shadow: none; 
    }

    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 15px;
      border-radius: 5px;
      background: #d3d3d3;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }
  
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    </style>
  </head>

  <body class="noselect" align="center" style="background-color:white">
    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr>
        <img id="cameraImage" src="" style="width:400px;height:250px"></td>
      </tr> 
      <tr>
        <td style="text-align:left"><b>Light:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="255" value="0" class="slider" id="Light" oninput='sendCamButtonInput("Light",value)'>
          </div>
        </td>   
      </tr>
      <tr>
        <td style="text-align:left"><b>Pan:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Pan" oninput='sendCamButtonInput("Pan",value)'>
          </div>
        </td>
      </tr> 
      <tr>
        <td style="text-align:left"><b>Tilt:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Tilt" oninput='sendCamButtonInput("Tilt",value)'>
          </div>
        </td>   
      </tr>
      <tr>
        <td style="text-align:left"><b>Speed:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="255" value="150" class="slider" id="Speed" oninput='sendCarButtonInput("Speed",value)'>
          </div>
        </td>
      </tr>
      <tr>
        <td></td>
        <!-- td class="button" onmousedown='sendCarButtonInput("MoveCar","1")' onmouseup='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8679;</span></td -->
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","1")' ontouchend='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8679;</span></td>
        <td></td>
      </tr>
      <tr>
        <!-- td class="button" onmousedown='sendCarButtonInput("MoveCar","3")' onmouseup='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8678;</span></td -->
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","3")' ontouchend='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8678;</span></td>
        <!-- td class="button" onmousedown='sendCarButtonInput("MoveCar","0")' onmouseup='sendCarButtonInput("MoveCar","0")></td -->  
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","0")' ontouchend='sendCarButtonInput("MoveCar","0")></td>
        <!-- td class="button" onmousedown='sendCarButtonInput("MoveCar","4")' onmouseup='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8680;</span></td -->
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","4")' ontouchend='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8680;</span></td>
      </tr>
      <tr>
        <td></td>
        <!-- td class="button" onmousedown='sendCarButtonInput("MoveCar","2")' onmouseup='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8681;</span></td -->
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","2")' ontouchend='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8681;</span></td>
        <td></td>
      </tr>
      <tr/><tr/>
    </table>
  
    <script>
      var websocketCamera;
      var websocketCamInput;
      var websocketCarInput;     
      
      function initCameraWebSocket() 
      {
        websocketCamera = new WebSocket(webSocketCameraUrl);
        websocketCamera.binaryType = 'blob';
        websocketCamera.onopen    = function(event){};
        websocketCamera.onclose   = function(event){setTimeout(initCameraWebSocket, 2000);};
        websocketCamera.onmessage = function(event)
        {
          var imageId = document.getElementById("cameraImage");
          imageId.src = URL.createObjectURL(event.data);
        };
      }
      
      function initCamInputWebSocket() 
      {
        websocketCamInput = new WebSocket(webSocketCamInputUrl);
        websocketCamInput.onopen    = function(event)
        {
          sendCamButtonInput("Light", document.getElementById("Light").value);
          sendCamButtonInput("Pan", document.getElementById("Pan").value);
          sendCamButtonInput("Tilt", document.getElementById("Tilt").value);                    
        };
        websocketCamInput.onclose   = function(event){setTimeout(initCamInputWebSocket, 2000);};
        websocketCamInput.onmessage = function(event){};        
      }

      function initCarInputWebSocket() 
      {
        websocketCarInput = new WebSocket(webSocketCarInputUrl);
        websocketCarInput.onopen    = function(event)
        {
          sendCarButtonInput("Speed", document.getElementById("Speed").value);                    
        };
        websocketCarInput.onclose   = function(event){setTimeout(initCarInputWebSocket, 2000);};
        websocketCarInput.onmessage = function(event){};        
      }

      function initWebSocket() 
      {
        initCameraWebSocket ();
        initCamInputWebSocket();
        initCarInputWebSocket();
      }

      function sendCarButtonInput(key, value) 
      {
        var data = key + "," + value;
        websocketCarInput.send(data);
        console.log(data);
      }

      function sendCamButtonInput(key, value) 
      {
        var data = key + "," + value;
        websocketCamInput.send(data);
        console.log(data);
      }

      window.onload = initWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
      });      

      // window.onload = initWebSocket;
      // document.getElementById("mainTable").addEventListener("mouseup", function(event){
      //   event.preventDefault()
      // });
    </script>
  </body>    
</html>
)HTMLHOMEPAGE";


String sendDistance(){
  return "25|";
}



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
    if (!WiFi.config(AP_LOCAL_IP, AP_GATEWAY_IP, AP_NETWORK_MASK)) {
      Serial.println("STA Failed to configure");
    }
    Serial.println(WiFi.localIP());
  } 
  if (type == SOFT_AP){
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid_AP, password_AP);
    delay(2000);
    if (!WiFi.softAPConfig(AP_LOCAL_IP, AP_GATEWAY_IP, AP_NETWORK_MASK)) {
      Serial.println("AP Config Failed");
    } else {
      IPAddress IP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(IP);
    }
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

  // Access-Control-Allow-Origin: *
  server.addHandler(&wsCarInput);

  server.on("/distanceCm", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", sendDistance().c_str());
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
    // request->addInterestingHeader(const String &name)
    // request->send_P(200, "text/plain", sendDistance().c_str());
  });

  server.begin();
  Serial.print("HTTP server started with max queued: ");
  Serial.println(WS_MAX_QUEUED_MESSAGES);
  Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());

  digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  delay(100);   
  digitalWrite(ledPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2000);                       // wait for a second
  digitalWrite(ledPin, LOW);    // turn the LED off by making the voltage LOW
  delay(100);    

}


void loop() 
{
  wsCarInput.cleanupClients();
  if (WiFi.softAPgetStationNum()!=wifiClientsCount){
    wifiClientsCount = WiFi.softAPgetStationNum();
    Serial.print("Stations connected now: ");
    Serial.println(wifiClientsCount);
  }
}

