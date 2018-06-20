#include "sketch_apr02a.h"

WiFiManager wifiManager;
ESP8266WebServer server(80);
#define  nomeHost  "AquaSmart"
String  etatGpio[4] = {"off","off","off","off"};
bool  flag_button = false;
const uint8_t GPIOUT[4] = {D5,D6,1,3};  //pinos que vão mudar o estado tomadas
const uint8_t GPIIN[4] = {D0,D2,D7,D8};
unsigned long time=0;

// const uint8_t GPIOIN[4] = {D0,D1,D2,D3};  //pinos que vão verificar o estado das tomadas
//==============================================================
//                  SETUP
//==============================================================
int pinCLK = D1; //saida
int pinSH = D3;  //saida
int pinDADO = D4;//Entrada
int INFOPINOUT[4]={0,0,0,0};//acumulador de informações

void setup(void){

  //definir pinos de saida
  pinMode(GPIOUT[0],OUTPUT);
  pinMode(GPIOUT[1],OUTPUT);
  pinMode(GPIOUT[2],OUTPUT);
  pinMode(GPIOUT[3],OUTPUT);
  pinMode(pinCLK,OUTPUT);
  pinMode(pinSH,OUTPUT);

  //definir pinos de entrada
  pinMode(pinDADO,INPUT);
  pinMode(GPIIN[0],INPUT);
  pinMode(GPIIN[1],INPUT);
  pinMode(GPIIN[2],INPUT);
  pinMode(GPIIN[3],INPUT);


  digitalWrite(pinSH,HIGH); //da enable
  digitalWrite(pinCLK,HIGH); //da enable

  delay(1000);
  // //serial.begin(115200);

  //Initialize File System
  SPIFFS.begin();
  //serial.println("File System Initialized");

  //Nome do hostname
  wifiManager.setHostname(nomeHost);

  //Initialize
  handleAPorSTA();
  ////serial.print("meu IP:");
  ////serial.println(WiFi.localIP());

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

  for(int i=0;i<5;i++){
    if(digitalRead(GPIIN[i])!=INFOPINOUT[i]){
      INFOPINOUT[i]=!INFOPINOUT[i];
    }
  }
  server.begin();
}

//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();
  if((millis()-time) >= 50){//requisita a cada 1 segundo
    for(int i=0;i<5;i++){
      if(digitalRead(GPIIN[i])!=INFOPINOUT[i]){
        INFOPINOUT[i]=!INFOPINOUT[i];
        digitalWrite(GPIOUT[i],INFOPINOUT[i]);
        flag_button = true;
      }
    }
    time += millis();
  }
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
  ////serial.println(message);
}

void handleAPorSTA(void){
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect(nomeHost,"123456789")){
    //serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    wdt_reset();
  }
}

void configModeCallback (WiFiManager *wifiManager){
  //serial.println("Entered config mode");
  //serial.println(WiFi.softAPIP());
  //serial.println(wifiManager->getConfigPortalSSID());
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
  //serial.println("Configuração do WiFi limpa");
  wdt_reset();
}

void apagaFiles(void){
  SPIFFS.format();
  //serial.println("Apagou geral");
  wdt_reset();
}

void alloff(void){
  //serial.println("Apagando tomadas ");
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
  //serial.println("Setando tomadas ");
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
  //serial.print("Update GPIO: ");
  //serial.print(GPIOUT[gpio]);
  //serial.print(" -> ");
  //serial.println(statePin);
  digitalWrite(GPIOUT[gpio],statePin == "true" ? HIGH : LOW);
}

// int pinCLK = D1;//saida
// int pinSH = D3;//saida
// int pinDADO = D4;
void verificatomada(void){
  int8_t nT = 4,aux;
  String result = "";

  if(flag_button){
    for(int i=0;i<5;i++){
      result += INFOPINOUT[i];
    }
    flag_button = false;
    //serial.print("medida nova -> ");
    //serial.println(result);
    server.send ( 200, "text/html", result);
    return;
  }
  digitalWrite(pinSH,LOW); //clock em 0
  digitalWrite(pinSH,HIGH); //clock em 0

  for(int8_t i = 0; i < nT; i++){
    digitalWrite(pinCLK,LOW); //clock em 0
    digitalWrite(pinCLK,HIGH); //clock em 1
    aux = digitalRead(pinDADO);
    result += aux;
    INFOPINOUT[i] = aux;
  }
  //serial.print("medida nova -> ");
  //serial.println(result);
  server.send ( 200, "text/html", result);
}
