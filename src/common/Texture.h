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
  Texture(const wchar_t* filename);
  ~Texture();

  Texture & operator = (const Texture & texture);

  bool loadFromFile(const wchar_t* fileName);
  bool saveToFile(const wchar_t* fileName);

  bool loadFromTGAFile(const wchar_t* fileName);
  bool saveToTGAFile(const wchar_t* fileName);
  bool saveToBMPFile(const wchar_t* fileName);

  ARGB * getColorBuffer() const;
  Color getTexelColor(const int x, const int y) const;
  Color getTexelColor(const float u, const float v) const;

  void resize(const int width, const int height);
  inline int getWidth() const { return width; };
  inline int getHeight() const { return height; };
};

