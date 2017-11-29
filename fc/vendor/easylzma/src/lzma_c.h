#ifndef __LZMA_C_H__
#define __LZMA_C_H__

#include <stdlib.h>

#include "easylzma/compress.h"
#include "easylzma/decompress.h"

#ifdef __cplusplus
extern "C" {
#endif

/* compress a chunk of memory and return a dynamically allocated buffer
* if successful. return value is an easylzma error code */
int simpleCompress(elzma_file_format format,
                   const unsigned char * inData,
                   size_t inLen,
                   unsigned char ** outData,
                   size_t * outLen);

/* decompress a chunk of memory and return a dynamically allocated buffer
* if successful. return value is an easylzma error code */
int simpleDecompress(elzma_file_format format,
                     const unsigned char * inData,
                     size_t inLen,
                     unsigned char ** outData,
                     size_t * outLen);

#ifdef __cplusplus
};
#endif

#endif

