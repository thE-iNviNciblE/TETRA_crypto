// Copyright 2023, Midnight Blue.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "hurdle.h"
#include "common.h"

const uint8_t g_abHurdleSbox[256] = {
    0xF4, 0x65, 0x01, 0x00, 0xBA, 0x7A, 0xA7, 0x47, 0x98, 0xDD, 0x9D, 0xAD, 0x96, 0x5D, 0xAA, 0x3D, 
    0x58, 0xC0, 0x72, 0xD8, 0x66, 0x4C, 0x3E, 0xE0, 0x80, 0x55, 0xDE, 0x90, 0x2A, 0x4B, 0x83, 0xA0, 
    0x51, 0x39, 0xED, 0x6C, 0x8A, 0x2C, 0x56, 0x60, 0x4A, 0x1F, 0xD0, 0x70, 0x6E, 0x33, 0x8B, 0x26, 
    0x2E, 0x6F, 0x89, 0x48, 0x5E, 0x40, 0xC3, 0xA4, 0xA9, 0xCF, 0x22, 0x50, 0xE1, 0x15, 0x0C, 0xAB, 
    0xD5, 0xF8, 0x5F, 0x36, 0x04, 0xA6, 0x4E, 0x92, 0x1E, 0x2B, 0x88, 0x30, 0x93, 0x45, 0x67, 0x16, 
    0x8C, 0x68, 0x23, 0x38, 0x61, 0x25, 0x1A, 0x81, 0x63, 0xCB, 0xC1, 0x13, 0x41, 0x37, 0x0E, 0x97, 
    0x5B, 0xCA, 0x57, 0x24, 0x4D, 0x17, 0xC4, 0xB9, 0xB3, 0xEF, 0x8D, 0x52, 0x32, 0x2F, 0xEC, 0x20, 
    0xD9, 0x11, 0xD1, 0x28, 0x79, 0xDA, 0xFB, 0xE9, 0xBB, 0x06, 0x77, 0xDB, 0xFC, 0xFE, 0xCD, 0x84, 
    0x1D, 0xA1, 0x54, 0x1B, 0xB0, 0xE4, 0xCC, 0x7C, 0x2D, 0x27, 0x31, 0x49, 0xF5, 0x02, 0x69, 0x53, 
    0x4F, 0x44, 0xDF, 0x18, 0x5C, 0x0F, 0xBC, 0x9B, 0x94, 0xBD, 0xDC, 0x0B, 0xA2, 0xC7, 0x09, 0xAC, 
    0xC6, 0x9F, 0x82, 0x1C, 0x05, 0x46, 0xC2, 0x34, 0x3C, 0x0D, 0x3B, 0xCE, 0xB7, 0xBE, 0x08, 0x9C, 
    0x6B, 0xEE, 0xE5, 0x87, 0xAF, 0xBF, 0xF2, 0xEB, 0x7B, 0x07, 0x64, 0xC5, 0xB6, 0xAE, 0x9A, 0x95, 
    0x35, 0xA5, 0x59, 0x12, 0x9E, 0xA3, 0xB8, 0x8E, 0x5A, 0xF7, 0x62, 0xD2, 0x3A, 0xA8, 0x7D, 0x85, 
    0xF6, 0xC8, 0x71, 0x29, 0xD6, 0xD7, 0x43, 0xF9, 0x78, 0x76, 0x73, 0x10, 0x91, 0x19, 0x0A, 0x99, 
    0xF0, 0xE6, 0x3F, 0x14, 0xF1, 0xE2, 0xB1, 0x86, 0xB4, 0xF3, 0x74, 0xFA, 0x6A, 0xB2, 0x21, 0x6D, 
    0xEA, 0xB5, 0xE7, 0xE3, 0xC9, 0xD3, 0x8F, 0x03, 0x75, 0xE8, 0xD4, 0x42, 0xFD, 0x7E, 0xFF, 0x7F};

