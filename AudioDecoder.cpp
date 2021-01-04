#include "AudioDecoder.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#ifdef __cplusplus
}
#endif

using namespace std;

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

AudioDecoder::AudioDecoder() : mASIndex(-1) {
    Trace(__FUNCTION__);
    mFormatCtx = avformat_alloc_context();
}

AudioDecoder::~AudioDecoder() {
    Trace(__FUNCTION__);
    if (mFormatCtx) {
        avformat_free_context(mFormatCtx);
    }
}

void AudioDecoder::decodeFile(string &infileName, string &outfileName) {
    Trace(__FUNCTION__);

    if (infileName.empty() || outfileName.empty()) {
        fprintf(stderr, "file name is empty!! do nothing!!\n");
        return;
    }

    AVStream *avStream = checkAVStream(infileName);
    AVCodecParameters *codecpar = avStream->codecpar;
    AVCodec *aCodec = avcodec_find_decoder(codecpar->codec_id);
    if (!aCodec) {
        fprintf(stderr, "avcodec_find_decoder failed!!\n");
        return;
    }
    /* open codec */
    AVCodecContext *aCodecCtx = avcodec_alloc_context3(aCodec);
    avcodec_parameters_to_context(aCodecCtx, codecpar);
    aCodecCtx->pkt_timebase = avStream->time_base;
    if (avcodec_open2(aCodecCtx, aCodec, nullptr) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return;
    }

    if (decodePacket(aCodecCtx, outfileName)) {
        fprintf(stderr, "decodePacket failed\n");
        return;
    }

    outputPcmInfo(aCodecCtx, outfileName);

    if (aCodecCtx) {
        avcodec_close(aCodecCtx);
        avcodec_free_context(&aCodecCtx);
    }
}

int AudioDecoder::getFormatFromSampleFmt(const char **fmt, enum AVSampleFormat sample_fmt) {
    Trace(__FUNCTION__);

    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        {AV_SAMPLE_FMT_U8, "u8", "u8"},        {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
        {AV_SAMPLE_FMT_S32, "s32be", "s32le"}, {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
        {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };
    *fmt = nullptr;

    for (size_t i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr, "sample format %s is not supported as output format\n", av_get_sample_fmt_name(sample_fmt));
    return -1;
}

void AudioDecoder::decode(AVCodecContext *dec_ctx, AVPacket *packet, AVFrame *frame, FILE *outfile) {
    Trace(__FUNCTION__);

    int i = 0, ch = 0;
    int ret = 0, data_size = 0;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, packet);
    if (ret < 0) {
        fprintf(stderr, "Error submitting the packet to the decoder, avcodec_send_packet errcode:%d\n", ret);
        return;
    }

    /* read all the output frames (in general there may be any number of them */
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            return;
        }
        data_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);
        if (data_size < 0) {
            /* This should not occur, checking just for paranoia */
            fprintf(stderr, "Failed to calculate data size\n");
            return;
        }
        for (i = 0; i < frame->nb_samples; i++)
            for (ch = 0; ch < dec_ctx->channels; ch++) fwrite(frame->data[ch] + data_size * i, 1, data_size, outfile);
    }

    return;
}

AVStream *AudioDecoder::checkAVStream(string &infileName) {
    int ret = 0;
    ret = avformat_open_input(&mFormatCtx, infileName.c_str(), nullptr, nullptr);
    if (ret != 0) {
        fprintf(stderr, "av_open_input_file failed ret:%d!!\n", ret);
        return nullptr;
    }

    ret = avformat_find_stream_info(mFormatCtx, nullptr);
    if (ret < 0) {
        printf("Couldn't find stream information ret :%d\n", ret);
        return nullptr;
    }
    mASIndex = -1;
    for (int i = 0; i < (int)(mFormatCtx->nb_streams); i++) {
        if (mFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            mASIndex = i;
            return mFormatCtx->streams[mASIndex];
        }
    }

    fprintf(stderr, "Do not find AVMEDIA_TYPE_AUDIO !!\n");
    return nullptr;
}

int AudioDecoder::decodePacket(AVCodecContext *aCodecCtx, string &outfileName) {
    FILE *outfile = fopen(outfileName.c_str(), "wb");
    if (!outfile) {
        fprintf(stderr, "Could not open %s\n", outfileName.c_str());
        return -1;
    }

    AVPacket *packet = av_packet_alloc();
    AVFrame *decodeFrame = av_frame_alloc();
    if (decodeFrame == nullptr) {
        fprintf(stderr, "Could not allocate audio frame context\n");
        av_packet_free(&packet);
        fclose(outfile);
        return -1;
    }
    while (av_read_frame(mFormatCtx, packet) >= 0) {
        if (packet->stream_index == mASIndex) {
            if (packet->size) {
                decode(aCodecCtx, packet, decodeFrame, outfile);
            }
        }
    }
    /* flush the decoder */
    packet->data = nullptr;
    packet->size = 0;
    decode(aCodecCtx, packet, decodeFrame, outfile);

    av_packet_free(&packet);
    av_frame_free(&decodeFrame);
    fclose(outfile);
    return 0;
}

void AudioDecoder::outputPcmInfo(AVCodecContext *aCodecCtx, string &outfileName) {
    /* print output pcm infomations, because there have no metadata of pcm */
    enum AVSampleFormat sFmt;
    sFmt = aCodecCtx->sample_fmt;

    if (av_sample_fmt_is_planar(sFmt)) {
        const char *packed = av_get_sample_fmt_name(sFmt);
        printf(
            "Warning: the sample format the decoder produced is planar "
            "(%s). This example will output the first channel only.\n",
            packed ? packed : "?");
        sFmt = av_get_packed_sample_fmt(sFmt);
    }

    int channels = aCodecCtx->channels;
    const char *fmt = nullptr;
    int ret = getFormatFromSampleFmt(&fmt, sFmt);
    if (!ret) {
        printf("Play %s with the parameter: format[%s] channels[%d] sample_rate[%d] \n",
               outfileName.c_str(), fmt, channels, aCodecCtx->sample_rate);
    }
}
