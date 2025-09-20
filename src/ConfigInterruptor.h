// ConfigInterruptor.h

#ifndef CONFIG_INTERRUPTOR_H
#define CONFIG_INTERRUPTOR_H

// --- MODO DE OPERAÇÃO ---
// Defina como 'true' para o dispositivo Mestre e 'false' para dispositivos Escravos.
#define MESTRE true

// --- CONFIGURAÇÃO DE REDE ---
// Defina o IP estático para este dispositivo.
#define IP_1 192
#define IP_2 168
#define IP_3 0
#define IP_4 100

// Defina o IP do dispositivo Mestre (só é usado se MESTRE = false).
#define IP_MESTRE_1 192
#define IP_MESTRE_2 168
#define IP_MESTRE_3 0
#define IP_MESTRE_4 101 // Exemplo de IP do Mestre

// As credenciais de Wi-Fi abaixo são apenas um fallback.
// O WiFiManager irá gerir a conexão principal.
#define STASSID "sua-rede-wifi"
#define STAPSK "sua-senha"
#define PORTA 80

#endif // CONFIG_INTERRUPTOR_H