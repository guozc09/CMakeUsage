#include <stdio.h>

#include <fstream>
#include <iostream>

#include "decoder.h"

using namespace std;

ofstream ofs;

int pcmCallback(char *data, int dataLen, int ts) {
    cout << "data len=" << dataLen << endl;
    ofs.write(data, dataLen);
    return 0;
}

int main(int argc, char **argv) {
    ofs.open("./out.pcm", ios::out | ios::app | ios::binary);
    Decoder decoder("./62bff14f534995249b89f1bf86e9ea68.ts", &pcmCallback);
    decoder.run();
    ofs.close();
    return 0;
}
