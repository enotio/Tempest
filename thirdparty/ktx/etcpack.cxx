// etcpack.cxx v1.06
//
// NO WARRANTY 
// 
// BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, ERICSSON MAKES NO
// REPRESENTATIONS OF ANY KIND, EXTENDS NO WARRANTIES OF ANY KIND; EITHER
// EXPRESS OR IMPLIED; INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE
// PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME
// THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION. ERICSSON
// MAKES NO WARRANTY THAT THE MANUFACTURE, SALE, LEASE, USE OR
// IMPORTATION WILL BE FREE FROM INFRINGEMENT OF PATENTS, COPYRIGHTS OR
// OTHER INTELLECTUAL PROPERTY RIGHTS OF OTHERS, AND IT SHALL BE THE SOLE
// RESPONSIBILITY OF THE LICENSEE TO MAKE SUCH DETERMINATION AS IS
// NECESSARY WITH RESPECT TO THE ACQUISITION OF LICENSES UNDER PATENTS
// AND OTHER INTELLECTUAL PROPERTY OF THIRD PARTIES;
// 
// IN NO EVENT WILL ERICSSON, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY
// GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF
// THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO
// LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
// YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY
// OTHER PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGES.
// 
// (C) Ericsson AB 2005. All Rights Reserved.
// 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> // Only needed for PSNR calculation, not for compression.
#include <time.h>
#include <sys/timeb.h>
#include "ktx_image.h"
 
#define CLAMP(ll,x,ul) (((x)<(ll)) ? (ll) : (((x)>(ul)) ? (ul) : (x)))
#define GETBITS(source, size, startpos)  (( (source) >> ((startpos)-(size)+1) ) & ((1<<(size)) -1))
#define GETBITSHIGH(source, size, startpos)  (( (source) >> (((startpos)-32)-(size)+1) ) & ((1<<(size)) -1))

typedef unsigned char uint8;
#define SQUARE(x) ((x)*(x))
#define JAS_ROUND(x) (((x) < 0.0 ) ? ((int)((x)-0.5)) : ((int)((x)+0.5)))

// The error metric Wr Wg Wb should be definied so that Wr^2 + Wg^2 + Wb^2 = 1.
// Hence it is easier to first define the squared values and derive the weights
// as their square-roots.

#define PERCEPTUAL_WEIGHT_R_SQUARED 0.299
#define PERCEPTUAL_WEIGHT_G_SQUARED 0.587
#define PERCEPTUAL_WEIGHT_B_SQUARED 0.114

// Alternative weights
//#define PERCEPTUAL_WEIGHT_R_SQUARED 0.3086
//#define PERCEPTUAL_WEIGHT_G_SQUARED 0.6094
//#define PERCEPTUAL_WEIGHT_B_SQUARED 0.082

#define PERCEPTUAL_WEIGHT_R (sqrt(PERCEPTUAL_WEIGHT_R_SQUARED))
#define PERCEPTUAL_WEIGHT_G (sqrt(PERCEPTUAL_WEIGHT_G_SQUARED))
#define PERCEPTUAL_WEIGHT_B (sqrt(PERCEPTUAL_WEIGHT_B_SQUARED))


double wR = PERCEPTUAL_WEIGHT_R;
double wG = PERCEPTUAL_WEIGHT_G;
double wB = PERCEPTUAL_WEIGHT_B;

double wR2 = PERCEPTUAL_WEIGHT_R_SQUARED;
double wG2 = PERCEPTUAL_WEIGHT_G_SQUARED;
double wB2 = PERCEPTUAL_WEIGHT_B_SQUARED;

#define RED(img,width,x,y)   img[3*(y*width+x)+0]
#define GREEN(img,width,x,y) img[3*(y*width+x)+1]
#define BLUE(img,width,x,y)  img[3*(y*width+x)+2]


// SLOW SCAN RANGE IS -5 to 5 in all three colors
#define SLOW_SCAN_MIN (-5)
#define SLOW_SCAN_MAX (5)
#define SLOW_SCAN_RANGE ((SLOW_SCAN_MAX-(SLOW_SCAN_MIN)+1))
#define SLOW_SCAN_OFFSET (-(SLOW_SCAN_MIN))
// We need this to guarrantee that at least one try is valid
#define SLOW_TRY_MIN (-4 - SLOW_SCAN_MAX)
#define SLOW_TRY_MAX (3 - (SLOW_SCAN_MIN))


// MEDIUM SCAN RANGE IS -3 to 3in all three colors
#define MEDIUM_SCAN_MIN (-3)
#define MEDIUM_SCAN_MAX (3)
#define MEDIUM_SCAN_RANGE ((MEDIUM_SCAN_MAX-(MEDIUM_SCAN_MIN)+1))
#define MEDIUM_SCAN_OFFSET (-(MEDIUM_SCAN_MIN))

// We need this to guarrantee that at least one try is valid
#define MEDIUM_TRY_MIN (-4 - MEDIUM_SCAN_MAX)
#define MEDIUM_TRY_MAX (3 - (MEDIUM_SCAN_MIN))

 
#define PUTBITS( dest, data, size, startpos) dest |= ( (data) & ((1<<(size))-1) ) << ((startpos)-(size)+1)
#define PUTBITSHIGH( dest, data, size, startpos) dest |= ( (data) & ((1<<(size))-1) ) << (((startpos)-32)-(size)+1)

int scramble[4] = {3, 2, 0, 1};

enum{FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T0, FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T1};
int orientation;

int ktx_mode;

typedef struct KTX_header_t
{
	uint8  identifier[12];
	unsigned int endianness;
	unsigned int glType;
	unsigned int glTypeSize;
	unsigned int glFormat;
	unsigned int glInternalFormat;
	unsigned int glBaseInternalFormat;
	unsigned int pixelWidth;
	unsigned int pixelHeight;
	unsigned int pixelDepth;
	unsigned int numberOfArrayElements;
	unsigned int numberOfFaces;
	unsigned int numberOfMipmapLevels;
	unsigned int bytesOfKeyValueData;
} 
KTX_header;
#define KTX_IDENTIFIER_REF  { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A }

#define KTX_ENDIAN_REF      (0x04030201)
#define KTX_ENDIAN_REF_REV  (0x01020304)

enum {GL_R=0x1903,GL_RG=0x8227,GL_RGB=0x1907,GL_RGBA=0x1908};
enum {GL_ETC1_RGB8_OES=0x8d64};


int ktx_identifier[] = KTX_IDENTIFIER_REF;

static int compressParamsEnc[16][4];

/* In etcdec.cxx */
void decompressBlockDiffFlip(unsigned int block_part1, unsigned int block_part2, uint8 *img,int width,int height,int startx,int starty);
void read_big_endian_2byte_word(unsigned short *blockadr, FILE *f);
void read_big_endian_4byte_word(unsigned int *blockadr, FILE *f);

bool fileExist( const char *filename)
{
	FILE *f=NULL;
	if((f=fopen(filename,"rb"))!=NULL)
	{
		fclose(f);
		return true;
	}
	return false;
}

bool expandToWidthDivByFour(uint8 *&img, int width, int height, int &expandedwidth, int &expandedheight)
{
	int wdiv4;
	int xx, yy;
	uint8 *newimg;

	wdiv4 = width /4;
	if( !(wdiv4 *4 == width) )
	{
     	expandedwidth = (wdiv4 + 1)*4;
		expandedheight = height;
	    newimg=(uint8*)malloc(3*expandedwidth*expandedheight);
		if(!newimg)
		{
			printf("Could not allocate memory to expand width\n");
			return false;
		}

		// First copy image
		for(yy = 0; yy<height; yy++)
		{
			for(xx = 0; xx < width; xx++)
			{
				newimg[(yy * expandedwidth)*3 + xx*3 + 0] = img[(yy * width)*3 + xx*3 + 0];
				newimg[(yy * expandedwidth)*3 + xx*3 + 1] = img[(yy * width)*3 + xx*3 + 1];
				newimg[(yy * expandedwidth)*3 + xx*3 + 2] = img[(yy * width)*3 + xx*3 + 2];
			}
		}

		// Then make the last column of pixels the same as the previous column.

		for(yy = 0; yy< height; yy++)
		{
			for(xx = width; xx < expandedwidth; xx++)
			{
				newimg[(yy * expandedwidth)*3 + (xx)*3 + 0] = img[(yy * width)*3 + (width-1)*3 + 0];
				newimg[(yy * expandedwidth)*3 + (xx)*3 + 1] = img[(yy * width)*3 + (width-1)*3 + 1];
				newimg[(yy * expandedwidth)*3 + (xx)*3 + 2] = img[(yy * width)*3 + (width-1)*3 + 2];
			}
		}

		// Now free the old image
		free(img);

		// Use the new image
		img = newimg;

		return true;
	}
	else
	{
		printf("Image already of even width\n");
		expandedwidth = width;
		expandedheight = height;
		return false;
	}
}


bool expandToHeightDivByFour(uint8 *&img, int width, int height, int &expandedwidth, int &expandedheight)
{
	int hdiv4;
	int xx, yy;
	int numlinesmissing;
	uint8 *newimg;


	hdiv4 = height/4;

	if( !(hdiv4 * 4 == height) )
	{
		expandedwidth = width;
		expandedheight = (hdiv4 + 1) * 4;
		numlinesmissing = expandedheight - height;
		newimg=(uint8*)malloc(3*expandedwidth*expandedheight);
		if(!newimg)
		{
			printf("Could not allocate memory to expand height\n");
			return false;
		}
		
		// First copy image. No need to reformat data.

		for(xx = 0; xx<3*width*height; xx++)
			newimg[xx] = img[xx];

		// Then copy up to three lines.

		for(yy = height; yy < height + numlinesmissing; yy++)
		{
			for(xx = 0; xx<width; xx++)
			{
				newimg[(yy*width)*3 + xx*3 + 0] = img[((height-1)*width)*3 + xx*3 + 0];
				newimg[(yy*width)*3 + xx*3 + 1] = img[((height-1)*width)*3 + xx*3 + 1];
				newimg[(yy*width)*3 + xx*3 + 2] = img[((height-1)*width)*3 + xx*3 + 2];
			}
		}

		// Now free the old image;
		free(img);

		// Use the new image:
		img = newimg;

		return true;

	}
	else
	{
		printf("Image height already divisible by four.\n");
		expandedwidth = width;
		expandedheight = height;
		return true;
	}
}

int find_pos_of_extension(char *src)
{
	int q=strlen(src);
	while(q>=0)		// find file name extension
	{
		if(src[q]=='.') break;
		q--;
	}
	if(q<0) 
		return -1;
	else
		return q;
}

bool readSrcFile(char *filename,uint8 *&img,int &width,int &height, int &expandedwidth, int &expandedheight)
{
	int w1,h1;
	int wdiv4, hdiv4;
	char str[255];


	// Delete temp file if it exists.
	if(fileExist("tmp.ppm"))
	{
		sprintf(str, "del tmp.ppm\n");
		system(str);
	}


	int q = find_pos_of_extension(filename);
	if(!strcmp(&filename[q],".ppm")) 
	{
		// Already a .ppm file. Just copy. 
		sprintf(str,"copy %s tmp.ppm \n", filename);
    printf("Copying source file to tmp.ppm\n");
	}
	else
	{
		// Converting from other format to .ppm 
		// 
		// Use your favorite command line image converter program,
		// for instance Image Magick. Just make sure the syntax can
		// be written as below:
		// 
		// C:\imconv source.jpg dest.ppm
		//
		sprintf(str,"imconv %s tmp.ppm\n", filename);
		printf("Converting source file from %s to .ppm\n", filename);
	}
	// Execute system call
	system(str);

	bool FLIP;
	if(orientation == FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T0)
		FLIP = false;
	else if(orientation == FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T1)
		FLIP = true;
	else
	{
		printf("orientation error\n");
		exit(1);
	}

	if(fReadPPM("tmp.ppm",w1,h1,img,FLIP))
	{
		width=w1;
		height=h1;
		system("del tmp.ppm");

		// Width must be divisible by 2 and height must be
		// divisible by 4. Otherwise, we will not compress 
		// the image. 

		wdiv4 = width / 4;
		hdiv4 = height / 4;

		expandedwidth = width;
		expandedheight = height;

		if( !(wdiv4 * 4 == width) )
		{
			printf(" Width = %d is not divisible by four... ", width);
			printf(" expanding image in x-dir... ");
			if(expandToWidthDivByFour(img, width, height, expandedwidth, expandedheight))
			{
				printf("OK.\n");
			}
			else
			{
				printf("\n Error: could not expand image\n");
				return false;
			}
		}
		if( !(hdiv4 * 4 == height))
		{
			printf(" Height = %d is not divisible by four... ", height);
			printf(" expanding image in y-dir...");
			if(expandToHeightDivByFour(img, expandedwidth, height, expandedwidth, expandedheight))
			{
				printf("OK.\n");
			}
			else
			{
				printf("\n Error: could not expand image\n");
				return false;
			}
		}
		if(!(expandedwidth == width && expandedheight == height))
		   printf("Active pixels: %dx%d. Expanded image: %dx%d\n",width,height,expandedwidth,expandedheight);
		return true;
	}
	return false;

}
bool readSrcFileNoExpand(char *filename,uint8 *&img,int &width,int &height)
{
	int w1,h1;
	char str[255];


	// Delete temp file if it exists.
	if(fileExist("tmp.ppm"))
	{
		sprintf(str, "del tmp.ppm\n");
		system(str);
	}


	int q = find_pos_of_extension(filename);
	if(!strcmp(&filename[q],".ppm")) 
	{
		// Already a .ppm file. Just copy. 
		sprintf(str,"copy %s tmp.ppm \n", filename);
    printf("Copying source file to tmp.ppm\n");
	}
	else
	{
		// Converting from other format to .ppm 
		// 
		// Use your favorite command line image converter program,
		// for instance Image Magick. Just make sure the syntax can
		// be written as below:
		// 
		// C:\imconv source.jpg dest.ppm
		//
		sprintf(str,"imconv %s tmp.ppm\n", filename);
		printf("Converting source file from %s to .ppm\n", filename);
	}
	// Execute system call
	system(str);

	// The current function is only used when comparing two ppm files --- we don't need to flip them. Hence reverse_y is false
	if(fReadPPM("tmp.ppm",w1,h1,img,false))
	{
		width=w1;
		height=h1;
		system("del tmp.ppm");

		return true;
	}
	return false;

}

