#include "airly.h"
#include "Render.h"


Render::Render(const char * exePath)
{
  loadScene(exePath);
  renderInProgress = false;
}


Render::~Render()
{
}

void Render::loadScene(const char * exePath)
{
  std::string skyboxTextureFileName = std::string(exePath) + "./textures/skybox.tga";
  std::string planeTextureFileName = std::string(exePath) + "./textures/himiya.tga";

  camera = Camera(Vector3(7.427f, 3.494f, -3.773f), Vector3(6.5981f, 3.127f, -3.352f), Vector3(-0.320f, 0.930f, 0.180f), 1.05f);

  scene = Scene(Color(0.95f, 0.95f, 1.0f), 0.15f);
  scene.setSkyboxTexture(skyboxTextureFileName.c_str());

  scene.addLight(Vector3(11.8e9f, 4.26e9f, 3.08e9f), 6.96e8f, Color(1.0f, 1.0f, 0.95f), 1.0f);
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
  image.resize(width, height);
  scanLinesRendered = 0;
}

void Render::renderBegin(int reflectNum, int sampleNum)
{
  renderReflectNum = reflectNum;
  renderSampleNum = sampleNum;
  scanLinesRendered = 0;
  renderInProgress = true;
}

int Render::renderNext(int scanLines)
{
  if (!renderInProgress)
    return -1;

  int scanLinesLeft = image.getHeight() - scanLinesRendered;
  scanLines = min(scanLines, scanLinesLeft);
  ARGB * buf = image.getColorBuffer();

  for (int y = scanLinesRendered + scanLines - 1; y >= scanLinesRendered; y--)
  for (int x = image.getWidth() - 1; x >= 0; x--)
  {
    const float rx = float(x) - image.getWidth() / 2.0f;
    const float ry = float(y) - image.getHeight() / 2.0f;
    const float rz = float(image.getWidth()) / 2.0f / tanf(camera.fov / 2.0f);

    Vector3 origin = camera.eye;
    Vector3 ray(rx, ry, rz);

    if (renderSampleNum < 0)
    {
      if (!(x % renderSampleNum || y % renderSampleNum))
      {
        ray = camera.view * ray;
        ARGB argb = scene.trace(origin, ray, renderReflectNum).argb();
        for (int qx = min(image.getWidth(), x + abs(renderSampleNum)) - 1; qx >= x; qx--)
        for (int qy = min(image.getHeight(), y + abs(renderSampleNum)) - 1; qy >= y; qy--)
          buf[qx + qy * image.getWidth()] = argb;
      }
    }
    else
    {
      for (int ssx = 0; ssx < renderSampleNum; ssx++)
      for (int ssy = 0; ssy < renderSampleNum; ssy++)
      {
        if (renderSampleNum > 1)
        {
          ray.x += float(ssx) / renderSampleNum;
          ray.y += float(ssy) / renderSampleNum;
        }
        ray = camera.view * ray;
        buf[x + y * image.getWidth()] = scene.trace(origin, ray, renderReflectNum).argb();
      }
    }
  }

  scanLinesRendered += scanLines;
  renderInProgress = (scanLinesLeft != 0);

  return scanLinesLeft;
}

void Render::renderAll(int reflectNum, int sampleNum)
{
  renderBegin(reflectNum, sampleNum);
  renderNext(image.getHeight());
}
