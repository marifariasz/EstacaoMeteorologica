#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "pico/time.h"
#include "lib/ssd1306.h"
#include "aht20.h"
#include "bmp280.h"
#include <math.h>
#include "lib/font.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "web_site.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"
#include "hardware/pio.h"

// Estrutura para dados do AHT20
AHT20_Data data;

extern char ip_display[24];

// Definições dos pinos
#define RED_LED_PIN 13
#define BLUE_LED_PIN 12
#define GREEN_LED_PIN 11            
#define PUSH_BUTTON_PIN 6     
#define BUZZER 21
#define LED_PIN 7              // Pino para matriz de LEDs WS2812B
#define LED_COUNT 25           // Número total de LEDs na matriz 5x5
// Definições dos pinos I2C
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1
#define SEA_LEVEL_PRESSURE 101325.0 // Pressão ao nível do mar em Pa
// Display na I2C
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define endereco 0x3C
// Constantes do sistema
#define DEBOUNCE_TIME_MS 200
#define SAMPLE_INTERVAL_MS 200
#define DISPLAY_UPDATE_MS 1000
#define BUZZER_INTERVAL_MS 2000
#define RAIN_THRESHOLD 80.0
#define ORANGE_ALERT_MIN 20.0
#define ORANGE_ALERT_MAX 30.0

// Variáveis globais
bool pump_active = false;
float altitude;
bool leds_enabled = true;
bool button_last_state = false;
absolute_time_t last_button_time;
float temperature = 0;
float pressure = 0;
ssd1306_t oled;

// Estrutura para LEDs
struct pixel_t {
    uint8_t G, R, B;
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

npLED_t leds[LED_COUNT];
PIO np_pio;
uint sm;

// Definições para a matriz de LEDs
#define TEMP_COLD 0
#define TEMP_IDEAL 1
#define TEMP_HOT 2
#define RAIN 3
#define ORANGE_ALERT 4

// Padrões para a matriz de LEDs 5x5
const uint8_t digits[5][5][5][3] = {
    // TEMP_COLD: Flocos de neve
    {{{0, 0, 100}, {0, 0, 0}, {0, 0, 100}, {0, 0, 0}, {0, 0, 100}},
     {{0, 0, 0}, {0, 0, 100}, {0, 0, 0}, {0, 0, 100}, {0, 0, 0}},
     {{0, 0, 100}, {0, 0, 0}, {0, 0, 100}, {0, 0, 0}, {0, 0, 100}},
     {{0, 0, 0}, {0, 0, 100}, {0, 0, 0}, {0, 0, 100}, {0, 0, 0}},
     {{0, 0, 100}, {0, 0, 0}, {0, 0, 100}, {0, 0, 0}, {0, 0, 100}}},
    // TEMP_IDEAL: Sol
    {{{0, 0, 0}, {0, 0, 0}, {100, 100, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {100, 100, 0}, {0, 255, 0}, {100, 100, 0}, {0, 0, 0}},
     {{100, 100, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {100, 100, 0}},
     {{0, 0, 0}, {100, 100, 0}, {0, 255, 0}, {100, 100, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {100, 100, 0}, {0, 0, 0}, {0, 0, 0}}},
    // TEMP_HOT: Chamas
    {{{255, 0, 0}, {255, 50, 0}, {255, 0, 0}, {255, 50, 0}, {255, 0, 0}},
     {{255, 50, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 50, 0}},
     {{255, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 0, 0}},
     {{255, 50, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {255, 50, 0}},
     {{255, 0, 0}, {255, 50, 0}, {255, 0, 0}, {255, 50, 0}, {255, 0, 0}}},
    // RAIN: Base para animação
    {{{0, 0, 100}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 100}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 100}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 100}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 100}}},
    // ORANGE_ALERT: Triângulo de alerta
    {{{0, 0, 0}, {0, 0, 0}, {255, 165, 0}, {0, 0, 0}, {0, 0, 0}},
     {{0, 0, 0}, {255, 165, 0}, {255, 165, 0}, {255, 165, 0}, {0, 0, 0}},
     {{255, 165, 0}, {255, 165, 0}, {255, 165, 0}, {255, 165, 0}, {255, 165, 0}},
     {{0, 0, 0}, {255, 165, 0}, {255, 165, 0}, {255, 165, 0}, {0, 0, 0}},
     {{0, 0, 0}, {0, 0, 0}, {255, 165, 0}, {0, 0, 0}, {0, 0, 0}}}
};

// Protótipos das funções
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b);
void npClear(void);
void npInit(uint pin);
void npWrite(void);
void npDisplayDigit(int digit);
int getIndex(int x, int y);
void start_buzzer(uint pin, uint frequency, uint duration_ms);
void stop_buzzer(void);
void init_hardware(void);
void handle_button(void);
void control_alerts(float temperature, float humidity);
void update_display(ssd1306_t *ssd, float temperature, float altitude, AHT20_Data *data, bool cor, char *ip_display);
double calculate_altitude(double pressure);

