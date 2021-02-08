# ipc-rtmp
rtmp推流sdk

# feature list
- [x] 低内存占用
- [x] 支持h264/h265
- [x] 支持g711/aac
- [x] rtmp推流

# 编译
- mkdir build
- cd build
- cmake ..
- make -j10

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

# sample
[示例代码](./src/sample/main.c)

# author
- treeswayinwind@gmail.com
- 企鹅: 279191230
