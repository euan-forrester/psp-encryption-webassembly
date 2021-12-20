/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * decrypt.c - Decryption routines using sceChnnlsv
 *
 * Copyright (c) 2005 Jim Paris <jim@jtan.com>
 * Coypright (c) 2005 psp123
 *
 * $Id: decrypt.c 1562 2005-12-10 20:52:45Z jim $
 */

// EMCC_CHANGE: Renamed to .cpp because the change of include file below ends up including C++-specific stuff

#include "decrypt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//#include <pspchnnlsv.h> // EMCC_CHANGE: This file is in the PSP SDK, and equivalent functionality can be found in the file below
#include "Core/HLE/sceChnnlsv.h" // EMCC_CHANGE: Replaces the file above
//#include "kernelcall/kernelcall.h" // EMCC_CHANGE: Replaced calls to the functions listed in this file with ones in the file above, to eliminate a bunch of roundaboutness that gets us to the same functions anyway

unsigned int align16(unsigned int v)
{
    return ((v + 0xF) >> 4) << 4;
}

/* Read, decrypt, and write a savedata file.  See main.c for example usage. */
int decrypt_file(const char *decrypted_filename,
         const char *encrypted_filename,
         const unsigned char *gamekey,
         const int mainSdkVersion)
{
    FILE *in, *out;
    int len, aligned_len;
    unsigned char *data, *cryptkey;
    int retval;

    /* Open file and get size */

    if ((in = fopen(encrypted_filename, "r")) == NULL) {
        retval = -1;
        goto out;
    }

    fseek(in, 0, SEEK_END);
    len = ftell(in);
    fseek(in, 0, SEEK_SET);

    if (len <= 0) {
        retval = -2;
        goto out1;
    }

    /* Allocate buffers */

    aligned_len = align16(len);

    if ((data = (unsigned char *) memalign(0x10, aligned_len)) == NULL) {
        retval = -3;
        goto out1;
    }

    if ((cryptkey = (unsigned char *) memalign(0x10, 0x10)) == NULL) {
        retval = -4;
        goto out2;
    }

    /* Fill buffers */

    if (gamekey != NULL)
        memcpy(cryptkey, gamekey, 0x10);

    memset(data + len, 0, aligned_len - len);
    if (fread(data, 1, len, in) != len) {
        retval = -5;
        goto out3;
    }

    /* Do the decryption */

    if ((retval = decrypt_data( gamekey ? (mainSdkVersion >= 4 ? 5 : 3) : 1, // 5 for sdk >= 4, else 3
                    data, &len, &aligned_len,
                    gamekey ? cryptkey : NULL)) < 0) {
        retval -= 100;
        goto out3;
    }

    /* Write the data out.  decrypt_data has set len correctly. */

    if ((out = fopen(decrypted_filename, "w")) == NULL) {
        retval = -6;
        goto out3;
    }

    if (fwrite(data, 1, len, out) != len) {
        retval = -7;
        goto out4;
    }

    /* All done.  Return file length. */
    retval = len;
 out4:
    fclose(out);
 out3:
    free(cryptkey);
 out2:
    free(data);
 out1:
    fclose(in);
 out:
    return retval;
}

extern "C" { // begin EMCC_CHANGE: This is a wrapper for decrypt_data(), similar to decrypt_file() above, but intended for passing in a memory buffer from javascript

// Note that the contents of data and len will be changed by this function

int decrypt_save_buffer(unsigned char *data, int *len, unsigned char *gamekey)
{
    if (!data || !len || !gamekey) {
        return -1;
    }

    int aligned_len = align16(*len);

    int retval = 0;

    const int mainSdkVersion = 6; // ???

    unsigned char *aligned_data = NULL;
    unsigned char *aligned_gamekey = NULL;

    if ((aligned_data = (unsigned char *) aligned_alloc(0x10, aligned_len)) == NULL) {
        retval = -3;
        goto cleanup;
    }

    if (gamekey) {
        if ((aligned_gamekey = (unsigned char *) aligned_alloc(0x10, 0x10)) == NULL) {
            retval = -4;
            goto cleanup;
        }
    }

    if (gamekey) memcpy(aligned_gamekey, gamekey, 0x10);
    memset(aligned_data, 0, aligned_len);
    memcpy(aligned_data, data, *len);

    if ((retval = decrypt_data(gamekey ? (mainSdkVersion >= 4 ? 5 : 3) : 1, // 5 for sdk >= 4, else 3
                    aligned_data, len, &aligned_len,
                    aligned_gamekey)) < 0) {
        retval -= 100;
        goto cleanup;
    }

    memcpy(data, aligned_data, *len);

cleanup:
    if (aligned_gamekey)    free(aligned_gamekey);
    if (aligned_data)       free(aligned_data);

    return retval;
}

} // end EMCC_CHANGE

/* Do the actual hardware decryption.
   mode is 3 for saves with a cryptkey, or 1 otherwise
   data, dataLen, and cryptkey must be multiples of 0x10.
   cryptkey is NULL if mode == 1.
*/
int decrypt_data(unsigned int mode, 
         unsigned char *data,
         int *dataLen,
         int *alignedLen,
         unsigned char *cryptkey)
{
    pspChnnlsvContext1 ctx1;
    pspChnnlsvContext2 ctx2;

    /* Need a 16-byte IV plus some data */
    if (*alignedLen <= 0x10)
        return -1;
    *dataLen -= 0x10;
    *alignedLen -= 0x10;

    /* Set up buffers */
    memset(&ctx1, 0, sizeof(pspChnnlsvContext1));
    memset(&ctx2, 0, sizeof(pspChnnlsvContext2));

    /* Perform the magic */
    //if (sceChnnlsv_E7833020_(&ctx1, mode) < 0) // EMCC_CHANGE
    if (sceSdSetIndex_(ctx1, mode) < 0)
        return -2;
    //if (sceChnnlsv_ABFDFC8B_(&ctx2, mode, 2, data, cryptkey) < 0) // EMCC_CHANGE
    if (sceSdCreateList_(ctx2, mode, 2, data, cryptkey) < 0)
        return -3;
    //if (sceChnnlsv_F21A1FCA_(&ctx1, data, 0x10) < 0) // EMCC_CHANGE
    if (sceSdRemoveValue_(ctx1, data, 0x10) < 0)
        return -4;
    //if (sceChnnlsv_F21A1FCA_(&ctx1, data + 0x10, *alignedLen) < 0) // EMCC_CHANGE
    if (sceSdRemoveValue_(ctx1, data + 0x10, *alignedLen) < 0)
        return -5;
    //if (sceChnnlsv_850A7FA1_(&ctx2, data + 0x10, *alignedLen) < 0) // EMCC_CHANGE
    if (sceSdSetMember_(ctx2, data + 0x10, *alignedLen) < 0)
        return -6;

    /* Verify that it decrypted correctly */
    //if (sceChnnlsv_21BE78B4_(&ctx2) < 0) // EMCC_CHANGE
    if (sceChnnlsv_21BE78B4_(ctx2) < 0)
        return -7;

    /* If desired, a new file hash from this PSP can be computed now:
       if (sceChnnlsv_C4C494F8(ctx1, newhash, cryptkey) < 0)
        return -8;
    */

    /* The decrypted data starts at data + 0x10, so shift it back. */
    memmove(data, data + 0x10, *dataLen);

    /* All done */
    return 0;
}
