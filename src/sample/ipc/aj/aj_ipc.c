// Last Update:2019-08-02 15:11:21
/**
 * @file aj_ipc.c
 * @brief 
 * @author felix
 * @version 0.1.00
 * @date 2019-04-10
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "devsdk.h"
#include "ipc.h"

#define BASIC() printf("%s:%d(%s) $ ", __FILE__, __LINE__, __FUNCTION__) 
#define LOGI(args...) BASIC();printf(args)
#define LOGE(args...) LOGI(args)

struct stream_info;

typedef struct {
    ipc_param_t param;
    int running;
    struct stream_info streams[2];
    ipc_dev_t *dev;
} aj_ipc_t;

typedef struct stream_info {
    int stream_no;
    aj_ipc_t *aj;
} stream_info_t;

static int aj_video_get_frame_callback( 
        int streamno,
        char *_pFrame,
        int _nLen,
        int _nIskey,
        double _dTimeStamp,
        unsigned long _nFrameIndex,
        unsigned long _nKeyFrameIndex,
        void *_pContext)
{
    stream_info_t *stream = (stream_info_t *)_pContext;
    aj_ipc_t *aj = stream->aj;

    if ( !_pFrame || !_nLen || !aj) {
        LOGE("check param error, aj = %p, _pFrame = %p, _nLen = %d\n", aj, _pFrame, _nLen );
        return -1;
    }

    if ( aj->param.video_cb ) {
        aj->param.video_cb( (uint8_t *)_pFrame, _nLen, _nIskey, (int64_t)_dTimeStamp, stream->stream_no );
    }

    return 0;
}

static int aj_audio_get_frame_callback( 
        char *_pFrame,
        int _nLen,
        double _dTimeStamp,
        unsigned long _nFrameIndex,
        void *_pContext )
{
    stream_info_t *stream = (stream_info_t *)_pContext;
    aj_ipc_t *aj = NULL;

    if (stream)
        aj = stream->aj;
    if ( !_pFrame || !_nLen || !aj ) {
        LOGE("check param error, aj = %p, _pFrame = %p, _nLen = %d\n", aj, _pFrame, _nLen );
        return -1;
    }

    if ( aj->param.audio_cb ) {
        aj->param.audio_cb((uint8_t *)_pFrame, _nLen, _dTimeStamp, stream->stream_no);
    }

    return 0;
}

int aj_alarm_callback( ALARM_ENTRY _alarm, void *pcontext)
{
    int alarm = 0;
    aj_ipc_t *aj = (aj_ipc_t *)pcontext;
    char *p = NULL;
    int  streamno = 0,i = 0;

    if ( !pcontext ) {
        return -1;
    }

    if ( _alarm.code == ALARM_CODE_MOTION_DETECT ) {
        alarm = EVENT_MOTION_DETECTION;
    } else if ( _alarm.code == ALARM_CODE_MOTION_DETECT_DISAPPEAR ) {
        alarm = EVENT_MOTION_DETECTION_DISAPEER;
    } else if ( _alarm.code == ALARM_CODE_JPEG_CAPTURED ) {
        alarm = EVENT_CAPTURE_PICTURE_SUCCESS;
    } else {
        /* do nothing */
    }

    if ( aj->event_cb ) {
        if ( alarm == EVENT_MOTION_DETECTION || alarm == EVENT_MOTION_DETECTION_DISAPEER ) {
            for ( i=0; i<aj->stream_number; i++ )
                aj->param.event_cb( alarm, _alarm.data, i );
        } else {
            aj->param.event_cb( alarm, _alarm.data, streamno );
        }
    }
    return 0;
}


