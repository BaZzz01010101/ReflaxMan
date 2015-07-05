#pragma pack(push, 1)

struct TGAFileHeader
{
  char idlen;       /* number of bytes in Ident Field */
  char colmptype;   /* color map type (0 for none) */
  char imagetype;   /* image type (2 is unmapped RGB) */
  short cmorg;       /* color map origin */
  short cmlen;       /* color map length in # of colors */
  char cmbits;      /* number of bits per color map entry */
  short xoff;
  short yoff;
  short xsize;
  short ysize;
  char bpix;       /* number of bits per pixel in pict */
  char imagedesc;  /* 0 for TGA and TARGA images */
};

struct BMPFileHeader
{
  unsigned short bfType;
  unsigned int bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned int bfOffBits;
};

struct BMPInfoHeader
{
  unsigned int biSize;
  int biWidth;
  int biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned int biCompression;
  unsigned int biSizeImage;
  int biXPelsPerMeter;
  int biYPelsPerMeter;
  unsigned int biClrUsed;
  unsigned int biClrImportant;
};

#pragma pack(pop)
