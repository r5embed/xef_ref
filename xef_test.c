//  xef_test.c
//  2018-10-26 Markku-Juhani O. Saarinen <mjos@pqshield,com>
//  (c) 2018   PQShield Ltd. All Rights Reserved.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "xef.h"

// Check for up to 9 errors
#define MAX_ERR 9

// Really quick test; timeout at 100ms for each case
#define TEST_TIMEOUT (CLOCKS_PER_SEC / 10)

// in xef_ref.c

extern const int xef_reg[5][3][10];

// fast randomized testing; returns the highest 100% rate

int xef_rand_tests(int f, size_t len)
{
    size_t i, j, l1, l2;
    uint8_t v1[256], v2[256];
    uint64_t n, ne, ok;
    clock_t timeout;
    int pl, bit, good, ep[MAX_ERR];

    uint64_t tot;

    memset(v1, 0, sizeof(v1));
    memset(v2, 0, sizeof(v2));

    pl = (len - 16) / 8;

    good = -1;
    n = 0;
    for (i = 0; i < 2 * f; i++)
        n += xef_reg[f - 1][pl][i];

    printf("| (%3d) XE%d-%d\t",  (int) (8 * len), f, (int) n);
    fflush(stdout);

    tot = 0;

    for (ne = 0; ne <= MAX_ERR; ne++) {

        n = 0;
        ok = 0;

        timeout = clock() + TEST_TIMEOUT;

        while (clock() < timeout) {

            n++;
            for (i = 0; i < len; i++) {
                v1[i] = rand() & 0xFF;
            }

            // copy
            memcpy(v2, v1, len);

            // user reference implementation to create the code
            l1 = xef_compute(v2, len, f);

            if (l1 <= 8 * len)
                continue;

            // error positions anywhere, including the code
            i = 0;
            while (i < ne) {
                bit = rand() % l1;
                for (j = 0; j < i; j++) {
                    if (ep[j] == bit)
                        break;
                }
                if (i == j) {
                    v2[bit >> 3] ^= 1 << (bit & 7);
                    ep[i++] = bit;
                }
            }

            // generate the difference
            // (test available optimized implementations in decoding)

            if (f == 5) {
                switch (len) {
                    case 16:
                        xe5_190_compute(v2);
                        xe5_190_fixerr(v2);
                        break;

                    case 24:
                        xe5_218_compute(v2);
                        xe5_218_fixerr(v2);
                        break;

                    case 32:
                        xe5_234_compute(v2);
                        xe5_234_fixerr(v2);
                        break;
                }
            } else {
                if (f == 2 && len == 16) {
                    xe2_53_compute(v2);
                    xe2_53_fixerr(v2);
                } else {
                    if (f == 4 && len == 24) {
                        xe4_163_compute(v2);
                        xe4_163_fixerr(v2);
                    } else {
                        // reference if not optimized not vailable
                        l2 = xef_compute(v2, len, f);
                        assert(l1 == l2);
                        l2 = xef_fixerr(v2, len, f);
                        assert(l2 == l1);
                    }
                }
            }

            l2 = (l1 + 7) >> 3;             // bytes

            if (memcmp(v1, v2, len) == 0) {

                // success
                ok++;

            } else {

               // code failed

                if (ne <= f) {
                    printf("\nFAIL!  n = %lu  ep=", n);
                    for (i = 0; i < ne; i++) {
                        printf(" %3d", ep[i]);
                    }

                    printf("\nDelta = ");
                    for (i = 0; i < l2; i++) {
                        if (i == len)
                            printf(":");
                        printf("%02X", v1[i] ^ v2[i]);
                    }
                    printf("\n");

                    memset(v2, 0, sizeof(v2));
                    for (i = 0; i < ne; i++) {
                        bit = ep[i];
                        v2[bit >> 3] ^= 1 << (bit & 7);
                    }
                    printf("Flips = ");
                    for (i = 0; i < l2; i++) {
                        if (i == len)
                            printf(":");
                        printf("%02X", v2[i]);
                    }
                    printf("\n");

                    exit(-1);

                    return ne - 1;
                }
            }

            memset(v2, 0, l2);

            // overflow ?
            assert(v2[l2] == 0);
        }

        printf("|%5.1f", 100.0 * ((double) ok) / ((double) n));
        fflush(stdout);

        if (ok == n)
            good = ne;
        tot += n;
    }
    printf("| %d\n", good);

    return good;
}

// stub main

int main(int argc, char **argv)
{
    int f;
    size_t len;

    srand(time(NULL));

    printf("| Errors        |  0  |  1  |  2  |  3  |  4  "
                           "|  5  |  6  |  7  |  8  |  9  |\n");
    printf("|---------------|-----|-----|-----|-----|-----"
                           "|-----|-----|-----|-----|-----|\n");

    for (f = 1; f <= 5; f++) {
        for (len = 16; len <= 32; len += 8) {
            xef_rand_tests(f, len);
        }
    }

    return 0;
}

