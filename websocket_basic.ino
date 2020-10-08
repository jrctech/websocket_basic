#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>


#ifndef STASSID
#define STASSID "Flia Ruzzante G"
#define STAPSK  "n0pc-3xif-aa4a"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const int led=2;

void handleRoot(){
  String s = "";
  s += "<head>";
  s += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  s += "<script>";
  s += "var Socket;";
  s += "function init(){";
  s += "Socket = new WebSocket('ws://' + window.location.hostname + ':81/');";
  s += "Socket.onmessage = function(event){document.getElementById('rxConsole').value += event.data;}"; 
  s += "}";
  s += "function SendText(){";
  s += "Socket.send(document.getElementById('txBar').value);";
  s += "document.getElementById('txBar').value = '';";
  s += "}";
  s += "function SendBrightness(){";
  s += "Socket.send('#' + document.getElementById('brightness').value);";
  s += "}";
  s += "</script>";
  s += "</head>";
  s += "<body onload = 'init()'>";
  s += "<div class=\"container\">";
  s += "<h1>Websockets Example</h1>";
  s += "<h3>This text is sent by ESP12E</h3>";
  s += "<textarea id='rxConsole' rows= '10' cols='50' readonly></textarea>";
  s += "<h3>Write here to send text to the ESP12E</h3>";
  s += "<input type='text' id='txBar' onkeydown='if(event.keyCode == 13) SendText();'>";
  s += "<h3>Move the slider to change brightness of the builtin led</h3>";
  s += "<input type='range' min = '0' max = '1023' value = '512' id='brightness' oninput='SendBrightness()'>";
  s += "</div>";
  s += "</body>";
  server.send(200, "text/html", s);
}

void handleNotFound(){
    server.send(404, "text/html", "The requested resource was not found!");
}

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led,1);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(led,0);

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}



void loop() {
  webSocket.loop();
  server.handleClient();

  if (Serial.available()>0){
    char c[] = {(char)Serial.read()};
    webSocket.broadcastTXT(c, sizeof(c));
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
    if (type == WStype_TEXT){
      if (payload[0]=='#'){
        uint16_t brightness = (uint16_t)strtol((const char *) &payload[1], NULL, 10);
        brightness = 1024 - brightness;
        analogWrite(led, brightness);
        Serial.print("Brightness: ");
        Serial.println(brightness);
      }
      else{
        Serial.print("Received: ");
        for (int i=0; i< length; i++)  
          Serial.print((char)payload[i]);
        Serial.println();
      }
    }
}
