#include "etc_dec.h"

#define GETBITSHIGH(source, size, startpos)  (( (source) >> (((startpos)-32)-(size)+1) ) & ((1<<(size)) -1))
#define GETBITS(source, size, startpos)  (( (source) >> ((startpos)-(size)+1) ) & ((1<<(size)) -1))

#define   RED_CHANNEL(img,width,x,y,channels) img[channels*(y*width+x)+0]
#define GREEN_CHANNEL(img,width,x,y,channels) img[channels*(y*width+x)+1]
#define  BLUE_CHANNEL(img,width,x,y,channels) img[channels*(y*width+x)+2]
#define ALPHA_CHANNEL(img,width,x,y,channels) img[channels*(y*width+x)+3]

#define   RED(img,width,x,y) img[3*(y*width+x)+0]
#define GREEN(img,width,x,y) img[3*(y*width+x)+1]
#define  BLUE(img,width,x,y) img[3*(y*width+x)+2]

#define CLAMP(ll,x,ul) (((x)<(ll)) ? (ll) : (((x)>(ul)) ? (ul) : (x)))
#define JAS_ROUND(x) (((x) < 0.0 ) ? ((int)((x)-0.5)) : ((int)((x)+0.5)))

#define PUTBITS( dest, data, size, startpos) dest |= ( (data) & ((1<<(size))-1) ) << ((startpos)-(size)+1)
#define PUTBITSHIGH( dest, data, size, startpos) dest |= ( (data) & ((1<<(size))-1) ) << (((startpos)-32)-(size)+1)

static const float PERCEPTUAL_WEIGHT_R_SQUARED = 0.299;
static const float PERCEPTUAL_WEIGHT_G_SQUARED = 0.587;
static const float PERCEPTUAL_WEIGHT_B_SQUARED = 0.114;

template< class T >
static T SQUARE( const T x ){
  return x*x;
  }

static int compressParams[16][4] = { {-8, -2,  2, 8},
                                     {-8, -2,  2, 8},
                                     {-17, -5, 5, 17},
                                     {-17, -5, 5, 17},
                                     {-29, -9, 9, 29},
                                     {-29, -9, 9, 29},
                                     {-42, -13, 13, 42},
                                     {-42, -13, 13, 42},
                                     {-60, -18, 18, 60},
                                     {-60, -18, 18, 60},
                                     {-80, -24, 24, 80},
                                     {-80, -24, 24, 80},
                                     {-106, -33, 33, 106},
                                     {-106, -33, 33, 106},
                                     {-183, -47, 47, 183},
                                     {-183, -47, 47, 183}};

#define compressParamsEnc compressParams

static int unscramble[4] = {2, 3, 1, 0};
static int   scramble[4] = {3, 2, 0, 1};

void decompressBlockDiffFlip( uint32_t block_part1,
                              uint32_t block_part2,
                              uint8_t *img,
                              int width, int height,
                              int startx, int starty ) {
  decompressBlockDiffFlipC(block_part1, block_part2, img, width, height, startx, starty, 3);
  }

