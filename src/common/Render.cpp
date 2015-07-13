#include "airly.h"
#include "Render.h"


Render::Render(const char * exePath)
{
  scanLinesRendered = 0;
  renderInProgress = false;
  additiveCounter = 0;

  loadScene(exePath);
}


Render::~Render()
{
}

void Render::loadScene(const char * exePath)
{
  std::string skyboxTextureFileName = std::string(exePath) + "./textures/skybox.tga";
  std::string planeTextureFileName = std::string(exePath) + "./textures/himiya.tga";

  camera = Camera(Vector3(7.427f, 3.494f, -3.773f), Vector3(6.5981f, 3.127f, -3.352f), 1.05f);

  scene = Scene(Color(0.95f, 0.95f, 1.0f), 0.15f);
  scene.setSkyboxTexture(skyboxTextureFileName.c_str());

  scene.addLight(Vector3(11.8e9f, 4.26e9f, 3.08e9f), 3.48e8f, Color(1.0f, 1.0f, 0.95f), 0.85f);
  //scene.addLight(Vector3(-1.26e9f, 11.8e9f, 1.08e9f), 6.96e8f, Color(1.0f, 0.5f, 0.5f), 0.2f);
  //scene.addLight(Vector3(11.8e9f, 4.26e9f, 3.08e9f), 6.96e9f, Color(1.0f, 1.0f, 0.95f), 0.85f);

  scene.addSphere(Vector3(-1.25f, 1.5f, -0.25f), 1.5f, Material(Material::mtMetal, Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f));
  scene.addSphere(Vector3(0.15f, 1.0f, 1.75f), 1.0f, Material(Material::mtMetal, Color(1.0f, 1.0f, 1.0f), 0.9f, 0.0f));

  scene.addSphere(Vector3(-3.0f, 0.6f, -3.0f), 0.6f, Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.0f, 0.0f));
  scene.addSphere(Vector3(-0.5f, 0.5f, -2.5f), 0.5f, Material(Material::mtDielectric, Color(0.5f, 1.0f, 0.15f), 0.8f, 0.0f));
  scene.addSphere(Vector3(1.0f, 0.4f, -1.5f), 0.4f, Material(Material::mtDielectric, Color(0.0f, 0.5f, 1.0f), 0.99f, 0.0f));

  scene.addSphere(Vector3(1.8f, 0.4f, 0.1f), 0.4f, Material(Material::mtMetal, Color(1.0f, 0.65f, 0.45f), 0.99f, 0.0f));
  scene.addSphere(Vector3(1.7f, 0.5f, 1.9f), 0.5f, Material(Material::mtMetal, Color(1.0f, 0.90f, 0.60f), 0.8f, 0.0f));
  scene.addSphere(Vector3(0.6f, 0.6f, 4.2f), 0.6f, Material(Material::mtMetal, Color(0.9f, 0.9f, 0.9f), 0.1f, 0.0f));

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
  image.resize(width * height);
  imageWidth = width;
  imageHeight = height;
  scanLinesRendered = 0;
  renderInProgress = false;
  additiveCounter = 0;
}

void Render::copyImage(Texture & texture)
{
  if (imageWidth == texture.getWidth() && imageHeight == texture.getHeight())
  {
    ARGB * pArgb = texture.getColorBuffer();
    for (int i = 0, cnt = imageWidth * imageHeight; i < cnt; i++)
      *pArgb = image[i].argb();
  }
  else texture.clear(0xFF00FF);
}

Color Render::imagePixel(int x, int y) 
{ 
  return additiveCounter > 1 ? 
         image[x + y * imageWidth] / float(additiveCounter) : 
         image[x + y * imageWidth]; 
}

void Render::renderBegin(int reflectNum, int sampleNum, bool additive)
{
  renderReflectNum = reflectNum;
  renderSampleNum = sampleNum;
  renderAdditive = additive;
  scanLinesRendered = 0;
  renderInProgress = true;
  renderCameraView = camera.view;
  renderCameraEye = camera.eye;

  if (additive)
    additiveCounter++;
  else
    additiveCounter = 0;
}

void Render::renderRestart()
{
  scanLinesRendered = 0;
  renderInProgress = true;
}

int Render::renderNext(int scanLines)
{
  if (!renderInProgress)
    return -1;

  int scanLinesLeft = imageHeight - scanLinesRendered;
  scanLines = min(scanLines, scanLinesLeft);

  for (int y = scanLinesRendered + scanLines - 1; y >= scanLinesRendered; y--)
  for (int x = imageWidth - 1; x >= 0; x--)
  {
    const float rx = float(x) - imageWidth / 2.0f;
    const float ry = float(y) - imageHeight / 2.0f;
    const float rz = float(imageWidth) / 2.0f / tanf(camera.fov / 2.0f);

    Vector3 origin = renderCameraEye;
    Vector3 ray(rx, ry, rz);

    if (renderSampleNum < 0)
    {
      if (!(x % renderSampleNum || y % renderSampleNum))
      {
        ray = renderCameraView * ray;
        
        Color col = scene.trace(origin, ray, renderReflectNum);
        for (int qx = min(imageWidth, x + abs(renderSampleNum)) - 1; qx >= x; qx--)
        for (int qy = min(imageHeight, y + abs(renderSampleNum)) - 1; qy >= y; qy--)
          image[qx + qy * imageWidth] = col;
      }
    }
    else
    {
      Color finColor;

      if (renderSampleNum > 1)
      {
        Vector3 randVec;
        finColor = Color(0.0f, 0.0f, 0.0f);

        if (renderAdditive)
          randVec = Vector3::randomInsideSphere(0.5f / renderSampleNum);

        for (int ssx = 0; ssx < renderSampleNum; ssx++)
        for (int ssy = 0; ssy < renderSampleNum; ssy++)
        {
          Vector3 ray(rx + float(ssx) / renderSampleNum, ry + float(ssy) / renderSampleNum, rz);

          if (renderAdditive)
            ray += randVec;

          ray = renderCameraView * ray;
          finColor += scene.trace(origin, ray, renderReflectNum);
        }

        finColor /= float(renderSampleNum * renderSampleNum);
      }
      else
      {
        if (renderAdditive)
          ray += Vector3::randomInsideSphere(0.5f);

        ray = renderCameraView * ray;
        finColor = scene.trace(origin, ray, renderReflectNum);
      }

      if (additiveCounter > 1)
        image[x + y * imageWidth] += finColor;
      else
        image[x + y * imageWidth] = finColor;

    }
  }

  scanLinesRendered += scanLines;
  renderInProgress = (scanLinesLeft != 0);

  return scanLinesLeft;
}

void Render::renderAll(int reflectNum, int sampleNum, bool additive)
{
  renderBegin(reflectNum, sampleNum, additive);
  renderNext(imageHeight);
}