static int g_abRotations[] = {
    5, 5, 5, 5, 5, 3, 7, 5,
    5, 5, 5, 7, 3, 5, 5, 5
};

static const uint8_t g_abRoundConstant[16] = {
    0x3C, 0xA7, 0xEC, 0x25, 0x79, 0x57, 0xDF, 0xC0, 0x38, 0x0A, 0x33, 0x1E, 0xF3, 0x8C, 0xF4, 0xF7,
};

#if __BYTE_ORDER == __LITTLE_ENDIAN
static const uint32_t g_adwReorder[16] = {
    0x00000000, 0x80000000, 0x00800000, 0x80800000,
    0x00008000, 0x80008000, 0x00808000, 0x80808000,
    0x00000080, 0x80000080, 0x00800080, 0x80800080,
    0x00008080, 0x80008080, 0x00808080, 0x80808080
};
#else
static const uint32_t g_adwReorder[16] = {
    0x00000000, 0x00000080, 0x00008000, 0x00008080,
    0x00800000, 0x00800080, 0x00808000, 0x00808080,
    0x80000000, 0x80000080, 0x80008000, 0x80008080,
    0x80800000, 0x80800080, 0x80808000, 0x80808080
};
#endif


void HURDLE_set_key_fw(uint8_t *k, HURDLE_CTX *lpContextOut) {

    /*
     * This is the key schedule as recovered from the firmware, clearly showing
     * the relation between the round keys. 
     * Note that 
     * - every round key is computed from its predecessor,
     * - alternatively, a round key can be computed from its successor, and
     * - the keys follow a circular pattern, i.e. (hypothetical) round key 16
     * equals round key 0.
     *
     * these are most likely design choices in order to allow for both encryption
     * and decryption without having to pre-compute the round keys.
     */

    /* round key 0 is the actual key */
    int i, j;
    *(uint32_t *)&lpContextOut->abRoundKeys[0] = *(uint32_t *)&k[0];
    *(uint32_t *)&lpContextOut->abRoundKeys[4] = *(uint32_t *)&k[4];
    *(uint32_t *)&lpContextOut->abRoundKeys[8] = *(uint32_t *)&k[8];
    *(uint32_t *)&lpContextOut->abRoundKeys[12] = *(uint32_t *)&k[12];

    for (i = 1; i < 16; i++) {
        /* create a rotated copy of the previous round key */ 
        for (j = 0; j < 16; j++) {
            /* rotation is byte-wise in order to be endianess agnostic */
            lpContextOut->abRoundKeys[i*16 + j] = lpContextOut->abRoundKeys[(i-1)*16+((j+g_abRotations[i]) & 0xf)];
        }
        /* xor in the constant vector */
        *(uint32_t *)&lpContextOut->abRoundKeys[i*16]    ^= *(uint32_t *)&g_abRoundConstant[0];
        *(uint32_t *)&lpContextOut->abRoundKeys[i*16+4]  ^= *(uint32_t *)&g_abRoundConstant[4];
        *(uint32_t *)&lpContextOut->abRoundKeys[i*16+8]  ^= *(uint32_t *)&g_abRoundConstant[8];
        *(uint32_t *)&lpContextOut->abRoundKeys[i*16+12] ^= *(uint32_t *)&g_abRoundConstant[12];
    }
}

