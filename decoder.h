#ifndef __DECODER_H__
#define __DECODER_H__

#define __STDC_CONSTANT_MACROS
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

using CPcmCallback = int (*)(char *data, int len, int ts);

class Decoder {
  public:
    Decoder(std::string url, CPcmCallback cbk);
    ~Decoder();

  public:
    void decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame);
    void run();

  private:
    std::string _url;
    CPcmCallback _CPcmCallback;
    AVFormatContext *_fmt_ctx = nullptr;
    AVCodec *_codec;
    AVCodecContext *_c;
    int _video_index;
    int _audio_index;
    AVFrame *frame;
};

#endif
