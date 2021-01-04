#include "AudioDecoder.h"

using namespace std;

int main(int argc, char **argv) {
    AudioDecoder decoder;
    string fileName = "../../doc/那女孩对我说";
    string infile = fileName + ".mp3";
    string outfile = fileName + ".pcm";
    decoder.decodeFile(infile, outfile);
    return 0;
}