void decompressBlockDiffFlipC( uint32_t block_part1,
                               uint32_t block_part2,
                               uint8_t *img,
                               int width,  int height,
                               int startx, int starty,
                               int channels) {
  (void)height;

  uint8_t avg_color[3], enc_color1[3], enc_color2[3];
  signed char diff[3];
  int table;
  int index,shift;
  int r,g,b;
  int diffbit;
  int flipbit;

  diffbit = (GETBITSHIGH(block_part1, 1, 33));
  flipbit = (GETBITSHIGH(block_part1, 1, 32));

  if( !diffbit ) {
    // We have diffbit = 0.

    // First decode left part of block.
    avg_color[0]= GETBITSHIGH(block_part1, 4, 63);
    avg_color[1]= GETBITSHIGH(block_part1, 4, 55);
    avg_color[2]= GETBITSHIGH(block_part1, 4, 47);

    // Here, we should really multiply by 17 instead of 16. This can
    // be done by just copying the four lower bits to the upper ones
    // while keeping the lower bits.
    avg_color[0] |= (avg_color[0] <<4);
    avg_color[1] |= (avg_color[1] <<4);
    avg_color[2] |= (avg_color[2] <<4);

    table = GETBITSHIGH(block_part1, 3, 39) << 1;

    unsigned int pixel_indices_MSB, pixel_indices_LSB;

    pixel_indices_MSB = GETBITS(block_part2, 16, 31);
    pixel_indices_LSB = GETBITS(block_part2, 16, 15);

    if( (flipbit) == 0 )
    {
      // We should not flip
      shift = 0;
      for(int x=startx; x<startx+2; x++)
      {
        for(int y=starty; y<starty+4; y++)
        {
          index  = ((pixel_indices_MSB >> shift) & 1) << 1;
          index |= ((pixel_indices_LSB >> shift) & 1);
          shift++;
          index=unscramble[index];

          r=RED_CHANNEL  (img,width,x,y,channels) = CLAMP(0,avg_color[0]+compressParams[table][index],255);
          g=GREEN_CHANNEL(img,width,x,y,channels) = CLAMP(0,avg_color[1]+compressParams[table][index],255);
          b=BLUE_CHANNEL (img,width,x,y,channels) = CLAMP(0,avg_color[2]+compressParams[table][index],255);
        }
      }
    }
    else
    {
      // We should flip
      shift = 0;
      for(int x=startx; x<startx+4; x++)
      {
        for(int y=starty; y<starty+2; y++)
        {
          index  = ((pixel_indices_MSB >> shift) & 1) << 1;
          index |= ((pixel_indices_LSB >> shift) & 1);
          shift++;
          index=unscramble[index];

          r=RED_CHANNEL(img,width,x,y,channels)  =CLAMP(0,avg_color[0]+compressParams[table][index],255);
          g=GREEN_CHANNEL(img,width,x,y,channels)=CLAMP(0,avg_color[1]+compressParams[table][index],255);
          b=BLUE_CHANNEL(img,width,x,y,channels) =CLAMP(0,avg_color[2]+compressParams[table][index],255);
        }
        shift+=2;
      }
    }

    // Now decode other part of block.
    avg_color[0]= GETBITSHIGH(block_part1, 4, 59);
    avg_color[1]= GETBITSHIGH(block_part1, 4, 51);
    avg_color[2]= GETBITSHIGH(block_part1, 4, 43);

    // Here, we should really multiply by 17 instead of 16. This can
    // be done by just copying the four lower bits to the upper ones
    // while keeping the lower bits.
    avg_color[0] |= (avg_color[0] <<4);
    avg_color[1] |= (avg_color[1] <<4);
    avg_color[2] |= (avg_color[2] <<4);

    table = GETBITSHIGH(block_part1, 3, 36) << 1;
    pixel_indices_MSB = GETBITS(block_part2, 16, 31);
    pixel_indices_LSB = GETBITS(block_part2, 16, 15);

    if( (flipbit) == 0 )
    {
      // We should not flip
      shift=8;
      for(int x=startx+2; x<startx+4; x++)
      {
        for(int y=starty; y<starty+4; y++)
        {
          index  = ((pixel_indices_MSB >> shift) & 1) << 1;
          index |= ((pixel_indices_LSB >> shift) & 1);
          shift++;
          index=unscramble[index];

          r=RED_CHANNEL(img,width,x,y,channels)  =CLAMP(0,avg_color[0]+compressParams[table][index],255);
          g=GREEN_CHANNEL(img,width,x,y,channels)=CLAMP(0,avg_color[1]+compressParams[table][index],255);
          b=BLUE_CHANNEL(img,width,x,y,channels) =CLAMP(0,avg_color[2]+compressParams[table][index],255);
        }
      }
    }
    else
    {
      // We should flip
      shift=2;
      for(int x=startx; x<startx+4; x++)
      {
        for(int y=starty+2; y<starty+4; y++)
        {
          index  = ((pixel_indices_MSB >> shift) & 1) << 1;
          index |= ((pixel_indices_LSB >> shift) & 1);
          shift++;
          index=unscramble[index];

          r=RED_CHANNEL(img,width,x,y,channels)  =CLAMP(0,avg_color[0]+compressParams[table][index],255);
          g=GREEN_CHANNEL(img,width,x,y,channels)=CLAMP(0,avg_color[1]+compressParams[table][index],255);
          b=BLUE_CHANNEL(img,width,x,y,channels) =CLAMP(0,avg_color[2]+compressParams[table][index],255);
        }
        shift += 2;
      }
    }
  }
  else
  {
    // We have diffbit = 1.

    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1    | dcol 2 | base col1    | dcol 2 | base col 1   | dcol 2 | table  | table  |diff|flip|
    //     | R1' (5 bits) | dR2    | G1' (5 bits) | dG2    | B1' (5 bits) | dB2    | cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------
    //
    //
    //     c) bit layout in bits 31 through 0 (in both cases)
    //
    //      31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3   2   1  0
    //      --------------------------------------------------------------------------------------------------
    //     |       most significant pixel index bits       |         least significant pixel index bits       |
    //     | p| o| n| m| l| k| j| i| h| g| f| e| d| c| b| a| p| o| n| m| l| k| j| i| h| g| f| e| d| c | b | a |
    //      --------------------------------------------------------------------------------------------------

    // First decode left part of block.
    enc_color1[0]= GETBITSHIGH(block_part1, 5, 63);
    enc_color1[1]= GETBITSHIGH(block_part1, 5, 55);
    enc_color1[2]= GETBITSHIGH(block_part1, 5, 47);

    // Expand from 5 to 8 bits
    avg_color[0] = (enc_color1[0] <<3) | (enc_color1[0] >> 2);
    avg_color[1] = (enc_color1[1] <<3) | (enc_color1[1] >> 2);
    avg_color[2] = (enc_color1[2] <<3) | (enc_color1[2] >> 2);

    table = GETBITSHIGH(block_part1, 3, 39) << 1;

    unsigned int pixel_indices_MSB, pixel_indices_LSB;

    pixel_indices_MSB = GETBITS(block_part2, 16, 31);
    pixel_indices_LSB = GETBITS(block_part2, 16, 15);

    if( (flipbit) == 0 )
    {
      // We should not flip
      shift = 0;
      for(int x=startx; x<startx+2; x++)
      {
        for(int y=starty; y<starty+4; y++)
        {
          index  = ((pixel_indices_MSB >> shift) & 1) << 1;
          index |= ((pixel_indices_LSB >> shift) & 1);
          shift++;
          index=unscramble[index];

          r=RED_CHANNEL(img,width,x,y,channels)  =CLAMP(0,avg_color[0]+compressParams[table][index],255);
          g=GREEN_CHANNEL(img,width,x,y,channels)=CLAMP(0,avg_color[1]+compressParams[table][index],255);
          b=BLUE_CHANNEL(img,width,x,y,channels) =CLAMP(0,avg_color[2]+compressParams[table][index],255);
        }
      }
    }
    else
    {
      // We should flip
      shift = 0;
      for(int x=startx; x<startx+4; x++)
      {
        for(int y=starty; y<starty+2; y++)
        {
          index  = ((pixel_indices_MSB >> shift) & 1) << 1;
          index |= ((pixel_indices_LSB >> shift) & 1);
          shift++;
          index=unscramble[index];

          r=RED_CHANNEL(img,width,x,y,channels)  =CLAMP(0,avg_color[0]+compressParams[table][index],255);
          g=GREEN_CHANNEL(img,width,x,y,channels)=CLAMP(0,avg_color[1]+compressParams[table][index],255);
          b=BLUE_CHANNEL(img,width,x,y,channels) =CLAMP(0,avg_color[2]+compressParams[table][index],255);
        }
        shift+=2;
      }
    }

    // Now decode right part of block.
    diff[0]= GETBITSHIGH(block_part1, 3, 58);
    diff[1]= GETBITSHIGH(block_part1, 3, 50);
    diff[2]= GETBITSHIGH(block_part1, 3, 42);

    // Extend sign bit to entire byte.
    diff[0] = (diff[0] << 5);
    diff[1] = (diff[1] << 5);
    diff[2] = (diff[2] << 5);
    diff[0] = diff[0] >> 5;
    diff[1] = diff[1] >> 5;
    diff[2] = diff[2] >> 5;

    //  Calculale second color
    enc_color2[0]= enc_color1[0] + diff[0];
    enc_color2[1]= enc_color1[1] + diff[1];
    enc_color2[2]= enc_color1[2] + diff[2];

    // Expand from 5 to 8 bits
    avg_color[0] = (enc_color2[0] <<3) | (enc_color2[0] >> 2);
    avg_color[1] = (enc_color2[1] <<3) | (enc_color2[1] >> 2);
    avg_color[2] = (enc_color2[2] <<3) | (enc_color2[2] >> 2);

    table = GETBITSHIGH(block_part1, 3, 36) << 1;
    pixel_indices_MSB = GETBITS(block_part2, 16, 31);
    pixel_indices_LSB = GETBITS(block_part2, 16, 15);

    if( (flipbit) == 0 )
    {
      // We should not flip
      shift=8;
      for(int x=startx+2; x<startx+4; x++)
      {
        for(int y=starty; y<starty+4; y++)
        {
          index  = ((pixel_indices_MSB >> shift) & 1) << 1;
          index |= ((pixel_indices_LSB >> shift) & 1);
          shift++;
          index=unscramble[index];

          r=RED_CHANNEL(img,width,x,y,channels)  =CLAMP(0,avg_color[0]+compressParams[table][index],255);
          g=GREEN_CHANNEL(img,width,x,y,channels)=CLAMP(0,avg_color[1]+compressParams[table][index],255);
          b=BLUE_CHANNEL(img,width,x,y,channels) =CLAMP(0,avg_color[2]+compressParams[table][index],255);
        }
      }
    }
    else
    {
      // We should flip
      shift=2;
      for(int x=startx; x<startx+4; x++) {
        for(int y=starty+2; y<starty+4; y++) {
          index  = ((pixel_indices_MSB >> shift) & 1) << 1;
          index |= ((pixel_indices_LSB >> shift) & 1);
          shift++;
          index=unscramble[index];

          r=RED_CHANNEL(img,width,x,y,channels)  = CLAMP(0,avg_color[0]+compressParams[table][index],255);
          g=GREEN_CHANNEL(img,width,x,y,channels)= CLAMP(0,avg_color[1]+compressParams[table][index],255);
          b=BLUE_CHANNEL(img,width,x,y,channels) = CLAMP(0,avg_color[2]+compressParams[table][index],255);
          }
        shift += 2;
        }
      }
    }
  }