int determineAction(int argc,char *argv[],char *dst)
{
	char *src;
	int q;

	enum {MODE_COMPRESS, MODE_UNCOMPRESS, MODE_PSNR};
    enum {SPEED_SLOW, SPEED_FAST, SPEED_MEDIUM};
	enum {METRIC_PERCEPTUAL, METRIC_NONPERCEPTUAL};

	int mode = MODE_COMPRESS;
	int speed = SPEED_FAST;
	int metric = METRIC_PERCEPTUAL;

    // A bit hackish: First check for the orientation flag. When this flag is set, remove it from the string and proceed with the rest of the arguments as before. 

	bool orientation_flag_found = false;
	orientation = FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T0;

    for(q = 1; q < argc && !orientation_flag_found; q++)
	{
		src = argv[q];
	    if(!strcmp(src, "-o"))
		{
			orientation_flag_found = true;
			src = argv[q+1];
			if(!strcmp(src, "topleftmapsto_s0t0"))
			{
				orientation = FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T0;
			}
			else if(!strcmp(src, "bottomleftmapsto_s0t0"))
			{
				orientation = FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T1;
			}
			else
			{
				return -1;
			}
			// At this stage in the code we know we have a valid orientation argument.
			// Now remove it from the argument list. 
            for(int xx=q+2; xx<argc; xx++)
			{
				argv[xx-2] = argv[xx];
			} 
			argc = argc - 2;
		}
	}


	// First check the number of arguments.

	if(argc == 3)
	{
		// We have a situation similar to this one:
		// etcpack img.ppm img.pkm


		// Find the extension of the first file name:	
		src = argv[1];
		q = find_pos_of_extension(src);
		if(q<0) return -1;

		// If we have etcpack img.pkm img.any

		if(!strcmp(&src[q],".pkm") || !strcmp(&src[q],".ktx")) 
		{
			// First argument is .pkm. Uncompress. 
			mode = MODE_UNCOMPRESS;			// uncompress from binary file format .pkm
			strcpy(dst,argv[2]);

			if(!strcmp(&src[q],".ktx"))
				ktx_mode = true;
			else
				ktx_mode = false;

			// Make sure second argument is not also .pkm or .ktx
			src = argv[2];
			q = find_pos_of_extension(src);
			if(q<0) return -1;
			if(!strcmp(&src[q],".pkm") || !strcmp(&src[q],".ktx")) 
			{
				printf("At least one argument has to be uncompressed (.ppm, .png, ..., not .ktx)\n");
				return -1;
			}

		}
		else
		{
			// The first argument was not .pkm. The second argument must then be .pkm.
			src = argv[2];
			q = find_pos_of_extension(src);
			if(q<0) return -1;
			
			if(!strcmp(&src[q],".pkm") || !strcmp(&src[q],".ktx")) 
			{
				// Second argument is .pkm. Compress. 
				mode = MODE_COMPRESS;			// compress to binary file format .pkm
				strcpy(dst,argv[2]);
				if(!strcmp(&src[q],".ktx"))
					ktx_mode = true;
				else
					ktx_mode = false;
			}
			else
			{
				printf("At least one argument has to be compressed (.pkm, .ktx) unless -p flag is used.\n");
					return -1;
			}
		}
	}
	else if(argc == 4)
	{
		// We must have the following situation:
		// etcpack -p img1.ppm img2.ppm
		src = argv[1];
		ktx_mode = false;
	    if(!strcmp(src, "-p"))
		{
			// We should do PSNR between argv[2] and argv[3]
			mode = MODE_PSNR;
			strcpy(dst,argv[3]);
		}
		else
		{
			// Error
			return -1;
		}
	}
	else if(argc == 5)
	{
		// We must be having one of the two following cases:
		// etcpack -s {fast|medium|slow} img.ppm img.pkm
		// etcpack -e {perceptual|nonperceptual} img.ppm img.pkm

		// First try if we have .pkm in the end (we should, otherwise arguments do not make sense).
		
		src = argv[4];
		q = find_pos_of_extension(src);
		if(q<0) return -1;

		if(!(!strcmp(&src[q],".pkm") || !strcmp(&src[q],".ktx"))) 
		{
			// The last argument is not .pkm or . Explain and give error:
			printf("Error: argument %s %s is not valid for decompression\n",argv[1],argv[2]);
			return -1;
		}

		if(!strcmp(&src[q],".ktx"))
			ktx_mode = true;
		else
			ktx_mode = false;

		// Make sure first file argument is not also .pkm or .ktx
		src = argv[3];
		q = find_pos_of_extension(src);
		if(q<0) return -1;
		if(!strcmp(&src[q],".pkm") || !strcmp(&src[q],".ktx")) 
		{
			printf("At least one argument has to be uncompressed (.ppm, .png, ..., not .ktx)\n");
			return -1;
		}

		mode = MODE_COMPRESS;

		// Ok, we are compressing. Set dst file:
		strcpy(dst,argv[4]);
		
		// Now see what the argument is:

		if(!strcmp(argv[1],"-s")) 
		{
			// We have argument -s. Now check for slow, medium or fast.
			
			if(!strcmp(argv[2],"slow")) 
			{
				speed = SPEED_SLOW;
			}
			else if(!strcmp(argv[2],"medium")) 
			{
				speed = SPEED_MEDIUM;
			}
			else if(!strcmp(argv[2],"fast")) 
			{
				speed = SPEED_FAST;
			}
			else
			{
				printf("Error: %s not part of switch %s\n",argv[2], argv[1]);
				exit(1);
			}

		}
		else if(!strcmp(argv[1],"-e")) 
		{

			// We have argument -e. Now check for perceptual or nonperceptual
			
			if(!strcmp(argv[2],"perceptual")) 
			{
				metric = METRIC_PERCEPTUAL;
			}
			else if(!strcmp(argv[2],"nonperceptual")) 
			{
				metric = METRIC_NONPERCEPTUAL;
			}
			else
			{
				printf("Error: %s not part of switch %s\n",argv[2], argv[1]);
				exit(1);
			}
		}
		else
		{
			printf("Error: cannot interpret switch %s %s\n",argv[1], argv[2]);
			return -1;
		}
	}
	else if(argc == 7)
	{
		// We must be having one of the two following cases:
		// etcpack -s {fast|medium|slow} -e {perceptual|nonperceptual} img.ppm img.pkm
		// etcpack -e {perceptual|nonperceptual} -s {slow|medium|fast} img.ppm img.pkm

		// First try if we have .pkm in the end (we should, otherwise arguments do not make sense).
		
		src = argv[6];
		q = find_pos_of_extension(src);
		if(q<0) return -1;

		if(!(!strcmp(&src[q],".pkm") || !strcmp(&src[q],".ktx"))) 
		{
			// The last argument is not .pkm. Explain and give error:
			printf("Error: argument %s %s is not valid for decompression\n",argv[1],argv[2]);
			return -1;
		}

		if(!strcmp(&src[q],".ktx"))
			ktx_mode = true;
		else
			ktx_mode = false;

		// Make sure first file argument is not also .pkm or .ktx
		src = argv[5];
		q = find_pos_of_extension(src);
		if(q<0) return -1;
		if(!strcmp(&src[q],".pkm") || !strcmp(&src[q],".ktx")) 
		{
			printf("At least one argument has to be uncompressed (.ppm, .png, ..., not .ktx)\n");
			return -1;
		}

		mode = MODE_COMPRESS;

		// Ok, we are compressing. Set dst file:

		strcpy(dst,argv[6]);
		
		// Now see what the argument is:

		if(!strcmp(argv[1],"-s")) 
		{
			// We have argument -s. Now check for slow, medium or fast. 
			
			if(!strcmp(argv[2],"slow")) 
			{
				speed = SPEED_SLOW;
			}
			else if(!strcmp(argv[2],"medium")) 
			{
				speed = SPEED_MEDIUM;
			}
			else if(!strcmp(argv[2],"fast")) 
			{
				speed = SPEED_FAST;
			}
			else
			{
				printf("Error: %s not part of switch %s\n",argv[2], argv[1]);
				exit(1);
			}

		}
		else if(!strcmp(argv[1],"-e")) 
		{

			// We have argument -e. Now check for perceptual or nonperceptual
			
			if(!strcmp(argv[2],"perceptual")) 
			{
				metric = METRIC_PERCEPTUAL;
			}
			else if(!strcmp(argv[2],"nonperceptual")) 
			{
				metric = METRIC_NONPERCEPTUAL;
			}
			else
			{
				printf("Error: %s not part of switch %s\n",argv[2], argv[1]);
				exit(1);
			}
		}
		else
		{
			printf("Error: cannot interpret switch %s %s\n",argv[1], argv[2]);
			return -1;
		}

		// Then take on the next input argument

		if(!strcmp(argv[3],"-s")) 
		{
			// We have argument -s. Now check for slow, medium or fast.
			
			if(!strcmp(argv[4],"slow")) 
			{
				speed = SPEED_SLOW;
			}
			else if(!strcmp(argv[4],"medium")) 
			{
				speed = SPEED_MEDIUM;
			}
			else if(!strcmp(argv[4],"fast")) 
			{
				speed = SPEED_FAST;
			}
			else
			{
				printf("Error: %s not part of switch %s\n",argv[4], argv[3]);
				exit(1);
			}
		}
		else if(!strcmp(argv[3],"-e")) 
		{
			// We have argument -e. Now check for perceptual or nonperceptual
			
			if(!strcmp(argv[4],"perceptual")) 
			{
				metric = METRIC_PERCEPTUAL;
			}
			else if(!strcmp(argv[4],"nonperceptual")) 
			{
				metric = METRIC_NONPERCEPTUAL;
			}
			else
			{
				printf("Error: %s not part of switch %s\n",argv[4], argv[3]);
				exit(1);
			}

		}
		else
		{
			printf("Error: cannot interpret switch %s %s\n",argv[3], argv[4]);
			return -1;
		}
	}
	else
	{
			printf("Error: wrong number of arguments.\n");
			return -1;
	}

	// 0: compress from .any to .pkm with SPEED_FAST, METRIC_NONPERCEPTUAL, 
	// 1: compress from .any to .pkm with SPEED_MEDIUM, METRIC_NONPERCEPTUAL, 
	// 2: compress from .any to .pkm with SPEED_SLOW, METRIC_NONPERCEPTUAL, 
	// 3: compress from .any to .pkm with SPEED_FAST, METRIC_PERCEPTUAL, 
	// 4: compress from .any to .pkm with SPEED_MEDIUM, METRIC_PERCEPTUAL, 
	// 5: compress from .any to .pkm with SPEED_SLOW, METRIC_PERCEPTUAL, 
	// 6: decompress from .pkm to .any
	// 7: calculate PSNR between .any and .any

	if(mode == MODE_UNCOMPRESS)
	{
		return 6;
	}
	else if(mode == MODE_PSNR)
	{
		return 7;
	}
	else // mode == MODE_COMPRESS
	{
		if(speed == SPEED_FAST && metric == METRIC_NONPERCEPTUAL)
			return 0;
		else if (speed == SPEED_MEDIUM && metric == METRIC_NONPERCEPTUAL)
			return 1;
		else if (speed == SPEED_SLOW && metric == METRIC_NONPERCEPTUAL)
			return 2;
		else if (speed == SPEED_FAST && metric == METRIC_PERCEPTUAL)
			return 3;
		else if (speed == SPEED_MEDIUM && metric == METRIC_PERCEPTUAL)
			return 4;
		else if (speed == SPEED_SLOW && metric == METRIC_PERCEPTUAL)
			return 5;
	}
	return -1;
}

void computeAverageColor2x4noQuantFloat(uint8 *img,int width,int height,int startx,int starty,float *avg_color)
{
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

void computeAverageColor4x2noQuantFloat(uint8 *img,int width,int height,int startx,int starty,float *avg_color)
{
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

int compressBlockWithTable2x4(uint8 *img,int width,int height,int startx,int starty,uint8 *avg_color,int table,unsigned int *pixel_indices_MSBp, unsigned int *pixel_indices_LSBp)
{
  (void)height;

	uint8 orig[3],approx[3];
	unsigned int pixel_indices_MSB=0, pixel_indices_LSB=0, pixel_indices = 0;
	int sum_error=0;
	int q, i;


	i = 0;
	for(int x=startx; x<startx+2; x++)
	{
		for(int y=starty; y<starty+4; y++)
		{
			int err;
			int best=0;
			int min_error=255*255*3*16;
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
				err=SQUARE(approx[0]-orig[0]) + SQUARE(approx[1]-orig[1]) + SQUARE(approx[2]-orig[2]);
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

float compressBlockWithTable2x4percep(uint8 *img,int width,int height,int startx,int starty,uint8 *avg_color,int table,unsigned int *pixel_indices_MSBp, unsigned int *pixel_indices_LSBp)
{
  (void)height;

	uint8 orig[3],approx[3];
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
int compressBlockWithTable4x2(uint8 *img,int width,int height,int startx,int starty,uint8 *avg_color,int table,unsigned int *pixel_indices_MSBp, unsigned int *pixel_indices_LSBp)
{
  (void)height;

	uint8 orig[3],approx[3];
	unsigned int pixel_indices_MSB=0, pixel_indices_LSB=0, pixel_indices = 0;
	int sum_error=0;
	int q;
	int i;


	i = 0;
	for(int x=startx; x<startx+4; x++)
	{
		for(int y=starty; y<starty+2; y++)
		{
			int err;
			int best=0;
			int min_error=255*255*3*16;
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
				err=SQUARE(approx[0]-orig[0]) + SQUARE(approx[1]-orig[1]) + SQUARE(approx[2]-orig[2]);
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
		i+=2;

	}

	*pixel_indices_MSBp = pixel_indices_MSB;
	*pixel_indices_LSBp = pixel_indices_LSB;


	return sum_error;
}

float compressBlockWithTable4x2percep(uint8 *img,int width,int height,int startx,int starty,uint8 *avg_color,int table,unsigned int *pixel_indices_MSBp, unsigned int *pixel_indices_LSBp)
{
  (void)height;

	uint8 orig[3],approx[3];
	unsigned int pixel_indices_MSB=0, pixel_indices_LSB=0, pixel_indices = 0;
	float sum_error=0;
	int q;
	int i;
	float wR2 = (float) PERCEPTUAL_WEIGHT_R_SQUARED;
	float wG2 = (float) PERCEPTUAL_WEIGHT_G_SQUARED;
	float wB2 = (float) PERCEPTUAL_WEIGHT_B_SQUARED;


	i = 0;
	for(int x=startx; x<startx+4; x++)
	{
		for(int y=starty; y<starty+2; y++)
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
				err=(float) wR2*SQUARE(approx[0]-orig[0]) + (float)wG2*SQUARE(approx[1]-orig[1]) + (float)wB2*SQUARE(approx[2]-orig[2]);
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
		i+=2;

	}

	*pixel_indices_MSBp = pixel_indices_MSB;
	*pixel_indices_LSBp = pixel_indices_LSB;


	return sum_error;
}

int tryalltables_3bittable2x4(uint8 *img,int width,int height,int startx,int starty,uint8 *avg_color, unsigned int &best_table,unsigned int &best_pixel_indices_MSB, unsigned int &best_pixel_indices_LSB)
{
	int min_error = 3*255*255*16;
	int q;
	int err;
	unsigned int pixel_indices_MSB, pixel_indices_LSB;

	for(q=0;q<16;q+=2)		// try all the 8 tables. 
	{

		err=compressBlockWithTable2x4(img,width,height,startx,starty,avg_color,q,&pixel_indices_MSB, &pixel_indices_LSB);

		if(err<min_error)
		{

			min_error=err;
			best_pixel_indices_MSB = pixel_indices_MSB;
			best_pixel_indices_LSB = pixel_indices_LSB;
			best_table=q >> 1;

		}
	}
	return min_error;
}

int tryalltables_3bittable2x4percep(uint8 *img,int width,int height,int startx,int starty,uint8 *avg_color, unsigned int &best_table,unsigned int &best_pixel_indices_MSB, unsigned int &best_pixel_indices_LSB)
{
	float min_error = 3*255*255*16;
	int q;
	float err;
	unsigned int pixel_indices_MSB, pixel_indices_LSB;

	for(q=0;q<16;q+=2)		// try all the 8 tables. 
	{

		err=compressBlockWithTable2x4percep(img,width,height,startx,starty,avg_color,q,&pixel_indices_MSB, &pixel_indices_LSB);

		if(err<min_error)
		{

			min_error=err;
			best_pixel_indices_MSB = pixel_indices_MSB;
			best_pixel_indices_LSB = pixel_indices_LSB;
			best_table=q >> 1;

		}
	}
	return (int) min_error;
}

int tryalltables_3bittable4x2(uint8 *img,int width,int height,int startx,int starty,uint8 *avg_color, unsigned int &best_table,unsigned int &best_pixel_indices_MSB, unsigned int &best_pixel_indices_LSB)
{
	int min_error = 3*255*255*16;
	int q;
	int err;
	unsigned int pixel_indices_MSB, pixel_indices_LSB;

	for(q=0;q<16;q+=2)		// try all the 8 tables. 
	{

		err=compressBlockWithTable4x2(img,width,height,startx,starty,avg_color,q,&pixel_indices_MSB, &pixel_indices_LSB);

		if(err<min_error)
		{

			min_error=err;
			best_pixel_indices_MSB = pixel_indices_MSB;
			best_pixel_indices_LSB = pixel_indices_LSB;
			best_table=q >> 1;
		}
	}

	return min_error;
}
int tryalltables_3bittable4x2percep(uint8 *img,int width,int height,int startx,int starty,uint8 *avg_color, unsigned int &best_table,unsigned int &best_pixel_indices_MSB, unsigned int &best_pixel_indices_LSB)
{
	float min_error = 3*255*255*16;
	int q;
	float err;
	unsigned int pixel_indices_MSB, pixel_indices_LSB;

	for(q=0;q<16;q+=2)		// try all the 8 tables. 
	{

		err=compressBlockWithTable4x2percep(img,width,height,startx,starty,avg_color,q,&pixel_indices_MSB, &pixel_indices_LSB);

		if(err<min_error)
		{

			min_error=err;
			best_pixel_indices_MSB = pixel_indices_MSB;
			best_pixel_indices_LSB = pixel_indices_LSB;
			best_table=q >> 1;
		}
	}

	return (int) min_error;
}

// The below code quantizes a float RGB value to RGB555. 
//

void quantize444ColorCombined(float *avg_col_in, int *enc_color, uint8 *avg_color)
{
	// Detta ar nummer tva
	float dr, dg, db;
	float kr, kg, kb;
	float wR2, wG2, wB2;
	uint8 low_color[3];
	uint8 high_color[3];
//	float min_error=255*255*8*3;
	float lowhightable[8];
//	unsigned int best_table=0;
//	unsigned int best_index=0;
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

	kr = (float)red_high - (float)red_low;
	kg = (float)green_high - (float)green_low;
	kb = (float)blue_high - (float)blue_low;

	// Note that dr, dg, and db are all negative.
	dr = red_low - red_average;
	dg = green_low - green_average;
	db = blue_low - blue_average;

	// Use straight (nonperceptive) weights.
	wR2 = (float) 1.0;
	wG2 = (float) 1.0;
	wB2 = (float) 1.0;

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

	low_color[0] = red_4bit_low;
	low_color[1] = green_4bit_low;
	low_color[2] = blue_4bit_low;

	high_color[0] = red_4bit_high;
	high_color[1] = green_4bit_high;
	high_color[2] = blue_4bit_high;

	switch(min_index)
	{
	case 0:
		// Since the step size is always 17 in RGB444 format (15*17=255),
		// kr = kg = kb = 17, which means that case 0 and case 7 will
		// always have equal projected error. Choose the one that is
		// closer to the desired color. 
		if(dr*dr + dg*dg + db*db > 3*8*8)
		{
			enc_color[0] = high_color[0];
			enc_color[1] = high_color[1];
			enc_color[2] = high_color[2];
		}
		else
		{
			enc_color[0] = low_color[0];
			enc_color[1] = low_color[1];
			enc_color[2] = low_color[2];
		}
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
		if(dr*dr + dg*dg + db*db > 3*8*8)
		{
			enc_color[0] = high_color[0];
			enc_color[1] = high_color[1];
			enc_color[2] = high_color[2];
		}
		else
		{
			enc_color[0] = low_color[0];
			enc_color[1] = low_color[1];
			enc_color[2] = low_color[2];
		}
		break;
	}

	// Expand 5-bit encoded color to 8-bit color
	avg_color[0] = (enc_color[0] << 3) | (enc_color[0] >> 2);
	avg_color[1] = (enc_color[1] << 3) | (enc_color[1] >> 2);
	avg_color[2] = (enc_color[2] << 3) | (enc_color[2] >> 2);	
}


// The below code quantizes a float RGB value to RGB555. 
//
void quantize555ColorCombined(float *avg_col_in, int *enc_color, uint8 *avg_color)
{
	float dr, dg, db;
	float kr, kg, kb;
	float wR2, wG2, wB2;
	uint8 low_color[3];
	uint8 high_color[3];
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

	kr = (float)red_high - (float)red_low;
	kg = (float)green_high - (float)green_low;
	kb = (float)blue_high - (float)blue_low;

	// Note that dr, dg, and db are all negative.

	dr = red_low - red_average;
	dg = green_low - green_average;
	db = blue_low - blue_average;

	// Use straight (nonperceptive) weights.
	wR2 = (float) 1.0;
	wG2 = (float) 1.0;
	wB2 = (float) 1.0;

	lowhightable[0] = wR2*wG2*SQUARE( (dr+ 0) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+ 0) );
	lowhightable[1] = wR2*wG2*SQUARE( (dr+kr) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+kr) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+ 0) );
	lowhightable[2] = wR2*wG2*SQUARE( (dr+ 0) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+kg) - (db+ 0) );
	lowhightable[3] = wR2*wG2*SQUARE( (dr+ 0) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+kb) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+kb) );
	lowhightable[4] = wR2*wG2*SQUARE( (dr+kr) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+kr) - (db+ 0) ) + wG2*wB2*SQUARE( (dg+kg) - (db+ 0) );
	lowhightable[5] = wR2*wG2*SQUARE( (dr+kr) - (dg+ 0) ) + wR2*wB2*SQUARE( (dr+kr) - (db+kb) ) + wG2*wB2*SQUARE( (dg+ 0) - (db+kb) );
	lowhightable[6] = wR2*wG2*SQUARE( (dr+ 0) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+ 0) - (db+kb) ) + wG2*wB2*SQUARE( (dg+kg) - (db+kb) );
	lowhightable[7] = wR2*wG2*SQUARE( (dr+kr) - (dg+kg) ) + wR2*wB2*SQUARE( (dr+kr) - (db+kb) ) + wG2*wB2*SQUARE( (dg+kg) - (db+kb) );


	float	min_value = lowhightable[0];
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

	low_color[0] = red_5bit_low;
	low_color[1] = green_5bit_low;
	low_color[2] = blue_5bit_low;

	high_color[0] = red_5bit_high;
	high_color[1] = green_5bit_high;
	high_color[2] = blue_5bit_high;

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