// Funções para a matriz de LEDs
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

void npClear() {
    for (uint i = 0; i < LED_COUNT; i++) {
        npSetLED(i, 0, 0, 0);
    }
    npWrite();
}

void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    npClear();
}

void npWrite() {
    for (uint i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}

int getIndex(int x, int y) {
    if (y % 2 == 0) {
        return 24 - (y * 5 + x);
    } else {
        return 24 - (y * 5 + (4 - x));
    }
}

void npDisplayDigit(int digit) {
    for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 5; row++) {
            int pos = getIndex(row, col);
            npSetLED(pos, digits[digit][col][row][0],
                          digits[digit][col][row][1],
                          digits[digit][col][row][2]);
        }
    }
    npWrite();
}

// Função para o buzzer (não bloqueante)
static bool buzzer_active = false;
static uint buzzer_pin = BUZZER;
static uint buzzer_slice_num;
static uint32_t buzzer_end_time;

void start_buzzer(uint pin, uint frequency, uint duration_ms) {
    if (!buzzer_active) {
        gpio_set_function(pin, GPIO_FUNC_PWM);
        buzzer_slice_num = pwm_gpio_to_slice_num(pin);
        pwm_config config = pwm_get_default_config();
        pwm_config_set_clkdiv(&config, clock_get_hz(clk_sys) / (frequency * 4096));
        pwm_init(buzzer_slice_num, &config, true);
        pwm_set_gpio_level(pin, 2048);
        buzzer_active = true;
        buzzer_end_time = to_ms_since_boot(get_absolute_time()) + duration_ms;
    }
}

void stop_buzzer() {
    if (buzzer_active && to_ms_since_boot(get_absolute_time()) >= buzzer_end_time) {
        pwm_set_gpio_level(buzzer_pin, 0);
        pwm_set_enabled(buzzer_slice_num, false);
        buzzer_active = false;
    }
}

// Função para calcular a altitude
double calculate_altitude(double pressure) {
    return 44330.0 * (1.0 - pow(pressure / SEA_LEVEL_PRESSURE, 0.1903));
}

// Função de inicialização
void init_hardware() {
    stdio_init_all();
    
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_put(RED_LED_PIN, 0);
    
    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_put(GREEN_LED_PIN, 0);
    
    gpio_init(BLUE_LED_PIN);
    gpio_set_dir(BLUE_LED_PIN, GPIO_OUT);
    gpio_put(BLUE_LED_PIN, 0);
    
    gpio_init(PUSH_BUTTON_PIN);
    gpio_set_dir(PUSH_BUTTON_PIN, GPIO_IN);
    gpio_pull_up(PUSH_BUTTON_PIN);

    gpio_init(BUZZER);
    gpio_set_dir(BUZZER, GPIO_OUT);
    
    last_button_time = get_absolute_time();
}

// Função para tratar o botão
void handle_button() {
    bool current_button_state = !gpio_get(PUSH_BUTTON_PIN);
    absolute_time_t current_time = get_absolute_time();
    
    if (absolute_time_diff_us(last_button_time, current_time) > (DEBOUNCE_TIME_MS * 1000)) {
        if (current_button_state && !button_last_state) {
            leds_enabled = !leds_enabled;
            last_button_time = current_time;
            printf("Botão pressionado - LEDs %s\n", leds_enabled ? "HABILITADOS" : "DESABILITADOS");
        }
        button_last_state = current_button_state;
    }
}

