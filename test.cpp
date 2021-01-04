#include "AudioDecoder.h"

using namespace std;

int main(int argc, char **argv) {
    AudioDecoder decoder(AV_CODEC_ID_MP3);
    string infile = "../../doc/test.mp3";
    string outfile = "../../doc/testout.pcm";
    decoder.decodeFile(infile, outfile);
    return 0;
}