// The below code quantizes a float RGB value to RGB444. 
// It is thus the same as the above function quantize444ColorCombined(), but it uses a 
// weighted error metric instead. 
// 
void quantize444ColorCombinedPerceptual(float *avg_col_in, int *enc_color, uint8 *avg_color)
{
	float dr, dg, db;
	float kr, kg, kb;
	float wR2, wG2, wB2;
	uint8 low_color[3];
	uint8 high_color[3];
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


// The below code quantizes a float RGB value to RGB555. 
// It is thus the same as the above function quantize555ColorCombined(), but it uses a 
// weighted error metric instead. 
// 
void quantize555ColorCombinedPerceptual(float *avg_col_in, int *enc_color, uint8 *avg_color)
{
	float dr, dg, db;
	float kr, kg, kb;
	float wR2, wG2, wB2;
	uint8 low_color[3];
	uint8 high_color[3];
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


void compressBlockDiffFlipSlow(uint8 *img,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{


	unsigned int compressed1_norm_diff, compressed2_norm_diff;
	unsigned int compressed1_norm_444, compressed2_norm_444;
	unsigned int compressed1_flip_diff, compressed2_flip_diff;
	unsigned int compressed1_flip_444, compressed2_flip_444;
	unsigned int best_err_norm_diff = 255*255*8*3;
	unsigned int best_err_norm_444 = 255*255*8*3;
	unsigned int best_err_flip_diff = 255*255*8*3;
	unsigned int best_err_flip_444 = 255*255*8*3;
	uint8 avg_color_quant1[3], avg_color_quant2[3];

	float avg_color_float1[3],avg_color_float2[3];
	int enc_color1[3], enc_color2[3], diff[3];
	int enc_base1[3], enc_base2[3];
	int enc_try1[3], enc_try2[3];
  //int min_error=255*255*8*3;
  int err;
	unsigned int best_pixel_indices1_MSB=0;
	unsigned int best_pixel_indices1_LSB=0;
	unsigned int best_pixel_indices2_MSB=0;
	unsigned int best_pixel_indices2_LSB=0;

	unsigned int best_table1=0, best_table2=0;
    int diffbit;

	int norm_err=0;
	int flip_err=0;
	int minerr;
	int dr1, dg1, db1, dr2, dg2, db2;

	// First try normal blocks 2x4:

	computeAverageColor2x4noQuantFloat(img,width,height,startx,starty,avg_color_float1);
	computeAverageColor2x4noQuantFloat(img,width,height,startx+2,starty,avg_color_float2);

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

    if( (diff[0] >= SLOW_TRY_MIN) && (diff[0] <= SLOW_TRY_MAX) && (diff[1] >= SLOW_TRY_MIN) && (diff[1] <= SLOW_TRY_MAX) && (diff[2] >= SLOW_TRY_MIN) && (diff[2] <= SLOW_TRY_MAX) )
	{
		diffbit = 1;

		enc_base1[0] = enc_color1[0];
		enc_base1[1] = enc_color1[1];
		enc_base1[2] = enc_color1[2];
		enc_base2[0] = enc_color2[0];
		enc_base2[1] = enc_color2[1];
		enc_base2[2] = enc_color2[2];

		int err1[SLOW_SCAN_RANGE][SLOW_SCAN_RANGE][SLOW_SCAN_RANGE];
		int err2[SLOW_SCAN_RANGE][SLOW_SCAN_RANGE][SLOW_SCAN_RANGE];

		// left part of block 
		for(dr1 = SLOW_SCAN_MIN; dr1<SLOW_SCAN_MAX+1; dr1++)
		{
			for(dg1 = SLOW_SCAN_MIN; dg1<SLOW_SCAN_MAX+1; dg1++)
			{
				for(db1 = SLOW_SCAN_MIN; db1<SLOW_SCAN_MAX+1; db1++)
				{
					enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
					enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
					enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);

					avg_color_quant1[0] = enc_try1[0] << 3 | (enc_try1[0] >> 2);
					avg_color_quant1[1] = enc_try1[1] << 3 | (enc_try1[1] >> 2);
					avg_color_quant1[2] = enc_try1[2] << 3 | (enc_try1[2] >> 2);

					// left part of block
					err1[dr1+SLOW_SCAN_OFFSET][dg1+SLOW_SCAN_OFFSET][db1+SLOW_SCAN_OFFSET] = tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
				}
			}
		}

		// right part of block
		for(dr2 = SLOW_SCAN_MIN; dr2<SLOW_SCAN_MAX+1; dr2++)
		{
			for(dg2 = SLOW_SCAN_MIN; dg2<SLOW_SCAN_MAX+1; dg2++)
			{
				for(db2 = SLOW_SCAN_MIN; db2<SLOW_SCAN_MAX+1; db2++)
				{
					enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
					enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
					enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

					avg_color_quant2[0] = enc_try2[0] << 3 | (enc_try2[0] >> 2);
					avg_color_quant2[1] = enc_try2[1] << 3 | (enc_try2[1] >> 2);
					avg_color_quant2[2] = enc_try2[2] << 3 | (enc_try2[2] >> 2);

					// left part of block
					err2[dr2+SLOW_SCAN_OFFSET][dg2+SLOW_SCAN_OFFSET][db2+SLOW_SCAN_OFFSET] = tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);
				}
			}
		}

		// Now see what combinations are both low in error and possible to
		// encode differentially.

		minerr = 255*255*3*8*2;

		for(dr1 = SLOW_SCAN_MIN; dr1<SLOW_SCAN_MAX+1; dr1++)
		{
			for(dg1 = SLOW_SCAN_MIN; dg1<SLOW_SCAN_MAX+1; dg1++)
			{
				for(db1 = SLOW_SCAN_MIN; db1<SLOW_SCAN_MAX+1; db1++)
				{
					for(dr2 = SLOW_SCAN_MIN; dr2<SLOW_SCAN_MAX+1; dr2++)
					{
						for(dg2 = SLOW_SCAN_MIN; dg2<SLOW_SCAN_MAX+1; dg2++)
						{
							for(db2 = SLOW_SCAN_MIN; db2<SLOW_SCAN_MAX+1; db2++)
							{								
								enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
								enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
								enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);
								enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
								enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
								enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

								// We must make sure that the difference between the tries still is less than allowed

								diff[0] = enc_try2[0]-enc_try1[0];	
								diff[1] = enc_try2[1]-enc_try1[1];	
								diff[2] = enc_try2[2]-enc_try1[2];

							    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
								{
									// The diff is OK, calculate total error:
									
									err = err1[dr1+SLOW_SCAN_OFFSET][dg1+SLOW_SCAN_OFFSET][db1+SLOW_SCAN_OFFSET] + err2[dr2+SLOW_SCAN_OFFSET][dg2+SLOW_SCAN_OFFSET][db2+SLOW_SCAN_OFFSET];

									if(err < minerr)
									{
										minerr = err;

										enc_color1[0] = enc_try1[0];
										enc_color1[1] = enc_try1[1];
										enc_color1[2] = enc_try1[2];
										enc_color2[0] = enc_try2[0];
										enc_color2[1] = enc_try2[1];
										enc_color2[2] = enc_try2[2];
									}
								}
							}
						}
					}
				}
			}
		}

		best_err_norm_diff = minerr;
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


		compressed1_norm_diff = 0;
		PUTBITSHIGH( compressed1_norm_diff, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[0], 5, 63);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[1], 5, 55);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[2], 5, 47);
 		PUTBITSHIGH( compressed1_norm_diff, diff[0],       3, 58);
 		PUTBITSHIGH( compressed1_norm_diff, diff[1],       3, 50);
 		PUTBITSHIGH( compressed1_norm_diff, diff[2],       3, 42);

		
		// left part of block
		tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_norm_diff, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_norm_diff, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_norm_diff, 0,             1, 32);

		compressed2_norm_diff = 0;
		PUTBITS( compressed2_norm_diff, (best_pixel_indices1_MSB     ), 8, 23);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices2_MSB     ), 8, 31);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices1_LSB     ), 8, 7);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices2_LSB     ), 8, 15);

	}
	// We should do this in any case...
	{
		diffbit = 0;
		// The difference is bigger than what fits in 555 plus delta-333, so we will have
		// to deal with 444 444.


		// Color for left block

		int besterr = 255*255*3*8;
        int bestri = 0, bestgi = 0, bestbi = 0;
		int ri, gi, bi;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color1[0] = ri;
					enc_color1[1] = gi;
					enc_color1[2] = bi;

					avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
					avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
					avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];

					// left part of block
					err = tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB,best_pixel_indices1_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		norm_err = besterr;
		
		enc_color1[0] = bestri;
		enc_color1[1] = bestgi;
		enc_color1[2] = bestbi;
		avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
		avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
		avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];


		// Color for right block

		besterr = 255*255*3*8;
        bestri = 0; bestgi = 0; bestbi = 0;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color2[0] = ri;
					enc_color2[1] = gi;
					enc_color2[2] = bi;

					avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
					avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
					avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

					// left part of block
					err = tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB,best_pixel_indices2_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}


		norm_err += besterr;
		best_err_norm_444 = norm_err;

		enc_color2[0] = bestri;
		enc_color2[1] = bestgi;
		enc_color2[2] = bestbi;
		avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
		avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
		avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


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


		compressed1_norm_444 = 0;
		PUTBITSHIGH( compressed1_norm_444, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[0], 4, 63);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[1], 4, 55);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[2], 4, 47);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[0], 4, 59);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[1], 4, 51);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[2], 4, 43);


		// left part of block
		tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_norm_444, best_table1, 3, 39);
 		PUTBITSHIGH( compressed1_norm_444, best_table2, 3, 36);
 		PUTBITSHIGH( compressed1_norm_444, 0,           1, 32);

		compressed2_norm_444 = 0;
		PUTBITS( compressed2_norm_444, (best_pixel_indices1_MSB     ), 8, 23);
		PUTBITS( compressed2_norm_444, (best_pixel_indices2_MSB     ), 8, 31);
		PUTBITS( compressed2_norm_444, (best_pixel_indices1_LSB     ), 8, 7);
		PUTBITS( compressed2_norm_444, (best_pixel_indices2_LSB     ), 8, 15);

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

    if( (diff[0] >= SLOW_TRY_MIN) && (diff[0] <= SLOW_TRY_MAX) && (diff[1] >= SLOW_TRY_MIN) && (diff[1] <= SLOW_TRY_MAX) && (diff[2] >= SLOW_TRY_MIN) && (diff[2] <= SLOW_TRY_MAX) )
	{
		diffbit = 1;

		enc_base1[0] = enc_color1[0];
		enc_base1[1] = enc_color1[1];
		enc_base1[2] = enc_color1[2];
		enc_base2[0] = enc_color2[0];
		enc_base2[1] = enc_color2[1];
		enc_base2[2] = enc_color2[2];

		int err1[SLOW_SCAN_RANGE][SLOW_SCAN_RANGE][SLOW_SCAN_RANGE];
		int err2[SLOW_SCAN_RANGE][SLOW_SCAN_RANGE][SLOW_SCAN_RANGE];

		// upper part of block
		for(dr1 = SLOW_SCAN_MIN; dr1<SLOW_SCAN_MAX+1; dr1++)
		{
			for(dg1 = SLOW_SCAN_MIN; dg1<SLOW_SCAN_MAX+1; dg1++)
			{
				for(db1 = SLOW_SCAN_MIN; db1<SLOW_SCAN_MAX+1; db1++)
				{
					enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
					enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
					enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);

					avg_color_quant1[0] = enc_try1[0] << 3 | (enc_try1[0] >> 2);
					avg_color_quant1[1] = enc_try1[1] << 3 | (enc_try1[1] >> 2);
					avg_color_quant1[2] = enc_try1[2] << 3 | (enc_try1[2] >> 2);

					// upper part of block
					err1[dr1+SLOW_SCAN_OFFSET][dg1+SLOW_SCAN_OFFSET][db1+SLOW_SCAN_OFFSET] = tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
				}
			}
		}

		// lower part of block
		for(dr2 = SLOW_SCAN_MIN; dr2<SLOW_SCAN_MAX+1; dr2++)
		{
			for(dg2 = SLOW_SCAN_MIN; dg2<SLOW_SCAN_MAX+1; dg2++)
			{
				for(db2 = SLOW_SCAN_MIN; db2<SLOW_SCAN_MAX+1; db2++)
				{
					enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
					enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
					enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

					avg_color_quant2[0] = enc_try2[0] << 3 | (enc_try2[0] >> 2);
					avg_color_quant2[1] = enc_try2[1] << 3 | (enc_try2[1] >> 2);
					avg_color_quant2[2] = enc_try2[2] << 3 | (enc_try2[2] >> 2);

					// lower part of block
					err2[dr2+SLOW_SCAN_OFFSET][dg2+SLOW_SCAN_OFFSET][db2+SLOW_SCAN_OFFSET] = tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);
				}
			}
		}

		// Now see what combinations are both low in error and possible to
		// encode differentially.

		minerr = 255*255*3*8*2;

		for(dr1 = SLOW_SCAN_MIN; dr1<SLOW_SCAN_MAX+1; dr1++)
		{
			for(dg1 = SLOW_SCAN_MIN; dg1<SLOW_SCAN_MAX+1; dg1++)
			{
				for(db1 = SLOW_SCAN_MIN; db1<SLOW_SCAN_MAX+1; db1++)
				{
					for(dr2 = SLOW_SCAN_MIN; dr2<SLOW_SCAN_MAX+1; dr2++)
					{
						for(dg2 = SLOW_SCAN_MIN; dg2<SLOW_SCAN_MAX+1; dg2++)
						{
							for(db2 = SLOW_SCAN_MIN; db2<SLOW_SCAN_MAX+1; db2++)
							{								
								enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
								enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
								enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);
								enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
								enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
								enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

								// We must make sure that the difference between the tries still is less than allowed

								diff[0] = enc_try2[0]-enc_try1[0];	
								diff[1] = enc_try2[1]-enc_try1[1];	
								diff[2] = enc_try2[2]-enc_try1[2];

							    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
								{
									// The diff is OK, calculate total error:
									
									err = err1[dr1+SLOW_SCAN_OFFSET][dg1+SLOW_SCAN_OFFSET][db1+SLOW_SCAN_OFFSET] + err2[dr2+SLOW_SCAN_OFFSET][dg2+SLOW_SCAN_OFFSET][db2+SLOW_SCAN_OFFSET];

									if(err < minerr)
									{
										minerr = err;

										enc_color1[0] = enc_try1[0];
										enc_color1[1] = enc_try1[1];
										enc_color1[2] = enc_try1[2];
										enc_color2[0] = enc_try2[0];
										enc_color2[1] = enc_try2[1];
										enc_color2[2] = enc_try2[2];
									}
								}
							}
						}
					}
				}
			}
		}


		flip_err = minerr;

		best_err_flip_diff = flip_err;

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


		// Pack bits into the first word. 

		compressed1_flip_diff = 0;
		PUTBITSHIGH( compressed1_flip_diff, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[0], 5, 63);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[1], 5, 55);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[2], 5, 47);
 		PUTBITSHIGH( compressed1_flip_diff, diff[0],       3, 58);
 		PUTBITSHIGH( compressed1_flip_diff, diff[1],       3, 50);
 		PUTBITSHIGH( compressed1_flip_diff, diff[2],       3, 42);




		// upper part of block
		tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
		// lower part of block
		tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);


 		PUTBITSHIGH( compressed1_flip_diff, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_flip_diff, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_flip_diff, 1,             1, 32);


		best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
		best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);
		
		compressed2_flip_diff = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);
	
	}
	{
		diffbit = 0;
		// The difference is bigger than what fits in 555 plus delta-333, so we will have
		// to deal with 444 444.


		// Color for upper block

		int besterr = 255*255*3*8;
        int bestri = 0, bestgi = 0, bestbi = 0;
		int ri, gi, bi;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color1[0] = ri;
					enc_color1[1] = gi;
					enc_color1[2] = bi;

					avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
					avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
					avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];

					// upper part of block
					err = tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		flip_err = besterr;

		enc_color1[0] = bestri;
		enc_color1[1] = bestgi;
		enc_color1[2] = bestbi;
		avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
		avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
		avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];


		// Color for lower block

		besterr = 255*255*3*8;
        bestri = 0; bestgi = 0; bestbi = 0;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color2[0] = ri;
					enc_color2[1] = gi;
					enc_color2[2] = bi;

					avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
					avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
					avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

					// left part of block
					err = tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		flip_err += besterr;
		best_err_flip_444 = flip_err;

		enc_color2[0] = bestri;
		enc_color2[1] = bestgi;
		enc_color2[2] = bestbi;
		avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
		avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
		avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


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


		compressed1_flip_444 = 0;
		PUTBITSHIGH( compressed1_flip_444, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[0], 4, 63);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[1], 4, 55);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[2], 4, 47);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[0], 4, 59);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[1], 4, 51);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[2], 4, 43);

		// upper part of block
		tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB,best_pixel_indices1_LSB);

		// lower part of block
		tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_flip_444, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_flip_444, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_flip_444, 1,             1, 32);

		best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
		best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);
		
		compressed2_flip_444 = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);


	}

	// Now lets see which is the best table to use. Only 8 tables are possible. 

	int compressed1_norm;
	int compressed2_norm;
	int compressed1_flip;
	int compressed2_flip;


	// See which of the norm blocks is better

	if(best_err_norm_diff <= best_err_norm_444)
	{
		compressed1_norm = compressed1_norm_diff;
		compressed2_norm = compressed2_norm_diff;
		norm_err = best_err_norm_diff;
	}
	else
	{
		compressed1_norm = compressed1_norm_444;
		compressed2_norm = compressed2_norm_444;
		norm_err = best_err_norm_444;
	}

	// See which of the flip blocks is better

	if(best_err_flip_diff <= best_err_flip_444)
	{
		compressed1_flip = compressed1_flip_diff;
		compressed2_flip = compressed2_flip_diff;
		flip_err = best_err_flip_diff;
	}
	else
	{
		compressed1_flip = compressed1_flip_444;
		compressed2_flip = compressed2_flip_444;
		flip_err = best_err_flip_444;
	}

	// See if flip or norm is better

  //unsigned int best_of_all;

	if(norm_err <= flip_err)
	{

		compressed1 = compressed1_norm | 0;
		compressed2 = compressed2_norm;
    //best_of_all = norm_err;
	}
	else
	{

		compressed1 = compressed1_flip | 1;
		compressed2 = compressed2_flip;
    //best_of_all = flip_err;
	}

}

