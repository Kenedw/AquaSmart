#include "sketch_apr02a.h"

WiFiManager wifiManager;
ESP8266WebServer server(80);

void setup(void)
{
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
    ESP.restart();
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
  ESP.restart();
}

void apagaFiles(void)
{
  SPIFFS.format();
  Serial.println("Apagou geral");
  ESP.restart();
}

void updateGPIO(int pin, bool state)
{

}
