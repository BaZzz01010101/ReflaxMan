#include "trace_math.h"
#include "Render.h"


Render::Render(const char * exePath)
{
  additiveCounter = 0;
  inProgress = false;
  curx = 0;
  cury = 0;

  loadScene(exePath);
}


Render::~Render()
{
}

void Render::loadScene(const char * exePath)
{
  const std::string skyboxTextureFileName = std::string(exePath) + "./textures/skybox.tga";
  const std::string planeTextureFileName = std::string(exePath) + "./textures/himiya.tga";

  camera = Camera(Vector3(7.427f, 3.494f, -3.773f), Vector3(6.5981f, 3.127f, -3.352f), 1.05f);

  scene = Scene(Color(0.95f, 0.95f, 1.0f), 0.15f);
  scene.setSkyboxTexture(skyboxTextureFileName.c_str());

  scene.addLight(Vector3(11.8e9f, 4.26e9f, 3.08e9f), 3.48e8f, Color(1.0f, 1.0f, 0.95f), 0.85f);
  //scene.addLight(Vector3(-1.26e9f, 11.8e9f, 1.08e9f), 6.96e8f, Color(1.0f, 0.5f, 0.5f), 0.2f);
  //scene.addLight(Vector3(11.8e9f, 4.26e9f, 3.08e9f), 6.96e9f, Color(1.0f, 1.0f, 0.95f), 0.85f);

  scene.addSphere(Vector3(-1.25f, 1.5f, -0.25f), 1.5f, Material(Material::mtMetal, Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f));
  scene.addSphere(Vector3(0.15f, 1.0f, 1.75f), 1.0f, Material(Material::mtMetal, Color(1.0f, 1.0f, 1.0f), 0.95f, 0.0f));

  scene.addSphere(Vector3(-3.0f, 0.6f, -3.0f), 0.6f, Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.0f, 0.0f));
  scene.addSphere(Vector3(-0.5f, 0.5f, -2.5f), 0.5f, Material(Material::mtDielectric, Color(0.5f, 1.0f, 0.15f), 0.75f, 0.0f));
  scene.addSphere(Vector3(1.0f, 0.4f, -1.5f), 0.4f, Material(Material::mtDielectric, Color(0.0f, 0.5f, 1.0f), 1.0f, 0.0f));

  scene.addSphere(Vector3(1.8f, 0.4f, 0.1f), 0.4f, Material(Material::mtMetal, Color(1.0f, 0.65f, 0.45f), 1.0f, 0.0f));
  scene.addSphere(Vector3(1.7f, 0.5f, 1.9f), 0.5f, Material(Material::mtMetal, Color(1.0f, 0.90f, 0.60f), 0.75f, 0.0f));
  scene.addSphere(Vector3(0.6f, 0.6f, 4.2f), 0.6f, Material(Material::mtMetal, Color(0.9f, 0.9f, 0.9f), 0.0f, 0.0f));

  Texture * planeTexture = scene.addTexture(planeTextureFileName.c_str());
  Triangle * tr1 = scene.addTriangle(Vector3(-14.0f, 0.0f, -10.0f), Vector3(-14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, -10.0f), Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.95f, 0.0f));
  tr1->setTexture(planeTexture, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
  Triangle * tr2 = scene.addTriangle(Vector3(-14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, -10.0f), Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.95f, 0.0f));
  tr2->setTexture(planeTexture, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);
}

void Render::setImageSize(unsigned int width, unsigned int height)
{
  assert(width > 0);
  assert(height > 0);

  if(width > 0 && height > 0)
  {
    if(width * height > image.size())
      image.resize(width * height);

    Color * pCol = &image.front();
    Color * endCol = pCol + width * height;

    while(pCol < endCol)
      *pCol++ = Color(0, 0, 0);

    imageWidth = width;
    imageHeight = height;
    additiveCounter = 0;
    inProgress = false;
    curx = 0;
    cury = 0;
  }
}

