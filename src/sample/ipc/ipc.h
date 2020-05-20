// Last Update:2019-07-31 16:22:55
/**
 * @file ipc.h
 * @brief  ipc interface, support multi manufacturer ipc
 * @author felix
 * @version 0.1.00
 * @date 2019-03-15
 */

#ifndef IPC_H
#define IPC_H

#include <stdint.h>

typedef enum {
    IPC_VIDEO_NONE = 0,
    IPC_VIDEO_H264 = 1,
    IPC_VIDEO_H265 = 2,
} IPCVideoFormat;

typedef enum {
    IPC_AUDIO_NONE = 0,
    IPC_AUDIO_PCMU = 1,
    IPC_AUDIO_PCMA = 2,
    IPC_AUDIO_AAC =  3,
} IPCAudioFormat;

enum {
    EVENT_CAPTURE_PICTURE_SUCCESS = 1,
    EVENT_CAPTURE_PICTURE_FAIL,
    EVENT_MOTION_DETECTION,
    EVENT_MOTION_DETECTION_DISAPEER,
};

typedef enum {
    VIDEO_ENCODE_TYPE_H264,
    VIDEO_ENCODE_TYPE_H265,
} VideoEncodeType;

typedef enum {
    IPC_AUDIO_BITDEPTH_8,
    IPC_AUDIO_BITDEPTH_16,
} IPCAudioBitDepth;

typedef struct {
    int sample_rate;
    int audio_type;
    int video_type;
    int audio_channels;
    int audio_bitdepth;
    unsigned char is_audio_enable;
} media_config_t;

typedef struct {
    int audio_type;
    int stream_number;
    int (*video_cb) (uint8_t *frame, int len, int iskey, int64_t timestamp, int stream_no);
    int (*audio_cb) (uint8_t *frame, int len, int64_t timestamp, int stream_no);
    int (*event_cb)(int event, void *data, int streamno);
    int (*log_cb)(char *log);
} ipc_param_t;

typedef struct ipc_dev_t {
    int (*init)( struct ipc_dev_t *ipc, ipc_param_t *param );
    void (*run)( struct ipc_dev_t *ipc );
    int (*capture_picture)( struct ipc_dev_t *ipc, char *path, char *file, int channelid );
    void (*deinit)( struct ipc_dev_t *ipc );
    int (*get_media_config)( int channelid, media_config_t *config );
    void (*stop)(struct ipc_dev_t *ipc);
    void *priv;
} ipc_dev_t;

extern int ipc_init(ipc_param_t *param);
extern void ipc_run();
extern int ipc_dev_register(ipc_dev_t *dev );
extern int ipc_capture_picture(char *path, char *file, int ch);
extern int ipc_get_media_config(int channelid, media_config_t *config);
extern void ipc_stop();

#endif  /*IPC_H*/

