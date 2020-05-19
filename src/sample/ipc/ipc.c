/**
* @file ipc.c
* @author rigensen
* @brief 
* @date 二  5/19 22:51:35 2020
*/
#include <stdlib.h>
#include <stdio.h>
#include "ipc.h"
#include "public.h"

static ipc_dev_t *ipc;

int ipc_init( ipc_param_t *param )
{
    int ret = 0;

    if ( !param ) {
        LOGE("check param error\n");
        goto err;
    }

    if ( !ipc || !ipc->init ) {
        LOGE("check ipc dev fail\n");
        goto err;
    }

    ret = ipc->init( ipc, param );
    if ( ret != 0 ) {
        LOGE("ipc init error\n");
        goto err;
    }

    return 0;
err:
    return -1;
}

void ipc_run()
{
    if ( !ipc || !ipc->run ) {
        LOGE("no ipc dev found!!!\n");
        return;
    }

    ipc->run( ipc );
}

int ipc_capture_picture( char *path, char *file, int channelid )
{
    if ( !ipc || !ipc->capture_picture ) {
        LOGE("param check error\n");
        return -1;
    }

    ipc->capture_picture( ipc, path, file, channelid );
    return 0;
}

int ipc_get_media_config( int channelid, media_config_t *config )
{
    if ( !ipc || !ipc->get_media_config ) {
        return -1;
    }

    return ipc->get_media_config( channelid, config );
}

int ipc_dev_register( ipc_dev_t *dev )
{
    if ( !dev ) {
        LOGE("check param error\n");
        goto err;
    }

    ipc = dev;
err:
    return -1;
}

void ipc_stop()
{
    if (ipc && ipc->stop)
        ipc->stop(ipc);
}

