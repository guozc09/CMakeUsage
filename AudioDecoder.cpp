#include "AudioDecoder.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#ifdef __cplusplus
}
#endif

using namespace std;

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

AudioDecoder::AudioDecoder(enum AVCodecID codecId) : mAVCodecCtx(nullptr), mChannels(0) {
    mPkt = av_packet_alloc();
    /* find the MPEG audio decoder */
    mAVCodec = avcodec_find_decoder(codecId);
    if (mAVCodec == nullptr) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    mParser = av_parser_init(mAVCodec->id);
    if (mParser == nullptr) {
        fprintf(stderr, "Parser not found\n");
        exit(1);
    }

    mAVCodecCtx = avcodec_alloc_context3(mAVCodec);
    if (mAVCodecCtx == nullptr) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }

    /* open it */
    if (avcodec_open2(mAVCodecCtx, mAVCodec, nullptr) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    mDecodedFrame = av_frame_alloc();
    if (mDecodedFrame == nullptr) {
        fprintf(stderr, "Could not allocate audio frame context\n");
        exit(1);
    }
}

AudioDecoder::~AudioDecoder() {
    if (mAVCodecCtx != nullptr)
        avcodec_free_context(&mAVCodecCtx);
    if (mParser != nullptr)
        av_parser_close(mParser);
    if (mDecodedFrame != nullptr)
        av_frame_free(&mDecodedFrame);
    if (mPkt != nullptr)
        av_packet_free(&mPkt);
}

void AudioDecoder::decodeFile(string &infileName, string &outfileName) {
    if (infileName.empty() || outfileName.empty()) {
        fprintf(stderr, "file name is empty!! do nothing!!\n");
        return;
    }
    /* decode until eof */
    int ret = 0;
    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    FILE *infile, *outfile;

    infile = fopen(infileName.c_str(), "rb");
    if (!infile) {
        fprintf(stderr, "Could not open %s\n", infileName.c_str());
        return;
    }
    outfile = fopen(outfileName.c_str(), "wb");
    if (!outfile) {
        fprintf(stderr, "Could not open %s\n", outfileName.c_str());
        fclose(infile);
        return;
    }

    uint8_t *data = inbuf;
    size_t data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, infile);
    while (data_size > 0) {
        ret = av_parser_parse2(mParser, mAVCodecCtx, &mPkt->data, &mPkt->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            return;
        }
        data += ret;
        data_size -= ret;

        if (mPkt->size) {
            decode(mAVCodecCtx, mPkt, mDecodedFrame, outfile);
        }

        if (data_size < AUDIO_REFILL_THRESH) {
            memmove(inbuf, data, data_size);
            data = inbuf;
            int len = fread(data + data_size, 1, AUDIO_INBUF_SIZE - data_size, infile);
            if (len > 0)
                data_size += len;
        }
    }

    /* flush the decoder */
    mPkt->data = nullptr;
    mPkt->size = 0;
    decode(mAVCodecCtx, mPkt, mDecodedFrame, outfile);

    /* print output pcm infomations, because there have no metadata of pcm */
    mSFmt = mAVCodecCtx->sample_fmt;

    if (av_sample_fmt_is_planar(mSFmt)) {
        const char *packed = av_get_sample_fmt_name(mSFmt);
        printf(
            "Warning: the sample format the decoder produced is planar "
            "(%s). This example will output the first channel only.\n",
            packed ? packed : "?");
        mSFmt = av_get_packed_sample_fmt(mSFmt);
    }

    mChannels = mAVCodecCtx->channels;

    const char *fmt = nullptr;
    ret = getFormatFromSampleFmt(&fmt, mSFmt);
    if (ret > 0) {
        printf(
            "Play the output audio file with the command:\n"
            "ffplay -infile %s -ac %d -ar %d %s\n",
            fmt, mChannels, mAVCodecCtx->sample_rate, outfileName.c_str());
    }

    fclose(infile);
    fclose(outfile);
}

int AudioDecoder::getFormatFromSampleFmt(const char **fmt, enum AVSampleFormat sample_fmt) {
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt;
        const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        {AV_SAMPLE_FMT_U8, "u8", "u8"},        {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
        {AV_SAMPLE_FMT_S32, "s32be", "s32le"}, {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
        {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };
    *fmt = nullptr;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr, "sample format %s is not supported as output format\n", av_get_sample_fmt_name(sample_fmt));
    return -1;
}

void AudioDecoder::decode(AVCodecContext *dec_ctx, AVPacket *pkt, AVFrame *frame, FILE *outfile) {
    int i = 0, ch = 0;
    int ret = 0, data_size = 0;

    /* send the packet with the compressed data to the decoder */
    ret = avcodec_send_packet(dec_ctx, pkt);
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