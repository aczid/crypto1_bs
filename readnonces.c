#include <stdio.h>
#include <stdlib.h>
#include "craptev1.h"
#include <string.h>

int readnonces(FILE *f, uint64_t *nonces) {
    int i, j;
    uint32_t nt;
    char par;
    long pos;

    i = 0;
    while(!feof(f)){
        nonces[i] = 0;
        pos = ftell(f);
        for(j = 0; j < 32; j += 8) {
            if(2 != fscanf(f, "%02x%c ", &nt, &par)) {
                fseek(f, pos, SEEK_SET);
                return i;
            }
            nonces[i] |= nt << j | (uint64_t)((par == '!') ^ parity(nt)) << (32 + j);
        }
        i++;
    }
    return i;
}