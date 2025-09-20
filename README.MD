# Interruptor Inteligente WiFi com ESP8266 (Master/Slave)

![Versão](https://img.shields.io/badge/versão-1.0-blue)
![Plataforma](https://img.shields.io/badge/Plataforma-PlatformIO-orange)
![Licença](https://img.shields.io/badge/licença-MIT-green)

Projeto de um interruptor inteligente (smart switch) baseado no microcontrolador ESP8266. Permite o controle de um relé via rede Wi-Fi através de uma API HTTP simples e inclui um modo de operação Mestre/Escravo para sincronizar múltiplos interruptores.

Para operar esse interruptor pode-se usar o aplicativo Automação Residêncial disponivel em [https://github.com/juliootto/automacao_residencial](https://github.com/juliootto/automacao_residencial).

Este projeto está configurado para ser compilado e gerido com **PlatformIO** no **Visual Studio Code**.

## ✨ Funcionalidades

* **Controle via Wi-Fi**: Ligue e desligue qualquer aparelho eletrónico através da sua rede local.
* **Servidor Web Integrado**: O ESP8266 hospeda um servidor web para receber comandos HTTP.
* **Modo Mestre/Escravo**: Configure um dispositivo como "Mestre" e outros como "Escravos" que irão espelhar o estado do mestre automaticamente.
* **API HTTP Simples**: Controle o interruptor facilmente através de requisições GET.
* **Função Sleep Timer**: Programe o interruptor para se desligar automaticamente após um tempo determinado.
* **Configuração Facilitada**: Usa a biblioteca **WiFiManager** para configurar as credenciais de Wi-Fi através de um portal web na primeira utilização.
* **IP Estático**: Permite a configuração de um endereço IP fixo para fácil acesso na rede.

## 📋 Pré-requisitos

* **Visual Studio Code**: O editor de código onde o projeto será desenvolvido.
* **Extensão PlatformIO IDE**: A extensão para o VS Code que gere todo o processo de compilação, dependências e upload.

## 🛠️ Hardware Necessário

* 1x Placa de desenvolvimento com **ESP8266** (NodeMCU, Wemos D1 Mini, etc.).
* 1x Módulo Relé de 5V compatível com microcontroladores de 3.3V.
* 1x Fonte de alimentação de 5V para o ESP8266 e o módulo relé.
* Jumpers para as conexões.

**Conexão Padrão:**
O pino de controle do relé (`relayPin`) está configurado por padrão para o pino `D6` (GPIO12).

## 📚 Bibliotecas e Dependências

A gestão das bibliotecas é feita automaticamente pelo PlatformIO através do ficheiro `platformio.ini`. Não é necessário instalar nada manualmente.

O ficheiro `platformio.ini` deste projeto deve ser semelhante a este:

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
monitor_speed = 9600
lib_deps =
    tzapu/WiFiManager @ 2.0.16-rc2
```

## ⚙️ Instalação e Configuração

**1. Clone o Repositório**
```bash
git clone [https://github.com/juliootto/interruptor-Inteligente.git](https://github.com/juliootto/interruptor-Inteligente.git)
cd seu-repositorio
```

**2. Abra o Projeto no VS Code**
* Abra o Visual Studio Code.
* Vá em `File > Open Folder...` (ou `Ficheiro > Abrir Pasta...`) e selecione a pasta do projeto que acabou de clonar.
* O PlatformIO deverá reconhecer o projeto automaticamente.

**3. Edite o Ficheiro de Configuração**
* Na estrutura de pastas, abra e edite o ficheiro `include/ConfigInterruptor.h`.
* Altere os valores de `MESTRE`, `IP_X` e `IP_MESTRE_X` de acordo com a sua necessidade.

```cpp
// include/ConfigInterruptor.h

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

// ...
#endif // CONFIG_INTERRUPTOR_H
```

> **Aviso de Segurança**: Tenha cuidado para não enviar informações sensíveis (como senhas de Wi-Fi, se as adicionar) para um repositório público. O uso do WiFiManager ajuda a evitar isso.

**4. Compile e Faça o Upload**
* Conecte a sua placa ESP8266 ao computador.
* Na barra de status inferior do VS Code, clique no ícone de seta (→) que corresponde ao comando "PlatformIO: Upload".
* O PlatformIO irá compilar o projeto, instalar as dependências e carregar o firmware na placa.

**5. Configure o Wi-Fi (Primeira Vez)**
* O processo permanece o mesmo: O ESP8266 criará um Access Point chamado **"InterruptorAP"**.
* Conecte-se a ele e aceda a `192.168.4.1` no navegador para configurar a sua rede Wi-Fi.

## 🚀 Como Usar (API)

Após a configuração, pode controlar o interruptor enviando requisições HTTP GET para o IP do seu ESP8266.

* **Ligar o Relé:** `http://192.168.0.100/relay?do=on`
* **Desligar o Relé:** `http://192.168.0.100/relay?do=off`
* **Verificar o Status:** `http://192.168.0.100/getStatus`
* **Agendar Desligamento (ex: 1 min):** `http://192.168.0.100/sleep?time=60000`

## 👑 Modo de Funcionamento (Mestre/Escravo)

### Modo Mestre
* **Configuração**: `define MESTRE true` no ficheiro `include/ConfigInterruptor.h`.
* **Comportamento**: Atua de forma independente e responde aos comandos da API.

### Modo Escravo
* **Configuração**: `define MESTRE false` e configure o `IP_MESTRE` corretamente.
* **Comportamento**: Ignora comandos da API e espelha o estado do dispositivo Mestre.

*Este projeto foi desenvolvido por Julio Otto.*