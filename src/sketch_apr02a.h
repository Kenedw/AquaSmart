/*
Kened Oliveira
04/04/2018
*/
#include <FS.h>   //Include File System Headers
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
void configModeCallback (void);
void handleAPorSTA      (void);
bool loadFromSpiffs     (String path);
void resetwifi          (void);
void apagaFiles         (void);
void updateGPIO         (int pin, bool state);


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
