// -----------------------------------------------------------------------------
// INCLUSÃO DE BIBLIOTECAS
// -----------------------------------------------------------------------------
#include <Arduino.h>
#include <ESP8266WiFi.h>          // Biblioteca principal para funcionalidades Wi-Fi do ESP8266.
#include <WiFiClient.h>           // Necessária para criar clientes HTTP.
#include <ESP8266WebServer.h>     // Para criar o servidor web que receberá os comandos.
#include <WiFiManager.h>          // Gerencia a conexão Wi-Fi de forma inteligente.
#include <user_interface.h>       // Biblioteca de baixo nível para acessar funcionalidades como os Timers do SDK.
#include <ESP8266HTTPClient.h>    // Para fazer requisições a outros servidores (usado no modo escravo).
#include "ConfigInterruptor.h"    // Ficheiro de configuração (contém definições de MESTRE, IPs, etc.).

// -----------------------------------------------------------------------------
// CONSTANTES E DEFINIÇÕES GLOBAIS
// -----------------------------------------------------------------------------

// Define constantes para os estados do relé para tornar o código mais legível.
#define RELE_LIGADO 1
#define RELE_DESLIGADO 0

// Flag para habilitar ou desabilitar mensagens de depuração via Serial. Muito útil para testes.
#define ESP8266_DEBUG true

// -----------------------------------------------------------------------------
// VARIÁVEIS GLOBAIS
// -----------------------------------------------------------------------------

os_timer_t tmr0;                // Estrutura que representa o nosso timer de software para a função sleep.
int relayPin = D6;              // Pino do ESP8266 onde o relé está conectado.
int relayStat = RELE_DESLIGADO; // Variável que armazena o estado atual do relé. Começa desligado.

// As configurações de rede são provavelmente carregadas do "ConfigInterruptor.h"
const char *ssid = STASSID;
const char *password = STAPSK;

//Constantes de status
const char* STATUS_ON_STR = "ON";
const char* STATUS_OFF_STR = "OFF";

// Configuração de IP estático para o dispositivo.
IPAddress ip(IP_1, IP_2, IP_3, IP_4); 
IPAddress ipMestre(IP_MESTRE_1, IP_MESTRE_2, IP_MESTRE_3, IP_MESTRE_4); 
IPAddress gateway(192, 168, 0, 1); 
IPAddress subnet(255, 255, 255, 0); 
IPAddress dns1(8, 8, 8, 8); 
IPAddress dns2(8, 8, 4, 4); 

// Cria uma instância do servidor web na porta 80 (HTTP padrão).
ESP8266WebServer server(80);

// Cria instâncias de cliente e cliente HTTP para fazer requisições externas.
WiFiClient client;
HTTPClient http;

// -----------------------------------------------------------------------------
// FUNÇÕES DO SERVIDOR WEB (ENDPOINTS)
// -----------------------------------------------------------------------------

/**
 * @brief Controla o relé (liga/desliga) com base nos parâmetros da URL.
 * Endpoint: /relay?do=on ou /relay?do=off
 */
void relayControl() {
  String message = "{\"status\":\"";
  if (server.arg("do") == "on") {
    relayStat = RELE_LIGADO;
    message += STATUS_ON_STR;
    message +="\"";
  } else {
    relayStat = RELE_DESLIGADO;
    message += STATUS_OFF_STR;
    message +="\"";
  }
  message += ",\"IP\":\"";
  message += WiFi.localIP().toString();
  message += "\"}";
  server.send(200, "application/json", message); // Responde com um JSON contendo o status e o IP.
}

/**
 * @brief Retorna o estado atual do relé ("ON" ou "OFF").
 * Endpoint: /getStatus
 */
void getStatus() {
  if (relayStat == RELE_DESLIGADO) {
    server.send(200, "text/plain", STATUS_OFF_STR);
  } else {
    server.send(200, "text/plain", STATUS_ON_STR);
  }
}

/**
 * @brief Agenda o desligamento do relé após um tempo especificado.
 * Endpoint: /sleep?time=<tempo_em_ms>
 */
void sleep() {
  String message;
  if (server.hasArg("time")) { // Verifica se o parâmetro 'time' foi enviado.
    double tempo = server.arg("time").toDouble();
    // Arma o timer 'tmr0' para chamar a função 'desliga' uma única vez ('false') após 'tempo' milissegundos.
    os_timer_arm(&tmr0, tempo, false); 
    message = "1"; // Sucesso
  } else {
    message = "0"; // Erro, parâmetro não fornecido.
  } 
  server.send(200, "text/plain", message); 
}

/**
 * @brief Função de callback para o timer. Simplesmente desliga o relé.
 * @param z Ponteiro de argumento genérico (não utilizado aqui).
 */
void desliga(void* z) {
  relayStat = RELE_DESLIGADO; 
}

