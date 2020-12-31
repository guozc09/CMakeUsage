#include <fstream>
#include <iostream>
#include "AudioDecoder.h"

using namespace std;

AudioDecoder::AudioDecoder(enum AVCodecID codecId)
    : mAVCodecCtx(nullptr) {
    mPkt = av_packet_alloc();
    /* find the MPEG audio decoder */
    mAVCodec = avcodec_find_decoder(codecId);
    if (mAVCodec == nullptr) {
        cerr << "Codec not found\n";
        exit(1);
    }

    mParser = av_parser_init(mAVCodec->id);
    if (mParser == nullptr) {
        cerr << "Parser not found\n";
        exit(1);
    }

    mAVCodecCtx = avcodec_alloc_context3(mAVCodec);
    if (mAVCodecCtx == nullptr) {
        cerr << "Could not allocate audio codec context\n";
        exit(1);
    }

    /* open it */
    if (avcodec_open2(mAVCodecCtx, mAVCodec, NULL) < 0) {
        cerr << "Could not open codec\n";
        exit(1);
    }
}

AudioDecoder::~AudioDecoder() {
    if (mAVCodecCtx != nullptr)
        avcodec_free_context(&mAVCodecCtx);
    if (mParser != nullptr)
        av_parser_close(mParser);
    if (mPkt != nullptr)
        av_packet_free(&mPkt);
}

void AudioDecoder::decode(ofstream &ofs) {

}
