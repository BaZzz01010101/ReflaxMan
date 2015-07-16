#pragma once
#include <vector>
#include "Color.h"

class Texture
{
private:
  unsigned int width;
  unsigned int height;
  std::vector<ARGB> colorBuf;

public:
  Texture();
  Texture(const unsigned int width, const unsigned int height);
  Texture(const char* filename);
  ~Texture();

  bool loadFromFile(const char* fileName);
  bool saveToFile(const char* fileName) const;

  bool loadFromTGAFile(const char* fileName);
  bool saveToTGAFile(const char* fileName) const;
  bool saveToBMPFile(const char* fileName) const;

  ARGB * getColorBuffer() const;
  Color getTexelColor(const unsigned int x, const unsigned int y) const;
  Color getTexelColor(const float u, const float v) const;

  void resize(unsigned int width, unsigned int height);
  void clear(ARGB bkColor);
  inline unsigned int getWidth() const { return width; };
  inline unsigned int getHeight() const { return height; };
};