/**
 * @brief Funções para lidar com rotas não encontradas no servidor.
 */
void handleRoot() {
  server.send(404, "text/plain", "Not Found");
}

void handleWebRequests(){ 
  server.send(404, "text/plain", "Not Found");
}

// -----------------------------------------------------------------------------
// FUNÇÃO CLIENTE HTTP (MODO ESCRAVO)
// -----------------------------------------------------------------------------

/**
 * @brief No modo Escravo, esta função conecta-se ao dispositivo Mestre para obter o seu estado.
 * @return Retorna o estado do Mestre (RELE_LIGADO ou RELE_DESLIGADO).
 */
bool getStatusMestre() {
  bool status = RELE_DESLIGADO; // Assume desligado por padrão em caso de falha.
  
  if (!MESTRE) { // Executa apenas se não for o dispositivo Mestre.
    String url = "http://" + ipMestre.toString() + "/getStatus"; 
    
    if (http.begin(client, url)) { // Inicia a conexão HTTP.
      int httpCode = http.GET(); // Faz a requisição GET.
      
      if (httpCode > 0) { // Verifica se obteve uma resposta válida.
        #if (ESP8266_DEBUG > 0)
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        #endif
        
        if (httpCode == HTTP_CODE_OK) { // Se a resposta foi "200 OK".
          String payload = http.getString(); // Pega o corpo da resposta.
          if (payload == STATUS_ON_STR) {
            status = RELE_LIGADO;
          } else {
            status = RELE_DESLIGADO;
          }
        }
      } else {
        #if (ESP8266_DEBUG > 0)
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        #endif
      }
      http.end(); // Fecha a conexão.
    } else {
      #if (ESP8266_DEBUG > 0)
        Serial.printf("[HTTP] Unable to connect to %s\n", url.c_str());
      #endif
    }
  }
  return status;
}

// -----------------------------------------------------------------------------
// SETUP - CONFIGURAÇÃO INICIAL
// -----------------------------------------------------------------------------
void setup(void) {
  pinMode(relayPin, OUTPUT);      // Configura o pino do relé como saída.
  digitalWrite(relayPin, HIGH);   // Define um estado inicial (verificar se HIGH é ligado ou desligado para o seu relé).
  Serial.begin(9600);             // Inicia a comunicação serial para depuração.

  // --- Configuração do WiFiManager ---
  WiFiManager wifiManager;
  wifiManager.setSTAStaticIPConfig(ip, gateway, subnet); // Define o IP estático.
  
  // Tenta conectar-se ao Wi-Fi. Se falhar, cria um Access Point para configuração.
  if (!wifiManager.autoConnect("InterruptorAP")) {
    #if (ESP8266_DEBUG > 0)
      Serial.println("Falha ao conectar. Reiniciando...");
    #endif
    delay(3000);
    ESP.restart(); // Reinicia o ESP para tentar novamente.
  }

  // Se chegou aqui, a conexão foi bem-sucedida.
  #if (ESP8266_DEBUG > 0)
    Serial.println("Conectado ao WiFi!");
    Serial.print("IP local: ");
    Serial.println(WiFi.localIP());
  #endif
  
  // --- Configuração do Servidor Web ---
  server.on("/relay", relayControl); 
  server.on("/getStatus", getStatus);   
  server.on("/sleep", sleep);       
  server.onNotFound(handleWebRequests); // Define uma função para URLs não encontradas.
  server.begin(); // Inicia o servidor.

  #if (ESP8266_DEBUG > 0)
    Serial.println("Servidor HTTP iniciado.");
  #endif

  // --- Configuração do Timer ---
  os_timer_setfn(&tmr0, desliga, NULL); 
  #if (ESP8266_DEBUG > 0)
    Serial.println("Timer configurado.");
  #endif
}

// -----------------------------------------------------------------------------
// LOOP - CÓDIGO PRINCIPAL
// -----------------------------------------------------------------------------
void loop(void) {
  // Processa continuamente as requisições de clientes HTTP. Essencial para o servidor funcionar.
  server.handleClient();

  // Lógica de Mestre/Escravo
  if (!MESTRE) {
    // Se for um dispositivo Escravo, atualiza o seu estado com base no estado do Mestre.
    relayStat = getStatusMestre();
  }

  // Atualiza o estado físico do relé com base na variável 'relayStat'.
  // ATENÇÃO: Muitos módulos de relé são "Active LOW", o que significa que LOW liga e HIGH desliga.
  // O seu código parece usar LOW para desligado e HIGH para ligado. Verifique se isso está correto para o seu hardware.
  if (relayStat == RELE_DESLIGADO) {
    digitalWrite(relayPin, LOW); // Envia sinal LOW para o pino.
  } else {
    digitalWrite(relayPin, HIGH); // Envia sinal HIGH para o pino.  
  }
  
  // Pequena pausa para evitar sobrecarregar o processador.
  delay(100);
}