// Função otimizada para controlar LEDs, buzzer e matriz
void control_alerts(float temperature, float humidity) {
    static uint8_t rain_frame = 0;
    static absolute_time_t last_buzzer_time = 0;
    static int last_pattern = -1;
    static bool last_raining = false;
    static bool last_orange_alert = false;
    absolute_time_t current_time = get_absolute_time();
    float temp_celsius = temperature / 100.0;

    if (!leds_enabled) {
        gpio_put(RED_LED_PIN, 0);
        gpio_put(GREEN_LED_PIN, 0);
        gpio_put(BLUE_LED_PIN, 0);
        if (last_pattern != -1) {
            npClear();
            last_pattern = -1;
        }
        stop_buzzer();
        return;
    }

    bool is_raining = (humidity > RAIN_THRESHOLD);
    bool is_orange_alert = (humidity >= ORANGE_ALERT_MIN && humidity <= ORANGE_ALERT_MAX);
    int current_pattern = -1;

    if (is_raining) {
        printf("Condição de chuva detectada (umidade: %.2f%%)\n", humidity);
        bool blink_state = (to_ms_since_boot(current_time) % 1000) < 500;
        gpio_put(RED_LED_PIN, blink_state);
        gpio_put(GREEN_LED_PIN, blink_state);
        gpio_put(BLUE_LED_PIN, blink_state);

        if (absolute_time_diff_us(last_buzzer_time, current_time) > BUZZER_INTERVAL_MS * 1000) {
            start_buzzer(BUZZER, 2000, 100);
            last_buzzer_time = current_time;
        }

        if (last_pattern != RAIN || last_raining != is_raining) {
            for (int col = 0; col < 5; col++) {
                for (int row = 0; row < 5; row++) {
                    int pos = getIndex(row, col);
                    if ((row + rain_frame) % 5 == 0) {
                        npSetLED(pos, 0, 0, 100);
                    } else {
                        npSetLED(pos, 0, 0, 0);
                    }
                }
            }
            npWrite();
        }
        rain_frame = (rain_frame + 1) % 5;
        current_pattern = RAIN;
    } else if (is_orange_alert) {
        printf("Alerta laranja: umidade %.2f%%\n", humidity);
        gpio_put(RED_LED_PIN, 1);    // Vermelho ligado
        gpio_put(GREEN_LED_PIN, 1);  // Verde ligado (laranja)
        gpio_put(BLUE_LED_PIN, 0);   // Azul apagado

        if (absolute_time_diff_us(last_buzzer_time, current_time) > BUZZER_INTERVAL_MS * 1000) {
            start_buzzer(BUZZER, 1500, 100); // Bipe moderado
            last_buzzer_time = current_time;
        }

        if (last_pattern != ORANGE_ALERT || last_orange_alert != is_orange_alert) {
            npDisplayDigit(ORANGE_ALERT);
        }
        current_pattern = ORANGE_ALERT;
    } else {
        if (temp_celsius < 20.0) {
            printf("Temperatura fria: %.2f C\n", temp_celsius);
            gpio_put(RED_LED_PIN, 1);
            gpio_put(GREEN_LED_PIN, 0);
            gpio_put(BLUE_LED_PIN, 0);
            current_pattern = TEMP_COLD;
            if (absolute_time_diff_us(last_buzzer_time, current_time) > BUZZER_INTERVAL_MS * 1000) {
                start_buzzer(BUZZER, 1000, 100);
                last_buzzer_time = current_time;
            }
        } else if (temp_celsius >= 20.0 && temp_celsius <= 30.0) {
            printf("Temperatura ideal: %.2f C\n", temp_celsius);
            gpio_put(RED_LED_PIN, 0);
            gpio_put(GREEN_LED_PIN, 1);
            gpio_put(BLUE_LED_PIN, 0);
            current_pattern = TEMP_IDEAL;
            stop_buzzer();
        } else {
            printf("Temperatura quente: %.2f C\n", temp_celsius);
            gpio_put(RED_LED_PIN, 0);
            gpio_put(GREEN_LED_PIN, 0);
            gpio_put(BLUE_LED_PIN, 1);
            current_pattern = TEMP_HOT;
            if (absolute_time_diff_us(last_buzzer_time, current_time) > BUZZER_INTERVAL_MS * 1000) {
                start_buzzer(BUZZER, 3000, 100);
                last_buzzer_time = current_time;
            }
        }
        if (last_pattern != current_pattern) {
            npDisplayDigit(current_pattern);
        }
    }

    last_pattern = current_pattern;
    last_raining = is_raining;
    last_orange_alert = is_orange_alert;
    stop_buzzer();
}

