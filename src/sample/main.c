#include "public.h"
#include "ini.h"
#include "ipc.h"
#include "rtmp_wapper.h"

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
    pthread_mutex_t mutex;
    RtmpContex *ctx;
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


int video_frame_callback (uint8_t *frame, int len, int iskey, int64_t timestamp, int streamno)
{
    if (!frame) {
        LOGE("check param error");
        goto err;
    }
    pthread_mutex_lock( &app.mutex );
    if(RtmpSendVideo(app.ctx, frame, len, iskey, timestamp) < 0) {
        LOGE("RtmpSendVideo error");
        pthread_mutex_unlock(&app.mutex);
        goto err;
    }
    pthread_mutex_unlock(&app.mutex);
    return 0;
err:
    return -1;
}

int audio_frame_callback (uint8_t *frame, int len, int64_t timestamp, int streamno)
{
    return 0;
    if (!frame) {
        LOGE("check param error");
        return -1;
    }
    pthread_mutex_lock(&app.mutex);
    if (RtmpSendAudio(app.ctx, frame, len, timestamp) < 0) {
        LOGE("RtmpSendAudio error, errno = %d", errno);
        pthread_mutex_unlock( &app.mutex );
        return -1;
    }
    pthread_mutex_unlock( &app.mutex );
    return 0;
}

int main()
{
    ipc_param_t param = 
    {
        .audio_type = IPC_AUDIO_AAC,
        .stream_number = 1,
        .video_cb = video_frame_callback,
        .audio_cb = audio_frame_callback,
        .event_cb = NULL,
        .log_cb = NULL,
    };
    int audiotype = RTMP_PUB_AUDIO_NONE;//RTMP_PUB_AUDIO_AAC;

    if (ini_parse(CONFIG_FILE, conf_handler, &app.conf) < 0) {
        printf("load config file:%s error\n", CONFIG_FILE);
        return 0;
    }
    dump_config(&app.conf);
    log_init(app.conf.mqtt_url, app.conf.mqtt_port, app.conf.mqtt_user, app.conf.mqtt_passwd);
    LOGI("config and log init done");
    app.ctx = RtmpNewContext(app.conf.rtmp_url, 10, audiotype, audiotype, RTMP_PUB_TIMESTAMP_ABSOLUTE);
    if (!app.ctx) {
        LOGE("new rtmp context error");
        return 0;
    }
    if (RtmpConnect(app.ctx) < 0) {
        LOGE("rtmp connect error");
        return 0;
    } else {
        LOGI("rtmp connect to %s success", app.conf.rtmp_url);
    }
    pthread_mutex_init(&app.mutex, NULL);
    ipc_init(&param);
    ipc_run();

    for (;;) sleep(1);

    return 0;
}

