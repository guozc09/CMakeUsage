#include <iostream>
#include "decoder.h"

using namespace std;

Decoder::Decoder(string url, CPcmCallback cbk)
    : mUrl(url),
      mCbk(cbk),
      mAVFormatCtx(nullptr),
      mAVCodec(nullptr),
      mAVCodecCtx(nullptr),
      mVideoIndex(-1),
      mAudioIndex(-1) {
    av_register_all();
    avformat_network_init();

    if (avformat_open_input(&mAVFormatCtx, mUrl.c_str(), NULL, NULL) < 0) {
        cout << "avformat_open_input false" << endl;
    }

    if (avformat_find_stream_info(mAVFormatCtx, nullptr) < 0) {
        cout << "avformat_find_stream_info false" << endl;
    }

    for (int i = 0; i < mAVFormatCtx->nb_streams; i++) {
        if (mAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            mVideoIndex = i;
            continue;
        }

        if (mAVFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            mAudioIndex = i;
            continue;
        }
    }

    mAVCodecCtx = mAVFormatCtx->streams[mAudioIndex]->codec;
    this->mAVCodec = avcodec_find_decoder(mAVCodecCtx->codec_id);
    if (!mAVCodec) {
        cout << "Codec not found" << endl;
    }

    if (avcodec_open2(mAVCodecCtx, mAVCodec, NULL) < 0) {
        avcodec_free_context(&mAVCodecCtx);  // free it
        this->mAVCodecCtx = nullptr;
        cout << "Could not open codec" << endl;
    }
}

Decoder::~Decoder() {
    if (mAVCodecCtx != NULL) {
        avcodec_close(mAVCodecCtx);
    }

    if (mAVFormatCtx != nullptr) {
        if (!(mAVFormatCtx->flags & AVFMT_NOFILE)) {
            avio_close(mAVFormatCtx->pb);
            mAVFormatCtx->pb = nullptr;
        }

        avformat_close_input(&mAVFormatCtx);
        mAVFormatCtx = nullptr;
    }
}

void Decoder::run() {
    AVFrame *decoded_frame = NULL;
    if (!(decoded_frame = av_frame_alloc())) {
        return;
    }

    AVPacket pkt = {0};
    while (true) {
        if (av_read_frame(mAVFormatCtx, &pkt) < 0) {
            av_packet_unref(&pkt);
            break;
        }

        if (pkt.stream_index != mAudioIndex) {
            av_packet_unref(&pkt);
            continue;
        }

        decode(mAVCodecCtx, &pkt, decoded_frame);
        av_packet_unref(&pkt);
    }

    av_frame_free(&decoded_frame);
    av_packet_unref(&pkt);
}

void Decoder::decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame) {
    int ret;
    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        return;
    }

    while (ret >= 0) {
        av_frame_unref(frame);
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return;
        }

        else if (ret < 0) {
            return;
        }

        int buff_size =
            av_samples_get_buffer_size(frame->linesize, frame->channels, frame->nb_samples, dec_ctx->sample_fmt, 0);
        if (mCbk == nullptr) {
            return;
        }

        mCbk((char *)(frame->data[0]), buff_size, frame->pts);
    }
}
