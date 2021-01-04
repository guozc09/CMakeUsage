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
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif

using namespace std;

class Trace {
  public:
    Trace(const char* func) : mFunc(func) {
      printf("%s in\n", mFunc);
    }
    ~Trace() {
      printf("%s out\n", mFunc);
    }
  private:
    const char* mFunc;
};

class AudioDecoder {
  public:
    AudioDecoder();
    ~AudioDecoder();

    void decodeFile(string &infileName, string &outfileName);

  private:
    AVStream *checkAVStream(string &infileName);
    int getFormatFromSampleFmt(const char **fmt, enum AVSampleFormat sample_fmt);
    void decode(AVCodecContext *decCtx, AVPacket *pkt, AVFrame *frame, FILE *outfile);
    int decodePacket(AVCodecContext *aCodecCtx, string &outfileName);
    void outputPcmInfo(AVCodecContext *aCodecCtx, string &outfileName);

    AVFormatContext *mFormatCtx;
    int mASIndex;
};

#endif