int aj_ipc_init( struct ipc_dev_t *dev, ipc_param_t *param )
{
    aj_ipc_t *aj = NULL;
    int rc = 0, i;

    if ( !dev || !param ) {
        LOGE("check param error\n");
        return -1;
    }

    LOGI("ipc init\n");
    aj = (aj_ipc_t *) malloc ( sizeof(aj_ipc_t) );
    if ( !aj ) {
        LOGE("malloc error\n");
        return -1;
    }
    memset( aj, 0, sizeof(aj_ipc_t) );
    aj->running = 1;
    aj->dev = dev;
    aj->param = *param;
    for (i=0; i<param->stream_number; i++) {
        aj->streams[i].stream_no = i;
        aj->stream[i].aj = aj;
    }
    dev->priv = (void*)aj;
    rc = dev_sdk_init(DEV_SDK_PROCESS_APP);
    if ( rc < 0 ) {
        LOGE("dev_sdk_init error\n");
        return -1;
    }
    dev_sdk_register_callback(aj_alarm_callback, NULL, NULL, aj);

    return 0;
}


static void aj_ipc_run( ipc_dev_t *dev )
{
    aj_ipc_t *aj = NULL;
    int i = 0;

    if(dev)
        aj = (aj_ipc_t *)dev->priv;
    if ( !aj )
        return;

    for (i=0; i<aj->parma.stream_number; i++) {
        dev_sdk_start_video(0, i, aj_video_get_frame_callback, &aj->streams[i]);
        dev_sdk_start_audio(0, i, aj_audio_get_frame_callback, &aj->streams[i]);
    }
}

static void aj_ipc_deinit(ipc_dev_t *dev)
{
    dev_sdk_stop_video(0, 1);
    dev_sdk_stop_audio(0, 1);
    dev_sdk_stop_audio_play();
    dev_sdk_release();
    if (dev->priv) {
        free( dev->priv );
    }
}

static void aj_ipc_stop(ipc_dev_t *dev)
{
    dev_sdk_stop_video(0, 0);
    dev_sdk_stop_audio(0, 0);
}

static int aj_get_media_config(int ch, media_config_t *config)
{
    AudioConfig audioConfig;
    VideoConfig videoConfig;
    char *video_encode, *audio_encode;

    if ( !config ) {
        LOGE("check param error\n");
        return -1;
    }

    dev_sdk_get_AudioConfig(&audioConfig);
    config->is_audio_enable = audioConfig.audioEncode.enable;
    config->sample_rate = audioConfig.audioEncode.sampleRate ;
    config->audio_channels = audioConfig.audioCapture.channels;
    config->audio_bitdepth = audioConfig.audioCapture.bitspersample;
    audio_encode = audioConfig.audioEncode.audioEncodeType.typeName; 
    if (!strcmp(audio_encode, "AAC")) {
        config->audio_type = IPC_AUDIO_AAC;
    } else if (!strcmp(audio_encode, "G.711A")) {
        config->audio_type = IPC_AUDIO_PCMA;
    } else if (strcmp(audio_encode, "G.711")) {
        config->audio_type = IPC_AUDIO_PCMU;
    } else {
        config->audio_type = IPC_AUDIO_NONE;
    }
    dev_sdk_get_VideoConfig(&videoConfig);
    video_encode = videoConfig.videoEncode.encodeCfg[0].encodeFormat.name;
    if (!strcmp(video_encode, "H264")) {
        config->video_type = IPC_VIDEO_H264;
    } else if (!strncmp(video_encode, "H265")) {
        config->video_type = IPC_VIDEO_H265;
    } else {
        config->video_type = IPC_VIDEO_NONE;
    }
    if (!audioConfig.audioCapture.channels)
        config->audio_channels = 1;

    return 0;
}

static int aj_ipc_capture_picture(struct ipc_dev_t *ipc, char *path, char *file, int streamno)
{
    aj_ipc_t *aj = NULL;

    if (ipc)
        aj = ipc->priv;
    if ( !aj ) {
        LOGE("check aj error\n");
        return -1;
    }

    dev_sdk_set_SnapJpegFile(streamno, 0, path, file);
    return 0;
}

static ipc_dev_t aj_ipc =
{
    .init = aj_ipc_init,
    .deinit = aj_ipc_deinit,
    .run = aj_ipc_run,
    .get_media_config = aj_get_media_config,
    .capture_picture = aj_ipc_capture_picture,
    .stop = aj_ipc_stop,
};

static void __attribute__((constructor)) aj_ipc_register()
{
    ipc_dev_register(&aj_ipc);
}

