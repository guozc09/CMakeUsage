#include <fstream>
#include <iostream>

#include "AudioDecoder.h"

using namespace std;

int main(int argc, char **argv) {
    ofstream ofs;
    ofs.open("./out.pcm", ios::out | ios::app | ios::binary);
    AudioDecoder decoder(AV_CODEC_ID_MP3);
    decoder.decode(ofs);
    ofs.close();
    return 0;
}