float compressBlockWithTable4x2percep( uint8_t *img,
                                       int width,int height,
                                       int startx,int starty,
                                       uint8_t *avg_color,
                                       int table,
                                       unsigned int *pixel_indices_MSBp,
                                       unsigned int *pixel_indices_LSBp) {
  (void)height;

  uint8_t orig[3],approx[3];
  unsigned int pixel_indices_MSB=0, pixel_indices_LSB=0, pixel_indices = 0;
  float sum_error=0;
  int q;
  int i;
  float wR2 = (float) PERCEPTUAL_WEIGHT_R_SQUARED;
  float wG2 = (float) PERCEPTUAL_WEIGHT_G_SQUARED;
  float wB2 = (float) PERCEPTUAL_WEIGHT_B_SQUARED;

  i = 0;
  for(int x=startx; x<startx+4; x++) {
    for(int y=starty; y<starty+2; y++) {
      float err;
      int best=0;
      float min_error=255*255*3*16;
      orig[0]=RED(img,width,x,y);
      orig[1]=GREEN(img,width,x,y);
      orig[2]=BLUE(img,width,x,y);

      for(q=0;q<4;q++) {
        approx[0]=CLAMP(0, avg_color[0]+compressParamsEnc[table][q],255);
        approx[1]=CLAMP(0, avg_color[1]+compressParamsEnc[table][q],255);
        approx[2]=CLAMP(0, avg_color[2]+compressParamsEnc[table][q],255);

        // Here we just use equal weights to R, G and B. Although this will
        // give visually worse results, it will give a better PSNR score.
        err=(float) wR2*SQUARE(approx[0]-orig[0]) + (float)wG2*SQUARE(approx[1]-orig[1]) + (float)wB2*SQUARE(approx[2]-orig[2]);
        if(err<min_error) {
          min_error=err;
          best=q;
          }
        }

      pixel_indices = scramble[best];

      PUTBITS( pixel_indices_MSB, (pixel_indices >> 1), 1, i);
      PUTBITS( pixel_indices_LSB, (pixel_indices & 1) , 1, i);
      i++;

      // In order to simplify hardware, the table {-12, -4, 4, 12} is indexed {11, 10, 00, 01}
      // so that first bit is sign bit and the other bit is size bit (4 or 12).
      // This means that we have to scramble the bits before storing them.

      sum_error+=min_error;
      }
    i+=2;
    }

  *pixel_indices_MSBp = pixel_indices_MSB;
  *pixel_indices_LSBp = pixel_indices_LSB;


  return sum_error;
  }

float compressBlockWithTable2x4percep( uint8_t *img,
                                       int width,int height,
                                       int startx,int starty,
                                       uint8_t *avg_color,
                                       int table,
                                       unsigned int *pixel_indices_MSBp,
                                       unsigned int *pixel_indices_LSBp ) {
  (void)height;

  uint8_t orig[3],approx[3];
  unsigned int pixel_indices_MSB=0, pixel_indices_LSB=0, pixel_indices = 0;
  float sum_error=0;
  int q, i;

  double wR2 = PERCEPTUAL_WEIGHT_R_SQUARED;
  double wG2 = PERCEPTUAL_WEIGHT_G_SQUARED;
  double wB2 = PERCEPTUAL_WEIGHT_B_SQUARED;

  i = 0;
  for(int x=startx; x<startx+2; x++)
  {
    for(int y=starty; y<starty+4; y++)
    {
      float err;
      int best=0;
      float min_error=255*255*3*16;
      orig[0]=RED(img,width,x,y);
      orig[1]=GREEN(img,width,x,y);
      orig[2]=BLUE(img,width,x,y);

      for(q=0;q<4;q++)
      {
        approx[0]=CLAMP(0, avg_color[0]+compressParamsEnc[table][q],255);
        approx[1]=CLAMP(0, avg_color[1]+compressParamsEnc[table][q],255);
        approx[2]=CLAMP(0, avg_color[2]+compressParamsEnc[table][q],255);

        // Here we just use equal weights to R, G and B. Although this will
        // give visually worse results, it will give a better PSNR score.
          err=(float)(wR2*SQUARE((approx[0]-orig[0])) + (float)wG2*SQUARE((approx[1]-orig[1])) + (float)wB2*SQUARE((approx[2]-orig[2])));
        if(err<min_error)
        {
          min_error=err;
          best=q;
        }

      }

      pixel_indices = scramble[best];

      PUTBITS( pixel_indices_MSB, (pixel_indices >> 1), 1, i);
      PUTBITS( pixel_indices_LSB, (pixel_indices & 1) , 1, i);

      i++;

      // In order to simplify hardware, the table {-12, -4, 4, 12} is indexed {11, 10, 00, 01}
      // so that first bit is sign bit and the other bit is size bit (4 or 12).
      // This means that we have to scramble the bits before storing them.


      sum_error+=min_error;
    }

  }

  *pixel_indices_MSBp = pixel_indices_MSB;
  *pixel_indices_LSBp = pixel_indices_LSB;

  return sum_error;
  }

int tryalltables_3bittable2x4percep( uint8_t *img,
                                     int width,int height,
                                     int startx,int starty,
                                     uint8_t *avg_color,
                                     unsigned int &best_table,
                                     unsigned int &best_pixel_indices_MSB,
                                     unsigned int &best_pixel_indices_LSB ) {
  float min_error = 3*255*255*16;
  int q;
  float err;
  unsigned int pixel_indices_MSB, pixel_indices_LSB;

  for(q=0;q<16;q+=2) {	// try all the 8 tables.

    err=compressBlockWithTable2x4percep(img,width,height,startx,starty,avg_color,q,&pixel_indices_MSB, &pixel_indices_LSB);

    if(err<min_error){
      min_error=err;
      best_pixel_indices_MSB = pixel_indices_MSB;
      best_pixel_indices_LSB = pixel_indices_LSB;
      best_table=q >> 1;
      }
    }

  return (int) min_error;
  }

int tryalltables_3bittable4x2percep( uint8_t *img,
                                     int width,int height,
                                     int startx,int starty,
                                     uint8_t *avg_color,
                                     unsigned int &best_table,
                                     unsigned int &best_pixel_indices_MSB,
                                     unsigned int &best_pixel_indices_LSB ) {
  float min_error = 3*255*255*16;
  float err;
  unsigned int pixel_indices_MSB, pixel_indices_LSB;

  for(int q=0;q<16;q+=2) {	// try all the 8 tables.
    err=compressBlockWithTable4x2percep(img,width,height,startx,starty,avg_color,q,&pixel_indices_MSB, &pixel_indices_LSB);

    if(err<min_error){

      min_error=err;
      best_pixel_indices_MSB = pixel_indices_MSB;
      best_pixel_indices_LSB = pixel_indices_LSB;
      best_table=q >> 1;
      }
    }

  return (int) min_error;
  }


void computeAverageColor2x4noQuantFloat(uint8_t *img,int width,int height,int startx,int starty,float *avg_color) {
  (void)height;

  int r=0,g=0,b=0;
  for(int y=starty; y<starty+4; y++)
  {
    for(int x=startx; x<startx+2; x++)
    {
      r+=RED(img,width,x,y);
      g+=GREEN(img,width,x,y);
      b+=BLUE(img,width,x,y);
    }
  }

  avg_color[0]=(float)(r/8.0);
  avg_color[1]=(float)(g/8.0);
  avg_color[2]=(float)(b/8.0);
  }

void computeAverageColor4x2noQuantFloat( uint8_t *img,
                                         int width,int height,
                                         int startx,int starty,
                                         float *avg_color) {
  (void)height;

  int r=0,g=0,b=0;
  for(int y=starty; y<starty+2; y++)
  {
    for(int x=startx; x<startx+4; x++)
    {
      r+=RED(img,width,x,y);
      g+=GREEN(img,width,x,y);
      b+=BLUE(img,width,x,y);
    }
  }

  avg_color[0]=(float)(r/8.0);
  avg_color[1]=(float)(g/8.0);
  avg_color[2]=(float)(b/8.0);
  }

