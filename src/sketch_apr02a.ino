#include "sketch_apr02a.h"

WiFiManager wifiManager;
ESP8266WebServer server(80);
#define  nomeHost  "AquaSmart"
String  etatGpio[4] = {"off","off","off","off"};
const uint8_t GPIOUT[4] = {D5,D6,D7,D8};  //pinos que vão mudar o estado tomadas
int   hora_rele[4]=[]
// const uint8_t GPIOIN[4] = {D0,D1,D2,D3};  //pinos que vão verificar o estado das tomadas
//WiFiUDP ntpUDP;

//String buf;

//int16_t utc = -3; //UTC -3:00 Brazil
//uint32_t currentMillis = 0;
//uint32_t previousMillis = 0;
 
// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = -3;     // Brasilia Time
//pacotes udp
WiFiUDP Udp;

// porta local para escutar pacotes UDP
unsigned int localPort = 8888;  
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);


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

  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  
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
  server.on("/mainPage", agendaTime);//do agendamento
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

  //timeClient.begin();
  //timeClient.update();
}
time_t prevDisplay = 0; // quando o relógio digital foi exibido

//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();
   if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { // atualiza a exibição somente se o tempo tiver mudado
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
  agendaTime();//chama a funcao que compara horarioNTC com horario agendado e aciona os reles

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
//agendamentos sendo comparados
void agendaTime(void){
   //condicionais de ligar//*****************************
   if (hour() == hora_rele1 && minute() == min_rele1)
   {
      digitalWrite(D1, 0);//Acende
   }

   if (hour() == hora_rele2 && minute() == min_rele2)
   {
      digitalWrite(D2, 0);//Acende
   }

   if (hour() == hora_rele3 && minute() == min_rele3)
   {
      digitalWrite(D3, 0);//Acende
   }

   if (hour() == hora_rele4 && minute() == min_rele4)
   {
      digitalWrite(D4, 0);//Acende
   }
   
  //condicionais de desligamento//***************************
   if (hour() == hora_rele11 && minute() == min_rele11)
   {
      digitalWrite(D1, 1);//desliga
   }

   if (hour() == hora_rele22 && minute() == min_rele22)
   {
      digitalWrite(D2, 1);//desliga
   }

   if (hour() == hora_rele33 && minute() == min_rele33)
   {
      digitalWrite(D3, 1);//desliga
   }

    if (hour() == hora_rele44 && minute() == min_rele44)
   {
      digitalWrite(D4, 1);//desliga
   }
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
//set agendamentos da funcao EnviaAgendamentos javascript
void setAgendamento(void){
  Serial.println("Recebendo valores agendados");
  String aux = "";
  String aux2 = "t";

  for(int i=1;i<=16;i++){
    aux = aux2 + i;
    if(server.hasArg(aux){
      hora_releaux
    }
}

//funcao que printa na serial o horario NTC
void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*-------- NTP code ----------*/

//const int NTP_PACKET_SIZE = 48; // O tempo de NTP está nos primeiros 48 bytes de mensagem 
byte packetBuffer[NTP_PACKET_SIZE]; //buffer para armazenar pacotes de entrada e saída
time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; //descartar quaisquer pacotes recebidos anteriormente 
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket("pool.ntp.br", 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
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

  Serial.print("medida nova -> ");
  Serial.println(result);
  server.send ( 200, "text/html", result);

}