void compressBlockDiffFlipMedium(uint8 *img,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{


	unsigned int compressed1_norm_diff, compressed2_norm_diff;
	unsigned int compressed1_norm_444, compressed2_norm_444;
	unsigned int compressed1_flip_diff, compressed2_flip_diff;
	unsigned int compressed1_flip_444, compressed2_flip_444;
	unsigned int best_err_norm_diff = 255*255*16*3;
	unsigned int best_err_norm_444 = 255*255*16*3;
	unsigned int best_err_flip_diff = 255*255*16*3;
	unsigned int best_err_flip_444 = 255*255*16*3;
	uint8 avg_color_quant1[3], avg_color_quant2[3];

	float avg_color_float1[3],avg_color_float2[3];
	int enc_color1[3], enc_color2[3], diff[3];
	int enc_base1[3], enc_base2[3];
	int enc_try1[3], enc_try2[3];
  //int min_error=255*255*16*3;
  int err;
	unsigned int best_pixel_indices1_MSB=0;
	unsigned int best_pixel_indices1_LSB=0;
	unsigned int best_pixel_indices2_MSB=0;
	unsigned int best_pixel_indices2_LSB=0;

	unsigned int best_table1=0, best_table2=0;
    int diffbit;

	int norm_err=0;
	int flip_err=0;
	int minerr;
	int dr1, dg1, db1, dr2, dg2, db2;

	// First try normal blocks 2x4:

	computeAverageColor2x4noQuantFloat(img,width,height,startx,starty,avg_color_float1);
	computeAverageColor2x4noQuantFloat(img,width,height,startx+2,starty,avg_color_float2);



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

    if( (diff[0] >= MEDIUM_TRY_MIN) && (diff[0] <= MEDIUM_TRY_MAX) && (diff[1] >= MEDIUM_TRY_MIN) && (diff[1] <= MEDIUM_TRY_MAX) && (diff[2] >= MEDIUM_TRY_MIN) && (diff[2] <= MEDIUM_TRY_MAX) )
	{
		diffbit = 1;

		enc_base1[0] = enc_color1[0];
		enc_base1[1] = enc_color1[1];
		enc_base1[2] = enc_color1[2];
		enc_base2[0] = enc_color2[0];
		enc_base2[1] = enc_color2[1];
		enc_base2[2] = enc_color2[2];

		int err1[MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE];
		int err2[MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE];

		// left part of block 
		for(dr1 = MEDIUM_SCAN_MIN; dr1<MEDIUM_SCAN_MAX+1; dr1++)
		{
			for(dg1 = MEDIUM_SCAN_MIN; dg1<MEDIUM_SCAN_MAX+1; dg1++)
			{
				for(db1 = MEDIUM_SCAN_MIN; db1<MEDIUM_SCAN_MAX+1; db1++)
				{
					enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
					enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
					enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);

					avg_color_quant1[0] = enc_try1[0] << 3 | (enc_try1[0] >> 2);
					avg_color_quant1[1] = enc_try1[1] << 3 | (enc_try1[1] >> 2);
					avg_color_quant1[2] = enc_try1[2] << 3 | (enc_try1[2] >> 2);

					// left part of block
					err1[dr1+MEDIUM_SCAN_OFFSET][dg1+MEDIUM_SCAN_OFFSET][db1+MEDIUM_SCAN_OFFSET] = tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
				}
			}
		}

		// right part of block
		for(dr2 = MEDIUM_SCAN_MIN; dr2<MEDIUM_SCAN_MAX+1; dr2++)
		{
			for(dg2 = MEDIUM_SCAN_MIN; dg2<MEDIUM_SCAN_MAX+1; dg2++)
			{
				for(db2 = MEDIUM_SCAN_MIN; db2<MEDIUM_SCAN_MAX+1; db2++)
				{
					enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
					enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
					enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

					avg_color_quant2[0] = enc_try2[0] << 3 | (enc_try2[0] >> 2);
					avg_color_quant2[1] = enc_try2[1] << 3 | (enc_try2[1] >> 2);
					avg_color_quant2[2] = enc_try2[2] << 3 | (enc_try2[2] >> 2);

					// left part of block
					err2[dr2+MEDIUM_SCAN_OFFSET][dg2+MEDIUM_SCAN_OFFSET][db2+MEDIUM_SCAN_OFFSET] = tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);
				}
			}
		}

		// Now see what combinations are both low in error and possible to
		// encode differentially.

		minerr = 255*255*3*8*2;

		for(dr1 = MEDIUM_SCAN_MIN; dr1<MEDIUM_SCAN_MAX+1; dr1++)
		{
			for(dg1 = MEDIUM_SCAN_MIN; dg1<MEDIUM_SCAN_MAX+1; dg1++)
			{
				for(db1 = MEDIUM_SCAN_MIN; db1<MEDIUM_SCAN_MAX+1; db1++)
				{
					for(dr2 = MEDIUM_SCAN_MIN; dr2<MEDIUM_SCAN_MAX+1; dr2++)
					{
						for(dg2 = MEDIUM_SCAN_MIN; dg2<MEDIUM_SCAN_MAX+1; dg2++)
						{
							for(db2 = MEDIUM_SCAN_MIN; db2<MEDIUM_SCAN_MAX+1; db2++)
							{								
								enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
								enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
								enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);
								enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
								enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
								enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

								// We must make sure that the difference between the tries still is less than allowed

								diff[0] = enc_try2[0]-enc_try1[0];	
								diff[1] = enc_try2[1]-enc_try1[1];	
								diff[2] = enc_try2[2]-enc_try1[2];

							    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
								{
									// The diff is OK, calculate total error:
									
									err = err1[dr1+MEDIUM_SCAN_OFFSET][dg1+MEDIUM_SCAN_OFFSET][db1+MEDIUM_SCAN_OFFSET] + err2[dr2+MEDIUM_SCAN_OFFSET][dg2+MEDIUM_SCAN_OFFSET][db2+MEDIUM_SCAN_OFFSET];

									if(err < minerr)
									{
										minerr = err;

										enc_color1[0] = enc_try1[0];
										enc_color1[1] = enc_try1[1];
										enc_color1[2] = enc_try1[2];
										enc_color2[0] = enc_try2[0];
										enc_color2[1] = enc_try2[1];
										enc_color2[2] = enc_try2[2];
									}
								}
							}
						}
					}
				}
			}
		}

		best_err_norm_diff = minerr;
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


		compressed1_norm_diff = 0;
		PUTBITSHIGH( compressed1_norm_diff, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[0], 5, 63);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[1], 5, 55);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[2], 5, 47);
 		PUTBITSHIGH( compressed1_norm_diff, diff[0],       3, 58);
 		PUTBITSHIGH( compressed1_norm_diff, diff[1],       3, 50);
 		PUTBITSHIGH( compressed1_norm_diff, diff[2],       3, 42);

		
		// left part of block
		tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_norm_diff, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_norm_diff, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_norm_diff, 0,             1, 32);

		compressed2_norm_diff = 0;
		PUTBITS( compressed2_norm_diff, (best_pixel_indices1_MSB     ), 8, 23);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices2_MSB     ), 8, 31);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices1_LSB     ), 8, 7);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices2_LSB     ), 8, 15);


	}
	else
	{
		diffbit = 0;
		// The difference is bigger than what fits in 555 plus delta-333, so we will have
		// to deal with 444 444.


		// Color for left block

		int besterr = 255*255*3*8;
        int bestri = 0, bestgi = 0, bestbi = 0;
		int ri, gi, bi;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color1[0] = ri;
					enc_color1[1] = gi;
					enc_color1[2] = bi;

					avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
					avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
					avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];

					// left part of block
					err = tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB,best_pixel_indices1_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		norm_err = besterr;
		
		enc_color1[0] = bestri;
		enc_color1[1] = bestgi;
		enc_color1[2] = bestbi;
		avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
		avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
		avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];


		// Color for right block

		besterr = 255*255*3*8;
        bestri = 0; bestgi = 0; bestbi = 0;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color2[0] = ri;
					enc_color2[1] = gi;
					enc_color2[2] = bi;

					avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
					avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
					avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

					// left part of block
					err = tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB,best_pixel_indices2_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}


		norm_err += besterr;
		best_err_norm_444 = norm_err;

		enc_color2[0] = bestri;
		enc_color2[1] = bestgi;
		enc_color2[2] = bestbi;
		avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
		avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
		avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


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


		compressed1_norm_444 = 0;
		PUTBITSHIGH( compressed1_norm_444, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[0], 4, 63);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[1], 4, 55);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[2], 4, 47);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[0], 4, 59);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[1], 4, 51);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[2], 4, 43);


		// left part of block
		tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_norm_444, best_table1, 3, 39);
 		PUTBITSHIGH( compressed1_norm_444, best_table2, 3, 36);
 		PUTBITSHIGH( compressed1_norm_444, 0,           1, 32);

		compressed2_norm_444 = 0;
		PUTBITS( compressed2_norm_444, (best_pixel_indices1_MSB     ), 8, 23);
		PUTBITS( compressed2_norm_444, (best_pixel_indices2_MSB     ), 8, 31);
		PUTBITS( compressed2_norm_444, (best_pixel_indices1_LSB     ), 8, 7);
		PUTBITS( compressed2_norm_444, (best_pixel_indices2_LSB     ), 8, 15);

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

    if( (diff[0] >= MEDIUM_TRY_MIN) && (diff[0] <= MEDIUM_TRY_MAX) && (diff[1] >= MEDIUM_TRY_MIN) && (diff[1] <= MEDIUM_TRY_MAX) && (diff[2] >= MEDIUM_TRY_MIN) && (diff[2] <= MEDIUM_TRY_MAX) )
	{
		diffbit = 1;

		enc_base1[0] = enc_color1[0];
		enc_base1[1] = enc_color1[1];
		enc_base1[2] = enc_color1[2];
		enc_base2[0] = enc_color2[0];
		enc_base2[1] = enc_color2[1];
		enc_base2[2] = enc_color2[2];

		int err1[MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE];
		int err2[MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE];

		// upper part of block
		for(dr1 = MEDIUM_SCAN_MIN; dr1<MEDIUM_SCAN_MAX+1; dr1++)
		{
			for(dg1 = MEDIUM_SCAN_MIN; dg1<MEDIUM_SCAN_MAX+1; dg1++)
			{
				for(db1 = MEDIUM_SCAN_MIN; db1<MEDIUM_SCAN_MAX+1; db1++)
				{
					enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
					enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
					enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);

					avg_color_quant1[0] = enc_try1[0] << 3 | (enc_try1[0] >> 2);
					avg_color_quant1[1] = enc_try1[1] << 3 | (enc_try1[1] >> 2);
					avg_color_quant1[2] = enc_try1[2] << 3 | (enc_try1[2] >> 2);

					// upper part of block
					err1[dr1+MEDIUM_SCAN_OFFSET][dg1+MEDIUM_SCAN_OFFSET][db1+MEDIUM_SCAN_OFFSET] = tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
				}
			}
		}

		// lower part of block
		for(dr2 = MEDIUM_SCAN_MIN; dr2<MEDIUM_SCAN_MAX+1; dr2++)
		{
			for(dg2 = MEDIUM_SCAN_MIN; dg2<MEDIUM_SCAN_MAX+1; dg2++)
			{
				for(db2 = MEDIUM_SCAN_MIN; db2<MEDIUM_SCAN_MAX+1; db2++)
				{
					enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
					enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
					enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

					avg_color_quant2[0] = enc_try2[0] << 3 | (enc_try2[0] >> 2);
					avg_color_quant2[1] = enc_try2[1] << 3 | (enc_try2[1] >> 2);
					avg_color_quant2[2] = enc_try2[2] << 3 | (enc_try2[2] >> 2);

					// lower part of block
					err2[dr2+MEDIUM_SCAN_OFFSET][dg2+MEDIUM_SCAN_OFFSET][db2+MEDIUM_SCAN_OFFSET] = tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);
				}
			}
		}

		// Now see what combinations are both low in error and possible to
		// encode differentially.

		minerr = 255*255*3*8*2;

		for(dr1 = MEDIUM_SCAN_MIN; dr1<MEDIUM_SCAN_MAX+1; dr1++)
		{
			for(dg1 = MEDIUM_SCAN_MIN; dg1<MEDIUM_SCAN_MAX+1; dg1++)
			{
				for(db1 = MEDIUM_SCAN_MIN; db1<MEDIUM_SCAN_MAX+1; db1++)
				{
					for(dr2 = MEDIUM_SCAN_MIN; dr2<MEDIUM_SCAN_MAX+1; dr2++)
					{
						for(dg2 = MEDIUM_SCAN_MIN; dg2<MEDIUM_SCAN_MAX+1; dg2++)
						{
							for(db2 = MEDIUM_SCAN_MIN; db2<MEDIUM_SCAN_MAX+1; db2++)
							{								
								enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
								enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
								enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);
								enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
								enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
								enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

								// We must make sure that the difference between the tries still is less than allowed

								diff[0] = enc_try2[0]-enc_try1[0];	
								diff[1] = enc_try2[1]-enc_try1[1];	
								diff[2] = enc_try2[2]-enc_try1[2];

							    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
								{
									// The diff is OK, calculate total error:
									
									err = err1[dr1+MEDIUM_SCAN_OFFSET][dg1+MEDIUM_SCAN_OFFSET][db1+MEDIUM_SCAN_OFFSET] + err2[dr2+MEDIUM_SCAN_OFFSET][dg2+MEDIUM_SCAN_OFFSET][db2+MEDIUM_SCAN_OFFSET];

									if(err < minerr)
									{
										minerr = err;

										enc_color1[0] = enc_try1[0];
										enc_color1[1] = enc_try1[1];
										enc_color1[2] = enc_try1[2];
										enc_color2[0] = enc_try2[0];
										enc_color2[1] = enc_try2[1];
										enc_color2[2] = enc_try2[2];
									}
								}
							}
						}
					}
				}
			}
		}


		flip_err = minerr;

		best_err_flip_diff = flip_err;

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


		// Pack bits into the first word. 

		compressed1_flip_diff = 0;
		PUTBITSHIGH( compressed1_flip_diff, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[0], 5, 63);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[1], 5, 55);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[2], 5, 47);
 		PUTBITSHIGH( compressed1_flip_diff, diff[0],       3, 58);
 		PUTBITSHIGH( compressed1_flip_diff, diff[1],       3, 50);
 		PUTBITSHIGH( compressed1_flip_diff, diff[2],       3, 42);




		// upper part of block
		tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
		// lower part of block
		tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);


 		PUTBITSHIGH( compressed1_flip_diff, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_flip_diff, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_flip_diff, 1,             1, 32);


		best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
		best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);
		
		compressed2_flip_diff = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);
	
	}
	else
	{
		diffbit = 0;
		// The difference is bigger than what fits in 555 plus delta-333, so we will have
		// to deal with 444 444.


		// Color for upper block

		int besterr = 255*255*3*8;
        int bestri = 0, bestgi = 0, bestbi = 0;
		int ri, gi, bi;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color1[0] = ri;
					enc_color1[1] = gi;
					enc_color1[2] = bi;

					avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
					avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
					avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];

					// upper part of block
					err = tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		flip_err = besterr;

		enc_color1[0] = bestri;
		enc_color1[1] = bestgi;
		enc_color1[2] = bestbi;
		avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
		avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
		avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];


		// Color for lower block

		besterr = 255*255*3*8;
        bestri = 0; bestgi = 0; bestbi = 0;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color2[0] = ri;
					enc_color2[1] = gi;
					enc_color2[2] = bi;

					avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
					avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
					avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

					// left part of block
					err = tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		flip_err += besterr;
		best_err_flip_444 = flip_err;

		enc_color2[0] = bestri;
		enc_color2[1] = bestgi;
		enc_color2[2] = bestbi;
		avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
		avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
		avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


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


		compressed1_flip_444 = 0;
		PUTBITSHIGH( compressed1_flip_444, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[0], 4, 63);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[1], 4, 55);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[2], 4, 47);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[0], 4, 59);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[1], 4, 51);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[2], 4, 43);

		// upper part of block
		tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB,best_pixel_indices1_LSB);

		// lower part of block
		tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_flip_444, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_flip_444, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_flip_444, 1,             1, 32);

		best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
		best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);
		
		compressed2_flip_444 = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);


	}

	// Now lets see which is the best table to use. Only 8 tables are possible. 

	int compressed1_norm;
	int compressed2_norm;
	int compressed1_flip;
	int compressed2_flip;


	// See which of the norm blocks is better

	if(best_err_norm_diff <= best_err_norm_444)
	{
		compressed1_norm = compressed1_norm_diff;
		compressed2_norm = compressed2_norm_diff;
		norm_err = best_err_norm_diff;
	}
	else
	{
		compressed1_norm = compressed1_norm_444;
		compressed2_norm = compressed2_norm_444;
		norm_err = best_err_norm_444;
	}

	// See which of the flip blocks is better

	if(best_err_flip_diff <= best_err_flip_444)
	{
		compressed1_flip = compressed1_flip_diff;
		compressed2_flip = compressed2_flip_diff;
		flip_err = best_err_flip_diff;
	}
	else
	{
		compressed1_flip = compressed1_flip_444;
		compressed2_flip = compressed2_flip_444;
		flip_err = best_err_flip_444;
	}

	// See if flip or norm is better

  //unsigned int best_of_all;

	if(norm_err <= flip_err)
	{

		compressed1 = compressed1_norm | 0;
		compressed2 = compressed2_norm;
    //best_of_all = norm_err;
	}
	else
	{

		compressed1 = compressed1_flip | 1;
		compressed2 = compressed2_flip;
    //best_of_all = flip_err;
	}

}
void compressBlockDiffFlipSlowPerceptual(uint8 *img,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{


	unsigned int compressed1_norm_diff, compressed2_norm_diff;
	unsigned int compressed1_norm_444, compressed2_norm_444;
	unsigned int compressed1_flip_diff, compressed2_flip_diff;
	unsigned int compressed1_flip_444, compressed2_flip_444;
	unsigned int best_err_norm_diff = 255*255*8*3;
	unsigned int best_err_norm_444 = 255*255*8*3;
	unsigned int best_err_flip_diff = 255*255*8*3;
	unsigned int best_err_flip_444 = 255*255*8*3;
	uint8 avg_color_quant1[3], avg_color_quant2[3];

	float avg_color_float1[3],avg_color_float2[3];
	int enc_color1[3], enc_color2[3], diff[3];
	int enc_base1[3], enc_base2[3];
	int enc_try1[3], enc_try2[3];
  //int min_error=255*255*8*3;
  int err;
	unsigned int best_pixel_indices1_MSB=0;
	unsigned int best_pixel_indices1_LSB=0;
	unsigned int best_pixel_indices2_MSB=0;
	unsigned int best_pixel_indices2_LSB=0;

	unsigned int best_table1=0, best_table2=0;
    int diffbit;

	int norm_err=0;
	int flip_err=0;
	int minerr;
	int dr1, dg1, db1, dr2, dg2, db2;

	// First try normal blocks 2x4:

	computeAverageColor2x4noQuantFloat(img,width,height,startx,starty,avg_color_float1);
	computeAverageColor2x4noQuantFloat(img,width,height,startx+2,starty,avg_color_float2);



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

    if( (diff[0] >= SLOW_TRY_MIN) && (diff[0] <= SLOW_TRY_MAX) && (diff[1] >= SLOW_TRY_MIN) && (diff[1] <= SLOW_TRY_MAX) && (diff[2] >= SLOW_TRY_MIN) && (diff[2] <= SLOW_TRY_MAX) )
	{
		diffbit = 1;

		enc_base1[0] = enc_color1[0];
		enc_base1[1] = enc_color1[1];
		enc_base1[2] = enc_color1[2];
		enc_base2[0] = enc_color2[0];
		enc_base2[1] = enc_color2[1];
		enc_base2[2] = enc_color2[2];

		int err1[SLOW_SCAN_RANGE][SLOW_SCAN_RANGE][SLOW_SCAN_RANGE];
		int err2[SLOW_SCAN_RANGE][SLOW_SCAN_RANGE][SLOW_SCAN_RANGE];

		// left part of block 
		for(dr1 = SLOW_SCAN_MIN; dr1<SLOW_SCAN_MAX+1; dr1++)
		{
			for(dg1 = SLOW_SCAN_MIN; dg1<SLOW_SCAN_MAX+1; dg1++)
			{
				for(db1 = SLOW_SCAN_MIN; db1<SLOW_SCAN_MAX+1; db1++)
				{
					enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
					enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
					enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);

					avg_color_quant1[0] = enc_try1[0] << 3 | (enc_try1[0] >> 2);
					avg_color_quant1[1] = enc_try1[1] << 3 | (enc_try1[1] >> 2);
					avg_color_quant1[2] = enc_try1[2] << 3 | (enc_try1[2] >> 2);

					// left part of block
					err1[dr1+SLOW_SCAN_OFFSET][dg1+SLOW_SCAN_OFFSET][db1+SLOW_SCAN_OFFSET] = tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
				}
			}
		}

		// right part of block
		for(dr2 = SLOW_SCAN_MIN; dr2<SLOW_SCAN_MAX+1; dr2++)
		{
			for(dg2 = SLOW_SCAN_MIN; dg2<SLOW_SCAN_MAX+1; dg2++)
			{
				for(db2 = SLOW_SCAN_MIN; db2<SLOW_SCAN_MAX+1; db2++)
				{
					enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
					enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
					enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

					avg_color_quant2[0] = enc_try2[0] << 3 | (enc_try2[0] >> 2);
					avg_color_quant2[1] = enc_try2[1] << 3 | (enc_try2[1] >> 2);
					avg_color_quant2[2] = enc_try2[2] << 3 | (enc_try2[2] >> 2);

					// left part of block
					err2[dr2+SLOW_SCAN_OFFSET][dg2+SLOW_SCAN_OFFSET][db2+SLOW_SCAN_OFFSET] = tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);
				}
			}
		}

		// Now see what combinations are both low in error and possible to
		// encode differentially.

		minerr = 255*255*3*8*2;

		for(dr1 = SLOW_SCAN_MIN; dr1<SLOW_SCAN_MAX+1; dr1++)
		{
			for(dg1 = SLOW_SCAN_MIN; dg1<SLOW_SCAN_MAX+1; dg1++)
			{
				for(db1 = SLOW_SCAN_MIN; db1<SLOW_SCAN_MAX+1; db1++)
				{
					for(dr2 = SLOW_SCAN_MIN; dr2<SLOW_SCAN_MAX+1; dr2++)
					{
						for(dg2 = SLOW_SCAN_MIN; dg2<SLOW_SCAN_MAX+1; dg2++)
						{
							for(db2 = SLOW_SCAN_MIN; db2<SLOW_SCAN_MAX+1; db2++)
							{								
								enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
								enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
								enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);
								enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
								enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
								enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

								// We must make sure that the difference between the tries still is less than allowed

								diff[0] = enc_try2[0]-enc_try1[0];	
								diff[1] = enc_try2[1]-enc_try1[1];	
								diff[2] = enc_try2[2]-enc_try1[2];

							    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
								{
									// The diff is OK, calculate total error:
									
									err = err1[dr1+SLOW_SCAN_OFFSET][dg1+SLOW_SCAN_OFFSET][db1+SLOW_SCAN_OFFSET] + err2[dr2+SLOW_SCAN_OFFSET][dg2+SLOW_SCAN_OFFSET][db2+SLOW_SCAN_OFFSET];

									if(err < minerr)
									{
										minerr = err;

										enc_color1[0] = enc_try1[0];
										enc_color1[1] = enc_try1[1];
										enc_color1[2] = enc_try1[2];
										enc_color2[0] = enc_try2[0];
										enc_color2[1] = enc_try2[1];
										enc_color2[2] = enc_try2[2];
									}
								}
							}
						}
					}
				}
			}
		}

		best_err_norm_diff = minerr;
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


		compressed1_norm_diff = 0;
		PUTBITSHIGH( compressed1_norm_diff, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[0], 5, 63);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[1], 5, 55);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[2], 5, 47);
 		PUTBITSHIGH( compressed1_norm_diff, diff[0],       3, 58);
 		PUTBITSHIGH( compressed1_norm_diff, diff[1],       3, 50);
 		PUTBITSHIGH( compressed1_norm_diff, diff[2],       3, 42);

		
		// left part of block
		tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_norm_diff, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_norm_diff, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_norm_diff, 0,             1, 32);

		compressed2_norm_diff = 0;
		PUTBITS( compressed2_norm_diff, (best_pixel_indices1_MSB     ), 8, 23);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices2_MSB     ), 8, 31);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices1_LSB     ), 8, 7);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices2_LSB     ), 8, 15);


	}
	// We should do this in any case...
	{
		diffbit = 0;
		// The difference is bigger than what fits in 555 plus delta-333, so we will have
		// to deal with 444 444.


		// Color for left block

		int besterr = 255*255*3*8;
        int bestri = 0, bestgi = 0, bestbi = 0;
		int ri, gi, bi;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color1[0] = ri;
					enc_color1[1] = gi;
					enc_color1[2] = bi;

					avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
					avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
					avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];

					// left part of block
					err = tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB,best_pixel_indices1_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		norm_err = besterr;
		
		enc_color1[0] = bestri;
		enc_color1[1] = bestgi;
		enc_color1[2] = bestbi;
		avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
		avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
		avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];


		// Color for right block

		besterr = 255*255*3*8;
        bestri = 0; bestgi = 0; bestbi = 0;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color2[0] = ri;
					enc_color2[1] = gi;
					enc_color2[2] = bi;

					avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
					avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
					avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

					// left part of block
					err = tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB,best_pixel_indices2_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}


		norm_err += besterr;
		best_err_norm_444 = norm_err;

		enc_color2[0] = bestri;
		enc_color2[1] = bestgi;
		enc_color2[2] = bestbi;
		avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
		avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
		avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


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


		compressed1_norm_444 = 0;
		PUTBITSHIGH( compressed1_norm_444, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[0], 4, 63);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[1], 4, 55);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[2], 4, 47);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[0], 4, 59);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[1], 4, 51);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[2], 4, 43);


		// left part of block
		tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_norm_444, best_table1, 3, 39);
 		PUTBITSHIGH( compressed1_norm_444, best_table2, 3, 36);
 		PUTBITSHIGH( compressed1_norm_444, 0,           1, 32);

		compressed2_norm_444 = 0;
		PUTBITS( compressed2_norm_444, (best_pixel_indices1_MSB     ), 8, 23);
		PUTBITS( compressed2_norm_444, (best_pixel_indices2_MSB     ), 8, 31);
		PUTBITS( compressed2_norm_444, (best_pixel_indices1_LSB     ), 8, 7);
		PUTBITS( compressed2_norm_444, (best_pixel_indices2_LSB     ), 8, 15);

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

    if( (diff[0] >= SLOW_TRY_MIN) && (diff[0] <= SLOW_TRY_MAX) && (diff[1] >= SLOW_TRY_MIN) && (diff[1] <= SLOW_TRY_MAX) && (diff[2] >= SLOW_TRY_MIN) && (diff[2] <= SLOW_TRY_MAX) )
	{
		diffbit = 1;

		enc_base1[0] = enc_color1[0];
		enc_base1[1] = enc_color1[1];
		enc_base1[2] = enc_color1[2];
		enc_base2[0] = enc_color2[0];
		enc_base2[1] = enc_color2[1];
		enc_base2[2] = enc_color2[2];

		int err1[SLOW_SCAN_RANGE][SLOW_SCAN_RANGE][SLOW_SCAN_RANGE];
		int err2[SLOW_SCAN_RANGE][SLOW_SCAN_RANGE][SLOW_SCAN_RANGE];

		// upper part of block
		for(dr1 = SLOW_SCAN_MIN; dr1<SLOW_SCAN_MAX+1; dr1++)
		{
			for(dg1 = SLOW_SCAN_MIN; dg1<SLOW_SCAN_MAX+1; dg1++)
			{
				for(db1 = SLOW_SCAN_MIN; db1<SLOW_SCAN_MAX+1; db1++)
				{
					enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
					enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
					enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);

					avg_color_quant1[0] = enc_try1[0] << 3 | (enc_try1[0] >> 2);
					avg_color_quant1[1] = enc_try1[1] << 3 | (enc_try1[1] >> 2);
					avg_color_quant1[2] = enc_try1[2] << 3 | (enc_try1[2] >> 2);

					// upper part of block
					err1[dr1+SLOW_SCAN_OFFSET][dg1+SLOW_SCAN_OFFSET][db1+SLOW_SCAN_OFFSET] = tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
				}
			}
		}

		// lower part of block
		for(dr2 = SLOW_SCAN_MIN; dr2<SLOW_SCAN_MAX+1; dr2++)
		{
			for(dg2 = SLOW_SCAN_MIN; dg2<SLOW_SCAN_MAX+1; dg2++)
			{
				for(db2 = SLOW_SCAN_MIN; db2<SLOW_SCAN_MAX+1; db2++)
				{
					enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
					enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
					enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

					avg_color_quant2[0] = enc_try2[0] << 3 | (enc_try2[0] >> 2);
					avg_color_quant2[1] = enc_try2[1] << 3 | (enc_try2[1] >> 2);
					avg_color_quant2[2] = enc_try2[2] << 3 | (enc_try2[2] >> 2);

					// lower part of block
					err2[dr2+SLOW_SCAN_OFFSET][dg2+SLOW_SCAN_OFFSET][db2+SLOW_SCAN_OFFSET] = tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);
				}
			}
		}

		// Now see what combinations are both low in error and possible to
		// encode differentially.

		minerr = 255*255*3*8*2;

		for(dr1 = SLOW_SCAN_MIN; dr1<SLOW_SCAN_MAX+1; dr1++)
		{
			for(dg1 = SLOW_SCAN_MIN; dg1<SLOW_SCAN_MAX+1; dg1++)
			{
				for(db1 = SLOW_SCAN_MIN; db1<SLOW_SCAN_MAX+1; db1++)
				{
					for(dr2 = SLOW_SCAN_MIN; dr2<SLOW_SCAN_MAX+1; dr2++)
					{
						for(dg2 = SLOW_SCAN_MIN; dg2<SLOW_SCAN_MAX+1; dg2++)
						{
							for(db2 = SLOW_SCAN_MIN; db2<SLOW_SCAN_MAX+1; db2++)
							{								
								enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
								enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
								enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);
								enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
								enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
								enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

								// We must make sure that the difference between the tries still is less than allowed

								diff[0] = enc_try2[0]-enc_try1[0];	
								diff[1] = enc_try2[1]-enc_try1[1];	
								diff[2] = enc_try2[2]-enc_try1[2];

							    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
								{
									// The diff is OK, calculate total error:
									
									err = err1[dr1+SLOW_SCAN_OFFSET][dg1+SLOW_SCAN_OFFSET][db1+SLOW_SCAN_OFFSET] + err2[dr2+SLOW_SCAN_OFFSET][dg2+SLOW_SCAN_OFFSET][db2+SLOW_SCAN_OFFSET];

									if(err < minerr)
									{
										minerr = err;

										enc_color1[0] = enc_try1[0];
										enc_color1[1] = enc_try1[1];
										enc_color1[2] = enc_try1[2];
										enc_color2[0] = enc_try2[0];
										enc_color2[1] = enc_try2[1];
										enc_color2[2] = enc_try2[2];
									}
								}
							}
						}
					}
				}
			}
		}


		flip_err = minerr;

		best_err_flip_diff = flip_err;

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


		// Pack bits into the first word. 

		compressed1_flip_diff = 0;
		PUTBITSHIGH( compressed1_flip_diff, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[0], 5, 63);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[1], 5, 55);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[2], 5, 47);
 		PUTBITSHIGH( compressed1_flip_diff, diff[0],       3, 58);
 		PUTBITSHIGH( compressed1_flip_diff, diff[1],       3, 50);
 		PUTBITSHIGH( compressed1_flip_diff, diff[2],       3, 42);




		// upper part of block
		tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
		// lower part of block
		tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);


 		PUTBITSHIGH( compressed1_flip_diff, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_flip_diff, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_flip_diff, 1,             1, 32);


		best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
		best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);
		
		compressed2_flip_diff = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);
	
	}
	{
		diffbit = 0;
		// The difference is bigger than what fits in 555 plus delta-333, so we will have
		// to deal with 444 444.


		// Color for upper block

		int besterr = 255*255*3*8;
        int bestri = 0, bestgi = 0, bestbi = 0;
		int ri, gi, bi;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color1[0] = ri;
					enc_color1[1] = gi;
					enc_color1[2] = bi;

					avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
					avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
					avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];

					// upper part of block
					err = tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		flip_err = besterr;

		enc_color1[0] = bestri;
		enc_color1[1] = bestgi;
		enc_color1[2] = bestbi;
		avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
		avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
		avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];


		// Color for lower block

		besterr = 255*255*3*8;
        bestri = 0; bestgi = 0; bestbi = 0;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color2[0] = ri;
					enc_color2[1] = gi;
					enc_color2[2] = bi;

					avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
					avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
					avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

					// left part of block
					err = tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		flip_err += besterr;
		best_err_flip_444 = flip_err;

		enc_color2[0] = bestri;
		enc_color2[1] = bestgi;
		enc_color2[2] = bestbi;
		avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
		avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
		avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


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


		compressed1_flip_444 = 0;
		PUTBITSHIGH( compressed1_flip_444, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[0], 4, 63);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[1], 4, 55);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[2], 4, 47);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[0], 4, 59);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[1], 4, 51);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[2], 4, 43);

		// upper part of block
		tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB,best_pixel_indices1_LSB);

		// lower part of block
		tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_flip_444, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_flip_444, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_flip_444, 1,             1, 32);

		best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
		best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);
		
		compressed2_flip_444 = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);


	}

	// Now lets see which is the best table to use. Only 8 tables are possible. 

	int compressed1_norm;
	int compressed2_norm;
	int compressed1_flip;
	int compressed2_flip;


	// See which of the norm blocks is better

	if(best_err_norm_diff <= best_err_norm_444)
	{
		compressed1_norm = compressed1_norm_diff;
		compressed2_norm = compressed2_norm_diff;
		norm_err = best_err_norm_diff;
	}
	else
	{
		compressed1_norm = compressed1_norm_444;
		compressed2_norm = compressed2_norm_444;
		norm_err = best_err_norm_444;
	}

	// See which of the flip blocks is better

	if(best_err_flip_diff <= best_err_flip_444)
	{
		compressed1_flip = compressed1_flip_diff;
		compressed2_flip = compressed2_flip_diff;
		flip_err = best_err_flip_diff;
	}
	else
	{
		compressed1_flip = compressed1_flip_444;
		compressed2_flip = compressed2_flip_444;
		flip_err = best_err_flip_444;
	}

	// See if flip or norm is better

  //unsigned int best_of_all;

	if(norm_err <= flip_err)
	{

		compressed1 = compressed1_norm | 0;
		compressed2 = compressed2_norm;
    //best_of_all = norm_err;
	}
	else
	{

		compressed1 = compressed1_flip | 1;
		compressed2 = compressed2_flip;
    //best_of_all = flip_err;
	}

}
 