void compressBlockDiffFlipAveragePerceptual( uint8_t *img,
                                             int width,int height,
                                             int startx,int starty,
                                             uint32_t &compressed1,
                                             uint32_t &compressed2 ) {
  uint32_t compressed1_norm, compressed2_norm;
  uint32_t compressed1_flip, compressed2_flip;
  uint8_t avg_color_quant1[3], avg_color_quant2[3];

  float avg_color_float1[3],avg_color_float2[3];
  int enc_color1[3], enc_color2[3], diff[3];
  //int min_error=255*255*8*3;
  //unsigned int best_table_indices1=0, best_table_indices2=0;
  unsigned int best_table1=0, best_table2=0;
    int diffbit;

  int norm_err=0;
  int flip_err=0;

  // First try normal blocks 2x4:

  computeAverageColor2x4noQuantFloat(img,width,height,startx,starty,avg_color_float1);
  computeAverageColor2x4noQuantFloat(img,width,height,startx+2,starty,avg_color_float2);

  // First test if avg_color1 is similar enough to avg_color2 so that
  // we can use differential coding of colors.


  float eps;

  enc_color1[0] = int( JAS_ROUND(31.0*avg_color_float1[0]/255.0) );
  enc_color1[1] = int( JAS_ROUND(31.0*avg_color_float1[1]/255.0) );
  enc_color1[2] = int( JAS_ROUND(31.0*avg_color_float1[2]/255.0) );
  enc_color2[0] = int( JAS_ROUND(31.0*avg_color_float2[0]/255.0) );
  enc_color2[1] = int( JAS_ROUND(31.0*avg_color_float2[1]/255.0) );
  enc_color2[2] = int( JAS_ROUND(31.0*avg_color_float2[2]/255.0) );

  diff[0] = enc_color2[0]-enc_color1[0];
  diff[1] = enc_color2[1]-enc_color1[1];
  diff[2] = enc_color2[2]-enc_color1[2];

    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
  {
    diffbit = 1;

    // The difference to be coded:

    diff[0] = enc_color2[0]-enc_color1[0];
    diff[1] = enc_color2[1]-enc_color1[1];
    diff[2] = enc_color2[2]-enc_color1[2];

    avg_color_quant1[0] = enc_color1[0] << 3 | (enc_color1[0] >> 2);
    avg_color_quant1[1] = enc_color1[1] << 3 | (enc_color1[1] >> 2);
    avg_color_quant1[2] = enc_color1[2] << 3 | (enc_color1[2] >> 2);
    avg_color_quant2[0] = enc_color2[0] << 3 | (enc_color2[0] >> 2);
    avg_color_quant2[1] = enc_color2[1] << 3 | (enc_color2[1] >> 2);
    avg_color_quant2[2] = enc_color2[2] << 3 | (enc_color2[2] >> 2);

    // Pack bits into the first word.

    //     ETC1_RGB8_OES:
    //
    //     a) bit layout in bits 63 through 32 if diffbit = 0
    //
    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1 | base col2 | base col1 | base col2 | base col1 | base col2 | table  | table  |diff|flip|
    //     | R1 (4bits)| R2 (4bits)| G1 (4bits)| G2 (4bits)| B1 (4bits)| B2 (4bits)| cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------
    //
    //     b) bit layout in bits 63 through 32 if diffbit = 1
    //
    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1    | dcol 2 | base col1    | dcol 2 | base col 1   | dcol 2 | table  | table  |diff|flip|
    //     | R1' (5 bits) | dR2    | G1' (5 bits) | dG2    | B1' (5 bits) | dB2    | cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------
    //
    //     c) bit layout in bits 31 through 0 (in both cases)
    //
    //      31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3   2   1  0
    //      --------------------------------------------------------------------------------------------------
    //     |       most significant pixel index bits       |         least significant pixel index bits       |
    //     | p| o| n| m| l| k| j| i| h| g| f| e| d| c| b| a| p| o| n| m| l| k| j| i| h| g| f| e| d| c | b | a |
    //      --------------------------------------------------------------------------------------------------


    compressed1_norm = 0;
    PUTBITSHIGH( compressed1_norm, diffbit,       1, 33);
    PUTBITSHIGH( compressed1_norm, enc_color1[0], 5, 63);
    PUTBITSHIGH( compressed1_norm, enc_color1[1], 5, 55);
    PUTBITSHIGH( compressed1_norm, enc_color1[2], 5, 47);
    PUTBITSHIGH( compressed1_norm, diff[0],       3, 58);
    PUTBITSHIGH( compressed1_norm, diff[1],       3, 50);
    PUTBITSHIGH( compressed1_norm, diff[2],       3, 42);

    unsigned int best_pixel_indices1_MSB;
    unsigned int best_pixel_indices1_LSB;
    unsigned int best_pixel_indices2_MSB;
    unsigned int best_pixel_indices2_LSB;

    norm_err = 0;

    // left part of block
    norm_err = tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

    // right part of block
    norm_err += tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

    PUTBITSHIGH( compressed1_norm, best_table1,   3, 39);
    PUTBITSHIGH( compressed1_norm, best_table2,   3, 36);
    PUTBITSHIGH( compressed1_norm,           0,   1, 32);

    compressed2_norm = 0;
    PUTBITS( compressed2_norm, (best_pixel_indices1_MSB     ), 8, 23);
    PUTBITS( compressed2_norm, (best_pixel_indices2_MSB     ), 8, 31);
    PUTBITS( compressed2_norm, (best_pixel_indices1_LSB     ), 8, 7);
    PUTBITS( compressed2_norm, (best_pixel_indices2_LSB     ), 8, 15);

  }
  else
  {
    diffbit = 0;
    // The difference is bigger than what fits in 555 plus delta-333, so we will have
    // to deal with 444 444.

    eps = (float) 0.0001;

    enc_color1[0] = int( ((float) avg_color_float1[0] / (17.0)) +0.5 + eps);
    enc_color1[1] = int( ((float) avg_color_float1[1] / (17.0)) +0.5 + eps);
    enc_color1[2] = int( ((float) avg_color_float1[2] / (17.0)) +0.5 + eps);
    enc_color2[0] = int( ((float) avg_color_float2[0] / (17.0)) +0.5 + eps);
    enc_color2[1] = int( ((float) avg_color_float2[1] / (17.0)) +0.5 + eps);
    enc_color2[2] = int( ((float) avg_color_float2[2] / (17.0)) +0.5 + eps);
    avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0];
    avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1];
    avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];
    avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0];
    avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1];
    avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


    // Pack bits into the first word.

    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1 | base col2 | base col1 | base col2 | base col1 | base col2 | table  | table  |diff|flip|
    //     | R1 (4bits)| R2 (4bits)| G1 (4bits)| G2 (4bits)| B1 (4bits)| B2 (4bits)| cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------

    compressed1_norm = 0;
    PUTBITSHIGH( compressed1_norm, diffbit,       1, 33);
    PUTBITSHIGH( compressed1_norm, enc_color1[0], 4, 63);
    PUTBITSHIGH( compressed1_norm, enc_color1[1], 4, 55);
    PUTBITSHIGH( compressed1_norm, enc_color1[2], 4, 47);
    PUTBITSHIGH( compressed1_norm, enc_color2[0], 4, 59);
    PUTBITSHIGH( compressed1_norm, enc_color2[1], 4, 51);
    PUTBITSHIGH( compressed1_norm, enc_color2[2], 4, 43);

    unsigned int best_pixel_indices1_MSB;
    unsigned int best_pixel_indices1_LSB;
    unsigned int best_pixel_indices2_MSB;
    unsigned int best_pixel_indices2_LSB;


    // left part of block
    norm_err = tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

    // right part of block
    norm_err += tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

    PUTBITSHIGH( compressed1_norm, best_table1,   3, 39);
    PUTBITSHIGH( compressed1_norm, best_table2,   3, 36);
    PUTBITSHIGH( compressed1_norm,           0,   1, 32);

    compressed2_norm = 0;
    PUTBITS( compressed2_norm, (best_pixel_indices1_MSB     ), 8, 23);
    PUTBITS( compressed2_norm, (best_pixel_indices2_MSB     ), 8, 31);
    PUTBITS( compressed2_norm, (best_pixel_indices1_LSB     ), 8, 7);
    PUTBITS( compressed2_norm, (best_pixel_indices2_LSB     ), 8, 15);


  }

  // Now try flipped blocks 4x2:

  computeAverageColor4x2noQuantFloat(img,width,height,startx,starty,avg_color_float1);
  computeAverageColor4x2noQuantFloat(img,width,height,startx,starty+2,avg_color_float2);

  // First test if avg_color1 is similar enough to avg_color2 so that
  // we can use differential coding of colors.

  enc_color1[0] = int( JAS_ROUND(31.0*avg_color_float1[0]/255.0) );
  enc_color1[1] = int( JAS_ROUND(31.0*avg_color_float1[1]/255.0) );
  enc_color1[2] = int( JAS_ROUND(31.0*avg_color_float1[2]/255.0) );
  enc_color2[0] = int( JAS_ROUND(31.0*avg_color_float2[0]/255.0) );
  enc_color2[1] = int( JAS_ROUND(31.0*avg_color_float2[1]/255.0) );
  enc_color2[2] = int( JAS_ROUND(31.0*avg_color_float2[2]/255.0) );

  diff[0] = enc_color2[0]-enc_color1[0];
  diff[1] = enc_color2[1]-enc_color1[1];
  diff[2] = enc_color2[2]-enc_color1[2];

    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
  {
    diffbit = 1;

    // The difference to be coded:

    diff[0] = enc_color2[0]-enc_color1[0];
    diff[1] = enc_color2[1]-enc_color1[1];
    diff[2] = enc_color2[2]-enc_color1[2];

    avg_color_quant1[0] = enc_color1[0] << 3 | (enc_color1[0] >> 2);
    avg_color_quant1[1] = enc_color1[1] << 3 | (enc_color1[1] >> 2);
    avg_color_quant1[2] = enc_color1[2] << 3 | (enc_color1[2] >> 2);
    avg_color_quant2[0] = enc_color2[0] << 3 | (enc_color2[0] >> 2);
    avg_color_quant2[1] = enc_color2[1] << 3 | (enc_color2[1] >> 2);
    avg_color_quant2[2] = enc_color2[2] << 3 | (enc_color2[2] >> 2);

    // Pack bits into the first word.

    compressed1_flip = 0;
    PUTBITSHIGH( compressed1_flip, diffbit,       1, 33);
    PUTBITSHIGH( compressed1_flip, enc_color1[0], 5, 63);
    PUTBITSHIGH( compressed1_flip, enc_color1[1], 5, 55);
    PUTBITSHIGH( compressed1_flip, enc_color1[2], 5, 47);
    PUTBITSHIGH( compressed1_flip, diff[0],       3, 58);
    PUTBITSHIGH( compressed1_flip, diff[1],       3, 50);
    PUTBITSHIGH( compressed1_flip, diff[2],       3, 42);



    unsigned int best_pixel_indices1_MSB;
    unsigned int best_pixel_indices1_LSB;
    unsigned int best_pixel_indices2_MSB;
    unsigned int best_pixel_indices2_LSB;

    // upper part of block
    flip_err = tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
    // lower part of block
    flip_err += tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

    PUTBITSHIGH( compressed1_flip, best_table1,   3, 39);
    PUTBITSHIGH( compressed1_flip, best_table2,   3, 36);
    PUTBITSHIGH( compressed1_flip,           1,   1, 32);

    best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
    best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);

    compressed2_flip = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);


  }
  else
  {
    diffbit = 0;
    // The difference is bigger than what fits in 555 plus delta-333, so we will have
    // to deal with 444 444.
    eps = (float) 0.0001;

    enc_color1[0] = int( ((float) avg_color_float1[0] / (17.0)) +0.5 + eps);
    enc_color1[1] = int( ((float) avg_color_float1[1] / (17.0)) +0.5 + eps);
    enc_color1[2] = int( ((float) avg_color_float1[2] / (17.0)) +0.5 + eps);
    enc_color2[0] = int( ((float) avg_color_float2[0] / (17.0)) +0.5 + eps);
    enc_color2[1] = int( ((float) avg_color_float2[1] / (17.0)) +0.5 + eps);
    enc_color2[2] = int( ((float) avg_color_float2[2] / (17.0)) +0.5 + eps);

    avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0];
    avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1];
    avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];
    avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0];
    avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1];
    avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1 | base col2 | base col1 | base col2 | base col1 | base col2 | table  | table  |diff|flip|
    //     | R1 (4bits)| R2 (4bits)| G1 (4bits)| G2 (4bits)| B1 (4bits)| B2 (4bits)| cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------


    // Pack bits into the first word.

    compressed1_flip = 0;
    PUTBITSHIGH( compressed1_flip, diffbit,       1, 33);
    PUTBITSHIGH( compressed1_flip, enc_color1[0], 4, 63);
    PUTBITSHIGH( compressed1_flip, enc_color1[1], 4, 55);
    PUTBITSHIGH( compressed1_flip, enc_color1[2], 4, 47);
    PUTBITSHIGH( compressed1_flip, enc_color2[0], 4, 59);
    PUTBITSHIGH( compressed1_flip, enc_color2[1], 4, 51);
    PUTBITSHIGH( compressed1_flip, enc_color2[2], 4, 43);

    unsigned int best_pixel_indices1_MSB;
    unsigned int best_pixel_indices1_LSB;
    unsigned int best_pixel_indices2_MSB;
    unsigned int best_pixel_indices2_LSB;

    // upper part of block
    flip_err = tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
    // lower part of block
    flip_err += tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

    PUTBITSHIGH( compressed1_flip, best_table1,   3, 39);
    PUTBITSHIGH( compressed1_flip, best_table2,   3, 36);
    PUTBITSHIGH( compressed1_flip,           1,   1, 32);

    best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
    best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);

    compressed2_flip = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);


  }

  // Now lets see which is the best table to use. Only 8 tables are possible.

  if(norm_err <= flip_err){
    compressed1 = compressed1_norm | 0;
    compressed2 = compressed2_norm;
    } else {
    compressed1 = compressed1_flip | 1;
    compressed2 = compressed2_flip;
    }
  }


