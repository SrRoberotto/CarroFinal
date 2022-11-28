//#include "esp_camera.h"
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
// #include "AsyncSonarLib.h"
#include <ESPAsyncWebServer.h>
#include <iostream>
#include <sstream>
#include "HC_SR04.h"


//Define a "classe" motor
struct MOTOR_PINS
{
  int pinEn;  //PWM
  int pinCMD1;
  int pinCMD2;    
};

//Vetor com os motores e a pinagem configurada
std::vector<MOTOR_PINS> motorPins = 
{
  {4, 16, 17}, //RIGHT_MOTOR Pins (EnA, CMD1, CMD2)
  {21, 18, 19}, //LEFT_MOTOR  Pins (EnB, CMD3, CMD4)
};


//Define a "classe" sensores
struct SENSOR_PINS
{
  int pinTrg;  
  int pinEcho;
  int distanceCm;    
};

//Vetor com os sensores e a pinagem configurada
std::vector<SENSOR_PINS> sensores = 
{
  {26, 27, 0}, //Trig, Echo, distancia
  //{21, 18, 0},
};


// ENUMS
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

// CONFIGURAÇÕES DO SENSOR DE DISTÂNCIA
// #define SOUND_SPEED 0.034
// #define TEMPERATURE_CORRECTION 28
// #define TRIG_PIN_A 26
// #define ECHO_PIN_A 27
// #define ECHO_INT_A 27 //interruption
#define MAX_DIST 200
#define MAX_TIME 15000
#define DISTANCIA_SEGURA 30 //em cm

// INICIALIZA SENSOR DE DISTANCIA
HC_SR04 sensor0(sensores[0].pinTrg, sensores[0].pinEcho, sensores[0].pinEcho, MAX_DIST);


// CONFIGURAÇÕES DO WIFI
const char* ssid_AP     = "SpyCarWifi";
const char* password_AP = "12345678";
const char* ssid_CLIENT = "Sem nome";
const char* password_CLIENT = "x1x1x1m4VEIA";
const int   wifi_Type = SOFT_AP;
// const int   wifi_Type = WIFI_CLIENT; 

// Set static IP
IPAddress AP_LOCAL_IP(10, 0, 1, 125);
IPAddress AP_GATEWAY_IP(10, 0, 1, 1);
IPAddress AP_NETWORK_MASK(255, 255, 0, 0);

int wifiClientsCount = 0;

AsyncWebServer server(80);  
AsyncWebSocket wsCarInput("/CarInput");

//OUTRAS CONFIGURAÇÕES
const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int PWMSpeedChannel = 4;
char buffer[40];

// CONFIGURAÇÃO DO LED INTERNO
const int ledPin = 1;

const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
    <script>
      var webSocketCarInputUrl  = "ws:\/\/10.0.1.125\/CarInput";
      var webSocketCameraUrl    = "ws:\/\/10.0.1.168\/Camera";
      var webSocketCamInputUrl  = "ws:\/\/10.0.1.168\/CamInput";
      var sensorReadings        = "http:\/\/10.0.1.125\/distanceCm"
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

      .units {
          font-size: 2rem;
          vertical-align:bottom;
      }
      .dht-labels{
          font-size: 2rem;
          vertical-align:middle;
          
      }
      .dht-values{
          font-size: 2rem;
          vertical-align:middle;
          font-weight: bold;
      }
    </style>
  </head>

  <body class="noselect" align="center" style="background-color:white">
    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr>
        <td colspan="3">
          <img id="cameraImage" src="" style="width:400px;height:250px"></td>
        </td>
      </tr> 
      <tr>
        <td style="text-align:left"><b>Farol:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="255" value="0" class="slider" id="Light" oninput='sendCamButtonInput("Light",value)'>
          </div>
        </td>   
      </tr>
      <tr>
        <td style="text-align:left"><b>Movimento horizontal:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Pan" oninput='sendCamButtonInput("Pan",value)'>
          </div>
        </td>
      </tr> 
      <tr>
        <td style="text-align:left"><b>Movimento vertical:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="180" value="90" class="slider" id="Tilt" oninput='sendCamButtonInput("Tilt",value)'>
          </div>
        </td>   
      </tr>
      <tr>
        <td style="text-align:left"><b>Velocidade:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="255" value="150" class="slider" id="Speed" oninput='sendCarButtonInput("Speed",value)'>
          </div>
        </td>
      </tr>
      <tr>
        <td colspan="3">
          <i class="fa fa-road" style="color:#00add6;"></i> 
          <span class="dht-labels">Distancia frontal: </span> 
          <span id="DistanceA" class="dht-values">%D1%</span>
          <span class="units">Cm</span>
        </td>
      </tr>
      <!----- Versao desktop -----
      <tr>
        <td></td>
        <td class="button" onmousedown='sendCarButtonInput("MoveCar","1")' onmouseup='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8679;</span></td>
        <td></td>
      </tr>
      <tr>
        <td class="button" onmousedown='sendCarButtonInput("MoveCar","3")' onmouseup='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8678;</span></td>
        <td class="button" onmousedown='sendCarButtonInput("MoveCar","0")' onmouseup='sendCarButtonInput("MoveCar","0")'></td>    
        <td class="button" onmousedown='sendCarButtonInput("MoveCar","4")' onmouseup='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8680;</span></td>
      </tr>
      <tr>
        <td></td>
        <td class="button" onmousedown='sendCarButtonInput("MoveCar","2")' onmouseup='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8681;</span></td>
        <td></td>
      </tr>
      ---- Fim da versao desktop ---->

      <!-- Versao mobile ---->
      <tr>
        <td></td>
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","1")' ontouchend='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8679;</span></td>
        <td></td>
      </tr>
      <tr>
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","3")' ontouchend='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8678;</span></td>
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","0")' ontouchend='sendCarButtonInput("MoveCar","0")'></td>
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","4")' ontouchend='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8680;</span></td>
      </tr>
      <tr>
        <td></td>
        <td class="button" ontouchstart='sendCarButtonInput("MoveCar","2")' ontouchend='sendCarButtonInput("MoveCar","0")'><span class="arrows" >&#8681;</span></td>
        <td></td>
      </tr>
      <!---- Fim da versao mobile ---->
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

      // Versao mobile
      window.onload = initWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
      });      

      // Versao desktop
      // window.onload = initWebSocket;
      // document.getElementById("mainTable").addEventListener("mouseup", function(event){
      //   event.preventDefault()
      // });

      setInterval(function ( ) {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              const values = this.responseText.split("|");
              document.getElementById("DistanceA").innerHTML = values[0];
              //document.getElementById("DistanceB").innerHTML = values[1];
            }
          };
          xhttp.open("GET", sensorReadings, true);
          xhttp.send();
        }, 1000 );
    </script>
  </body>    
