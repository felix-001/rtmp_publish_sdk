// Last Update:2018-12-27 16:17:04
/**
 * @file rtmp_wapper.c
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-12-14
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dbg_internal.h"
#include "h264_decode.h"
#include "rtmp_publish.h"
#include "adts.h"
#include "rtmp_wapper.h"

#define MAX_NALUS_PER_FRAME 64
#define MAX_ADTS_PER_FRAME 128

RtmpContex * RtmpNewContext( const char * _url, unsigned int _nTimeout,
                                 RtmpPubAudioType _nInputAudioType,
                                 RtmpPubAudioType _nOutputAudioType,
                                 RtmpPubTimeStampPolicy _nTimePolic)
{
    RtmpContex *ctx = NULL;
    int ret = 0;

    if ( !_url ) {
        return NULL;
    }

    ctx = ( RtmpContex *) malloc ( sizeof(RtmpContex) );
    if ( !ctx  ) {
        LOG_E("malloc error\n");
        return NULL;
    }

    ctx->pPubCtx = RtmpPubNew( _url, _nTimeout, _nInputAudioType, _nOutputAudioType, _nTimePolic );
    if ( !ctx->pPubCtx ) {
        return NULL;
    }

    ret = RtmpPubInit( ctx->pPubCtx );
    if ( ret != 0 ) {
        RtmpPubDel( ctx->pPubCtx );
        return NULL;
    }

    ctx->nVideoRestart = 1;
    ctx->nAudioRestart = 1;

    return ctx;
}

int RtmpSendVideo( RtmpContex *_pConext, char *_pData,
                   unsigned int _nSize, int _nIsKey, unsigned int _nPresentationTime )
{
    NalUnit nalus[MAX_NALUS_PER_FRAME], *pNalu = nalus;
    int ret = 0, i = 0;
    int size = MAX_NALUS_PER_FRAME;
    char *pBuf = ( char *)malloc( _nSize ), *pBufAddr = pBuf;

    if ( !_pConext || !_pData ) {
        LOG_E("check param error\n");
        if ( pBuf ) {
            free( pBuf );
        }
        return -1;
    }

    if ( !pBuf ) {
        LOG_E("malloc error\n");
        return -1;
    }

    memset( pBuf, 0, _nSize );
    memset( nalus, 0, sizeof(nalus) );
    ret = H264ParseNalUnit( _pData, _nSize, nalus, &size );
    if ( ret != DECODE_OK ) {
        LOG_E("H264DecodeFrame error, ret = %d\n", ret );
        goto err;
    }

    if ( size < 0 ) {
        LOG_E("check size error\n");
        goto err;
    }

    for ( i=0; i<size; i++ ) {
        switch( pNalu->type ) {
        case NALU_TYPE_SPS:
            if ( pNalu->addr && pNalu->size > 0 && _pConext->nVideoRestart ) {
                LOGI("set sps\n");
                RtmpPubSetVideoTimebase( _pConext->pPubCtx, _nPresentationTime );
                RtmpPubSetSps( _pConext->pPubCtx, pNalu->addr, pNalu->size );
            } else {
                /* do nothing */
            }
            break;
        case NALU_TYPE_PPS:
            if ( pNalu->addr && pNalu->size > 0 && _pConext->nVideoRestart ) {
                LOGI("set pps\n");
                RtmpPubSetPps( _pConext->pPubCtx, pNalu->addr, pNalu->size );
                _pConext->nVideoRestart = 0;
            } else {
                /* do nothing */
            }
            break;
        case NALU_TYPE_IDR:
        case NALU_TYPE_SLICE:
            if ( pNalu->addr && pNalu->size > 0 ) {
                memcpy( pBuf, pNalu->addr, pNalu->size );
                pBuf += pNalu->size;
            } else {
                LOG_E("get nalu error, pNalu->addr = %p, pNalu->size = %d\n", pNalu->addr, pNalu->size );
            }
            break;
        default:
            break;
        }
        pNalu++;
    }

    if ( size >= MAX_NALUS_PER_FRAME ) {
        LOG_E("nalus over flow\n");
    }

    if ( _nIsKey ) {
        ret = RtmpPubSendVideoKeyframe( _pConext->pPubCtx, pBufAddr, pBuf-pBufAddr, _nPresentationTime );
        if ( ret != 0 ) {
            LOG_E("RtmpPubSendVideoKeyframe() error, ret = %d, errno = %d\n", ret, errno );
            goto err;
        }
    } else {
        ret = RtmpPubSendVideoInterframe( _pConext->pPubCtx, pBufAddr, pBuf-pBufAddr, _nPresentationTime );
        if ( ret != 0 ) {
            LOG_E("RtmpPubSendVideoInterframe() error, ret = %d, errno = %d\n", ret, errno );
            goto err;
        }
    }

    free( pBufAddr );
    return 0;
