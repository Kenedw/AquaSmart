#include "sketch_apr02a.h"

WiFiManager wifiManager;
ESP8266WebServer server(80);
#define  nomeHost  "AquaSmart"
String  etatGpio[4] = {"off","off","off","off"};
const uint8_t GPIOUT[4] = {D5,D6,D7,D8};  //pinos que vão mudar o estado tomadas
// const uint8_t GPIOIN[4] = {D0,D1,D2,D3};  //pinos que vão verificar o estado das tomadas
//==============================================================
//                  SETUP
//==============================================================
int pinCLK = D1; //saida
int pinSH = D3;  //saida
int pinDADO = D4;//Entrada
void setup(void){

  //definir pinos de saida
  pinMode(GPIOUT[0],OUTPUT);
  pinMode(GPIOUT[1],OUTPUT);
  pinMode(GPIOUT[2],OUTPUT);
  pinMode(GPIOUT[3],OUTPUT);

  //definir pinos de entrada
  pinMode(pinCLK,OUTPUT);
  pinMode(pinSH,OUTPUT);
  pinMode(pinDADO,INPUT);

  digitalWrite(pinSH,HIGH); //da enable
  digitalWrite(pinCLK,HIGH); //da enable

  delay(1000);
  Serial.begin(115200);

  //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");

  //Nome do hostname
  wifiManager.setHostname(nomeHost);

  //Initialize
  handleAPorSTA();
  Serial.print("meu IP:");
  Serial.println(WiFi.localIP());

  //Initialize Webserver
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
  server.on("/",handleRoot);
  server.on("/callback",verificatomada);
  server.on("/alloff",alloff);
  server.on("/setTomadas",setTomadas);
  server.on("/resetwifi",resetwifi);  // Chamada dos métodos de configuração
  server.on("/apagadado",apagaFiles);
  server.on("/mainPage", HTTP_OPTIONS, []() {
    server.sendHeader("Location", "/mainPage",true);   //Redirect to our html web page
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    server.send(302, "text/plain", "" );
  });

  server.on("/mainPage", HTTP_GET, []() {
    String response ;
    // ... some code to prepare the response data...
    server.sendHeader("Location", "/mainPage",true);   //Redirect to our html web page
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    server.send(302, "text/plain", response.c_str() );
  });
  server.begin();
}

//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();
}

void handleRoot(void){
  server.sendHeader("Location", "/index.html",true);   //Redirect to our html web page
  server.send(302, "text/plane","");
}

void handleWebRequests(void){
  if(loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

void handleAPorSTA(void){
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect(nomeHost,"123456789")){
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    wdt_reset();
  }
}

void configModeCallback (WiFiManager *wifiManager){
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(wifiManager->getConfigPortalSSID());
}

bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "text/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".svg")) dataType = "image/svg";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";

  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}

void resetwifi(void){
  wifiManager.resetSettings();
  Serial.println("Configuração do WiFi limpa");
  wdt_reset();
}

void apagaFiles(void){
  SPIFFS.format();
  Serial.println("Apagou geral");
  wdt_reset();
}

void alloff(void){
  Serial.println("Apagando tomadas ");
  String aux = "";
  String aux2 = "t";
  for(int i=1;i<=4;i++){
    aux = aux2 + i;
    if( server.hasArg(aux) ){
      updateGPIO(i-1,"false");
    }
  }
}

void setTomadas(void){
  Serial.println("Setando tomadas ");
  String aux = "";
  String aux2 = "t";

  for(int i=1;i<=4;i++){
    aux = aux2 + i;
    if( server.hasArg(aux) ){
      updateGPIO(i-1,server.arg(aux));
    }
  }

  server.send ( 200, "text/html", "/mainPage.html" );
}

void updateGPIO(int gpio, String statePin){
  Serial.print("Update GPIO: ");
  Serial.print(GPIOUT[gpio]);
  Serial.print(" -> ");
  Serial.println(statePin);
  digitalWrite(GPIOUT[gpio],statePin == "true" ? HIGH : LOW);
}

// int pinCLK = D1;//saida
// int pinSH = D3;//saida
// int pinDADO = D4;
void verificatomada(void){
  int8_t nT = 4;
  String result = "";
  digitalWrite(pinSH,LOW); //clock em 0
  digitalWrite(pinSH,HIGH); //clock em 0

  for(int8_t i = 0; i < nT; i++){
    digitalWrite(pinCLK,LOW); //clock em 0
    digitalWrite(pinCLK,HIGH); //clock em 1
    result += digitalRead(pinDADO);
  }
  // digitalWrite(pinCLK,LOW); //clock em 0
  // // Serial.print(digitalRead(pinDADO));
  // result += digitalRead(pinDADO);
  //
  // digitalWrite(pinCLK,HIGH); //clock em 0
  // digitalWrite(pinCLK,LOW); //clock em 0
  // // Serial.print(digitalRead(pinDADO));
  // result += digitalRead(pinDADO);
  //
  // digitalWrite(pinCLK,HIGH); //clock em 0
  // digitalWrite(pinCLK,LOW); //clock em 0
  // // Serial.print(digitalRead(pinDADO));
  // result += digitalRead(pinDADO);
  //
  // digitalWrite(pinCLK,HIGH); //clock em 0
  // digitalWrite(pinCLK,LOW); //clock em 0
  // // Serial.print(digitalRead(pinDADO));
  // result += digitalRead(pinDADO);

  // digitalWrite(pinCLK,HIGH); //clock em 0
  // digitalWrite(pinCLK,LOW); //clock em 0
  // // Serial.print(digitalRead(pinDADO));
  // result += digitalRead(pinDADO);
  //
  // digitalWrite(pinCLK,HIGH); //clock em 0
  // digitalWrite(pinCLK,LOW); //clock em 0
  // // Serial.print(digitalRead(pinDADO));
  // result += digitalRead(pinDADO);
  //
  // digitalWrite(pinCLK,HIGH); //clock em 0
  // digitalWrite(pinCLK,LOW); //clock em 0
  // // Serial.print(digitalRead(pinDADO));
  // result += digitalRead(pinDADO);
  //
  // digitalWrite(pinCLK,HIGH); //clock em 0
  // digitalWrite(pinCLK,LOW); //clock em 0
  // // Serial.println(digitalRead(pinDADO));
  // result += digitalRead(pinDADO);
  //
  Serial.print("medida nova -> ");
  Serial.println(result);
  server.send ( 200, "text/html", result);

}