void compressBlockDiffFlipMediumPerceptual(uint8 *img,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{
	unsigned int compressed1_norm_diff, compressed2_norm_diff;
	unsigned int compressed1_norm_444, compressed2_norm_444;
	unsigned int compressed1_flip_diff, compressed2_flip_diff;
	unsigned int compressed1_flip_444, compressed2_flip_444;
	unsigned int best_err_norm_diff = 255*255*16*3;
	unsigned int best_err_norm_444 = 255*255*16*3;
	unsigned int best_err_flip_diff = 255*255*16*3;
	unsigned int best_err_flip_444 = 255*255*16*3;
	uint8 avg_color_quant1[3], avg_color_quant2[3];

	float avg_color_float1[3],avg_color_float2[3];
	int enc_color1[3], enc_color2[3], diff[3];
	int enc_base1[3], enc_base2[3];
	int enc_try1[3], enc_try2[3];
  //int min_error=255*255*16*3;
  int err;
	unsigned int best_pixel_indices1_MSB=0;
	unsigned int best_pixel_indices1_LSB=0;
	unsigned int best_pixel_indices2_MSB=0;
	unsigned int best_pixel_indices2_LSB=0;

	unsigned int best_table1=0, best_table2=0;
    int diffbit;

	int norm_err=0;
	int flip_err=0;
	int minerr;
	int dr1, dg1, db1, dr2, dg2, db2;

	// First try normal blocks 2x4:

	computeAverageColor2x4noQuantFloat(img,width,height,startx,starty,avg_color_float1);
	computeAverageColor2x4noQuantFloat(img,width,height,startx+2,starty,avg_color_float2);



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

    if( (diff[0] >= MEDIUM_TRY_MIN) && (diff[0] <= MEDIUM_TRY_MAX) && (diff[1] >= MEDIUM_TRY_MIN) && (diff[1] <= MEDIUM_TRY_MAX) && (diff[2] >= MEDIUM_TRY_MIN) && (diff[2] <= MEDIUM_TRY_MAX) )
	{
		diffbit = 1;

		enc_base1[0] = enc_color1[0];
		enc_base1[1] = enc_color1[1];
		enc_base1[2] = enc_color1[2];
		enc_base2[0] = enc_color2[0];
		enc_base2[1] = enc_color2[1];
		enc_base2[2] = enc_color2[2];

		int err1[MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE];
		int err2[MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE];

		// left part of block 
		for(dr1 = MEDIUM_SCAN_MIN; dr1<MEDIUM_SCAN_MAX+1; dr1++)
		{
			for(dg1 = MEDIUM_SCAN_MIN; dg1<MEDIUM_SCAN_MAX+1; dg1++)
			{
				for(db1 = MEDIUM_SCAN_MIN; db1<MEDIUM_SCAN_MAX+1; db1++)
				{
					enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
					enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
					enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);

					avg_color_quant1[0] = enc_try1[0] << 3 | (enc_try1[0] >> 2);
					avg_color_quant1[1] = enc_try1[1] << 3 | (enc_try1[1] >> 2);
					avg_color_quant1[2] = enc_try1[2] << 3 | (enc_try1[2] >> 2);

					// left part of block
					err1[dr1+MEDIUM_SCAN_OFFSET][dg1+MEDIUM_SCAN_OFFSET][db1+MEDIUM_SCAN_OFFSET] = tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
				}
			}
		}

		// right part of block
		for(dr2 = MEDIUM_SCAN_MIN; dr2<MEDIUM_SCAN_MAX+1; dr2++)
		{
			for(dg2 = MEDIUM_SCAN_MIN; dg2<MEDIUM_SCAN_MAX+1; dg2++)
			{
				for(db2 = MEDIUM_SCAN_MIN; db2<MEDIUM_SCAN_MAX+1; db2++)
				{
					enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
					enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
					enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

					avg_color_quant2[0] = enc_try2[0] << 3 | (enc_try2[0] >> 2);
					avg_color_quant2[1] = enc_try2[1] << 3 | (enc_try2[1] >> 2);
					avg_color_quant2[2] = enc_try2[2] << 3 | (enc_try2[2] >> 2);

					// left part of block
					err2[dr2+MEDIUM_SCAN_OFFSET][dg2+MEDIUM_SCAN_OFFSET][db2+MEDIUM_SCAN_OFFSET] = tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);
				}
			}
		}

		// Now see what combinations are both low in error and possible to
		// encode differentially.

		minerr = 255*255*3*8*2;

		for(dr1 = MEDIUM_SCAN_MIN; dr1<MEDIUM_SCAN_MAX+1; dr1++)
		{
			for(dg1 = MEDIUM_SCAN_MIN; dg1<MEDIUM_SCAN_MAX+1; dg1++)
			{
				for(db1 = MEDIUM_SCAN_MIN; db1<MEDIUM_SCAN_MAX+1; db1++)
				{
					for(dr2 = MEDIUM_SCAN_MIN; dr2<MEDIUM_SCAN_MAX+1; dr2++)
					{
						for(dg2 = MEDIUM_SCAN_MIN; dg2<MEDIUM_SCAN_MAX+1; dg2++)
						{
							for(db2 = MEDIUM_SCAN_MIN; db2<MEDIUM_SCAN_MAX+1; db2++)
							{								
								enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
								enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
								enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);
								enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
								enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
								enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

								// We must make sure that the difference between the tries still is less than allowed

								diff[0] = enc_try2[0]-enc_try1[0];	
								diff[1] = enc_try2[1]-enc_try1[1];	
								diff[2] = enc_try2[2]-enc_try1[2];

							    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
								{
									// The diff is OK, calculate total error:
									
									err = err1[dr1+MEDIUM_SCAN_OFFSET][dg1+MEDIUM_SCAN_OFFSET][db1+MEDIUM_SCAN_OFFSET] + err2[dr2+MEDIUM_SCAN_OFFSET][dg2+MEDIUM_SCAN_OFFSET][db2+MEDIUM_SCAN_OFFSET];

									if(err < minerr)
									{
										minerr = err;

										enc_color1[0] = enc_try1[0];
										enc_color1[1] = enc_try1[1];
										enc_color1[2] = enc_try1[2];
										enc_color2[0] = enc_try2[0];
										enc_color2[1] = enc_try2[1];
										enc_color2[2] = enc_try2[2];
									}
								}
							}
						}
					}
				}
			}
		}

		best_err_norm_diff = minerr;
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


		compressed1_norm_diff = 0;
		PUTBITSHIGH( compressed1_norm_diff, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[0], 5, 63);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[1], 5, 55);
 		PUTBITSHIGH( compressed1_norm_diff, enc_color1[2], 5, 47);
 		PUTBITSHIGH( compressed1_norm_diff, diff[0],       3, 58);
 		PUTBITSHIGH( compressed1_norm_diff, diff[1],       3, 50);
 		PUTBITSHIGH( compressed1_norm_diff, diff[2],       3, 42);

		
		// left part of block
		tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_norm_diff, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_norm_diff, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_norm_diff, 0,             1, 32);

		compressed2_norm_diff = 0;
		PUTBITS( compressed2_norm_diff, (best_pixel_indices1_MSB     ), 8, 23);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices2_MSB     ), 8, 31);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices1_LSB     ), 8, 7);
		PUTBITS( compressed2_norm_diff, (best_pixel_indices2_LSB     ), 8, 15);


	}
	else
	{
		diffbit = 0;
		// The difference is bigger than what fits in 555 plus delta-333, so we will have
		// to deal with 444 444.


		// Color for left block

		int besterr = 255*255*3*8;
        int bestri = 0, bestgi = 0, bestbi = 0;
		int ri, gi, bi;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color1[0] = ri;
					enc_color1[1] = gi;
					enc_color1[2] = bi;

					avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
					avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
					avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];

					// left part of block
					err = tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB,best_pixel_indices1_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		norm_err = besterr;
		
		enc_color1[0] = bestri;
		enc_color1[1] = bestgi;
		enc_color1[2] = bestbi;
		avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
		avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
		avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];


		// Color for right block

		besterr = 255*255*3*8;
        bestri = 0; bestgi = 0; bestbi = 0;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color2[0] = ri;
					enc_color2[1] = gi;
					enc_color2[2] = bi;

					avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
					avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
					avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

					// left part of block
					err = tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB,best_pixel_indices2_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}


		norm_err += besterr;
		best_err_norm_444 = norm_err;

		enc_color2[0] = bestri;
		enc_color2[1] = bestgi;
		enc_color2[2] = bestbi;
		avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
		avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
		avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


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


		compressed1_norm_444 = 0;
		PUTBITSHIGH( compressed1_norm_444, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[0], 4, 63);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[1], 4, 55);
 		PUTBITSHIGH( compressed1_norm_444, enc_color1[2], 4, 47);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[0], 4, 59);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[1], 4, 51);
 		PUTBITSHIGH( compressed1_norm_444, enc_color2[2], 4, 43);


		// left part of block
		tryalltables_3bittable2x4percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		tryalltables_3bittable2x4percep(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_norm_444, best_table1, 3, 39);
 		PUTBITSHIGH( compressed1_norm_444, best_table2, 3, 36);
 		PUTBITSHIGH( compressed1_norm_444, 0,           1, 32);

		compressed2_norm_444 = 0;
		PUTBITS( compressed2_norm_444, (best_pixel_indices1_MSB     ), 8, 23);
		PUTBITS( compressed2_norm_444, (best_pixel_indices2_MSB     ), 8, 31);
		PUTBITS( compressed2_norm_444, (best_pixel_indices1_LSB     ), 8, 7);
		PUTBITS( compressed2_norm_444, (best_pixel_indices2_LSB     ), 8, 15);

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

    if( (diff[0] >= MEDIUM_TRY_MIN) && (diff[0] <= MEDIUM_TRY_MAX) && (diff[1] >= MEDIUM_TRY_MIN) && (diff[1] <= MEDIUM_TRY_MAX) && (diff[2] >= MEDIUM_TRY_MIN) && (diff[2] <= MEDIUM_TRY_MAX) )
	{
		diffbit = 1;

		enc_base1[0] = enc_color1[0];
		enc_base1[1] = enc_color1[1];
		enc_base1[2] = enc_color1[2];
		enc_base2[0] = enc_color2[0];
		enc_base2[1] = enc_color2[1];
		enc_base2[2] = enc_color2[2];

		int err1[MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE];
		int err2[MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE][MEDIUM_SCAN_RANGE];

		// upper part of block
		for(dr1 = MEDIUM_SCAN_MIN; dr1<MEDIUM_SCAN_MAX+1; dr1++)
		{
			for(dg1 = MEDIUM_SCAN_MIN; dg1<MEDIUM_SCAN_MAX+1; dg1++)
			{
				for(db1 = MEDIUM_SCAN_MIN; db1<MEDIUM_SCAN_MAX+1; db1++)
				{
					enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
					enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
					enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);

					avg_color_quant1[0] = enc_try1[0] << 3 | (enc_try1[0] >> 2);
					avg_color_quant1[1] = enc_try1[1] << 3 | (enc_try1[1] >> 2);
					avg_color_quant1[2] = enc_try1[2] << 3 | (enc_try1[2] >> 2);

					// upper part of block
					err1[dr1+MEDIUM_SCAN_OFFSET][dg1+MEDIUM_SCAN_OFFSET][db1+MEDIUM_SCAN_OFFSET] = tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
				}
			}
		}

		// lower part of block
		for(dr2 = MEDIUM_SCAN_MIN; dr2<MEDIUM_SCAN_MAX+1; dr2++)
		{
			for(dg2 = MEDIUM_SCAN_MIN; dg2<MEDIUM_SCAN_MAX+1; dg2++)
			{
				for(db2 = MEDIUM_SCAN_MIN; db2<MEDIUM_SCAN_MAX+1; db2++)
				{
					enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
					enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
					enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

					avg_color_quant2[0] = enc_try2[0] << 3 | (enc_try2[0] >> 2);
					avg_color_quant2[1] = enc_try2[1] << 3 | (enc_try2[1] >> 2);
					avg_color_quant2[2] = enc_try2[2] << 3 | (enc_try2[2] >> 2);

					// lower part of block
					err2[dr2+MEDIUM_SCAN_OFFSET][dg2+MEDIUM_SCAN_OFFSET][db2+MEDIUM_SCAN_OFFSET] = tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);
				}
			}
		}

		// Now see what combinations are both low in error and possible to
		// encode differentially.

		minerr = 255*255*3*8*2;

		for(dr1 = MEDIUM_SCAN_MIN; dr1<MEDIUM_SCAN_MAX+1; dr1++)
		{
			for(dg1 = MEDIUM_SCAN_MIN; dg1<MEDIUM_SCAN_MAX+1; dg1++)
			{
				for(db1 = MEDIUM_SCAN_MIN; db1<MEDIUM_SCAN_MAX+1; db1++)
				{
					for(dr2 = MEDIUM_SCAN_MIN; dr2<MEDIUM_SCAN_MAX+1; dr2++)
					{
						for(dg2 = MEDIUM_SCAN_MIN; dg2<MEDIUM_SCAN_MAX+1; dg2++)
						{
							for(db2 = MEDIUM_SCAN_MIN; db2<MEDIUM_SCAN_MAX+1; db2++)
							{								
								enc_try1[0] = CLAMP(0,enc_base1[0]+dr1,31);
								enc_try1[1] = CLAMP(0,enc_base1[1]+dg1,31);
								enc_try1[2] = CLAMP(0,enc_base1[2]+db1,31);
								enc_try2[0] = CLAMP(0,enc_base2[0]+dr2,31);
								enc_try2[1] = CLAMP(0,enc_base2[1]+dg2,31);
								enc_try2[2] = CLAMP(0,enc_base2[2]+db2,31);

								// We must make sure that the difference between the tries still is less than allowed

								diff[0] = enc_try2[0]-enc_try1[0];	
								diff[1] = enc_try2[1]-enc_try1[1];	
								diff[2] = enc_try2[2]-enc_try1[2];

							    if( (diff[0] >= -4) && (diff[0] <= 3) && (diff[1] >= -4) && (diff[1] <= 3) && (diff[2] >= -4) && (diff[2] <= 3) )
								{
									// The diff is OK, calculate total error:
									
									err = err1[dr1+MEDIUM_SCAN_OFFSET][dg1+MEDIUM_SCAN_OFFSET][db1+MEDIUM_SCAN_OFFSET] + err2[dr2+MEDIUM_SCAN_OFFSET][dg2+MEDIUM_SCAN_OFFSET][db2+MEDIUM_SCAN_OFFSET];

									if(err < minerr)
									{
										minerr = err;

										enc_color1[0] = enc_try1[0];
										enc_color1[1] = enc_try1[1];
										enc_color1[2] = enc_try1[2];
										enc_color2[0] = enc_try2[0];
										enc_color2[1] = enc_try2[1];
										enc_color2[2] = enc_try2[2];
									}
								}
							}
						}
					}
				}
			}
		}


		flip_err = minerr;

		best_err_flip_diff = flip_err;

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


		// Pack bits into the first word. 

		compressed1_flip_diff = 0;
		PUTBITSHIGH( compressed1_flip_diff, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[0], 5, 63);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[1], 5, 55);
 		PUTBITSHIGH( compressed1_flip_diff, enc_color1[2], 5, 47);
 		PUTBITSHIGH( compressed1_flip_diff, diff[0],       3, 58);
 		PUTBITSHIGH( compressed1_flip_diff, diff[1],       3, 50);
 		PUTBITSHIGH( compressed1_flip_diff, diff[2],       3, 42);




		// upper part of block
		tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
		// lower part of block
		tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);


 		PUTBITSHIGH( compressed1_flip_diff, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_flip_diff, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_flip_diff, 1,             1, 32);


		best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
		best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);
		
		compressed2_flip_diff = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);
	
	}
	else
	{
		diffbit = 0;
		// The difference is bigger than what fits in 555 plus delta-333, so we will have
		// to deal with 444 444.


		// Color for upper block

		int besterr = 255*255*3*8;
        int bestri = 0, bestgi = 0, bestbi = 0;
		int ri, gi, bi;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color1[0] = ri;
					enc_color1[1] = gi;
					enc_color1[2] = bi;

					avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
					avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
					avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];

					// upper part of block
					err = tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		flip_err = besterr;

		enc_color1[0] = bestri;
		enc_color1[1] = bestgi;
		enc_color1[2] = bestbi;
		avg_color_quant1[0] = enc_color1[0] << 4 | enc_color1[0]; 
		avg_color_quant1[1] = enc_color1[1] << 4 | enc_color1[1]; 
		avg_color_quant1[2] = enc_color1[2] << 4 | enc_color1[2];


		// Color for lower block

		besterr = 255*255*3*8;
        bestri = 0; bestgi = 0; bestbi = 0;

		for(ri = 0; ri<15; ri++)
		{
			for(gi = 0; gi<15; gi++)
			{
				for(bi = 0; bi<15; bi++)
				{
					enc_color2[0] = ri;
					enc_color2[1] = gi;
					enc_color2[2] = bi;

					avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
					avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
					avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];

					// left part of block
					err = tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

					if(err<besterr)
					{
						bestri = ri; bestgi = gi; bestbi = bi;
						besterr = err;
					}


				}
			}
		}

		flip_err += besterr;
		best_err_flip_444 = flip_err;

		enc_color2[0] = bestri;
		enc_color2[1] = bestgi;
		enc_color2[2] = bestbi;
		avg_color_quant2[0] = enc_color2[0] << 4 | enc_color2[0]; 
		avg_color_quant2[1] = enc_color2[1] << 4 | enc_color2[1]; 
		avg_color_quant2[2] = enc_color2[2] << 4 | enc_color2[2];


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


		compressed1_flip_444 = 0;
		PUTBITSHIGH( compressed1_flip_444, diffbit,       1, 33);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[0], 4, 63);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[1], 4, 55);
 		PUTBITSHIGH( compressed1_flip_444, enc_color1[2], 4, 47);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[0], 4, 59);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[1], 4, 51);
 		PUTBITSHIGH( compressed1_flip_444, enc_color2[2], 4, 43);

		// upper part of block
		tryalltables_3bittable4x2percep(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB,best_pixel_indices1_LSB);

		// lower part of block
		tryalltables_3bittable4x2percep(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

 		PUTBITSHIGH( compressed1_flip_444, best_table1,   3, 39);
 		PUTBITSHIGH( compressed1_flip_444, best_table2,   3, 36);
 		PUTBITSHIGH( compressed1_flip_444, 1,             1, 32);

		best_pixel_indices1_MSB |= (best_pixel_indices2_MSB << 2);
		best_pixel_indices1_LSB |= (best_pixel_indices2_LSB << 2);
		
		compressed2_flip_444 = ((best_pixel_indices1_MSB & 0xffff) << 16) | (best_pixel_indices1_LSB & 0xffff);


	}

	// Now lets see which is the best table to use. Only 8 tables are possible. 

	int compressed1_norm;
	int compressed2_norm;
	int compressed1_flip;
	int compressed2_flip;


	// See which of the norm blocks is better

	if(best_err_norm_diff <= best_err_norm_444)
	{
		compressed1_norm = compressed1_norm_diff;
		compressed2_norm = compressed2_norm_diff;
		norm_err = best_err_norm_diff;
	}
	else
	{
		compressed1_norm = compressed1_norm_444;
		compressed2_norm = compressed2_norm_444;
		norm_err = best_err_norm_444;
	}

	// See which of the flip blocks is better

	if(best_err_flip_diff <= best_err_flip_444)
	{
		compressed1_flip = compressed1_flip_diff;
		compressed2_flip = compressed2_flip_diff;
		flip_err = best_err_flip_diff;
	}
	else
	{
		compressed1_flip = compressed1_flip_444;
		compressed2_flip = compressed2_flip_444;
		flip_err = best_err_flip_444;
	}

	// See if flip or norm is better

  //unsigned int best_of_all;

	if(norm_err <= flip_err)
	{

		compressed1 = compressed1_norm | 0;
		compressed2 = compressed2_norm;
    //best_of_all = norm_err;
	}
	else
	{

		compressed1 = compressed1_flip | 1;
		compressed2 = compressed2_flip;
    //best_of_all = flip_err;
	}

}

