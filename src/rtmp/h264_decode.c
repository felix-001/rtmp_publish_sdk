// Last Update:2018-12-26 17:20:22
/**
 * @file h264_decode.c
 * @brief 
 * @author liyq
 * @version 0.1.00
 * @date 2018-12-14
 */

#include <string.h>
#include <arpa/inet.h>
#include "dbg_internal.h"
#include "h264_decode.h"

/*
 * the NAL start prefix code (it can also be 0x00000001, depends on the encoder implementation)
 * */
#define NALU_START_CODE (0x00000001)

static int H264DecodeNalu( int *_pIndex, char *_pData, int _nStartCodeLen, OUT NalUnit *_pNalus, int _nMax )
{
    NalUnit *pNalu = _pNalus + *_pIndex;

    if ( !_pNalus ) {
        return DECODE_PARARM_ERROR;
    }

    pNalu->addr = _pData;
    pNalu->type = (*_pData) & 0x1F;
    if ( *_pIndex > 0 )
        ( _pNalus + *_pIndex - 1)->size = _pData - ( _pNalus + *_pIndex - 1 )->addr - _nStartCodeLen;

    (*_pIndex) ++;
    if ( *_pIndex >= _nMax ) {
        return DECODE_BUF_OVERFLOW;
    }
    return 0;
}

int H264DecodeFrame( char *_pFrame, int _nLen, OUT NalUnit *_pNalus, int *_pSize )
{
    char *pStart = _pFrame, *pEnd = pStart + _nLen;
    unsigned int *pStartCode = NULL, ret = 0;
    int nIndex = 0;

    if ( !_pFrame || _nLen <= 0 || !_pNalus || !_pSize ) {
        return DECODE_PARARM_ERROR;
    }

    while( pStart <= pEnd ) {
        pStartCode = (unsigned int *)pStart;
        if ( htonl(*pStartCode) == NALU_START_CODE ) {
            pStart += 4;// skip start code
            ret = H264DecodeNalu( &nIndex, pStart, 4, _pNalus, *_pSize );
            if ( ret < 0 ) {
                return DECODE_FRAME_FAIL;
            }
        } else if (( htonl(*pStartCode) >> 8 ) == NALU_START_CODE ) {
            pStart += 3;
            ret = H264DecodeNalu( &nIndex, pStart, 3,  _pNalus, *_pSize );
            if ( ret < 0 ) {
                return DECODE_FRAME_FAIL;
            }
        } else {
            pStart++;
        }
    }

    /* the last one */
    _pNalus += nIndex -1 ;
    if ( nIndex == 1 ) {
        _pNalus->size = pEnd - _pNalus ->addr ;
    } else {
        _pNalus->size = pEnd - ( _pNalus - 1 )->addr ;
    }

    *_pSize = nIndex;

    return DECODE_OK;
}

