#ifndef __DECODER_H__
#define __DECODER_H__

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <memory>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif
#include "libavformat/avformat.h"
#include "libavutil/avassert.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/timestamp.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#ifdef __cplusplus
}
#endif

using namespace std;

using CPcmCallback = int (*)(char *data, int len, int ts);

class Decoder {
  public:
    Decoder(string url, CPcmCallback cbk);
    ~Decoder();

  public:
    void run();
    void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame);

  private:
    string mUrl;
    CPcmCallback mCbk;
    AVFormatContext *mAVFormatCtx;
    AVCodec *mAVCodec;
    AVCodecContext *mAVCodecCtx;
    int mVideoIndex;
    int mAudioIndex;
};

#endif
