/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-async-web-server-espasyncwebserver-library/
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "SATC IOT";
const char* password = "IOT2021#";

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String header;
String stateLED_R = "OFF";
String stateLED_G = "OFF";
String stateLED_B = "OFF";
const int LED_R = D2;
const int LED_G = D3;
const int LED_B = D5;
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;
#define trigPin D6
#define echoPin D7
#define luz D10
long duration;
float distance;
bool luzA;

float sensor();
void outputState(int output);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="refresh" content="1">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Informações</title>
    <style>
        html{display: inline-block; margin: 0px auto; text-align: center; margin: auto; background-color: #1C1C1C;}
        div{text-align: center;}
        header{height: 100px; color: #D2691E; margin-top: 40px;}
        h4{margin: 10px;}
        .tudo{margin: auto; background-color: #1C1C1C;}
        .conteudo{display: flex; justify-content: space-around;}
        .sensor, .luminus{display: flex;flex-direction: column;justify-content: center; border-style: solid; height: 300px; padding: 10px;margin: auto; width: 45%; background-color: #D2691E; white-space: nowrap;}
    </style>
    <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.2.1/css/bootstrap.min.css" integrity="sha384-GJzZqFGwb1QTTN6wy59ffF1BuGJpLSa9DkKMp0DgiMDm4iYMj70gZWKYbI706tWS" crossorigin="anonymous">
</head>
<body>
    <div class="tudo">
        <header>
            <h1>Informações</h1>        
        </header>
        <div class="conteudo">
            <div class="sensor">
                <h4>Verificação da Distância</h4>
                %DISTANCIA%              
            </div>
            <div class="luminus">
                <h4>Verificação da Luz</h4>
                <div>
                    %ICON%
                </div>                
            </div>
        </div>
    </div>
    <script>function toggleCheckbox(element) {
        var xhr = new XMLHttpRequest();
        if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
        else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
        xhr.send();
      }
      </script>
    <script src="https://code.jquery.com/jquery-3.3.1.slim.min.js" integrity="sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo" crossorigin="anonymous"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.6/umd/popper.min.js" integrity="sha384-wHAiFfRlMFy6i5SRaxvfOCifBUQy1xHdJ/yoi7FRNXMRBu5WHdZYu1hA6ZOblgut" crossorigin="anonymous"></script>
    <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.2.1/js/bootstrap.min.js" integrity="sha384-B0UglyR+jN6CkvvICOB2joaf5I4l3gm9GU6Hc1og6Ls7i6U/mkkaduKaBhlAXv9k" crossorigin="anonymous"></script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "DISTANCIA"){
    String distancia = "<h4>Sensor de Distância:"+String(sensor(),2)+"cm</h4>";
    if(sensor() < 10){
      distancia += "<h4 style=\"color: purple\" >Alerta</h4>";
    }
    return distancia;
  }
  
  if(var == "ICON"){
    String icons = "";
    outputState(luz);
    if(!digitalRead(luz)){
      icons = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"50\" height=\"50\" fill=\"currentColor\" class=\"bi bi-lightbulb\" viewBox=\"0 0 16 16\"><path d=\"M2 6a6 6 0 1 1 10.174 4.31c-.203.196-.359.4-.453.619l-.762 1.769A.5.5 0 0 1 10.5 13a.5.5 0 0 1 0 1 .5.5 0 0 1 0 1l-.224.447a1 1 0 0 1-.894.553H6.618a1 1 0 0 1-.894-.553L5.5 15a.5.5 0 0 1 0-1 .5.5 0 0 1 0-1 .5.5 0 0 1-.46-.302l-.761-1.77a1.964 1.964 0 0 0-.453-.618A5.984 5.984 0 0 1 2 6zm6-5a5 5 0 0 0-3.479 8.592c.263.254.514.564.676.941L5.83 12h4.342l.632-1.467c.162-.377.413-.687.676-.941A5 5 0 0 0 8 1z\"/></svg>";
    }else{
      icons = "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"50\" height=\"50\" fill=\"currentColor\" class=\"bi bi-lightbulb-off\" viewBox=\"0 0 16 16\"><path fill-rule=\"evenodd\" d=\"M2.23 4.35A6.004 6.004 0 0 0 2 6c0 1.691.7 3.22 1.826 4.31.203.196.359.4.453.619l.762 1.769A.5.5 0 0 0 5.5 13a.5.5 0 0 0 0 1 .5.5 0 0 0 0 1l.224.447a1 1 0 0 0 .894.553h2.764a1 1 0 0 0 .894-.553L10.5 15a.5.5 0 0 0 0-1 .5.5 0 0 0 0-1 .5.5 0 0 0 .288-.091L9.878 12H5.83l-.632-1.467a2.954 2.954 0 0 0-.676-.941 4.984 4.984 0 0 1-1.455-4.405l-.837-.836zm1.588-2.653.708.707a5 5 0 0 1 7.07 7.07l.707.707a6 6 0 0 0-8.484-8.484zm-2.172-.051a.5.5 0 0 1 .708 0l12 12a.5.5 0 0 1-.708.708l-12-12a.5.5 0 0 1 0-.708z\"/></svg>";
    }
    return icons;
  }

  
  return String();
}

void outputState(int output){
  luzA = digitalRead(output);
  if(luzA == LOW)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  Serial.println("\nLuz:");
  if(luzA == 1)
  {
    Serial.println("Ligado");
  }
  else
  {
    Serial.println("Desligado");
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_R,OUTPUT);
  digitalWrite(LED_R,LOW);
  pinMode(LED_G,OUTPUT);
  digitalWrite(LED_G,LOW);
  pinMode(LED_B,OUTPUT);
  digitalWrite(LED_B,LOW);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(luz, INPUT);
  delay(2000);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
    }
    Serial.print("GPIO: ");
    Serial.print(inputMessage1);
    Serial.print(" - Set to: ");
    Serial.println(inputMessage2);
    request->send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();
}

void loop() {}

float sensor()
{
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW); 
  duration = pulseIn(echoPin, HIGH); 
  distance = duration*0.0171;
  Serial.println("\nDistância(cm):");
  Serial.println(distance);
  if(distance < 10)
  {
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_R, LOW);
    delay(100);
    digitalWrite(LED_R, HIGH);
    delay(100);
  }
  else
  {
    digitalWrite(LED_G,HIGH);
    digitalWrite(LED_R, LOW);
  }
  
  return distance;
}
