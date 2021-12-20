/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * encrypt.c - Encryption routines using sceChnnlsv
 *
 * Copyright (c) 2005 Jim Paris <jim@jtan.com>
 * Coypright (c) 2005 psp123
 *
 * $Id: encrypt.c 1560 2005-12-10 01:16:32Z jim $
 */

// EMCC_CHANGE: Renamed to .cpp because the change of include file below ends up including C++-specific stuff

#include "encrypt.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//#include <pspchnnlsv.h> // EMCC_CHANGE: This file is in the PSP SDK, and equivalent functionality can be found in the file below
#include "Core/HLE/sceChnnlsv.h" // EMCC_CHANGE: Replaces the file above
//#include "kernelcall/kernelcall.h" // EMCC_CHANGE: Replaced calls to the functions listed in this file with ones in the file above, to eliminate a bunch of roundaboutness that gets us to the same functions anyway

static inline int align16(unsigned int v)
{
    return ((v + 0xF) >> 4) << 4;
}

int fopen_getsize(const char *filename, FILE **fd, int *size)
{
    if ((*fd = fopen(filename, "r")) == NULL)
        return -1;

    fseek(*fd, 0, SEEK_END);
    *size = ftell(*fd);
    fseek(*fd, 0, SEEK_SET);

    if (*size <= 0) {
        fclose(*fd);
        return -2;
    }

    return 0;
}

/* Encrypt the given plaintext file, and update the message
   authentication hashes in the param.sfo.  The data_filename is
   usually the final component of encrypted_filename, e.g. "DATA.BIN".
   See main.c for an example of usage. */
int encrypt_file(const char *plaintext_filename,
         const char *encrypted_filename,
         const char *data_filename,
         const char *paramsfo_filename,
         const char *paramsfo_filename_out,
         const unsigned char *gamekey,
         const int mainSdkVersion)
{
    FILE *in = NULL, *out = NULL, *sfo = NULL;
    unsigned char *data = NULL, *cryptkey = NULL, *hash = NULL;
    unsigned char paramsfo[0x1330];
    int len, aligned_len, tmp;
    int retval;

    /* Open plaintext and param.sfo files and get size */
    
    if (fopen_getsize(plaintext_filename, &in, &len) < 0) {
        retval = -1;
        goto out;
    }

    if (fopen_getsize(paramsfo_filename, &sfo, &tmp) < 0) {
        retval = -2;
        goto out;
    }

    /* Verify size of param.sfo; all known saves use this size */

    if (tmp != 0x1330) { 
        retval = -3;
        goto out;
    }

    /* Allocate buffers.  data has 0x10 bytes extra for the IV. */

    aligned_len = align16(len);

    if ((data = 
         (unsigned char *) memalign(0x10, aligned_len + 0x10)) == NULL) {
        retval = -4;
        goto out;
    }

    if ((cryptkey = (unsigned char *) memalign(0x10, 0x10)) == NULL) {
        retval = -5;
        goto out;
    }

    if ((hash = (unsigned char *) memalign(0x10, 0x10)) == NULL) {
        retval = -6;
        goto out;
    }

    /* Fill buffers. */

    memset(data + len, 0, aligned_len - len);
    if (fread(data, 1, len, in) != len) {
        retval = -7;
        goto out;
    }

    if (fread(paramsfo, 1, 0x1330, sfo) != 0x1330) {
        retval = -8;
        goto out;
    }

    if (gamekey != NULL)
        memcpy(cryptkey, gamekey, 0x10);

    /* Do the encryption */

    if ((retval = encrypt_data( gamekey ? (mainSdkVersion >= 4 ? 5 : 3) : 1,    // 5 for sdk >= 4, 3 otherwise
                    data, 
                    &len, &aligned_len,
                    hash, 
                    gamekey ? cryptkey : NULL)) < 0) {
        retval -= 1000;
        goto out;
    }

    /* Update the param.sfo hashes */

    if ((retval = update_hashes(paramsfo, 0x1330, 
                    data_filename, hash,
                    gamekey ? 3 : 1)) < 0) {
        retval -= 2000;
        goto out;
    }
    
    /* Write the data to the file.  encrypt_data has already set len. */

    if ((out = fopen(encrypted_filename, "w")) == NULL) {
        retval = -9;
        goto out;
    }

    if (fwrite(data, 1, len, out) != len) {
        retval = -10;
        goto out;
    }

        /* Reopen param.sfo, and write the updated copy out. */

    fclose(sfo);  
    if ((sfo = fopen(paramsfo_filename_out, "w")) == NULL) {
        retval = -11;
        goto out;
    }

    if (fwrite(paramsfo, 1, 0x1330, sfo) != 0x1330) {
        retval = -12;
        goto out;
    }

    /* All done.  Return file length. */

    retval = len;

 out:
    if(out) fclose(out);
    if(hash) free(hash);
    if(cryptkey) free(cryptkey);
    if(data) free(data);
    if(sfo) fclose(sfo);
    if(in) fclose(in);

    return retval;
}

