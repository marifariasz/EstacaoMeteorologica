#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "web_site/html.h"
#include "web_site.h"

char ip_display[24] = "Sem IP";

#define WIFI_SSID "Mariana_Camila"
#define WIFI_PASS "maricamila123"
// Declaração de que as variáveis existem em outro lugar
extern float altitude;
extern bool pump_active;
struct nivel_agua nivelConfig = {250, 550};

struct http_state {
    char response[5300];
    size_t len;
    size_t sent;
};


// Variáveis estáticas para guardar os últimos dados recebidos do main.c
static float g_temp_bmp = 0.0f;
static float g_temp_aht = 0.0f;
static float g_humidity = 0.0f;
static float g_altitude = 0.0f;


// Dentro de web_site.c

// Exemplo de um CGI Handler para a URL /dados.json
const char * cgi_dados_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]) {
    // Buffer para criar a string JSON
    static char json_buffer[256];

    // Formata a string JSON com os valores atuais das variáveis globais
    snprintf(json_buffer, sizeof(json_buffer),
             "{\"temp_bmp\": %.2f, \"temp_aht\": %.2f, \"humidity\": %.2f, \"altitude\": %.1f}",
             g_temp_bmp, g_temp_aht, g_humidity, g_altitude);

    // Retorna o ponteiro para o buffer com o JSON
    return json_buffer;
}

// Em algum lugar na inicialização, você deve registrar este handler:
// static const tCGI cgi_handlers[] = {
//     {"/dados.json", cgi_dados_handler},
// };
// http_set_cgi_handlers(cgi_handlers, 1);

static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;
    if (hs->sent >= hs->len) {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}



static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs) {
        pbuf_free(p);
        tcp_close(tpcb);
        tcp_recved(tpcb, p->tot_len);
        return ERR_MEM;
    }
    hs->sent = 0;

// Dentro da função http_recv

   if (strstr(req, "GET /dados")) {
 printf("Recebida requisição GET /dados\n");

        // ALINHA DE DEBUG USANDO AS VARIÁVEIS CORRETAS
 printf("Valores atuais: temp_bmp=%.2f, temp_aht=%.2f, umidade=%.2f, altitude=%.2f, bomba=%d\n",
g_temp_bmp, g_temp_aht, g_humidity, g_altitude, pump_active ? 1 : 0);

 char json_payload[128];
        // ALTERE AQUI PARA USAR AS VARIÁVEIS g_...
 int json_len = snprintf(json_payload, sizeof(json_payload),
"{\"temperatura_bmp\":%.2f, \"temperatura_aht\":%.2f, \"umidade\":%.2f, \"altitude\":%.2f, \"bomba\":%d}\r\n",
 g_temp_bmp, g_temp_aht, g_humidity, g_altitude, pump_active ? 1 : 0);

        // O resto da função continua igual...
hs->len = snprintf(hs->response, sizeof(hs->response),
                    // ...
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Access-Control-Allow-Origin: *\r\n" // Adicionado CORS
            "Connection: close\r\n"
            "\r\n"
            "%s",
            json_len, json_payload);
    }
    else if (strstr(req, "GET /valor_min_max/") != NULL) {
        printf("Recebida requisição GET /valor_min_max/\n");
        char *pos = strstr(req, "/valor_min_max/");
        if (pos) {
            pos += strlen("/valor_min_max/");
            char valores[32];
            strncpy(valores, pos, sizeof(valores) - 1);
            valores[sizeof(valores) - 1] = '\0';
            char *token = strtok(valores, "s");
            int minimo = 0;
            int maximo = 0;
            if (token != NULL) {
                minimo = atoi(token);
                token = strtok(NULL, "s");
                if (token != NULL) {
                    maximo = atoi(token);
                    nivelConfig.min = (uint16_t)minimo;
                    nivelConfig.max = (uint16_t)maximo;
                    printf("Novo Min: %d | Novo Max: %d\n", nivelConfig.min, nivelConfig.max);
                }
            }
        }
        hs->len = snprintf(hs->response, sizeof(hs->response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Access-Control-Allow-Origin: *\r\n" // Adicionado CORS
            "Connection: close\r\n"
            "\r\n"
            "OK");
    }
    else {
        printf("Recebida requisição GET para página principal\n");
        char html_com_dados[6000]; // Aumentado para evitar truncamento
        snprintf(html_com_dados, sizeof(html_com_dados), html);

        hs->len = snprintf(hs->response, sizeof(hs->response),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "Access-Control-Allow-Origin: *\r\n" // Adicionado CORS
            "Connection: close\r\n"
            "\r\n"
            "%s",
            (int)strlen(html_com_dados), html_com_dados);
    }

    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);
    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    pbuf_free(p);
    return ERR_OK;
}

static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB TCP\n");
        return;
    }
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

void init_web_site(void) {
    if (cyw43_arch_init()) {
        printf("Erro\n");
        return;
    }

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Erro ao se conectar\n");
        return;
    }

    uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    snprintf(ip_display, sizeof(ip_display), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    printf("IP obtido: %s\n", ip_display);

    start_http_server();
}

// Dentro de web_site.c



// Esta função é chamada pelo main.c para atualizar os valores
void update_web_dados(float temp_bmp, float temp_aht, float humidity, float altitude) {
    g_temp_bmp = temp_bmp;
    g_temp_aht = temp_aht;
    g_humidity = humidity;
    g_altitude = altitude;
}