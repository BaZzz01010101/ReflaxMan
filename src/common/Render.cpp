#include "trace_math.h"
#include "Render.h"


Render::Render(const char * exePath)
{
  additiveCounter = 0;
  scanLinesRendered = 0;
  inProgress = false;

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

void Render::setImageSize(int width, int height)
{
  assert(width > 0);
  assert(height > 0);

  if(width > 0 && height > 0)
  {
    if(width * height > (int)image.size())
      image.resize(width * height);

    Color * pCol = &image.front();
    Color * endCol = pCol + width * height;

    while(pCol < endCol)
      *pCol++ = Color(0, 0, 0);

    imageWidth = width;
    imageHeight = height;
    additiveCounter = 0;
    scanLinesRendered = 0;
    inProgress = false;
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
  scanLinesRendered = 0;
  inProgress = true;
  renderCameraView = camera.view;
  renderCameraEye = camera.eye;

  if (additive)
    additiveCounter++;
  else
    additiveCounter = 0;
}

int Render::renderNext(int scanLines)
{
  assert(scanLines > 0);
  assert(inProgress);

  if (!inProgress)
    return -1;

  if (scanLines > 0)
  {
    const Vector3 origin = renderCameraEye;
    const float sqRenderSampleNum = float(renderSampleNum * renderSampleNum);
    const float rz = float(imageWidth) / 2.0f / tanf(camera.fov / 2.0f);
    const float imageWidthHalf = imageWidth / 2.0f;
    const float imageHeightHalf = imageHeight / 2.0f;
    const int endScanLine = scanLinesRendered + min(scanLines, imageHeight - scanLinesRendered);

    for (int y = scanLinesRendered; y < endScanLine; y++)
    for (int x = 0; x < imageWidth; x++)
    {
      const float rx = float(x) - imageWidthHalf;
      const float ry = float(y) - imageHeightHalf;
      Vector3 ray(rx, ry, rz);

      if (renderSampleNum < 0)
      {
        if (!(x % renderSampleNum || y % renderSampleNum))
        {
          ray = renderCameraView * ray;
          const Color tracedColor = scene.trace(origin, ray, renderReflectNum);
          const int endQx = min(imageWidth, x + abs(renderSampleNum));
          const int endQy = min(imageHeight, y + abs(renderSampleNum));

          for (int qx = x; qx < endQx; qx++)
          for (int qy = y; qy < endQy; qy++)
            image[qx + qy * imageWidth] = tracedColor;
        }
      }
      else if (renderSampleNum > 0)
      {
        Color finColor;
        const float rndx = renderAdditive ? float(fastrand()) / FAST_RAND_MAX : 0;
        const float rndy = renderAdditive ? float(fastrand()) / FAST_RAND_MAX : 0;
        //const Vector3 randVec = renderAdditive ? Vector3::randomInsideSphere(0.5f / renderSampleNum) : Vector3(0, 0, 0);
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
          image[x + y * imageWidth] += finColor;
        else
          image[x + y * imageWidth] = finColor;

      }
    }

    scanLinesRendered += scanLines;
    inProgress = (scanLinesRendered < imageHeight);
  }

  return imageHeight - scanLinesRendered;
}

void Render::renderAll(int reflectNum, int sampleNum, bool additive)
{
  renderBegin(reflectNum, sampleNum, additive);
  renderNext(imageHeight);
}