void HURDLE_set_key(uint8_t *k, HURDLE_CTX *lpContextOut) {

    // Simplified key schedule by precomputing rotates and xor constants
    uint8_t abKeyBytes[256] = {
        /*  0      1      2      3      4      5      6      7      8      9     10     11     12     13     14     15
        |-------- UNUSED --------|                                                          |------ BARELY USED -----|   */
        k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], 
        k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], 
        k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], 
        k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], 
        k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], 
        k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], 
        k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], 
        k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], 
        k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], 
        k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], 
        k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], 
        k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], 
        k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], 
        k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], 
        k[ 6], k[ 7], k[ 8], k[ 9], k[10], k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], 
        k[11], k[12], k[13], k[14], k[15], k[ 0], k[ 1], k[ 2], k[ 3], k[ 4], k[ 5], k[ 6], k[ 7], k[ 8], k[ 9], k[10]}; 
    static const uint8_t abKeyXorConsts[256] = {
        // 0      1      2      3      4      5      6      7      8      9     10     11     12     13     14     15
        0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,   // rk00
        0x3C,  0xA7,  0xEC,  0x25,  0x79,  0x57,  0xDF,  0xC0,  0x38,  0x0A,  0x33,  0x1E,  0xF3,  0x8C,  0xF4,  0xF7,   // rk01
        0x6B,  0x78,  0x2C,  0x1D,  0x73,  0x64,  0xC1,  0x33,  0xB4,  0xFE,  0xC4,  0x22,  0x54,  0x60,  0xD1,  0x8E,   // rk02
        0x58,  0x66,  0xDF,  0x91,  0x87,  0x93,  0xFD,  0x94,  0x58,  0xDB,  0xBD,  0x75,  0x8B,  0xA0,  0xE9,  0x84,   // rk03
        0xAF,  0x5A,  0x78,  0x7D,  0xA2,  0xEA,  0xAA,  0x4B,  0x98,  0xE3,  0xB7,  0x46,  0x95,  0x53,  0x65,  0x70,   // rk04
        0x41,  0x05,  0x06,  0x8F,  0x32,  0xCF,  0x3C,  0x77,  0x7E,  0x9F,  0x60,  0x7B,  0x83,  0x23,  0xAE,  0x8F,   // rk05
        0x4B,  0xD9,  0x73,  0x45,  0x02,  0xD4,  0xFC,  0x6E,  0xB7,  0x4B,  0x36,  0x18,  0x7C,  0xBE,  0x3B,  0xCB,   // rk06
        0xE8,  0x5B,  0x82,  0x92,  0x32,  0x61,  0xC7,  0xBC,  0x86,  0x31,  0xF8,  0x55,  0x2A,  0xFF,  0xB1,  0xF5,   // rk07
        0x5D,  0x60,  0x50,  0xA3,  0x48,  0xAF,  0x8A,  0xEA,  0xC7,  0xBB,  0xC6,  0xF6,  0xA8,  0x0E,  0x66,  0xC5,   // rk08
        0x93,  0x2D,  0x06,  0xE2,  0xC2,  0x91,  0x29,  0x68,  0x36,  0x6C,  0xF6,  0x43,  0x93,  0xDC,  0x57,  0xBF,   // rk09
        0xAD,  0x8E,  0x84,  0x13,  0x15,  0xA1,  0x9C,  0x53,  0xE4,  0x5D,  0x8C,  0x8D,  0xDE,  0x8A,  0x16,  0x35,   // rk10
        0x6F,  0x43,  0xB1,  0xA9,  0xF4,  0x89,  0x55,  0xD6,  0x0D,  0xA7,  0xBD,  0x9A,  0xE0,  0x99,  0x55,  0x6B,   // rk11
        0x95,  0x53,  0x65,  0x70,  0xAF,  0x5A,  0x78,  0x7D,  0xA2,  0xEA,  0xAA,  0x4B,  0x98,  0xE3,  0xB7,  0x46,   // rk12
        0x66,  0xDF,  0x91,  0x87,  0x93,  0xFD,  0x94,  0x58,  0xDB,  0xBD,  0x75,  0x8B,  0xA0,  0xE9,  0x84,  0x58,   // rk13
        0xC1,  0x33,  0xB4,  0xFE,  0xC4,  0x22,  0x54,  0x60,  0xD1,  0x8E,  0x6B,  0x78,  0x2C,  0x1D,  0x73,  0x64,   // rk14
        0x1E,  0xF3,  0x8C,  0xF4,  0xF7,  0x3C,  0xA7,  0xEC,  0x25,  0x79,  0x57,  0xDF,  0xC0,  0x38,  0x0A,  0x33};  // rk15

    // Xor original key byte with round- and offset-specific xor byte
    for (int i = 0; i < 256; i++) {
        lpContextOut->abRoundKeys[i] = abKeyBytes[i] ^ abKeyXorConsts[i];
    }

    // The first four bytes of each round key are not used and can be zeroed if desired
    // for (int i = 0; i < 16; i++) {
    //     lpContextOut->abRoundKeys[i*16] = 0;
    //     lpContextOut->abRoundKeys[i*16+1] = 0;
    //     lpContextOut->abRoundKeys[i*16+2] = 0;
    //     lpContextOut->abRoundKeys[i*16+3] = 0;
    // }
}

