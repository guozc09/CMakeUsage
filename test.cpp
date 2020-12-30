#include <stdio.h>

#include "decoder.h"

FILE *fb;

int PcmCallback(char *data, int lenl, int ts) {
    printf("data len=%d\n", lenl);
    fwrite(data, lenl, 1, fb);
    return 0;
}

int main(int argc, char **argv) {
    fb = fopen("./out.pcm", "w+");
    Decoder *tf = new Decoder("./62bff14f534995249b89f1bf86e9ea68.ts", &PcmCallback);
    tf->run();
    delete tf;
    fclose(fb);
    return 0;
}