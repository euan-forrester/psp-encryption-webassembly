// EMCC_CHANGE: This file is modeled off of the corresponding file in the PPSSPP repo, which contains the code that decrypted an executable.
//              Seemed liks as good a place as any to put our wrapper code

#include "Core/ELF/PrxDecrypter.h"
#include "ext/zlib/zlib.h"

extern "C"
{

int decrypt_executable(const u8 *inbuf, u8 *outbuf, u32 size)
{
    return pspDecryptPRX(inbuf, outbuf, size);
}

}