void HURDLE_f(uint8_t abOutput[4], const uint8_t abRhs[4], const uint8_t *lpRoundKey) {
    #define PUSH_OUTPUT_NIBBLE(x) do { \
        dwOutputBits >>= 1; \
        dwOutputBits |= g_adwReorder[(x) & 0xf]; \
    } while (0);

    uint32_t dwOutputBits = 0;
    uint8_t bSboxState = 0;

    bSboxState = g_abHurdleSbox[(abRhs[3] + lpRoundKey[15]) & 0xff];
    bSboxState = g_abHurdleSbox[((abRhs[2] + lpRoundKey[14]) ^ bSboxState) & 0xff];
    bSboxState = g_abHurdleSbox[((abRhs[1] + lpRoundKey[13]) ^ bSboxState) & 0xff];
    bSboxState = g_abHurdleSbox[((abRhs[0] + lpRoundKey[12]) ^ bSboxState) & 0xff];
    bSboxState = g_abHurdleSbox[((abRhs[3] + lpRoundKey[11]) ^ bSboxState) & 0xff]; PUSH_OUTPUT_NIBBLE(bSboxState); // Generates dwOutputBits & 0x01010101
    bSboxState = g_abHurdleSbox[((abRhs[1] + lpRoundKey[10]) ^ bSboxState) & 0xff]; PUSH_OUTPUT_NIBBLE(bSboxState); // Generates dwOutputBits & 0x02020202
    bSboxState = g_abHurdleSbox[((abRhs[2] + lpRoundKey[ 9]) ^ bSboxState) & 0xff]; PUSH_OUTPUT_NIBBLE(bSboxState); // Generates dwOutputBits & 0x04040404
    bSboxState = g_abHurdleSbox[((abRhs[0] + lpRoundKey[ 8]) ^ bSboxState) & 0xff]; PUSH_OUTPUT_NIBBLE(bSboxState); // Generates dwOutputBits & 0x08080808
    bSboxState = g_abHurdleSbox[((abRhs[1] + lpRoundKey[ 7]) ^ bSboxState) & 0xff]; PUSH_OUTPUT_NIBBLE(bSboxState); // Generates dwOutputBits & 0x10101010
    bSboxState = g_abHurdleSbox[((abRhs[3] + lpRoundKey[ 6]) ^ bSboxState) & 0xff]; PUSH_OUTPUT_NIBBLE(bSboxState); // Generates dwOutputBits & 0x20202020
    bSboxState = g_abHurdleSbox[((abRhs[0] + lpRoundKey[ 5]) ^ bSboxState) & 0xff]; PUSH_OUTPUT_NIBBLE(bSboxState); // Generates dwOutputBits & 0x40404040
    bSboxState = g_abHurdleSbox[((abRhs[2] + lpRoundKey[ 4]) ^ bSboxState) & 0xff]; PUSH_OUTPUT_NIBBLE(bSboxState); // Generates dwOutputBits & 0x80808080

    *(uint32_t *)abOutput = dwOutputBits;
}