double calcBlockPerceptualErrorRGB( uint8_t *img,
                                    uint8_t *imgdec,
                                    int width,  int height,
                                    int startx, int starty ) {
  (void)height;
  int xx,yy;
  double err;

  err = 0;

  for(xx = startx; xx< startx+4; xx++) {
    for(yy = starty; yy<starty+4; yy++) {
      err += PERCEPTUAL_WEIGHT_R_SQUARED*SQUARE(1.0*RED(img,width,xx,yy)  - 1.0*RED(imgdec, width, xx,yy));
      err += PERCEPTUAL_WEIGHT_G_SQUARED*SQUARE(1.0*GREEN(img,width,xx,yy)- 1.0*GREEN(imgdec, width, xx,yy));
      err += PERCEPTUAL_WEIGHT_B_SQUARED*SQUARE(1.0*BLUE(img,width,xx,yy) - 1.0*BLUE(imgdec, width, xx,yy));
      }
    }

  return err;
  }

void quantize555ColorCombinedPerceptual(float *avg_col_in, int *enc_color, uint8_t *avg_color) {
  float dr, dg, db;
  float kr, kg, kb;
  float wR2, wG2, wB2;
  uint8_t low_color[3];
  uint8_t high_color[3];
  //float min_error=255*255*8*3;
  float lowhightable[8];
  //unsigned int best_table=0;
  //unsigned int best_index=0;
  int q;
  float kval = (float) (255.0/31.0);


  // These are the values that we want to have:
  float red_average, green_average, blue_average;

  int red_5bit_low, green_5bit_low, blue_5bit_low;
  int red_5bit_high, green_5bit_high, blue_5bit_high;

  // These are the values that we approximate with:
  int red_low, green_low, blue_low;
  int red_high, green_high, blue_high;

  red_average = avg_col_in[0];
  green_average = avg_col_in[1];
  blue_average = avg_col_in[2];

  // Find the 5-bit reconstruction levels red_low, red_high
  // so that red_average is in interval [red_low, red_high].
  // (The same with green and blue.)

  red_5bit_low = (int) (red_average/kval);
  green_5bit_low = (int) (green_average/kval);
  blue_5bit_low = (int) (blue_average/kval);

  red_5bit_high = CLAMP(0, red_5bit_low + 1, 31);
  green_5bit_high  = CLAMP(0, green_5bit_low + 1, 31);
  blue_5bit_high = CLAMP(0, blue_5bit_low + 1, 31);

  red_low   = (red_5bit_low << 3) | (red_5bit_low >> 2);
  green_low = (green_5bit_low << 3) | (green_5bit_low >> 2);
  blue_low = (blue_5bit_low << 3) | (blue_5bit_low >> 2);

  red_high   = (red_5bit_high << 3) | (red_5bit_high >> 2);
  green_high = (green_5bit_high << 3) | (green_5bit_high >> 2);
  blue_high = (blue_5bit_high << 3) | (blue_5bit_high >> 2);

  low_color[0] = red_5bit_low;
  low_color[1] = green_5bit_low;
  low_color[2] = blue_5bit_low;

  high_color[0] = red_5bit_high;
  high_color[1] = green_5bit_high;
  high_color[2] = blue_5bit_high;

  kr = (float)red_high - (float)red_low;
  kg = (float)green_high - (float)green_low;
  kb = (float)blue_high - (float)blue_low;

  // Note that dr, dg, and db are all negative.

  dr = red_low - red_average;
  dg = green_low - green_average;
  db = blue_low - blue_average;

  // Perceptual weights to use
  wR2 = (float) PERCEPTUAL_WEIGHT_R_SQUARED;
  wG2 = (float) PERCEPTUAL_WEIGHT_G_SQUARED;
  wB2 = (float) PERCEPTUAL_WEIGHT_B_SQUARED;

  lowhightable[0] = wR2*wG2*SQUARE( (dr+ 0) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+ 0) );
  lowhightable[1] = wR2*wG2*SQUARE( (dr+kr) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+kr) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+ 0) );
  lowhightable[2] = wR2*wG2*SQUARE( (dr+ 0) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+kg) - (db+ 0) );
  lowhightable[3] = wR2*wG2*SQUARE( (dr+ 0) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+kb) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+kb) );
  lowhightable[4] = wR2*wG2*SQUARE( (dr+kr) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+kr) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+kg) - (db+ 0) );
  lowhightable[5] = wR2*wG2*SQUARE( (dr+kr) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+kr) - (db+kb) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+kb) );
  lowhightable[6] = wR2*wG2*SQUARE( (dr+ 0) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+kb) ) + wG2*wB2*SQUARE( (dg+kg) - (db+kb) );
  lowhightable[7] = wR2*wG2*SQUARE( (dr+kr) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+kr) - (db+kb) ) + wG2*wB2*SQUARE( (dg+kg) - (db+kb) );


  float min_value = lowhightable[0];
  int min_index = 0;

  for(q = 1; q<8; q++)
  {
    if(lowhightable[q] < min_value)
    {
      min_value = lowhightable[q];
      min_index = q;
    }
  }

  //float drh = red_high-red_average;
  //float dgh = green_high-green_average;
  //float dbh = blue_high-blue_average;

  switch(min_index)
  {
  case 0:
    enc_color[0] = low_color[0];
    enc_color[1] = low_color[1];
    enc_color[2] = low_color[2];
    break;
  case 1:
    enc_color[0] = high_color[0];
    enc_color[1] = low_color[1];
    enc_color[2] = low_color[2];
    break;
  case 2:
    enc_color[0] = low_color[0];
    enc_color[1] = high_color[1];
    enc_color[2] = low_color[2];
    break;
  case 3:
    enc_color[0] = low_color[0];
    enc_color[1] = low_color[1];
    enc_color[2] = high_color[2];
    break;
  case 4:
    enc_color[0] = high_color[0];
    enc_color[1] = high_color[1];
    enc_color[2] = low_color[2];
    break;
  case 5:
    enc_color[0] = high_color[0];
    enc_color[1] = low_color[1];
    enc_color[2] = high_color[2];
    break;
  case 6:
    enc_color[0] = low_color[0];
    enc_color[1] = high_color[1];
    enc_color[2] = high_color[2];
    break;
  case 7:
    enc_color[0] = high_color[0];
    enc_color[1] = high_color[1];
    enc_color[2] = high_color[2];
    break;
  }

  // Expand 5-bit encoded color to 8-bit color
  avg_color[0] = (enc_color[0] << 3) | (enc_color[0] >> 2);
  avg_color[1] = (enc_color[1] << 3) | (enc_color[1] >> 2);
  avg_color[2] = (enc_color[2] << 3) | (enc_color[2] >> 2);
  }

