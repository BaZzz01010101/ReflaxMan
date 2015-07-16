#pragma once
#include <vector>
#include "Color.h"

class Texture
{
private:
  int width;
  int height;
  std::vector<ARGB> colorBuf;

public:
  Texture();
  Texture(const int width, const int height);
  Texture(const char* filename);
  ~Texture();

  bool loadFromFile(const char* fileName);
  bool saveToFile(const char* fileName) const;

  bool loadFromTGAFile(const char* fileName);
  bool saveToTGAFile(const char* fileName) const;
  bool saveToBMPFile(const char* fileName) const;

  ARGB * getColorBuffer() const;
  Color getTexelColor(const int x, const int y) const;
  Color getTexelColor(const float u, const float v) const;

  void resize(int width, int height);
  void clear(ARGB bkColor);
  inline int getWidth() const { return width; };
  inline int getHeight() const { return height; };
};