void Render::copyImage(Texture & texture) const
{
  assert(imageWidth > 0);
  assert(imageHeight > 0);
  assert(texture.getWidth() > 0);
  assert(texture.getHeight() > 0);
  assert(imageWidth == texture.getWidth());
  assert(imageHeight == texture.getHeight());

  if (imageWidth == texture.getWidth() && imageHeight == texture.getHeight())
  {
    const Color * imagePixel = &image.front();
    ARGB * texPixel = texture.getColorBuffer();
    const ARGB * endTexPixel = texPixel + imageWidth * imageHeight;
    while (texPixel < endTexPixel)
      *texPixel++ = (imagePixel++)->argb();
  }
  else
    texture.clear(0);
}

Color Render::imagePixel(int x, int y) const
{
  assert(x >= 0);
  assert(y >= 0);

  if (x >= 0 && y >= 0)
    return additiveCounter > 1 ? 
           image[x + y * imageWidth] / float(additiveCounter) : 
           image[x + y * imageWidth];
  else
    return Color(0, 0, 0);
}

void Render::renderBegin(int reflectNum, int sampleNum, bool additive)
{
  assert(reflectNum > 0);
  assert(sampleNum != 0);

  renderReflectNum = reflectNum;
  renderSampleNum = sampleNum;
  renderAdditive = additive;
  inProgress = true;
  curx = 0;
  cury = 0;
  renderCameraView = camera.view;
  renderCameraEye = camera.eye;

  if (additive)
    additiveCounter++;
  else
    additiveCounter = 0;
}

bool Render::renderNext(unsigned int pixels)
{
  assert(pixels);
  assert(inProgress);
  assert(curx < imageWidth);
  assert(cury < imageHeight);

  if (!pixels || !inProgress || curx >= imageWidth || cury >= imageHeight)
    return false;
  
  const Vector3 origin = renderCameraEye;
  const float sqRenderSampleNum = float(renderSampleNum * renderSampleNum);
  const float rz = float(imageWidth) / 2.0f / tanf(camera.fov / 2.0f);
  const float imageWidthHalf = imageWidth / 2.0f;
  const float imageHeightHalf = imageHeight / 2.0f;

  while (pixels > 0)
  {
    const float rx = float(curx) - imageWidthHalf;
    const float ry = float(cury) - imageHeightHalf;
    Vector3 ray(rx, ry, rz);

    if (renderSampleNum < 0)
    {
      unsigned int absRenderSampleNum = abs(renderSampleNum);

      if (!(curx % absRenderSampleNum || cury % absRenderSampleNum))
      {
        ray = renderCameraView * ray;
        const Color tracedColor = scene.trace(origin, ray, renderReflectNum);
        const int endQx = min(imageWidth, curx + absRenderSampleNum);
        const int endQy = min(imageHeight, cury + absRenderSampleNum);

        for (int qx = curx; qx < endQx; qx++)
        for (int qy = cury; qy < endQy; qy++)
          image[qx + qy * imageWidth] = tracedColor;
      }
    }
    else if (renderSampleNum > 0)
    {
      Color finColor;
      const float rndx = renderAdditive ? float(fastrand()) / FAST_RAND_MAX : 0;
      const float rndy = renderAdditive ? float(fastrand()) / FAST_RAND_MAX : 0;
      finColor = Color(0.0f, 0.0f, 0.0f);

      for (int ssx = 0; ssx < renderSampleNum; ssx++)
      for (int ssy = 0; ssy < renderSampleNum; ssy++)
      {
        Vector3 ray = Vector3(rx + float(ssx) / renderSampleNum + rndx, ry + float(ssy) / renderSampleNum + rndy, rz);
        ray = renderCameraView * ray;
        finColor += scene.trace(origin, ray, renderReflectNum);
      }

      finColor /= sqRenderSampleNum;

      if (additiveCounter > 1)
        image[curx + cury * imageWidth] += finColor;
      else
        image[curx + cury * imageWidth] = finColor;

    }

    pixels--;
    curx++;

    if (curx == imageWidth)
    {
      curx = 0;
      cury++;
    }

    if (cury == imageHeight)
    {
      inProgress = false;
      break;
    }
  }

  return inProgress;
}

void Render::renderAll(int reflectNum, int sampleNum, bool additive)
{
  renderBegin(reflectNum, sampleNum, additive);
  renderNext(imageHeight);
}

float Render::getRenderProgress() const
{ 
  return float(curx + cury * imageWidth) * 100.0f / imageWidth / imageHeight;
}
