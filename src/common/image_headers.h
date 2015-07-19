#include <stdint.h>
#pragma pack(push, 1)

struct TGAFileHeader
{
  int8_t idlen;       /* number of bytes in Ident Field */
  int8_t colmptype;   /* color map type (0 for none) */
  int8_t imagetype;   /* image type (2 is unmapped RGB) */
  int16_t cmorg;       /* color map origin */
  int16_t cmlen;       /* color map length in # of colors */
  int8_t cmbits;      /* number of bits per color map entry */
  int16_t xoff;
  int16_t yoff;
  int16_t xsize;
  int16_t ysize;
  int8_t bpix;       /* number of bits per pixel in pict */
  int8_t imagedesc;  /* 0 for TGA and TARGA images */
};

struct BMPFileHeader
{
  uint16_t bfType;
  uint32_t bfSize;
  uint16_t bfReserved1;
  uint16_t bfReserved2;
  uint32_t bfOffBits;
};

struct BMPInfoHeader
{
  uint32_t biSize;
  int32_t biWidth;
  int32_t biHeight;
  uint16_t biPlanes;
  uint16_t biBitCount;
  uint32_t biCompression;
  uint32_t biSizeImage;
  int32_t biXPelsPerMeter;
  int32_t biYPelsPerMeter;
  uint32_t biClrUsed;
  uint32_t biClrImportant;
};

#pragma pack(pop)
