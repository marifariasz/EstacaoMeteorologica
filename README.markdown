# 🌦️ Estação Meteorológica com Raspberry Pi Pico 🌡️

Bem-vindo ao projeto da **Estação Meteorológica**! Este sistema, desenvolvido para o **Raspberry Pi Pico**, monitora condições ambientais (temperatura, umidade, pressão, altitude) e exibe alertas visuais e sonoros com base em condições predefinidas. Com uma interface web moderna, você pode visualizar dados em tempo real e configurar os limites do **alerta laranja** (umidade) de forma dinâmica! 🚨

---

## 🎯 Funcionalidades

- **Monitoramento Ambiental** 📊
  - **Sensores**: BMP280 (temperatura e pressão) e AHT20 (temperatura e umidade).
  - Calcula altitude com base na pressão atmosférica.
  - Detecta condições de chuva (>80% de umidade) e temperaturas (fria, ideal, quente).

- **Alertas Visuais e Sonoros** 🔔
  - **LEDs RGB**: Indicam estados (chuva, alerta laranja, temperatura fria/ideal/quente).
  - **Matriz de LEDs 5x5 (WS2812B)**: Exibe padrões (flocos de neve, sol, chamas, chuva, triângulo de alerta laranja).
  - **Buzzer**: Emite bipes específicos para cada alerta (não bloqueante para performance).

- **Interface Web** 🌐
  - Acesse via `http://<IP_DO_PICO>/` para visualizar dados em tempo real (temperatura, umidade, altitude, estado da bomba).
  - Configure os limites do **alerta laranja** (umidade mínima e máxima) via formulário.
  - Estilização moderna com **Tailwind CSS** (CDN).

- **Otimização** ⚡
  - Loop principal executa a cada ~200ms.
  - Cache para display OLED e matriz de LEDs para reduzir latência.
  - Buzzer não bloqueante com PWM.

---

## 🛠️ Requisitos

### Hardware
- **Raspberry Pi Pico W** com suporte Wi-Fi (CYW43).
- **Sensores**:
  - BMP280 (I2C, SDA: pino 0, SCL: pino 1).
  - AHT20 (I2C, mesmo barramento).
- **Display OLED SSD1306** (I2C, SDA: pino 14, SCL: pino 15, endereço 0x3C).
- **Matriz de LEDs WS2812B** (5x5, pino 7).
- **LEDs RGB** (vermelho: pino 13, verde: pino 11, azul: pino 12).
- **Buzzer** (pino 21).
- **Botão** (pino 6, pull-up).

### Software
- **Pico SDK** (v1.5.0 ou superior).
- **CMake** e **Ninja** para compilação.
- **Conexão Wi-Fi**.
- **Navegador web** para acessar a interface.

---

## 🚀 Instalação

1. **Clone o Repositório**
   ```bash
   git clone <URL_DO_REPOSITORIO>
   cd Estacao_Meteorologica
   ```

2. **Configure o Pico SDK**
   - Certifique-se de que o `PICO_SDK_PATH` está definido:
     ```bash
     export PICO_SDK_PATH=/caminho/para/pico-sdk
     ```
   - Inclua as bibliotecas necessárias (`pico_stdlib`, `hardware_i2c`, `hardware_pwm`, `hardware_pio`, `pico_cyw43_arch_lwip_threadsafe_background`).

3. **Compile o Projeto**
   ```bash
   mkdir build
   cd build
   cmake ..
   ninja
   ```

4. **Carregue o Firmware**
   - Conecte o Pico via USB.
   - Copie o arquivo `Estacao_Meteorologica.uf2` para o Pico:
     ```bash
     cp Estacao_Meteorologica.uf2 /media/<USER>/RPI-RP2/
     ```

5. **Conecte à Wi-Fi**
   - Edite `web_site.c` para configurar o SSID e senha da rede Wi-Fi:
     ```c
     #define WIFI_SSID "Sua_Rede"
     #define WIFI_PASS "Sua_Senha"
     ```
   - O IP do Pico será exibido no terminal serial (ex.: `192.168.1.100`).

---

## 🌐 Uso

1. **Acesse a Interface Web**
   - Abra um navegador e acesse `http://<IP_DO_PICO>/` (ex.: `http://192.168.1.100/`).
   - A página exibe:
     - Temperatura (BMP280 e AHT20).
     - Umidade (AHT20).
     - Altitude (calculada via BMP280).
     - Estado da bomba (ligada/desligada).
     - Limites do alerta laranja (inicialmente 20.0%–30.0%).

