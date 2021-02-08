# ipc-rtmp
摄像头rtmp推流

# API
## 创建rtmp推流实例
```
RtmpContex * RtmpNewContext( const char * _url, unsigned int _nTimeout,
                                 RtmpPubAudioType _nInputAudioType,
                                 RtmpPubAudioType _nOutputAudioType,
                                 RtmpPubTimeStampPolicy _nTimePolic);
```

## 连接rtmp流媒体服务
```
int RtmpConnect( RtmpContex * _pConext);
```

## 发送音频帧
```
int RtmpSendAudio( RtmpContex *_pConext, char *_pData,
                   unsigned int _nSize, unsigned int _nPresentationTime );
```

## 发送视频帧
```
int RtmpSendVideo( RtmpContex *_pConext, char *_pData,
                   unsigned int _nSize, int _nIsKey, unsigned int _nPresentationTime );
```

## 删除rtmp推流实例
```
int RtmpDestroy( RtmpContex * _pConext );
```

# author
- treeswayinwind@gmail.com
- 企鹅: 279191230
