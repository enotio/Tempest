#ifndef ETC_DEC_H
#define ETC_DEC_H

#include <cstdint>

typedef struct KTX_header_t {
  uint8_t  identifier[12];
  uint32_t endianness;
  uint32_t glType;
  uint32_t glTypeSize;
  uint32_t glFormat;
  uint32_t glInternalFormat;
  uint32_t glBaseInternalFormat;
  uint32_t pixelWidth;
  uint32_t pixelHeight;
  uint32_t pixelDepth;
  uint32_t numberOfArrayElements;
  uint32_t numberOfFaces;
  uint32_t numberOfMipmapLevels;
  uint32_t bytesOfKeyValueData;
} KTX_header;

#define KTX_ENDIAN_REF      (0x04030201)

void decompressBlockDiffFlip( uint32_t block_part1,
                              uint32_t block_part2,
                              uint8_t *img,
                              int width, int height,
                              int startx, int starty );

void decompressBlockDiffFlipC( uint32_t block_part1,
                               uint32_t block_part2,
                               uint8_t *img,
                               int width, int height,
                               int startx, int starty,
                               int channels);


void compressBlockDiffFlipFastPerceptual( uint8_t *img,
                                          uint8_t *imgdec,
                                          int width,int height,
                                          int startx,int starty,
                                          uint32_t &compressed1,
                                          uint32_t &compressed2);

#endif // ETC_DEC_H
