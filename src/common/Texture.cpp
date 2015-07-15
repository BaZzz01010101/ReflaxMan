#include "trace_math.h"
#include "Texture.h"
#include "image_headers.h"


Texture::Texture()
{
  width = 0;
  height = 0;
}

Texture::Texture(const int width, const int height)
{
  resize(width, height);
}

Texture::Texture(const char* filename)
{
  width = 0;
  height = 0;
  bool success = loadFromFile(filename);
  assert(success);
}

Texture::~Texture()
{
}

bool Texture::loadFromTGAFile(const char * fileName)
{
  bool retVal = false;
  FILE * fh = fopen(fileName, "rb");
  if (fh)
  {
    TGAFileHeader header;

    if (fread((void *)&header, sizeof(TGAFileHeader), 1, fh) 
      && 
      header.imagetype == 2)
    {
      width = header.xsize;
      height = header.ysize;
      const int bpp = header.bpix;
      const int colorOffset = sizeof(header)+header.idlen + header.cmlen * header.cmbits / 8;

      if (!fseek(fh, colorOffset, SEEK_SET) 
        && 
        (bpp == 24 || bpp == 32))
      {
        int pixelCount = width * height;
        colorBuf.resize(pixelCount);
        ARGB * pColor = getColorBuffer();
        const int pixelSize = bpp / 8;

        while (pixelCount > 0)
        {
          if (!fread((void *)pColor++, pixelSize, 1, fh))
            break;

          pixelCount--;
        }

        if (!pixelCount)
          retVal = true;
      }
    }
    fclose(fh);
  }

  if (!retVal)
  {
    colorBuf.resize(0);
    width = 0;
    height = 0;
  }

  return retVal;
}

bool Texture::saveToTGAFile(const char * fileName) const
{
  bool retVal = false;
  FILE * fh = fopen(fileName, "wb");

  if (fh)
  {
    TGAFileHeader header;
    header.idlen = 0;
    header.colmptype = 0;
    header.imagetype = 2;
    header.cmorg = 0;
    header.cmlen = 0;
    header.cmbits = 0;
    header.xoff = 0;
    header.yoff = 0;
    header.xsize = short(width);
    header.ysize = short(height);
    header.bpix = 32;
    header.imagedesc = 0;

    retVal = (fwrite((void *)&header, sizeof(TGAFileHeader), 1, fh) &&
              fwrite((void *)getColorBuffer(), width * height * 4, 1, fh));
    fclose(fh);
  }

  return retVal;
}

bool Texture::saveToBMPFile(const char * fileName) const
{
  bool retVal = false;
  FILE * fh = fopen(fileName, "wb");

  if (fh)
  {
    BMPFileHeader fileHeader;
    fileHeader.bfType = 'MB';
    fileHeader.bfSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + width * height * 4;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    BMPInfoHeader infoHeader;
    infoHeader.biSize = sizeof(BMPInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = 0;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    retVal = (fwrite((void *)&fileHeader, sizeof(BMPFileHeader), 1, fh) &&
              fwrite((void *)&infoHeader, sizeof(BMPInfoHeader), 1, fh) &&
              fwrite((void *)getColorBuffer(), width * height * 4, 1, fh));
    fclose(fh);
  }

  return retVal;
}

bool Texture::loadFromFile(const char * fileName)
{
  const std::string fileNameString = fileName;
  const size_t dotIndex = fileNameString.find_last_of('.');

  if (dotIndex != std::string::npos)
  {
    const size_t extLen = fileNameString.length() - dotIndex;
    const std::string fileExt = fileNameString.substr(dotIndex, extLen);

    if (fileExt == ".tga")
      return loadFromTGAFile(fileName);
  }
  return false;
}

bool Texture::saveToFile(const char * fileName) const
{
  const std::string fileNameString = fileName;
  const size_t dotIndex = fileNameString.find_last_of('.');

  if (dotIndex != std::string::npos)
  {
    const size_t extLen = fileNameString.length() - dotIndex;
    const std::string fileExt = fileNameString.substr(dotIndex, extLen);

    if (fileExt == ".tga")
      return saveToTGAFile(fileName);
    if (fileExt == ".bmp")
      return saveToBMPFile(fileName);
  }
  return false;
}

ARGB * Texture::getColorBuffer() const
{
  assert(!colorBuf.empty());

  return (ARGB *) & colorBuf.front();
}

Color Texture::getTexelColor(const int x, const int y) const
{
  assert(x >= 0);
  assert(x < width);
  assert(y >= 0);
  assert(y < height);

  // return black texture if out of bounds or gray chess if texture is empty
  if (x < 0 || x >= width || y < 0 || y >= height)
    return Color(0, 0, 0);
  else if(colorBuf.empty())
    return (((x * 50 / width) % 2) ^ ((y * 50 / width) % 2)) ? Color(0.5f, 0.5f, 0.5f) : Color(0.75f, 0.75f, 0.75f);

  return Color(colorBuf[clamp(x, 0, width - 1) + width * clamp(y, 0, height - 1)]);
}

Color Texture::getTexelColor(const float u, const float v) const
{
  assert(u >= 0.0f);
  assert(u <= 1.0f);
  assert(v >= 0.0f);
  assert(v <= 1.0f);

  // return black texture if out of bounds or gray chess if texture is empty
  if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
    return Color(0.0f, 0.0f, 0.0f);
  else if (colorBuf.empty())
      return ((int(u * 50) % 2) ^ (int(v * 50) % 2)) ? Color(0.5f, 0.5f, 0.5f) : Color(0.75f, 0.75f, 0.75f);

  const float fx = clamp(u, 0.0f, 1.0f - FLT_EPSILON) * width;
  const float fy = clamp(v, 0.0f, 1.0f - FLT_EPSILON) * height;
  const int x = int(fx);
  const int y = int(fy);

  // bilinear filtering
  if (x < width - 1 && y < height - 1)
  {
    const Color color00 = getTexelColor(x, y);
    const Color color01 = getTexelColor(x, y + 1);
    const Color color10 = getTexelColor(x + 1, y);
    const Color color11 = getTexelColor(x + 1, y + 1);

    const float uFrac = fx - floor(fx);
    const float vFrac = fy - floor(fy);
    const float uOpFrac = 1 - uFrac;
    const float vOpFrac = 1 - vFrac;

    return (color00 * uOpFrac + color10 * uFrac) * vOpFrac + (color01 * uOpFrac + color11 * uFrac) * vFrac;
  }
  else
    return getTexelColor(int(fx), int(fy));
}

void Texture::resize(const int width, const int height)
{
  this->width = width;
  this->height = height;
  colorBuf.resize(width * height);
}

void Texture::clear(ARGB bkColor)
{
  if (!colorBuf.empty())
    for (ARGB * pCol = &colorBuf.front(), *lastPCol = &colorBuf.back(); pCol <= lastPCol; ++pCol)
      *pCol = bkColor;
}