err:
    free( pBufAddr );
    return -1;
}

int RtmpSendAudio( RtmpContex *_pConext, char *_pData,
                   unsigned int _nSize, unsigned int _nPresentationTime )
{
    int ret = 0, nSize = MAX_ADTS_PER_FRAME, i = 0;
    Adts adts[ MAX_ADTS_PER_FRAME ], *pAdts = adts;
    char *pBuf = (char *) malloc ( _nSize ),  *pBufAddr = NULL;;

    if ( !_pConext || !_pData || _nSize == 0 ) {
        if ( pBuf ) {
            LOG_E("check param error\n");
            free( pBuf );
        }
        return -1;
    }

    if ( !pBuf ) {
        LOG_E("malloc error\n");
        return -1;
    }

    if ( !_pConext->pPubCtx ) {
        LOG_E("check pPubCtx error\n");
        return -1;
    }

    pBufAddr = pBuf;

    if ( _pConext->nAudioRestart ) {
        char audioSpecCfg[] = { 0x14, 0x10 };

        RtmpPubSetAudioTimebase( _pConext->pPubCtx, _nPresentationTime );
        RtmpPubSetAac( _pConext->pPubCtx, audioSpecCfg, sizeof(audioSpecCfg) );
        _pConext->nAudioRestart = 0;
    }
    memset( adts, 0, sizeof(adts) );
    ret = AacDecodeAdts( _pData, _nSize, adts, &nSize );
    if ( ret != ADTS_DECODE_OK ) {
        LOGI("adts decode fail, ret = %d\n", ret );
        goto err;
    }

    if ( nSize <= 0 || nSize >= MAX_ADTS_PER_FRAME ) {
        LOGI("check nSize error, nSize = %d\n", nSize );
        goto err;
    }

    memset( pBuf, 0, _nSize );
    for ( i=0; i<nSize; i++ ) {
        if ( pAdts->addr && pAdts->size > 0 ) {
            memcpy( pBuf, pAdts->addr, pAdts->size );
            pBuf += pAdts->size;
        } else {
            LOG_E("found invalid adts!!!\n");
        }
        pAdts++;
    }

    ret = RtmpPubSendAudioFrame( _pConext->pPubCtx, pBufAddr, pBuf - pBufAddr, _nPresentationTime );
    if ( ret < 0 ) {
        LOG_E("RtmpPubSendAudioFrame() error, ret = %d\n", ret );
        goto err;
    }

    free( pBufAddr );
    return 0;
err:
    free( pBufAddr );
    return -1;
}


int RtmpConnect( RtmpContex * _pConext)
{
    return (RtmpPubConnect(_pConext->pPubCtx) );
}

int RtmpDestroy( RtmpContex * _pConext )
{
    if ( !_pConext ) {
        LOG_E("check param error\n");
        return -1;
    }

    if ( _pConext->pPubCtx)
    RtmpPubDel( _pConext->pPubCtx );
    free( _pConext );

    return 0;
}

