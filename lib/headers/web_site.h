#ifndef WEB_SITE_H
#define WEB_SITE_H

#include "lwip/tcp.h"

struct nivel_agua {
    uint16_t min;
    uint16_t max;
};

extern struct nivel_agua nivelConfig;
extern char ip_display[24];

void init_web_site(void);
void update_web_dados(float temp_bmp, float temp_aht, float humidity, float altitude, float orange_min, float orange_max);
void update_orange_alert_limits(float min, float max);

#endif