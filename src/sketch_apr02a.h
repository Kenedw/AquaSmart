/*
Kened Oliveira
04/04/2018
*/
#include <ESP8266mDNS.h>
#include <FS.h>   //Include File System Headers
#include <ESP8266mDNS.h>
#include "../lib/WiFiManager/WiFiManager.h"
void configModeCallback (void);
void handleAPorSTA      (void);
bool loadFromSpiffs     (String path);
void resetwifi          (void);
void apagaFiles         (void);
void verificatomada     (void);
void modificatomada     (void);
void updateGPIO         (int gpio, String DxValue);
void alloff             (void);



/*
manipula a pagina index, faz com que ela seja chamada e devolve como resposta
o codigo 302 do HTTP, que manda a pagina como encontrado no redirecionamento,
*/
void handleRoot         (void);
/*
trata a requisição de arquivo não encontrado, devolvendo uma
resposta bem formatada do problema
*/
void handleWebRequests  (void);
/*
manipula a configuração se o modulo sera um ponto de acesso ou estação e
se tera um ip estatico
*/
void handleAPorSTA      (void);
// void saveConfig();
// void loadConfig();

//Estrutura de configuração EEPROM
// typedef struct
// {
//   char ssid[30];
//   char password[30];
//   IPAddress ip;
//   IPAddress gateway;
// } WifiConfig;
// #ifdef ESP8266MDNS_H
//   if(MDNS.begin("AquaSmart")){
//     MDNS.addService("http", "tcp", 80);
//   }
// #endif
