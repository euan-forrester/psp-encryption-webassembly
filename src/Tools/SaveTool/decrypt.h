/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * decrypt.h - Declarations for functions in decrypt.c
 *
 * Copyright (c) 2005 Jim Paris <jim@jtan.com>
 * Coypright (c) 2005 psp123
 *
 * $Id: decrypt.h 1562 2005-12-10 20:52:45Z jim $
 */

//#include <pspchnnlsv.h> // EMCC_CHANGE: Not actually needed for this file. See changes in decrypt.cpp

/* Detect the samegame format and decrypt it.  See main.c for an example.  */
int decrypt_file(const char *decrypted_filename,
         const char *encrypted_filename,
         const unsigned char *gamekey,
         const int mainSdkVersion);

/* Do the actual hardware decryption.
   mode is 3 for saves with a cryptkey, or 1 otherwise.
   data, alignedLen, and cryptkey must be multiples of 0x10.
   cryptkey is NULL if mode == 1.
*/
extern "C" { // EMCC_CHANGE: See change to .cpp at top of decrypt.cpp
int decrypt_data(unsigned int mode, 
         unsigned char *data,
         int *dataLen,
         int *alignedLen,
         unsigned char *cryptkey);
}