void quantize444ColorCombinedPerceptual(float *avg_col_in, int *enc_color, uint8_t *avg_color){
  float dr, dg, db;
  float kr, kg, kb;
  float wR2, wG2, wB2;
  uint8_t low_color[3];
  uint8_t high_color[3];
  //float min_error=255*255*8*3;
  float lowhightable[8];
  //unsigned int best_table=0;
  //unsigned int best_index=0;
  int q;
  float kval = (float) (255.0/15.0);


  // These are the values that we want to have:
  float red_average, green_average, blue_average;

  int red_4bit_low, green_4bit_low, blue_4bit_low;
  int red_4bit_high, green_4bit_high, blue_4bit_high;

  // These are the values that we approximate with:
  int red_low, green_low, blue_low;
  int red_high, green_high, blue_high;

  red_average = avg_col_in[0];
  green_average = avg_col_in[1];
  blue_average = avg_col_in[2];

  // Find the 5-bit reconstruction levels red_low, red_high
  // so that red_average is in interval [red_low, red_high].
  // (The same with green and blue.)

  red_4bit_low = (int) (red_average/kval);
  green_4bit_low = (int) (green_average/kval);
  blue_4bit_low = (int) (blue_average/kval);

  red_4bit_high = CLAMP(0, red_4bit_low + 1, 15);
  green_4bit_high  = CLAMP(0, green_4bit_low + 1, 15);
  blue_4bit_high = CLAMP(0, blue_4bit_low + 1, 15);

  red_low   = (red_4bit_low << 4) | (red_4bit_low >> 0);
  green_low = (green_4bit_low << 4) | (green_4bit_low >> 0);
  blue_low = (blue_4bit_low << 4) | (blue_4bit_low >> 0);

  red_high   = (red_4bit_high << 4) | (red_4bit_high >> 0);
  green_high = (green_4bit_high << 4) | (green_4bit_high >> 0);
  blue_high = (blue_4bit_high << 4) | (blue_4bit_high >> 0);

  low_color[0] = red_4bit_low;
  low_color[1] = green_4bit_low;
  low_color[2] = blue_4bit_low;

  high_color[0] = red_4bit_high;
  high_color[1] = green_4bit_high;
  high_color[2] = blue_4bit_high;

  kr = (float)red_high - (float)red_low;
  kg = (float)green_high - (float)green_low;
  kb = (float)blue_high- (float)blue_low;

  // Note that dr, dg, and db are all negative.

  dr = red_low - red_average;
  dg = green_low - green_average;
  db = blue_low - blue_average;

  // Perceptual weights to use
  wR2 = (float) PERCEPTUAL_WEIGHT_R_SQUARED;
  wG2 = (float) PERCEPTUAL_WEIGHT_G_SQUARED;
  wB2 = (float) PERCEPTUAL_WEIGHT_B_SQUARED;

  lowhightable[0] = wR2*wG2*SQUARE( (dr+ 0) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+ 0) );
  lowhightable[1] = wR2*wG2*SQUARE( (dr+kr) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+kr) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+ 0) );
  lowhightable[2] = wR2*wG2*SQUARE( (dr+ 0) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+kg) - (db+ 0) );
  lowhightable[3] = wR2*wG2*SQUARE( (dr+ 0) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+kb) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+kb) );
  lowhightable[4] = wR2*wG2*SQUARE( (dr+kr) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+kr) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+kg) - (db+ 0) );
  lowhightable[5] = wR2*wG2*SQUARE( (dr+kr) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+kr) - (db+kb) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+kb) );
  lowhightable[6] = wR2*wG2*SQUARE( (dr+ 0) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+kb) ) + wG2*wB2*SQUARE( (dg+kg) - (db+kb) );
  lowhightable[7] = wR2*wG2*SQUARE( (dr+kr) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+kr) - (db+kb) ) + wG2*wB2*SQUARE( (dg+kg) - (db+kb) );


  float min_value = lowhightable[0];
  int min_index = 0;

  for(q = 1; q<8; q++)
  {
    if(lowhightable[q] < min_value)
    {
      min_value = lowhightable[q];
      min_index = q;
    }
  }

  //float drh = red_high-red_average;
  //float dgh = green_high-green_average;
  //float dbh = blue_high-blue_average;

  switch(min_index)
  {
  case 0:
    enc_color[0] = low_color[0];
    enc_color[1] = low_color[1];
    enc_color[2] = low_color[2];
    break;
  case 1:
    enc_color[0] = high_color[0];
    enc_color[1] = low_color[1];
    enc_color[2] = low_color[2];
    break;
  case 2:
    enc_color[0] = low_color[0];
    enc_color[1] = high_color[1];
    enc_color[2] = low_color[2];
    break;
  case 3:
    enc_color[0] = low_color[0];
    enc_color[1] = low_color[1];
    enc_color[2] = high_color[2];
    break;
  case 4:
    enc_color[0] = high_color[0];
    enc_color[1] = high_color[1];
    enc_color[2] = low_color[2];
    break;
  case 5:
    enc_color[0] = high_color[0];
    enc_color[1] = low_color[1];
    enc_color[2] = high_color[2];
    break;
  case 6:
    enc_color[0] = low_color[0];
    enc_color[1] = high_color[1];
    enc_color[2] = high_color[2];
    break;
  case 7:
    enc_color[0] = high_color[0];
    enc_color[1] = high_color[1];
    enc_color[2] = high_color[2];
    break;
  }

  // Expand encoded color to eight bits
  avg_color[0] = (enc_color[0] << 4) | enc_color[0];
  avg_color[1] = (enc_color[1] << 4) | enc_color[1];
  avg_color[2] = (enc_color[2] << 4) | enc_color[2];
  }