void compressBlockDiffFlipAverage(uint8 *img,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{
	unsigned int compressed1_norm, compressed2_norm;
	unsigned int compressed1_flip, compressed2_flip;
	uint8 avg_color_quant1[3], avg_color_quant2[3];

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
		norm_err = tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		norm_err += tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

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
		norm_err = tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		norm_err += tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

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
		flip_err = tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
		// lower part of block
		flip_err += tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

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
		flip_err = tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
		// lower part of block
		flip_err += tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

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

void compressBlockDiffFlipCombined(uint8 *img,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{
	unsigned int compressed1_norm, compressed2_norm;
	unsigned int compressed1_flip, compressed2_flip;
	uint8 avg_color_quant1[3], avg_color_quant2[3];

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

	uint8 dummy[3];

	quantize555ColorCombined(avg_color_float1, enc_color1, dummy);
	quantize555ColorCombined(avg_color_float2, enc_color2, dummy);

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
		norm_err = tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		norm_err += tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

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

		uint8 dummy[3];
		quantize444ColorCombined(avg_color_float1, enc_color1, dummy);
		quantize444ColorCombined(avg_color_float2, enc_color2, dummy);

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
		norm_err = tryalltables_3bittable2x4(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);

		// right part of block
		norm_err += tryalltables_3bittable2x4(img,width,height,startx+2,starty,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

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

	quantize555ColorCombined(avg_color_float1, enc_color1, dummy);
	quantize555ColorCombined(avg_color_float2, enc_color2, dummy);

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
		flip_err = tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
		// lower part of block
		flip_err += tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

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

		uint8 dummy[3];
		quantize444ColorCombined(avg_color_float1, enc_color1, dummy);
		quantize444ColorCombined(avg_color_float2, enc_color2, dummy);

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
		flip_err = tryalltables_3bittable4x2(img,width,height,startx,starty,avg_color_quant1,best_table1,best_pixel_indices1_MSB, best_pixel_indices1_LSB);
		// lower part of block
		flip_err += tryalltables_3bittable4x2(img,width,height,startx,starty+2,avg_color_quant2,best_table2,best_pixel_indices2_MSB, best_pixel_indices2_LSB);

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

void compressBlockDiffFlipAveragePerceptual(uint8 *img,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{

	unsigned int compressed1_norm, compressed2_norm;
	unsigned int compressed1_flip, compressed2_flip;
	uint8 avg_color_quant1[3], avg_color_quant2[3];

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

void compressBlockDiffFlipCombinedPerceptual(uint8 *img,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{

	unsigned int compressed1_norm, compressed2_norm;
	unsigned int compressed1_flip, compressed2_flip;
	uint8 avg_color_quant1[3], avg_color_quant2[3];

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

	uint8 dummy[3];

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

double calcBlockErrorRGB(uint8 *img, uint8 *imgdec, int width, int height, int startx, int starty)
{
  (void)height;
	int xx,yy;
	double err;

	err = 0;

	for(xx = startx; xx< startx+4; xx++)
	{
		for(yy = starty; yy<starty+4; yy++)
		{
 			err += SQUARE(1.0*RED(img,width,xx,yy)  - 1.0*RED(imgdec, width, xx,yy));
 			err += SQUARE(1.0*GREEN(img,width,xx,yy)- 1.0*GREEN(imgdec, width, xx,yy));
 			err += SQUARE(1.0*BLUE(img,width,xx,yy) - 1.0*BLUE(imgdec, width, xx,yy));
		}
	}

	return err;
}

double calcBlockPerceptualErrorRGB(uint8 *img, uint8 *imgdec, int width, int height, int startx, int starty)
{
  (void)height;
	int xx,yy;
	double err;

	err = 0;

	for(xx = startx; xx< startx+4; xx++)
	{
		for(yy = starty; yy<starty+4; yy++)
		{
 			err += PERCEPTUAL_WEIGHT_R_SQUARED*SQUARE(1.0*RED(img,width,xx,yy)  - 1.0*RED(imgdec, width, xx,yy));
 			err += PERCEPTUAL_WEIGHT_G_SQUARED*SQUARE(1.0*GREEN(img,width,xx,yy)- 1.0*GREEN(imgdec, width, xx,yy));
 			err += PERCEPTUAL_WEIGHT_B_SQUARED*SQUARE(1.0*BLUE(img,width,xx,yy) - 1.0*BLUE(imgdec, width, xx,yy));
		}
	}

	return err;
}

void compressBlockDiffFlipFast(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{
	unsigned int average_block1;
	unsigned int average_block2;
	double error_average;

	unsigned int combined_block1;
	unsigned int combined_block2;
	double error_combined;

	// First quantize the average color to the nearest neighbor.
	compressBlockDiffFlipAverage(img, width, height, startx, starty, average_block1, average_block2);
	decompressBlockDiffFlip(average_block1, average_block2, imgdec, width, height, startx, starty);
	error_average = calcBlockErrorRGB(img, imgdec, width, height, startx, starty);

	compressBlockDiffFlipCombined(img, width, height, startx, starty, combined_block1, combined_block2);
	decompressBlockDiffFlip(combined_block1, combined_block2, imgdec, width, height, startx, starty);
	error_combined = calcBlockErrorRGB(img, imgdec, width, height, startx, starty);

	if(error_combined < error_average)
	{
		compressed1 = combined_block1;
		compressed2 = combined_block2;
	}
	else
	{
		compressed1 = average_block1;
		compressed2 = average_block2;
	}
}

void compressBlockDiffFlipFastPerceptual(uint8 *img, uint8 *imgdec,int width,int height,int startx,int starty, unsigned int &compressed1, unsigned int &compressed2)
{
	unsigned int average_block1;
	unsigned int average_block2;
	double error_average;

	unsigned int combined_block1;
	unsigned int combined_block2;
	double error_combined;

	// First quantize the average color to the nearest neighbor.
	compressBlockDiffFlipAveragePerceptual(img, width, height, startx, starty, average_block1, average_block2);
	decompressBlockDiffFlip(average_block1, average_block2, imgdec, width, height, startx, starty);
	error_average = calcBlockPerceptualErrorRGB(img, imgdec, width, height, startx, starty);

	compressBlockDiffFlipCombinedPerceptual(img, width, height, startx, starty, combined_block1, combined_block2);
	decompressBlockDiffFlip(combined_block1, combined_block2, imgdec, width, height, startx, starty);
	error_combined = calcBlockPerceptualErrorRGB(img, imgdec, width, height, startx, starty);

	if(error_combined < error_average)
	{
		compressed1 = combined_block1;
		compressed2 = combined_block2;
	}
	else
	{
		compressed1 = average_block1;
		compressed2 = average_block2;
	}
}

void write_big_endian_2byte_word(unsigned short *blockadr, FILE *f)
{
	uint8 bytes[2];
	unsigned short block;

	block = blockadr[0];

	bytes[0] = (block >> 8) & 0xff;
	bytes[1] = (block >> 0) & 0xff;

	fwrite(&bytes[0],1,1,f);
	fwrite(&bytes[1],1,1,f);
}

void write_big_endian_4byte_word(unsigned int *blockadr, FILE *f)
{
	uint8 bytes[4];
	unsigned int block;

	block = blockadr[0];

	bytes[0] = (block >> 24) & 0xff;
	bytes[1] = (block >> 16) & 0xff;
	bytes[2] = (block >> 8) & 0xff;
	bytes[3] = (block >> 0) & 0xff;

	fwrite(&bytes[0],1,1,f);
	fwrite(&bytes[1],1,1,f);
	fwrite(&bytes[2],1,1,f);
	fwrite(&bytes[3],1,1,f);
}

#define ETC1_RGB_NO_MIPMAPS 0
#define ETC1_RGBA_NO_MIPMAPS 1
#define ETC1_RGB_MIPMAPS 2
#define ETC1_RGBA_MIPMAPS 3

void compressImageFile(uint8 *img,int width,int height,char *dstfile, int expandedwidth, int expandedheight, int action)
{
	FILE *f;
	int x,y,w,h;
	unsigned int block1, block2;
	unsigned short wi, hi;
	unsigned char magic[4];
	unsigned char version[2];
	unsigned short texture_type;
	uint8 *imgdec;

	imgdec = (unsigned char*) malloc(expandedwidth*expandedheight*3);
	if(!imgdec)
	{
		printf("Could not allocate decompression buffer --- exiting\n");
		exit(1);
	}

	magic[0]   = 'P'; magic[1]   = 'K'; magic[2] = 'M'; magic[3] = ' '; 
	version[0] = '1'; version[1] = '0';
	texture_type = ETC1_RGB_NO_MIPMAPS;

  if( (f=fopen(dstfile,"wb")) )
	{
		w=expandedwidth/4;  w*=4;
		h=expandedheight/4; h*=4;
		wi = w;
		hi = h;

		if(ktx_mode)
		{
			printf("Outputting to .kxt file...\n");
			//.ktx file: KTX header followed by compressed binary data.
			KTX_header header;
			//identifier
			for(int i=0; i<12; i++) 
			{
				header.identifier[i]=ktx_identifier[i];
			}
			//endianess int.. if this comes out reversed, all of the other ints will too.
			header.endianness=KTX_ENDIAN_REF;
			
			//these values are always 0/1 for compressed textures.
			header.glType=0;
			header.glTypeSize=1;
			header.glFormat=0;

			header.pixelWidth=width;
			header.pixelHeight=height;
			header.pixelDepth=0;

			//we only support single non-mipmapped non-cubemap textures..
			header.numberOfArrayElements=0;
			header.numberOfFaces=1;
			header.numberOfMipmapLevels=1;

			//and no metadata..
			header.bytesOfKeyValueData=0;
			
			int halfbytes=1;
			//header.glInternalFormat=?
			//header.glBaseInternalFormat=?
			if(texture_type==ETC1_RGB_NO_MIPMAPS) 
			{
				header.glBaseInternalFormat=GL_RGB;
				header.glInternalFormat=GL_ETC1_RGB8_OES;
			}
			else 
			{
				printf("internal error: bad format!\n");
				exit(1);
			}
			//write header
			fwrite(&header,sizeof(KTX_header),1,f);
			
			//write size of compressed data.. which depend on the expanded size..
			unsigned int imagesize=(w*h*halfbytes)/2;
			fwrite(&imagesize,sizeof(int),1,f);
		}
		else
		{
			printf("outputting to .pkm file...\n");
			// Write magic number
			fwrite(&magic[0], sizeof(unsigned char), 1, f);
			fwrite(&magic[1], sizeof(unsigned char), 1, f);
			fwrite(&magic[2], sizeof(unsigned char), 1, f);
			fwrite(&magic[3], sizeof(unsigned char), 1, f);
	
			// Write version
			fwrite(&version[0], sizeof(unsigned char), 1, f);
			fwrite(&version[1], sizeof(unsigned char), 1, f);

			// Write texture type
			write_big_endian_2byte_word(&texture_type, f);

			// Write binary header: the width and height as unsigned 16-bit words
			write_big_endian_2byte_word(&wi, f);
			write_big_endian_2byte_word(&hi, f);

			// Also write the active pixels. For instance, if we want to compress
			// a 128 x 129 image, we have to extend it to 128 x 132 pixels.
			// Then the wi and hi written above will be 128 and 132, but the
			// additional information that we write below will be 128 and 129,
			// to indicate that it is only the top 129 lines of data in the 
			// decompressed image that will be valid data, and the rest will
			// be just garbage. 

			unsigned short activew, activeh;
			activew = width;
			activeh = height;

			write_big_endian_2byte_word(&activew, f);
			write_big_endian_2byte_word(&activeh, f);
		}

		int totblocks = expandedheight/4 * expandedwidth/4;
		int countblocks = 0;

		/// xxx
		for(y=0;y<expandedheight/4;y++)
		{
			for(x=0;x<expandedwidth/4;x++)
			{
				countblocks++;

				switch(action)
				{
				case 0:
					// FAST only tests the two most likely base colors.
					compressBlockDiffFlipFast(img, imgdec, expandedwidth, expandedheight, 4*x, 4*y, block1, block2);
					break;
 				case 1:
					// The MEDIUM version tests all colors in a 3x3x3 cube around the average colors
					// This increases the likelihood that the differential mode is selected.
					compressBlockDiffFlipMedium(img,expandedwidth,expandedheight,4*x,4*y, block1, block2);		
					printf("Compressed %d of %d blocks, %.1f%% finished.\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", countblocks, totblocks, 100.0*countblocks/(1.0*totblocks));
 					break;
 				case 2:
					// The SLOW version tests all colors in a a 5x5x5 cube around the average colors
					// It also tries the nondifferential mode for each block even if the differential succeeds.
					compressBlockDiffFlipSlow(img,expandedwidth,expandedheight,4*x,4*y, block1, block2);		
					printf("Compressed %d of %d blocks, %.1f%% finished.\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", countblocks, totblocks, 100.0*countblocks/(1.0*totblocks));
					break;
 				case 3:
					// FAST with PERCEPTUAL error metric
					compressBlockDiffFlipFastPerceptual(img, imgdec, expandedwidth, expandedheight, 4*x, 4*y, block1, block2);
 					break;
 				case 4:
					// MEDIUM with PERCEPTUAL error metric
					compressBlockDiffFlipMediumPerceptual(img,expandedwidth,expandedheight,4*x,4*y, block1, block2);
					printf("Compressed %d of %d blocks, %.1f%% finished.\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", countblocks, totblocks, 100.0*countblocks/(1.0*totblocks));
 					break;
				case 5:
					// SLOW with PERCEPTUAL error metric
					compressBlockDiffFlipSlowPerceptual(img,expandedwidth,expandedheight,4*x,4*y, block1, block2);		
					printf("Compressed %d of %d blocks, %.1f%% finished.\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b", countblocks, totblocks, 100.0*countblocks/(1.0*totblocks));
					break;
				default:
				    printf("Not implemented.\n");
					exit(1);
				    break;
				}
				write_big_endian_4byte_word(&block1, f);
				write_big_endian_4byte_word(&block2, f);

			}
		}
  printf("\n");

  fclose(f);
  free(imgdec);
  printf("Saved file <%s>.\n",dstfile);
	}
}

double calculatePSNR(uint8 *lossyimg, uint8 *origimg, int width, int height)
{
	// calculate Mean Square Error (MSE)

	int x,y;
	double MSE;
	double PSNR;
	double err;
	MSE = 0;

	// Note: This calculation of PSNR uses the formula
	//
	// PSNR = 10 * log_10 ( 255^2 / MSE ) 
	// 
	// where the MSE is calculated as
	//
	// 1/(N*M) * sum ( 1/3 * ((R' - R)^2 + (G' - G)^2 + (B' - B)^2) ) 
	//
	// The reason for having the 1/3 factor is the following:
	// Presume we have a grayscale image, that is acutally just the red component 
	// of a color image.. The squared error is then (R' - R)^2.
	// Assume that we have a certain signal to noise ratio, say 30 dB. If we add
	// another two components (say green and blue) with the same signal to noise 
	// ratio, we want the total signal to noise ratio be the same. For the
	// squared error to remain constant we must divide by three after adding
	// together the squared errors of the components. 

	for(y=0;y<height;y++)
	{
		for(x=0;x<width;x++)
		{
			err = lossyimg[y*width*3+x*3+0] - origimg[y*width*3+x*3+0];
		    MSE = MSE + ((err * err)/3.0);
			err = lossyimg[y*width*3+x*3+1] - origimg[y*width*3+x*3+1];
		    MSE = MSE + ((err * err)/3.0);
			err = lossyimg[y*width*3+x*3+2] - origimg[y*width*3+x*3+2];
		    MSE = MSE + ((err * err)/3.0);
		}
	}
	MSE = MSE / (width * height);
	if(MSE == 0)
	{
		printf("********************************************************************\n");
		printf("There is no difference at all between image files --- infinite PSNR.\n");
		printf("********************************************************************\n");
	}
	printf("RMSE = %f \n",sqrt(MSE * 3.0));
	PSNR = (float)(10*log((double)((255*255)/MSE))/log((double)10));
	return PSNR;
}

double calculatePerceptuallyWeightedPSNR(uint8 *lossyimg, uint8 *origimg, int width, int height)
{
	// calculate Perceptually Weighted Mean Square Error (wMSE)

	int x,y;
	double wMSE;
	double PSNR;
	double err;
	wMSE = 0;

	// Note: This calculation of PSNR uses the formula
	//
	// PSNR = 10 * log_10 ( 255^2 / wMSE ) 
	// 
	// where the wMSE is calculated as
	//
	// 1/(N*M) * sum ( ( w1*(R' - R)^2 + w2*(G' - G)^2 + w3*(B' - B)^2) ) 
	//
	// The reason for having the 1/3 factor is the following:
	// Presume we have a grayscale image, that is acutally just the red component 
	// of a color image.. The squared error is then (R' - R)^2.
	// Assume that we have a certain signal to noise ratio, say 30 dB. If we add
	// another two components (say green and blue) with the same signal to noise 
	// ratio, we want the total signal to noise ratio be the same. For the
	// squared error to remain constant we must divide by three after adding
	// together the squared errors of the components. 

	double w1 = 0.299, w2 = 0.587, w3 = 0.114;

	for(y=0;y<height;y++)
	{
		for(x=0;x<width;x++)
		{
			err = lossyimg[y*width*3+x*3+0] - origimg[y*width*3+x*3+0];
		    wMSE = wMSE + (w1*(err * err));
			err = lossyimg[y*width*3+x*3+1] - origimg[y*width*3+x*3+1];
		    wMSE = wMSE + (w2*(err * err));
			err = lossyimg[y*width*3+x*3+2] - origimg[y*width*3+x*3+2];
		    wMSE = wMSE + (w3*(err * err));
		}
	}
	wMSE = wMSE / (width * height);
	PSNR = (float)(10*log((double)((255*255)/wMSE))/log((double)10));
	return PSNR;
}

void readCompressParamsEnc(void)
{
	compressParamsEnc[0][0]  =  -8; compressParamsEnc[0][1]  =  -2; compressParamsEnc[0][2]  =  2; compressParamsEnc[0][3]  =   8;
	compressParamsEnc[1][0]  =  -8; compressParamsEnc[1][1]  =  -2; compressParamsEnc[1][2]  =  2; compressParamsEnc[1][3]  =   8;
	compressParamsEnc[2][0]  = -17; compressParamsEnc[2][1]  =  -5; compressParamsEnc[2][2]  =  5; compressParamsEnc[2][3]  =  17;
	compressParamsEnc[3][0]  = -17; compressParamsEnc[3][1]  =  -5; compressParamsEnc[3][2]  =  5; compressParamsEnc[3][3]  =  17;
	compressParamsEnc[4][0]  = -29; compressParamsEnc[4][1]  =  -9; compressParamsEnc[4][2]  =  9; compressParamsEnc[4][3]  =  29;
	compressParamsEnc[5][0]  = -29; compressParamsEnc[5][1]  =  -9; compressParamsEnc[5][2]  =  9; compressParamsEnc[5][3]  =  29;
	compressParamsEnc[6][0]  = -42; compressParamsEnc[6][1]  = -13; compressParamsEnc[6][2]  = 13; compressParamsEnc[6][3]  =  42;
	compressParamsEnc[7][0]  = -42; compressParamsEnc[7][1]  = -13; compressParamsEnc[7][2]  = 13; compressParamsEnc[7][3]  =  42;
 	compressParamsEnc[8][0]  = -60; compressParamsEnc[8][1]  = -18; compressParamsEnc[8][2]  = 18; compressParamsEnc[8][3]  =  60;
 	compressParamsEnc[9][0]  = -60; compressParamsEnc[9][1]  = -18; compressParamsEnc[9][2]  = 18; compressParamsEnc[9][3]  =  60;
	compressParamsEnc[10][0] = -80; compressParamsEnc[10][1] = -24; compressParamsEnc[10][2] = 24; compressParamsEnc[10][3] =  80;
	compressParamsEnc[11][0] = -80; compressParamsEnc[11][1] = -24; compressParamsEnc[11][2] = 24; compressParamsEnc[11][3] =  80;
	compressParamsEnc[12][0] =-106; compressParamsEnc[12][1] = -33; compressParamsEnc[12][2] = 33; compressParamsEnc[12][3] = 106;
	compressParamsEnc[13][0] =-106; compressParamsEnc[13][1] = -33; compressParamsEnc[13][2] = 33; compressParamsEnc[13][3] = 106;
	compressParamsEnc[14][0] =-183; compressParamsEnc[14][1] = -47; compressParamsEnc[14][2] = 47; compressParamsEnc[14][3] = 183;
	compressParamsEnc[15][0] =-183; compressParamsEnc[15][1] = -47; compressParamsEnc[15][2] = 47; compressParamsEnc[15][3] = 183;
}

double calculatePSNRfile(char *srcfile, uint8 *origimg)
{
	FILE *f;
	int x,y;
	int width,height;
	unsigned int block_part1, block_part2;
	uint8 *img;
	unsigned short w, h;
	unsigned char magic[4];
	unsigned char version[2];
	unsigned short texture_type;
	int active_width;
	int active_height;
  //int format;

  if( (f=fopen(srcfile,"rb")) )
	{
		// Load table
		readCompressParamsEnc();

		if(ktx_mode)
		{
			//read ktx header..
			KTX_header header;
			fread(&header,sizeof(KTX_header),1,f);
			//read size parameter, which we don't actually need..
			unsigned int bitsize;
			fread(&bitsize,sizeof(unsigned int),1,f);
	
			active_width = header.pixelWidth;
			active_height = header.pixelHeight;
			w = ((active_width+3)/4)*4;
			h = ((active_height+3)/4)*4;
			width=w;
			height=h;

			if(header.glInternalFormat==GL_ETC1_RGB8_OES) 
			{
        //format=ETC1_RGB_NO_MIPMAPS;
			}
			else {
				printf("ktx file has unknown glInternalFormat (not etc compressed)!\n");
				exit(1);
			}
		}
		else
		{
			// Read magic nunmber
			fread(&magic[0], sizeof(unsigned char), 1, f);
			fread(&magic[1], sizeof(unsigned char), 1, f);
			fread(&magic[2], sizeof(unsigned char), 1, f);
			fread(&magic[3], sizeof(unsigned char), 1, f);
			if(!(magic[0] == 'P' && magic[1] == 'K' && magic[2] == 'M' && magic[3] == ' '))
			{
				printf("\n\n The file %s is not a .pkm file.\n",srcfile);
				exit(1);
			}

			// Read version
			fread(&version[0], sizeof(unsigned char), 1, f);
			fread(&version[1], sizeof(unsigned char), 1, f);
			if(!(version[0] == '1' && version[1] == '0'))
			{
				printf("\n\n The file %s is not of version 1.0 but of version %c.%c.\n",srcfile, version[0], version[1]);
				exit(1);
			}

			// Read texture type
			read_big_endian_2byte_word(&texture_type, f);
			if(!(texture_type == ETC1_RGB_NO_MIPMAPS))
			{
				printf("\n\n The file %s does not contain a texture of known format.\n", srcfile);
        printf("Known formats: ETC1_RGB_NO_MIPMAPS.\n");
				exit(1);
			}

			read_big_endian_2byte_word(&w, f);
			read_big_endian_2byte_word(&h, f);
			width = w;
			height = h;

			read_big_endian_2byte_word(&w, f);
			read_big_endian_2byte_word(&h, f);
			active_width = w;
			active_height = h;
		}

		printf("width = %d, height = %d\n",width, height);

		img=(uint8*)malloc(3*width*height);
		if(!img)
		{
			printf("Error: could not allocate memory\n");
			exit(0);
		}
		
		for(y=0;y<height/4;y++)
		{
			for(x=0;x<width/4;x++)
			{
				read_big_endian_4byte_word(&block_part1,f);
				read_big_endian_4byte_word(&block_part2,f);
				decompressBlockDiffFlip(block_part1, block_part2,img,width,height,4*x,4*y);
			}
		}

		// calculate Mean Square Error (MSE)

		double MSE;
		double wMSE;
		double PSNR;
		double wPSNR;
		double err;
		MSE = 0;
		wMSE = 0;
		for(y=0;y<active_height;y++)
		{
			for(x=0;x<active_width;x++)
			{
				err = img[y*width*3+x*3+0] - origimg[y*width*3+x*3+0];
		        MSE  += ((err * err)/3.0);
				wMSE += PERCEPTUAL_WEIGHT_R_SQUARED * (err*err);
				err = img[y*width*3+x*3+1] - origimg[y*width*3+x*3+1];
		        MSE  += ((err * err)/3.0);
				wMSE += PERCEPTUAL_WEIGHT_G_SQUARED * (err*err);
				err = img[y*width*3+x*3+2] - origimg[y*width*3+x*3+2];
		        MSE  += ((err * err)/3.0);
				wMSE += PERCEPTUAL_WEIGHT_B_SQUARED * (err*err);
			}
		}
		MSE = MSE / (active_width * active_height);
		wMSE = wMSE / (active_width * active_height);
        PSNR = (float)(10*log((double)((255*255)/MSE))/log((double)10));
		wPSNR = (float)(10*log((double)((255*255)/wMSE))/log((double)10));

		printf("Perceptually weighted PSNR = (%f)\n",wPSNR);
		free(img);
		return PSNR;
	}
	else
	{
		printf("Error: could not open <%s>.\n",srcfile);
		return -1;
	}
}

void compressFile(char *srcfile,char *dstfile, int action)
{
	uint8 *srcimg;
	int width,height;
	int extendedwidth, extendedheight;
	struct _timeb tstruct;
	int tstart;
	int tstop;
	double PSNR;

	printf("\n");
	switch(action)
	{
		case 0:
			printf("Using FAST compression mode and NONPERCEPTUAL error metric\n");
			break;
		case 1:
			printf("Using MEDIUM-speed compression mode and NONPERCEPTUAL error metric\n");
			break;
		case 2:
			printf("Using SLOW compression mode and NONPERCEPTUAL error metric\n");
			break;
		case 3:
			printf("Using FAST compression mode and PERCEPTUAL error metric\n");
			break;
		case 4:
			printf("Using MEDIUM-speed compression mode and PERCEPTUAL error metric\n");
			break;
		case 5:
			printf("Using SLOW compression mode and PERCEPTUAL error metric\n");
			break;
	}
	printf("Using the orientation that maps the first pixel in .ppm file to ");
	if(orientation == FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T0)
		printf("s=0, t=0.\n");
	else if(orientation == FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T1)
		printf("s=0, t=1.\n");

	readCompressParamsEnc();
	if(readSrcFile(srcfile,srcimg,width,height,extendedwidth, extendedheight))
	{
		printf("Compressing...\n");
		tstart=time(NULL);
		_ftime( &tstruct );
		tstart=tstart*1000+tstruct.millitm;
		compressImageFile(srcimg,width,height,dstfile,extendedwidth, extendedheight, action);			
		tstop = time(NULL);
		_ftime( &tstruct );
		tstop = tstop*1000+tstruct.millitm;
		printf( "It took %u milliseconds to compress:\n", tstop - tstart);
		PSNR = calculatePSNRfile(dstfile, srcimg);
		free(srcimg);
		printf("PSNR = %f\n",PSNR);
	}
}

double calculatePSNRTwoFiles(char *srcfile1,char *srcfile2)
{
	uint8 *srcimg1;
	uint8 *srcimg2;
	int width1, height1;
	int width2, height2;
	double PSNR;
	double perceptually_weighted_PSNR;

	if(readSrcFileNoExpand(srcfile1,srcimg1,width1,height1))
	{
		if(readSrcFileNoExpand(srcfile2,srcimg2,width2,height2))
		{
			if((width1 == width2) && (height1 == height2))
			{
				PSNR = calculatePSNR(srcimg1, srcimg2, width1, height1);
				printf("PSNR = %f\n",PSNR);
				perceptually_weighted_PSNR = calculatePerceptuallyWeightedPSNR(srcimg1, srcimg2, width1, height1);
				printf("perceptually weighted PSNR = (%f)\n",perceptually_weighted_PSNR);

			}
			else
			{
				printf("\n Width and height do no not match for image: width, height = (%d, %d) and (%d, %d)\n",width1,height1, width2, height2);
			}
		}
		else
		{
			printf("Couldn't open file %s.\n",srcfile2);
		}
	}
	else
	{
		printf("Couldn't open file %s.\n",srcfile1);
	}

	return PSNR;
}


void uncompressFile(char *srcfile,char *dstfile)
{
	FILE *f;
	int width,height;
	unsigned int block_part1, block_part2;
	uint8 *img, *newimg;
	char str[300];
	unsigned short w, h;
	int xx, yy;
	unsigned char magic[4];
	unsigned char version[2];
	unsigned short texture_type;
	int active_width;
	int active_height;
//	int format;



  if( (f=fopen(srcfile,"rb")) )
	{
		// Load table
		readCompressParamsEnc();

		if(ktx_mode)
		{
			//read ktx header..
			KTX_header header;
			fread(&header,sizeof(KTX_header),1,f);
			//read size parameter, which we don't actually need..
			unsigned int bitsize;
			fread(&bitsize,sizeof(unsigned int),1,f);
	
			active_width = header.pixelWidth;
			active_height = header.pixelHeight;
			w = ((active_width+3)/4)*4;
			h = ((active_height+3)/4)*4;
			width=w;
			height=h;

			if(header.glInternalFormat==GL_ETC1_RGB8_OES) 
			{
        //format=ETC1_RGB_NO_MIPMAPS;
			}
			else {
				printf("ktx file has unknown glInternalFormat (not etc compressed)!\n");
				exit(1);
			}
		}
		else
		{
			// Read magic nunmber
			fread(&magic[0], sizeof(unsigned char), 1, f);
			fread(&magic[1], sizeof(unsigned char), 1, f);
			fread(&magic[2], sizeof(unsigned char), 1, f);
			fread(&magic[3], sizeof(unsigned char), 1, f);
			if(!(magic[0] == 'P' && magic[1] == 'K' && magic[2] == 'M' && magic[3] == ' '))
			{
				printf("\n\n The file %s is not a .pkm file.\n",srcfile);
				exit(1);
			}
	
			// Read version
			fread(&version[0], sizeof(unsigned char), 1, f);
			fread(&version[1], sizeof(unsigned char), 1, f);
			if(!(version[0] == '1' && version[1] == '0'))
			{
				printf("\n\n The file %s is not of version 1.0 but of version %c.%c.\n",srcfile, version[0], version[1]);
				exit(1);
			}

			// Read texture type
			read_big_endian_2byte_word(&texture_type, f);
			if(!(texture_type == ETC1_RGB_NO_MIPMAPS))
			{
				printf("\n\n The file %s does not contain a ETC1_RGB_NO_MIPMAPS texture.\n", srcfile);
				exit(1);
			}

			// Read how many pixels the blocks make up

			read_big_endian_2byte_word(&w, f);
			read_big_endian_2byte_word(&h, f);
			width = w;
			height = h;

			// Read how many pixels contain active data (the rest are just
			// for making sure we have a 2*a x 4*b size).

			read_big_endian_2byte_word(&w, f);
			read_big_endian_2byte_word(&h, f);
			active_width = w;
			active_height = h;
		}

		printf("Width = %d, Height = %d\n",width, height);
		printf("active pixel area: top left %d x %d area.\n",active_width, active_height);

		img=(uint8*)malloc(3*width*height);
		if(!img)
		{
			printf("Error: could not allocate memory\n");
			exit(0);
		}
		

		for(int y=0;y<height/4;y++)
		{
			for(int x=0;x<width/4;x++)
			{

				read_big_endian_4byte_word(&block_part1,f);
				read_big_endian_4byte_word(&block_part2,f);
				decompressBlockDiffFlip(block_part1, block_part2,img,width,height,4*x,4*y);
			}
		}

		// Ok, and now only write out the active pixels to the .ppm file.
		// (But only if the active pixels differ from the total pixels)

		if( !(height == active_height && width == active_width) )
		{
			newimg=(uint8*)malloc(3*active_width*active_height);
			if(!newimg)
			{
				printf("Error: could not allocate memory\n");
				exit(0);
			}
			
			// Convert from total area to active area:

			for(yy = 0; yy<active_height; yy++)
			{
				for(xx = 0; xx< active_width; xx++)
				{
					newimg[ (yy*active_width)*3 + xx*3 + 0 ] = img[ (yy*width)*3 + xx*3 + 0];
					newimg[ (yy*active_width)*3 + xx*3 + 1 ] = img[ (yy*width)*3 + xx*3 + 1];
					newimg[ (yy*active_width)*3 + xx*3 + 2 ] = img[ (yy*width)*3 + xx*3 + 2];
				}
			}

			free(img);
			img = newimg;
		}

		if(orientation == FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T0)
			fWritePPM("tmp.ppm",active_width,active_height,img,false);
		else if(orientation == FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T1)
			fWritePPM("tmp.ppm",active_width,active_height,img,true);
		else
		{
			printf("error, orientation not any of the two legal values.\n");
			exit(1);
		}

		printf("Saved file tmp.ppm \n\n");

		// Delete destination file if it exists
		if(fileExist(dstfile))
		{
			sprintf(str, "del %s\n",dstfile);	
			system(str);
		}


		int q = find_pos_of_extension(dstfile);
		if(!strcmp(&dstfile[q],".ppm")) 
		{
			// Already a .ppm file. Just rename. 
			sprintf(str,"move tmp.ppm %s\n",dstfile);
			printf("Renaming destination file to %s\n",dstfile);
		}
		else
		{
			// Converting from .ppm to other file format
			// 
			// Use your favorite command line image converter program,
			// for instance Image Magick. Just make sure the syntax can
			// be written as below:
			// 
			// C:\imconv source.ppm dest.jpg
			//
			sprintf(str,"imconv tmp.ppm %s\n",dstfile);
			printf("Converting destination file from .ppm to %s\n",dstfile);
		}
		// Execute system call
		system(str);

		fclose(f);
		free(img);
	}
	else
	{
		printf("Error: could not open <%s>.\n",srcfile);
	}
}

int main(int argc,char *argv[])
{
	if(argc==3 || argc==4 || argc == 5 || argc == 7 || argc == 9)
	{
		int action;
		// The source file is always the second last one. 
		char *srcfile=argv[argc-2];
	    char dstfile[200];
		action=determineAction(argc,argv,dstfile);
		if(action ==-1)
		{
			printf("Error: not a valid argument and/or file extension.\n");
			printf("Run etcpack without arguments to see argument list.\n");
		}
		else
		{
			
			if(!fileExist(srcfile))
			{
				printf("Error: file <%s> does not exist.\n",srcfile);
				exit(0);
			}
			// 0: compress from .any to .pkm with SPEED_FAST, METRIC_NONPERCEPTUAL, 
			// 1: compress from .any to .pkm with SPEED_MEDIUM, METRIC_NONPERCEPTUAL, 
			// 2: compress from .any to .pkm with SPEED_SLOW, METRIC_NONPERCEPTUAL, 
			// 3: compress from .any to .pkm with SPEED_FAST, METRIC_PERCEPTUAL, 
			// 4: compress from .any to .pkm with SPEED_MEDIUM, METRIC_PERCEPTUAL, 
			// 5: compress from .any to .pkm with SPEED_SLOW, METRIC_PERCEPTUAL, 
			// 6: decompress from .pkm to .any
			// 7: calculate PSNR between .any and .any

			if(action == 6)
			{
				printf("Uncompressing from .pkm file ...\n");
				printf("Using the orientation that maps the first pixel in .ppm file to ");
				if(orientation == FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T0)
					printf("s=0, t=0.\n");
				else if(orientation == FIRST_PIXEL_IN_PPM_FILE_MAPS_TO_S0T1)
					printf("s=0, t=1.\n");

				uncompressFile(srcfile,dstfile);
			}
			else if(action == 7)
			{
				printf("Calculating PSNR between files...\n");
			    calculatePSNRTwoFiles(srcfile,dstfile);
			}
			else
			{
				compressFile(srcfile, dstfile, action);
			}
		}
	}
	else
	{
		printf("ETCPACK v1.06\n");
		printf("Usage: etcpack srcfile dstfile\n\nCompresses and decompresses images using the Ericsson Texture Compression (ETC) scheme.\n\n");
		printf("      -s {fast|medium|slow}              Compression speed. Slow = best quality\n");
		printf("                                         (default: fast)\n");
		printf("      -e {perceptual|nonperceptual}      Error metric: Perceptual (nicest) or \n");
		printf("                                         nonperceptual (highest PSNR)\n");
		printf("                                         (default: perceptual)\n");
		printf("      -o {topleftmapsto_s0t0|            Orientation: Which pixel (top left or\n");
		printf("          bottomleftmapsto_s0t0}         bottom left) that will map to texture\n");
        printf("                                         coordinates (s=0, t=0). \n");
		printf("                                         (default: topleftmapsto_s0t0.) For a \n");
		printf("                                         .ppm file this means that the first \n");
        printf("                                         pixel in the file will be mapped to \n");
        printf("                                         s=0, t=0 by default.\n");
		printf("                                                             \n");
		printf("Examples: \n");
		printf("  etcpack img.ppm img.ktx                Compresses img.ppm to img.ktx\n");
		printf("  etcpack img.ppm img.pkm                Compresses img.ppm to img.pkm\n");
		printf("  etcpack img.ktx img_copy.ppm           Decompresses img.ktx to img_copy.ppm\n");
		printf("  etcpack -s slow img.ppm img.ktx        Compress using the slow mode.\n");
		printf("  etcpack -p orig.ppm copy.ppm           Calculate PSNR between orig and copy\n\n");

	}
 	return 0;

}
