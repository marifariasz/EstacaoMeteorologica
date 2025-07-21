#ifndef WEB_SITE_H
#define WEB_SITE_H

#include <stdint.h>

struct nivel_agua {
    uint16_t min;
    uint16_t max;
};

extern struct nivel_agua nivelConfig;
extern char ip_display[24];
extern float temperatura_bmp;
extern float temperatura_aht;
extern float umidade;
extern float altitude;
extern bool pump_active;

void init_web_site(void);
void update_web_dados(float temp_bmp, float temp_aht, float umi, float alt);

#endif