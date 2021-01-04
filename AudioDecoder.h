#ifndef __AUDIO_DECODER_H__
#define __AUDIO_DECODER_H__

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <memory>
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif

using namespace std;

class AudioDecoder {
  public:
    AudioDecoder(enum AVCodecID codecId);
    ~AudioDecoder();

    void decodeFile(string &infileName, string &outfileName);

  private:
    int getFormatFromSampleFmt(const char **fmt, enum AVSampleFormat sample_fmt);
    void decode(AVCodecContext *decCtx, AVPacket *pkt, AVFrame *frame, FILE *outfile);

    AVCodec *mAVCodec;
    AVCodecContext *mAVCodecCtx;
    AVCodecParserContext *mParser;
    AVPacket *mPkt;
    AVFrame *mDecodedFrame;
    enum AVSampleFormat mSFmt;
    int mChannels;
};

#endif
