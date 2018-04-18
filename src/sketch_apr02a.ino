#include "sketch_apr02a.h"

WiFiManager wifiManager;
ESP8266WebServer server(80);
String  etatGpio[4] = {"OFF","OFF","OFF","OFF"};
const uint8_t GPIOUT[4] = {D5,D6,D7,D8};  //pinos que vão mudar o estado tomadas
const uint8_t GPIOIN[4] = {D0,D1,D2,D3};  //pinos que vão verificar o estado das tomadas

void setup(void)
{
  for ( int x = 0 ; x < 5 ; x++ )
  {
    pinMode(GPIOUT[x],OUTPUT);
  }

  for ( int x = 0 ; x < 5 ; x++ )
  {
    pinMode(GPIOIN[x],INPUT);
  }

  delay(1000);
  Serial.begin(115200);

  //Initialize File System
  SPIFFS.begin();
  Serial.println("File System Initialized");

  //Initialize
  handleAPorSTA();
  Serial.print("meu IP:");
  Serial.println(WiFi.localIP());

  //Initialize Webserver
  server.on("/",handleRoot);
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
  server.on("/mainPage",HTTP_GET,verificatomada);
  server.on("/mainPage",HTTP_POST,modificatomada);
  server.on("/resetwifi",resetwifi);  // Chamada dos métodos de configuração
  server.on("/apagadado",apagaFiles);
  server.begin();
}

void loop(void)
{
 server.handleClient();
}

void handleRoot(void)
{
  server.sendHeader("Location", "/index.html",true);   //Redirect to our html web page
  server.send(302, "text/plane","");

  String login = server.arg("inputEmail");
  String senha = server.arg("inputPassword");
  Serial.println(login + "--" + senha);
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

void handleAPorSTA(void)
{
  wifiManager.setAPCallback(configModeCallback);
  if(!wifiManager.autoConnect("AquaSmart","123456789"))
  {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    wdt_reset();
    delay(1000);
  }
}

void configModeCallback (WiFiManager *wifiManager)
{
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
  else if(path.endsWith(".js")) dataType = "application/javascript";
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

void resetwifi(void)
{
  wifiManager.resetSettings();
  Serial.println("Configuração do WiFi limpa");
  wdt_reset();
}

void apagaFiles(void)
{
  SPIFFS.format();
  Serial.println("Apagou geral");
  wdt_reset();
}

void modificatomada(void)
{
  if( server.hasArg("T1") )
  {
    updateGPIO(0,server.arg("T1"));
  }
  else if ( server.hasArg("T2") )
  {
    updateGPIO(1,server.arg("T2"));
  }
  else if ( server.hasArg("T3") )
  {
    updateGPIO(2,server.arg("T3"));
  }
  else if ( server.hasArg("T4") )
  {
    updateGPIO(3,server.arg("T4"));
  }
  else
  {
    server.send ( 302, "text/html", "/mainPage.html" );
  }
}

void updateGPIO(int gpio, String DxValue)
{
  Serial.println("");
  Serial.println("Update GPIO ");
  Serial.print(GPIOUT[gpio]);
  Serial.print(" -> ");
  Serial.println(DxValue);

  if ( DxValue == "1" )
  {
    digitalWrite(GPIOUT[gpio], HIGH);
    etatGpio[gpio] = "On";
    Serial.println(gpio + etatGpio[gpio]);
    server.send ( 302, "text/html", "/mainPage.html" );
  }
  else if ( DxValue == "0" )
  {
    digitalWrite(GPIOUT[gpio], LOW);
    etatGpio[gpio] = "Off";
    Serial.println(gpio + etatGpio[gpio]);
    server.send ( 302, "text/html", "/mainPage.html" );
  }
  else
  {
    Serial.println("Erro valor incorreto");
  }
}

void verificatomada(void)
{
  for(int i=0;i<5;i++)
  {
    if(digitalRead(GPIOIN[i])){
      etatGpio[i] = "On";
      Serial.println(etatGpio[i]);
    }
    else
    {
      etatGpio[i] = "Off";
      Serial.println(etatGpio[i]);
    }
  }
}
