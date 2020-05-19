// Last Update:2018-12-27 16:04:43
/**
 * @file rtmp_wapper.h
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-12-16
 */

#ifndef RTMP_WAPPER_H
#define RTMP_WAPPER_H


#include "rtmp_publish.h"

typedef struct {
    RtmpPubContext *pPubCtx;
    int nVideoRestart;
    int nAudioRestart;
} RtmpContex;

extern RtmpContex * RtmpNewContext( const char * _url, unsigned int _nTimeout,
                                 RtmpPubAudioType _nInputAudioType,
                                 RtmpPubAudioType _nOutputAudioType,
                                 RtmpPubTimeStampPolicy _nTimePolic);
extern int RtmpConnect( RtmpContex * _pConext);
extern int RtmpSendAudio( RtmpContex *_pConext, char *_pData,
                   unsigned int _nSize, unsigned int _nPresentationTime );
extern int RtmpSendVideo( RtmpContex *_pConext, char *_pData,
                   unsigned int _nSize, int _nIsKey, unsigned int _nPresentationTime );
extern int RtmpDestroy( RtmpContex * _pConext );

#endif  /*RTMP_WAPPER_H*/