extern "C" { // begin EMCC_CHANGE: This is a wrapper for encrypt_data(), similar to encrypt_file() above, but intended for passing in a memory buffer from javascript

// Note that output_filename is the basename of the file which will be written out (e.g. "DRACULA.BIN")
// Note that the contents of data, data_len, and paramssfo_data will all be changed by this function

int encrypt_save_buffer(unsigned char *data, int *data_len, unsigned char *paramssfo_data, int paramssfo_len, char *output_filename, unsigned char *gamekey)
{
    if (!data || !data_len || !paramssfo_data || !paramssfo_len || !output_filename || !gamekey) {
        return -1;
    }

    if (paramssfo_len != 0x1330) {
        return -2;
    }

    int retval = 0;

    int aligned_data_len = align16(*data_len);

    unsigned char *aligned_data = NULL;
    unsigned char *aligned_gamekey = NULL;
    unsigned char *hash = NULL;

    if ((aligned_data = (unsigned char *) aligned_alloc(0x10, aligned_data_len)) == NULL) {
        retval = -3;
        goto cleanup;
    }

    if (gamekey) {
        if ((aligned_gamekey = (unsigned char *) aligned_alloc(0x10, 0x10)) == NULL) {
            retval = -4;
            goto cleanup;
        }
    }

    if ((hash = (unsigned char *) aligned_alloc(0x10, 0x10)) == NULL) {
        retval = -5;
        goto cleanup;
    }

    if (gamekey) {
        memcpy(aligned_gamekey, gamekey, 0x10);
    }
    memset(aligned_data, 0, aligned_data_len);
    memcpy(aligned_data, data, *data_len);

    /* Do the encryption */

    if ((retval = encrypt_data( gamekey ? (5) : 1,
                                data,
                                data_len, &aligned_data_len,
                                hash,
                                aligned_gamekey)) < 0) {
        retval -= 1000;
        goto cleanup;
    }

    /* Update the param.sfo hashes */

    if ((retval = update_hashes(paramssfo_data, paramssfo_len,
                                output_filename, hash,
                                paramssfo_data[0x11b0]>>4/*gamekey ? 3 : 1*/)) < 0) { // Copied from https://github.com/BrianBTB/SED-PC/blob/master/SED/encrypt.cpp#L198
        retval -= 2000;
        goto cleanup;
    }

    memcpy(data, aligned_data, *data_len);

cleanup:
    if (hash)               free(hash);
    if (aligned_gamekey)    free(aligned_gamekey);
    if (aligned_data)       free(aligned_data);

    return retval;
}

} // end EMCC_CHANGE

/* Do the actual hardware encryption.
   mode is 3 for saves with a cryptkey, or 1 otherwise
   data, dataLen, and cryptkey must be multiples of 0x10.
   cryptkey is NULL if mode == 1.
*/
int encrypt_data(unsigned int mode, 
         unsigned char *data,
         int *dataLen,
         int *alignedLen,
         unsigned char *hash,
         unsigned char *cryptkey)
{
    pspChnnlsvContext1 ctx1;
    pspChnnlsvContext2 ctx2;

    /* Make room for the IV in front of the data. */
    memmove(data + 0x10, data, *alignedLen);
    
    /* Set up buffers */
    memset(&ctx1, 0, sizeof(pspChnnlsvContext1));
    memset(&ctx2, 0, sizeof(pspChnnlsvContext2));
    memset(hash, 0, 0x10);
    memset(data, 0, 0x10);

    /* Build the 0x10-byte IV and setup encryption */
        //if (sceChnnlsv_ABFDFC8B_(&ctx2, mode, 1, data, cryptkey) < 0) // EMCC_CHANGE
        if (sceSdCreateList_(ctx2, mode, 1, data, cryptkey) < 0)
                return -1;
        //if (sceChnnlsv_E7833020_(&ctx1, mode) < 0)
        if (sceSdSetIndex_(ctx1, mode) < 0) // EMCC_CHANGE
                return -2;
        //if (sceChnnlsv_F21A1FCA_(&ctx1, data, 0x10) < 0) // EMCC_CHANGE
        if (sceSdRemoveValue_(ctx1, data, 0x10) < 0)
                return -3;
        //if (sceChnnlsv_850A7FA1_(&ctx2, data + 0x10, *alignedLen) < 0) // EMCC_CHANGE
        if (sceSdSetMember_(ctx2, data + 0x10, *alignedLen) < 0)
                return -4;
    
    /* Clear any extra bytes left from the previous steps */
    memset(data + 0x10 + *dataLen, 0, *alignedLen - *dataLen);

    /* Encrypt the data */
        //if (sceChnnlsv_F21A1FCA_(&ctx1, data + 0x10, *alignedLen) < 0) // EMCC_CHANGE
        if (sceSdRemoveValue_(ctx1, data + 0x10, *alignedLen) < 0)
                return -5;

    /* Verify encryption */
        //if (sceChnnlsv_21BE78B4_(&ctx2) < 0) // EMCC_CHANGE
        if (sceChnnlsv_21BE78B4_(ctx2) < 0)
                return -6;

    /* Build the file hash from this PSP */
    //if (sceChnnlsv_C4C494F8_(&ctx1, hash, cryptkey) < 0) // EMCC_CHANGE
    if (sceSdGetLastIndex_(ctx1, hash, cryptkey) < 0)
                return -7;

    /* Adjust sizes to account for IV */
    *alignedLen += 0x10;
    *dataLen += 0x10;

    /* All done */
    return 0;
}
