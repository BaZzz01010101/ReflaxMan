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
  Texture(const Texture & texture);
  Texture(const int width, const int height);
  Texture(const char* filename);
  ~Texture();

  Texture & operator = (const Texture & texture);

  bool loadFromFile(const char* fileName);
  bool saveToFile(const char* fileName);

  bool loadFromTGAFile(const char* fileName);
  bool saveToTGAFile(const char* fileName);
  bool saveToBMPFile(const char* fileName);

  ARGB * getColorBuffer() const;
  Color getTexelColor(const int x, const int y) const;
  Color getTexelColor(const float u, const float v) const;

  void resize(const int width, const int height);
  inline int getWidth() const { return width; };
  inline int getHeight() const { return height; };
};