void compressBlockDiffFlipCombinedPerceptual( uint8_t *img,
                                              int width,int height,
                                              int startx,int starty,
                                              unsigned int &compressed1, unsigned int &compressed2) {
  unsigned int compressed1_norm, compressed2_norm;
  unsigned int compressed1_flip, compressed2_flip;
  uint8_t avg_color_quant1[3], avg_color_quant2[3];

  float avg_color_float1[3],avg_color_float2[3];
  int enc_color1[3], enc_color2[3], diff[3];
  //int min_error=255*255*8*3;
  //unsigned int best_table_indices1=0, best_table_indices2=0;
  unsigned int best_table1=0, best_table2=0;
    int diffbit;

  int norm_err=0;
  int flip_err=0;

  // First try normal blocks 2x4:

  computeAverageColor2x4noQuantFloat(img,width,height,startx,starty,avg_color_float1);
  computeAverageColor2x4noQuantFloat(img,width,height,startx+2,starty,avg_color_float2);

  // First test if avg_color1 is similar enough to avg_color2 so that
  // we can use differential coding of colors.


  //float eps;

  uint8_t dummy[3];

  quantize555ColorCombinedPerceptual(avg_color_float1, enc_color1, dummy);
  quantize555ColorCombinedPerceptual(avg_color_float2, enc_color2, dummy);

  diff[0] = enc_color2[0]-enc_color1[0];
  diff[1] = enc_color2[1]-enc_color1[1];
  diff[2] = enc_color2[2]-enc_color1[2];

    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
  {
    diffbit = 1;

    // The difference to be coded:

    diff[0] = enc_color2[0]-enc_color1[0];
    diff[1] = enc_color2[1]-enc_color1[1];
    diff[2] = enc_color2[2]-enc_color1[2];

    avg_color_quant1[0] = enc_color1[0] << 3 | (enc_color1[0] >> 2);
    avg_color_quant1[1] = enc_color1[1] << 3 | (enc_color1[1] >> 2);
    avg_color_quant1[2] = enc_color1[2] << 3 | (enc_color1[2] >> 2);
    avg_color_quant2[0] = enc_color2[0] << 3 | (enc_color2[0] >> 2);
    avg_color_quant2[1] = enc_color2[1] << 3 | (enc_color2[1] >> 2);
    avg_color_quant2[2] = enc_color2[2] << 3 | (enc_color2[2] >> 2);

    // Pack bits into the first word.

    //     ETC1_RGB8_OES:
    //
    //     a) bit layout in bits 63 through 32 if diffbit = 0
    //
    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1 | base col2 | base col1 | base col2 | base col1 | base col2 | table  | table  |diff|flip|
    //     | R1 (4bits)| R2 (4bits)| G1 (4bits)| G2 (4bits)| B1 (4bits)| B2 (4bits)| cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------
    //
    //     b) bit layout in bits 63 through 32 if diffbit = 1
    //
    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1    | dcol 2 | base col1    | dcol 2 | base col 1   | dcol 2 | table  | table  |diff|flip|
    //     | R1' (5 bits) | dR2    | G1' (5 bits) | dG2    | B1' (5 bits) | dB2    | cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------
    //
    //     c) bit layout in bits 31 through 0 (in both cases)
    //
    //      31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3   2   1  0
    //      --------------------------------------------------------------------------------------------------
    //     |       most significant pixel index bits       |         least significant pixel index bits       |
    //     | p| o| n| m| l| k| j| i| h| g| f| e| d| c| b| a| p| o| n| m| l| k| j| i| h| g| f| e| d| c | b | a |
    //      --------------------------------------------------------------------------------------------------


    compressed1_norm = 0;
    PUTBITSHIGH( compressed1_norm, diffbit,       1, 33);
    PUTBITSHIGH( compressed1_norm, enc_color1[0], 5, 63);
    PUTBITSHIGH( compressed1_norm, enc_color1[1], 5, 55);
    PUTBITSHIGH( compressed1_norm, enc_color1[2], 5, 47);
    PUTBITSHIGH( compressed1_norm, diff[0],       3, 58);
    PUTBITSHIGH( compressed1_norm, diff[1],       3, 50);
    PUTBITSHIGH( compressed1_norm, diff[2],       3, 42);

    unsigned int best_pixel_indices1_MSB;
    unsigned int best_pixel_indices1_LSB;
    unsigned int best_pixel_indices2_MSB;
    unsigned int best_pixel_indices2_LSB;

    norm_err = 0;

    // left part of block
    norm_err = tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

    // right part of block
    norm_err += tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

    PUTBITSHIGH( compressed1_norm, best_table1,   3, 39);
    PUTBITSHIGH( compressed1_norm, best_table2,   3, 36);
    PUTBITSHIGH( compressed1_norm,           0,   1, 32);

    compressed2_norm = 0;
    PUTBITS( compressed2_norm, (best_pixel_indices1_MSB     ), 8, 23);
    PUTBITS( compressed2_norm, (best_pixel_indices2_MSB     ), 8, 31);
    PUTBITS( compressed2_norm, (best_pixel_indices1_LSB     ), 8, 7);
    PUTBITS( compressed2_norm, (best_pixel_indices2_LSB     ), 8, 15);

  }
  else
  {
    diffbit = 0;
    // The difference is bigger than what fits in 555 plus delta-333, so we will have
    // to deal with 444 444.

    //eps = (float) 0.0001;

    quantize444ColorCombinedPerceptual(avg_color_float1, enc_color1, dummy);
    quantize444ColorCombinedPerceptual(avg_color_float2, enc_color2, dummy);

    avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0];
    avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1];
    avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];
    avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0];
    avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1];
    avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


    // Pack bits into the first word.

    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1 | base col2 | base col1 | base col2 | base col1 | base col2 | table  | table  |diff|flip|
    //     | R1 (4bits)| R2 (4bits)| G1 (4bits)| G2 (4bits)| B1 (4bits)| B2 (4bits)| cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------

    compressed1_norm = 0;
    PUTBITSHIGH( compressed1_norm, diffbit,       1, 33);
    PUTBITSHIGH( compressed1_norm, enc_color1[0], 4, 63);
    PUTBITSHIGH( compressed1_norm, enc_color1[1], 4, 55);
    PUTBITSHIGH( compressed1_norm, enc_color1[2], 4, 47);
    PUTBITSHIGH( compressed1_norm, enc_color2[0], 4, 59);
    PUTBITSHIGH( compressed1_norm, enc_color2[1], 4, 51);
    PUTBITSHIGH( compressed1_norm, enc_color2[2], 4, 43);

    unsigned int best_pixel_indices1_MSB;
    unsigned int best_pixel_indices1_LSB;
    unsigned int best_pixel_indices2_MSB;
    unsigned int best_pixel_indices2_LSB;


    // left part of block
    norm_err = tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

    // right part of block
    norm_err += tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

    PUTBITSHIGH( compressed1_norm, best_table1,   3, 39);
    PUTBITSHIGH( compressed1_norm, best_table2,   3, 36);
    PUTBITSHIGH( compressed1_norm,           0,   1, 32);

    compressed2_norm = 0;
    PUTBITS( compressed2_norm, (best_pixel_indices1_MSB     ), 8, 23);
    PUTBITS( compressed2_norm, (best_pixel_indices2_MSB     ), 8, 31);
    PUTBITS( compressed2_norm, (best_pixel_indices1_LSB     ), 8, 7);
    PUTBITS( compressed2_norm, (best_pixel_indices2_LSB     ), 8, 15);


  }

  // Now try flipped blocks 4x2:

  computeAverageColor4x2noQuantFloat(img,width,height,startx,starty,avg_color_float1);
  computeAverageColor4x2noQuantFloat(img,width,height,startx,starty+2,avg_color_float2);

  // First test if avg_color1 is similar enough to avg_color2 so that
  // we can use differential coding of colors.


  quantize555ColorCombinedPerceptual(avg_color_float1, enc_color1, dummy);
  quantize555ColorCombinedPerceptual(avg_color_float2, enc_color2, dummy);

  diff[0] = enc_color2[0]-enc_color1[0];
  diff[1] = enc_color2[1]-enc_color1[1];
  diff[2] = enc_color2[2]-enc_color1[2];

    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
  {
    diffbit = 1;

    // The difference to be coded:

    diff[0] = enc_color2[0]-enc_color1[0];
    diff[1] = enc_color2[1]-enc_color1[1];
    diff[2] = enc_color2[2]-enc_color1[2];

    avg_color_quant1[0] = enc_color1[0] << 3 | (enc_color1[0] >> 2);
    avg_color_quant1[1] = enc_color1[1] << 3 | (enc_color1[1] >> 2);
    avg_color_quant1[2] = enc_color1[2] << 3 | (enc_color1[2] >> 2);
    avg_color_quant2[0] = enc_color2[0] << 3 | (enc_color2[0] >> 2);
    avg_color_quant2[1] = enc_color2[1] << 3 | (enc_color2[1] >> 2);
    avg_color_quant2[2] = enc_color2[2] << 3 | (enc_color2[2] >> 2);

    // Pack bits into the first word.

    compressed1_flip = 0;
    PUTBITSHIGH( compressed1_flip, diffbit,       1, 33);
    PUTBITSHIGH( compressed1_flip, enc_color1[0], 5, 63);
    PUTBITSHIGH( compressed1_flip, enc_color1[1], 5, 55);
    PUTBITSHIGH( compressed1_flip, enc_color1[2], 5, 47);
    PUTBITSHIGH( compressed1_flip, diff[0],       3, 58);
    PUTBITSHIGH( compressed1_flip, diff[1],       3, 50);
    PUTBITSHIGH( compressed1_flip, diff[2],       3, 42);



    unsigned int best_pixel_indices1_MSB;
    unsigned int best_pixel_indices1_LSB;
    unsigned int best_pixel_indices2_MSB;
    unsigned int best_pixel_indices2_LSB;

    // upper part of block
    flip_err = tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
    // lower part of block
    flip_err += tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

    PUTBITSHIGH( compressed1_flip, best_table1,   3, 39);
    PUTBITSHIGH( compressed1_flip, best_table2,   3, 36);
    PUTBITSHIGH( compressed1_flip,           1,   1, 32);

    best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
    best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);

    compressed2_flip = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);


  }
  else
  {
    diffbit = 0;
    // The difference is bigger than what fits in 555 plus delta-333, so we will have
    // to deal with 444 444.
    //eps = (float) 0.0001;

    quantize444ColorCombinedPerceptual(avg_color_float1, enc_color1, dummy);
    quantize444ColorCombinedPerceptual(avg_color_float2, enc_color2, dummy);

    avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0];
    avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1];
    avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];
    avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0];
    avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1];
    avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

    //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34  33  32
    //      ---------------------------------------------------------------------------------------------------
    //     | base col1 | base col2 | base col1 | base col2 | base col1 | base col2 | table  | table  |diff|flip|
    //     | R1 (4bits)| R2 (4bits)| G1 (4bits)| G2 (4bits)| B1 (4bits)| B2 (4bits)| cw 1   | cw 2   |bit |bit |
    //      ---------------------------------------------------------------------------------------------------


    // Pack bits into the first word.

    compressed1_flip = 0;
    PUTBITSHIGH( compressed1_flip, diffbit,       1, 33);
    PUTBITSHIGH( compressed1_flip, enc_color1[0], 4, 63);
    PUTBITSHIGH( compressed1_flip, enc_color1[1], 4, 55);
    PUTBITSHIGH( compressed1_flip, enc_color1[2], 4, 47);
    PUTBITSHIGH( compressed1_flip, enc_color2[0], 4, 59);
    PUTBITSHIGH( compressed1_flip, enc_color2[1], 4, 51);
    PUTBITSHIGH( compressed1_flip, enc_color2[2], 4, 43);

    unsigned int best_pixel_indices1_MSB;
    unsigned int best_pixel_indices1_LSB;
    unsigned int best_pixel_indices2_MSB;
    unsigned int best_pixel_indices2_LSB;

    // upper part of block
    flip_err = tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
    // lower part of block
    flip_err += tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

    PUTBITSHIGH( compressed1_flip, best_table1,   3, 39);
    PUTBITSHIGH( compressed1_flip, best_table2,   3, 36);
    PUTBITSHIGH( compressed1_flip,           1,   1, 32);

    best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
    best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);

    compressed2_flip = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);


  }

  // Now lets see which is the best table to use. Only 8 tables are possible.

  if(norm_err <= flip_err)
  {

    compressed1 = compressed1_norm | 0;
    compressed2 = compressed2_norm;

  }
  else
  {

    compressed1 = compressed1_flip | 1;
    compressed2 = compressed2_flip;
  }
}

void compressBlockDiffFlipFastPerceptual( uint8_t *img,
                                          uint8_t *imgdec,
                                          int width,int height,
                                          int startx,int starty,
                                          uint32_t &compressed1,
                                          uint32_t &compressed2) {
  uint32_t average_block1, average_block2;
  double error_average;

  uint32_t combined_block1;
  uint32_t combined_block2;
  double error_combined;

  // First quantize the average color to the nearest neighbor.
  compressBlockDiffFlipAveragePerceptual(img, width, height, startx, starty, average_block1, average_block2);
  decompressBlockDiffFlip(average_block1, average_block2, imgdec, width, height, startx, starty);
  error_average = calcBlockPerceptualErrorRGB(img, imgdec, width, height, startx, starty);

  compressBlockDiffFlipCombinedPerceptual(img, width, height, startx, starty, combined_block1, combined_block2);
  decompressBlockDiffFlip(combined_block1, combined_block2, imgdec, width, height, startx, starty);
  error_combined = calcBlockPerceptualErrorRGB(img, imgdec, width, height, startx, starty);

  if(error_combined < error_average) {
    compressed1 = combined_block1;
    compressed2 = combined_block2;
    } else 	{
    compressed1 = average_block1;
    compressed2 = average_block2;
    }
  }