</html>
)HTMLHOMEPAGE";

// Verifica a distancia atual nos sensores
void checkDistance(){
  if(sensor0.isFinished()){
    sensores[0].distanceCm = sensor0.getRange();
    sensor0.start();    
  }
}

// Verifica se o carrinho pode ir na direcao informada
bool motorCanRun(int motorDirection, std::vector<SENSOR_PINS> s){
  if (motorDirection == FORWARD && s[0].distanceCm < DISTANCIA_SEGURA){
    Serial.println("Motor nao pode rodar nesse sentido");
    return false;
  } else {
    return true;
  }
}

// Envia a resposta HTTP das distâncias medidas
String sendDistance(std::vector<SENSOR_PINS> s){
  if (s[0].distanceCm > DISTANCIA_SEGURA){  
    sprintf(buffer, "%d|", s[0].distanceCm);
  } else{
    sprintf(buffer, ">%d<|", s[0].distanceCm);
  }
  return buffer;
}

// Comandos físico dos motores
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

// Comandos lógicos dos motores e direções
void moveCar(int inputValue)
{
  Serial.printf("Got value as %d\n", inputValue);  
  switch(inputValue)
  {

    case UP:
      if (motorCanRun(FORWARD, sensores)){
        rotateMotor(RIGHT_MOTOR, FORWARD);
        rotateMotor(LEFT_MOTOR, FORWARD);     
      }             
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

// Página HTML inicial
void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

// Erro 404
void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}

// Comandos via SOCKET
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
  //pinMode(ledPin, OUTPUT);
      
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

void setupServer(){
  // INICIALIZA SERVER HTTP
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
  wsCarInput.onEvent(onCarInputWebSocketEvent);

  server.addHandler(&wsCarInput);

  server.on("/distanceCm", HTTP_GET, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", sendDistance(sensores).c_str());
    response->addHeader("Access-Control-Allow-Origin","*");
    request->send(response);
    // request->send_P(200, "text/plain", sendDistance().c_str());
  });

  server.begin();

  Serial.print("HTTP server started with max queued: ");
  Serial.println(WS_MAX_QUEUED_MESSAGES);
}

// Programa principal - SETUP
void setup(void) 
{
  Serial.begin(115200);
  sensor0.begin();
  setUpMotorPinModes();
  setupWIFI(wifi_Type);
  setupServer();

  Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n", ESP.getPsramSize(), ESP.getFreePsram());
}

// Programa principal - LOOP
void loop() 
{
  wsCarInput.cleanupClients();
  checkDistance();
  if (WiFi.softAPgetStationNum()!=wifiClientsCount){
    wifiClientsCount = WiFi.softAPgetStationNum();
    Serial.print("Stations connected now: ");
    Serial.println(wifiClientsCount);
  }


}