// Função otimizada para atualizar o display
void update_display(ssd1306_t *ssd, float temperature, float altitude, AHT20_Data *data, bool cor, char *ip_display) {
    static float last_temp = -999.0;
    static float last_alt = -999.0;
    static float last_aht_temp = -999.0;
    static float last_humidity = -999.0;
    static absolute_time_t last_display_time = 0;
    absolute_time_t current_time = get_absolute_time();

    if (absolute_time_diff_us(last_display_time, current_time) < DISPLAY_UPDATE_MS * 1000 &&
        fabs(temperature - last_temp) < 100.0 &&
        fabs(altitude - last_alt) < 1.0 &&
        fabs(data->temperature - last_aht_temp) < 1.0 &&
        fabs(data->humidity - last_humidity) < 1.0) {
        return;
    }

    char str_tmp1[5];
    char str_alt[5];
    char str_tmp2[5];
    char str_umi[5];

    sprintf(str_tmp1, "%.1fC", temperature / 100.0);
    sprintf(str_alt, "%.0fm", altitude);
    if (data->temperature != 0 && data->humidity != 0) {
        sprintf(str_tmp2, "%.1fC", data->temperature);
        sprintf(str_umi, "%.1f%%", data->humidity);
    } else {
        sprintf(str_tmp2, "Erro");
        sprintf(str_umi, "---");
    }

    ssd1306_fill(ssd, !cor);
    ssd1306_rect(ssd, 3, 3, 122, 60, cor, !cor);
    ssd1306_line(ssd, 3, 25, 123, 25, cor);
    ssd1306_line(ssd, 3, 37, 123, 37, cor);
    ssd1306_draw_string(ssd, "CEPEDI   TIC37", 8, 6);
    ssd1306_draw_string(ssd, ip_display, 10, 16);
    ssd1306_draw_string(ssd, "BMP280  AHT20", 10, 28);
    ssd1306_line(ssd, 63, 25, 63, 60, cor);
    ssd1306_draw_string(ssd, str_tmp1, 14, 41);
    ssd1306_draw_string(ssd, str_alt, 14, 52);
    ssd1306_draw_string(ssd, str_tmp2, 73, 41);
    ssd1306_draw_string(ssd, str_umi, 73, 52);
    ssd1306_send_data(ssd);

    last_temp = temperature;
    last_alt = altitude;
    last_aht_temp = data->temperature;
    last_humidity = data->humidity;
    last_display_time = current_time;
}

int main() {
    init_hardware();
    npInit(LED_PIN);

    init_web_site();

    i2c_init(I2C_PORT_DISP, 400 * 1000);
    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_DISP);
    gpio_pull_up(I2C_SCL_DISP);
    ssd1306_t ssd;
    ssd1306_init(&ssd, 128, 64, false, endereco, I2C_PORT_DISP);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    bmp280_init(I2C_PORT);
    struct bmp280_calib_param params;
    bmp280_get_calib_params(I2C_PORT, &params);

    aht20_reset(I2C_PORT);
    aht20_init(I2C_PORT);

    int32_t raw_temp_bmp, raw_pressure;
    bool cor = true;

    while (true) {
        absolute_time_t loop_start = get_absolute_time();

        bmp280_read_raw(I2C_PORT, &raw_temp_bmp, &raw_pressure);
        temperature = bmp280_convert_temp(raw_temp_bmp, &params);
        pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);
        altitude = calculate_altitude(pressure);

        if (to_ms_since_boot(get_absolute_time()) % 1000 == 0) {
            printf("Pressao = %.3f kPa\n", pressure / 1000.0);
            printf("Temperatura BMP: = %.2f C\n", temperature / 100.0);
            printf("Altitude estimada: %.2f m\n", altitude);
        }

        if (aht20_read(I2C_PORT, &data)) {
            if (to_ms_since_boot(get_absolute_time()) % 1000 == 0) {
                printf("Temperatura AHT: %.2f C\n", data.temperature);
                printf("Umidade: %.2f %%\n\n\n");
            }
        } else {
            data.humidity = 0;
            if (to_ms_since_boot(get_absolute_time()) % 1000 == 0) {
                printf("Erro na leitura do AHT20!\n\n\n");
            }
        }

        handle_button();
        control_alerts(temperature, data.humidity);
        update_display(&ssd, temperature, altitude, &data, cor, ip_display);
        update_web_dados(temperature / 100.0, data.temperature, data.humidity, altitude);

        int32_t elapsed = absolute_time_diff_us(loop_start, get_absolute_time()) / 1000;
        int32_t sleep_time = SAMPLE_INTERVAL_MS - elapsed;
        if (sleep_time > 0) {
            sleep_ms(sleep_time);
        }
    }
}