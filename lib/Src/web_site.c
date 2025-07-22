#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/tcp.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "web_site/html.h" // Presumindo que o html está aqui
#include "web_site.h"
// 1. INCLUA OS NOVOS ARQUIVOS HTML
#include "index_html.h"
#include "charts_html.h"



// Declaração de variáveis externas que vêm do main.c
extern void update_orange_alert_limits(float min, float max);
extern bool pump_active;
char ip_display[24] = "Conectando...";

// Estrutura para os limites, se necessário (parece ser usada apenas aqui)

// Variáveis estáticas para guardar os últimos dados recebidos do main.c
static float g_temp_bmp = 0.0f;
static float g_temp_aht = 0.0f;
static float g_humidity = 0.0f;
static float g_altitude = 0.0f;
static float g_orange_min = 20.0f;
static float g_orange_max = 30.0f;

#define WIFI_SSID "Mariana_Camila" // Coloque seu Wi-Fi aqui
#define WIFI_PASS "maricamila123"  // Coloque sua senha aqui

// Estrutura para o estado da conexão HTTP
struct http_state {
    char response[6000]; // Aumentado para comportar a página HTML inteira
    size_t len;
    size_t sent;
};


// --- BLOCO DE CÓDIGO REMOVIDO ---
// As funções cgi_dados_handler, cgi_set_orange_limits_handler e a array cgi_handlers
// foram removidas pois pertenciam a uma outra arquitetura de servidor (httpd)
// e causavam os erros de compilação. A lógica delas já está na função http_recv.


// Callback chamado quando os dados são enviados com sucesso
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;

    if (hs->sent >= hs->len) {
        tcp_close(tpcb);
        free(hs);
    }
    return ERR_OK;
}

// Callback principal, chamado quando uma requisição é recebida
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    struct http_state *hs = malloc(sizeof(struct http_state));
    if (!hs) {
        pbuf_free(p);
        tcp_recved(tpcb, p->tot_len);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    hs->sent = 0;

    const char* html_to_send = NULL; // Ponteiro para o HTML a ser enviado

    // Endpoint para obter os dados em JSON (usado por ambas as páginas)
// Bloco CORRIGIDO
if (strstr(req, "GET /dados")) {
    char json_payload[256];
    int json_len = snprintf(json_payload, sizeof(json_payload),
                            "{\"temperatura_bmp\":%.2f,\"temperatura_aht\":%.2f,\"umidade\":%.2f,\"altitude\":%.2f,\"bomba\":%d,\"orange_min\":%.1f,\"orange_max\":%.1f}",
                            g_temp_bmp, g_temp_aht, g_humidity, g_altitude, pump_active ? 1 : 0, g_orange_min, g_orange_max);
    // ... resto do código ...
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %d\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n%s",
                           json_len, json_payload);

    // Endpoint para a página de gráficos
    } else if (strstr(req, "GET /charts.html")) {
        html_to_send = charts_html;

    // Endpoint para configurar os limites via POST
    } else if (strstr(req, "POST /set_orange_limits")) {
        // ... (lógica do POST continua a mesma) ...
        char *body = strstr(req, "\r\n\r\n");
        if(body){body+=4;float min=0,max=0;char*param=strtok(body,"&");while(param){if(strncmp(param,"orange_min=",11)==0){min=atof(param+11);}else if(strncmp(param,"orange_max=",11)==0){max=atof(param+11);}param=strtok(NULL,"&");}update_orange_alert_limits(min,max);}
        hs->len = snprintf(hs->response, sizeof(hs->response), "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");

    // Se não for nenhuma das requisições acima, serve a página principal
    } else {
        html_to_send = index_html;
    }

    // Se um HTML foi selecionado, prepara a resposta HTTP
    if (html_to_send) {
        hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",
                           (int)strlen(html_to_send), html_to_send);
    }

    // Envia a resposta
    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);
    tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}

// Callback para aceitar novas conexões
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

// Inicia o servidor TCP na porta 80
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

// Função principal de inicialização do Wi-Fi e do servidor
void init_web_site(void) {
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar CYW43\n");
        return;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi %s...\n", WIFI_SSID);

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Erro ao se conectar ao Wi-Fi\n");
        return;
    }
    
    printf("Conectado com sucesso!\n");
    
    // Obtém e exibe o endereço IP
    uint32_t ip_addr = cyw43_state.netif[0].ip_addr.addr;
    snprintf(ip_display, sizeof(ip_display), "%lu.%lu.%lu.%lu", 
             ip_addr & 0xFF, (ip_addr >> 8) & 0xFF, (ip_addr >> 16) & 0xFF, (ip_addr >> 24) & 0xFF);
    printf("IP obtido: %s\n", ip_display);

    start_http_server();

    // --- LINHA REMOVIDA ---
    // A chamada a http_set_cgi_handlers foi removida pois causava erro de compilação.
}

// Função chamada pelo main.c para atualizar os dados dos sensores
void update_web_dados(float temp_bmp, float temp_aht, float humidity, float altitude, float orange_min, float orange_max) {
    g_temp_bmp = temp_bmp;
    g_temp_aht = temp_aht;
    g_humidity = humidity;
    g_altitude = altitude;
    g_orange_min = orange_min;
    g_orange_max = orange_max;
}