2. **Configure o Alerta Laranja**
   - Insira os valores de **mínimo** e **máximo** (em %) no formulário.
   - Clique em **Atualizar**.
   - A página mostrará "Limites atualizados com sucesso!" e atualizará os valores exibidos.

3. **Alertas**
   - **Chuva** (>80% umidade):
     - LEDs RGB piscam (vermelho, verde, azul).
     - Matriz de LEDs mostra animação de chuva.
     - Buzzer: 2000 Hz, 100ms a cada 2s.
   - **Alerta Laranja** (umidade entre os limites configurados):
     - LEDs RGB: Laranja (vermelho + verde).
     - Matriz: Triângulo laranja.
     - Buzzer: 1500 Hz, 100ms a cada 2s.
   - **Temperatura**:
     - Fria (<20°C): LED vermelho, matriz com flocos de neve, buzzer 1000 Hz.
     - Ideal (20–30°C): LED verde, matriz com sol, sem buzzer.
     - Quente (>30°C): LED azul, matriz com chamas, buzzer 3000 Hz.
   - **Botão** (pino 6): Habilita/desabilita LEDs e buzzer.

4. **Monitoramento Serial**
   - Conecte o Pico via USB e abra um terminal serial (ex.: `minicom -D /dev/ttyACM0 -b 115200`).
   - Veja logs como:
     ```
     Temperatura BMP: 25.50 C
     Umidade: 20.00 %
     Alerta laranja: umidade 20.0% (limites: 15.0% - 25.0%)
     ```

---

## 🧪 Testes

### 1. Teste da Interface Web
- Acesse `http://<IP_DO_PICO>/`.
- Verifique se os dados (temperatura, umidade, etc.) são atualizados a cada 2s.
- Insira limites (ex.: 15.0 e 25.0) e clique em "Atualizar".
- Confirme:
  - Mensagem: "Limites atualizados com sucesso!".
  - Valores atualizados na interface.
  - Console do navegador (F12) sem erros.

### 2. Teste do Alerta Laranja
- Simule umidade na faixa configurada (ex.: edite `data.humidity = 20.0` em `station.c` para teste).
- Verifique:
  - LEDs RGB: Laranja (vermelho + verde).
  - Matriz: Triângulo laranja.
  - Buzzer: Bipe de 1500 Hz por 100ms a cada 2s.
  - Serial: `Alerta laranja: umidade 20.0% (limites: 15.0% - 25.0%)`.

### 3. Teste de Outros Alertas
- **Chuva**: Configure `data.humidity = 85.0` e verifique LEDs piscando, animação de chuva, e buzzer.
- **Temperatura**: Teste faixas (<20°C, 20–30°C, >30°C) via `temperature` em `station.c`.
- **Botão**: Pressione o botão (pino 6) para desabilitar/habilitar alertas.

### 4. Teste de Performance
- Confirme que o loop principal executa a cada ~200ms (verifique logs no serial).
- Assegure-se de que LEDs, matriz e buzzer operam sem "engasgos".

---

## ⚙️ Arquivos Principais

- **`station.c`**: Lógica principal, controle de sensores, alertas, display OLED, e matriz de LEDs.
- **`web_site.c`**: Servidor HTTP com endpoints `/dados` (JSON) e `/set_orange_limits` (POST para limites).
- **`web_site/html.h`**: Página web com HTML e JavaScript para monitoramento e configuração.
- **`aht20.c` / `bmp280.c`**: Drivers dos sensores.
- **`ws2818b.pio`**: Controle da matriz WS2812B.

---

## 🎨 Personalizações

- **Padrão da Matriz de LEDs**:
  - Modifique o padrão do alerta laranja em `station.c` (ex.: substitua o triângulo por uma exclamação):
    ```c
    {{{0, 0, 0}, {0, 0, 0}, {255, 165, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {255, 165, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {255, 165, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {255, 165, 0}, {0, 0, 0}, {0, 0, 0}}}
    ```

- **CSS Inline**:
  - Se o Tailwind CDN não for viável, substitua por CSS inline em `web_site/html.h`. Exemplo:
    ```c
    "<style>\n"
    "body { background-color: #f3f4f6; font-family: sans-serif; }\n"
    ".container { max-width: 800px; margin: 0 auto; padding: 16px; }\n"
    // ...
    "</style>\n"
    ```

- **Endpoint Unificado**:
  - Para unificar `/valor_min_max/` e `/set_orange_limits`, modifique `web_site.c` para usar um único endpoint (ex.: GET `/config/min/max/orange_min/orange_max`).

---


## 🌟 Vídeo

Desenvolvido para o curso **TIC37** no **CEPEDI**.
