#include "public.h"
#include "ini.h"

#define CONFIG_FILE "/tmp/oem/app/rtmp.conf"

typedef struct {
    char *rtmp_url;
    char *mqtt_url;
    char *mqtt_user;
    char *mqtt_passwd;
    char *mqtt_port;
} config_t;

typedef struct {
    config_t conf;
} app_t;

static app_t app;

static void dump_config(config_t *conf)
{
    printf("--- rtmp_url: %s\n", conf->rtmp_url);
    printf("--- mqtt_url: %s\n", conf->mqtt_url);
    if (conf->mqtt_user)
        printf("--- mqtt_user: %s\n", conf->mqtt_user);
    if (conf->mqtt_passwd)
        printf("--- mqtt_passwd: %s\n", conf->mqtt_passwd);
    printf("--- mqtt_port: %s\n", conf->mqtt_port);
}

static int conf_handler(void* user, const char* section, const char* name, const char* value)
{
    config_t *config = (config_t *) user;

#define PARSE_STR_CONF(_name, field) if (!strcmp(_name, name)) config->field = strdup(value)
#define PARSE_INT_CONF(_name, field) if(!strcmp(_name, name)) config->field = atoi(value)

    PARSE_STR_CONF("RTMP_URL", rtmp_url);
    PARSE_STR_CONF("MQTT_URL", mqtt_url);
    PARSE_STR_CONF("MQTT_USER", mqtt_user);
    PARSE_STR_CONF("MQTT_PASSWD", mqtt_passwd);
    PARSE_STR_CONF("MQTT_PORT", mqtt_port);
}

int main()
{
    if (ini_parse(CONFIG_FILE, conf_handler, &app.conf) < 0) {
        printf("load config file:%s error\n", CONFIG_FILE);
        return 0;
    }
    dump_config(&app.conf);
    log_init(app.conf.mqtt_url, app.conf.mqtt_port, app.conf.mqtt_user, app.conf.mqtt_passwd);
    LOGI("config and log init done");

    for (;;) sleep(1);

    return 0;
}

