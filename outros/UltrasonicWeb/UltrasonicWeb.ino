
#include <Config.h>

#include <ESPmDNS.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
//#include <Adafruit_Sensor.h>


#include "AsyncSonarLib.h"

const int trigPinA = 26;
const int echoPinA = 34;
const int trigPinB = 27;
const int echoPinB = 35;

#define SOUND_SPEED 0.034

const char* ssid = "Sem nome";
const char* password = "x1x1x1m4VEIA";
AsyncWebServer server(80);

void PingRecieved(AsyncSonar&);
void TimeOut0(AsyncSonar&);
void TimeOut1(AsyncSonar&);

AsyncSonar sonarA0(A0, PingRecieved, TimeOut0);
AsyncSonar sonarA1(A1, PingRecieved, TimeOut1);

String readSensor(int trigPin, int echoPin) {
  long duration;
  float distanceCm;

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distanceCm = duration * SOUND_SPEED / 2;
  Serial.println("Distance:");
  Serial.println(distanceCm);
  return String(distanceCm);
}

String readDistance() {
  sonarA0.Update(&sonarA1);
	sonarA1.Update(&sonarA0);

  String resposta = "";
  resposta += readSensor(trigPinA, echoPinA);
  resposta += "|";
  resposta += readSensor(trigPinB, echoPinB);
  return resposta;
}

// ping complete callback
// (this example shows how to access sonar from callback)
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping");
	Serial.print(&sonar == &sonarA1);  // print '0' if sonar A0, '1' if sonar A1
	Serial.print(": ");
	Serial.println(sonar.GetMeasureMM());
}

// timeout callbacks
// (this example shows how to use different callbacks for each sensor)
void TimeOut0(AsyncSonar& sonar)
{
	Serial.println("TimeOut0");
}

void TimeOut1(AsyncSonar& sonar)
{
	Serial.println("TimeOut1");
}


const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <style>
      html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
      }
      h2 { font-size: 3.0rem; }
      p { font-size: 3.0rem; }
      .units { font-size: 1.2rem; }
      .dht-labels{
        font-size: 1.5rem;
        vertical-align:middle;
        padding-bottom: 15px;
      }
    </style>
  </head>
  <body>
    <h2>Ultrasonic sensor reading on server</h2>
    
    <p>
    <i class="fa fa-road" style="color:#00add6;"></i> 
      <span class="dht-labels">Distance A</span> 
      <span id="DistanceA">%DISTANCEA%</span>
      <sup class="units">Cm</sup>
    </p>
    <p>
    <i class="fa fa-road" style="color:#00add6;"></i> 
      <span class="dht-labels">Distance B</span> 
      <span id="DistanceB">%DISTANCEB%</span>
      <sup class="units">Cm</sup>
    </p>
  </body>
  <script>  
  setInterval(function ( ) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        const values = this.responseText.split("|");
        document.getElementById("DistanceA").innerHTML = values[0];
        document.getElementById("DistanceB").innerHTML = values[1];
      }
    };
    xhttp.open("GET", "/distanceCm", true);
    xhttp.send();
  }, 1000 ) ;

  </script>
</html>)rawliteral";

String processor(const String& var) {
  //Serial.println(var);
  if (var == "DISTANCE") {
    return readDistance();
  }
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(trigPinA, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPinA, INPUT);
  pinMode(trigPinB, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPinB, INPUT);

  sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA1.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(500);  // start in 500ms



  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("connected to wifi ....");
  if (!MDNS.begin("esp32")) {
    Serial.println("Error starting mDNS");
    return;
  }

  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/distanceCm", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", readDistance().c_str());
  });


  // Start server
  server.begin();
}

void loop() {

}