void HURDLE_encrypt(uint8_t abOutput[8], const uint8_t abInput[8], HURDLE_CTX *lpKey, uint8_t eEncryptMode) {
    uint32_t dwLhs, dwRhs, dwTemp;
    int i;

    /* start at first/last round key depending on encrypt/decrypt mode */
    uint8_t *lpRoundKey = (eEncryptMode == HURDLE_DECRYPT) ? &lpKey->abRoundKeys[240] : lpKey->abRoundKeys;

    /* copy state */
    dwLhs = *(uint32_t *)&abInput[0];
    dwRhs = *(uint32_t *)&abInput[4];

    for (i = 0; i < 16; i++) {
        // Round function
        HURDLE_f((uint8_t *)&dwTemp, (uint8_t *)&dwRhs, lpRoundKey);

        /* perform a left-right switcharoo */
        // printf("lhs %08X rhs %08X + feedback %08X\n", dwLhs, dwRhs, dwTemp);
        dwTemp ^= dwLhs;
        dwLhs = dwRhs;
        dwRhs = dwTemp;

        /* move to next/previous round key depending on encrypt/decrypt mode */
        lpRoundKey += (eEncryptMode == HURDLE_DECRYPT) ? -16 : 16;
    }

    *(uint32_t *)&abOutput[0] = dwRhs;
    *(uint32_t *)&abOutput[4] = dwLhs;
}


void HURDLE_enc_cbc(uint8_t abCiphertext[16], const uint8_t abPlaintext[16], uint8_t abKey[16]) {
    // 0x8100a0
    uint8_t abIntermediate[8];
    HURDLE_CTX stCipher;

    HURDLE_set_key(abKey, &stCipher);
    HURDLE_encrypt(abCiphertext, abPlaintext, &stCipher, HURDLE_ENCRYPT);
    *(uint32_t *)&abIntermediate[0] = *(uint32_t *)&abCiphertext[0] ^ *(uint32_t *)&abPlaintext[8];
    *(uint32_t *)&abIntermediate[4] = *(uint32_t *)&abCiphertext[4] ^ *(uint32_t *)&abPlaintext[12];
    HURDLE_encrypt(&abCiphertext[8], abIntermediate, &stCipher, HURDLE_ENCRYPT);
}


void HURDLE_dec_cts(uint8_t abPlaintext[15], const uint8_t abCiphertext[15], uint8_t abKey[16]) {
    // 0x8100a0
    uint8_t abIntermediate[16];
    HURDLE_CTX stCipher;
    HURDLE_set_key(abKey, &stCipher);

    HURDLE_encrypt(&abIntermediate[8], &abCiphertext[7], &stCipher, HURDLE_DECRYPT);
    *(uint32_t *)&abIntermediate[0] = *(uint32_t *)&abCiphertext[0];
    *(uint32_t *)&abIntermediate[4] = *(uint32_t *)&abCiphertext[4];
    abIntermediate[7] = abIntermediate[15];
    HURDLE_encrypt(&abIntermediate[0], &abIntermediate[0], &stCipher, HURDLE_DECRYPT);
    *(uint32_t *)&abIntermediate[8]  ^= *(uint32_t *)&abCiphertext[0];
    *(uint16_t *)&abIntermediate[12] ^= *(uint16_t *)&abCiphertext[4];
    *(uint8_t  *)&abIntermediate[14] ^= *(uint8_t  *)&abCiphertext[6];

    *(uint32_t *)&abPlaintext[0]  = *(uint32_t *)&abIntermediate[0];
    *(uint32_t *)&abPlaintext[4]  = *(uint32_t *)&abIntermediate[4];
    *(uint32_t *)&abPlaintext[8]  = *(uint32_t *)&abIntermediate[8];
    *(uint16_t *)&abPlaintext[12] = *(uint16_t *)&abIntermediate[12];
    *(uint8_t  *)&abPlaintext[14] = *(uint8_t  *)&abIntermediate[14];
}
