#include "airly.h"

#include "Scene.h"
#include "Sphere.h"
#include "Triangle.h"


Scene::Scene(const wchar_t* exePath) 
{
  std::wstring skyboxTextureFileName = std::wstring(exePath) + L".\\textures\\skybox.tga";
  std::wstring planeTextureFileName = std::wstring(exePath) + L".\\textures\\himiya.tga";
  bool retVal = skybox.loadTexture(skyboxTextureFileName.c_str());
  assert(retVal);
  retVal = planeTexture.loadFromFile(planeTextureFileName.c_str());
  assert(retVal);

  camera = Camera(Vector3(7.427f, 3.494f, -3.773f), Vector3(6.5981f, 3.127f, -3.352f), Vector3(-0.320f, 0.930f, 0.180f), 1.05f);

  sceneLights.push_back(new OmniLight(Vector3(100.0f, 36.0f, 26.0f), Color(1.0f, 1.0f, 1.0f), 1.0f));

  diffLightColor = { 1.0f, 1.0f, 1.0f };
  diffLightPower = 0.2f;
  mainLightColor = sceneLights.empty() ? diffLightColor * diffLightPower : sceneLights.front()->color * sceneLights.front()->power;

  sceneObjects.push_back(new Sphere(Vector3(-1.25f, 1.5f, -0.25f), 1.5f, Material(Material::mtMetal, Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(0.15f, 1.0f, 1.75f), 1.0f, Material(Material::mtMetal, Color(1.0f, 1.0f, 1.0f), 0.9f, 0.0f)));

  sceneObjects.push_back(new Sphere(Vector3(-3.0f, 0.6f, -3.0f), 0.6f, Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.1f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(-0.5f, 0.5f, -2.5f), 0.5f, Material(Material::mtDielectric, Color(0.5f, 1.0f, 0.15f), 0.8f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(1.0f, 0.4f, -1.5f), 0.4f, Material(Material::mtDielectric, Color(0.0f, 0.5f, 1.0f), 0.99f, 0.0f)));

  sceneObjects.push_back(new Sphere(Vector3(1.8f, 0.4f, 0.1f), 0.4f, Material(Material::mtMetal, Color(1.0f, 0.65f, 0.45f), 0.99f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(1.7f, 0.5f, 1.9f), 0.5f, Material(Material::mtMetal, Color(1.0f, 0.90f, 0.60f), 0.8f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(0.6f, 0.6f, 4.2f), 0.6f, Material(Material::mtMetal, Color(0.9f, 0.9f, 0.9f), 0.1f, 0.0f)));

  Triangle* tr = NULL;

  tr = new Triangle(Vector3(-14.0f, 0.0f, -10.0f), Vector3(-14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, -10.0f), Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.95f, 0.0f));
  tr->setTexture(&planeTexture, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
  sceneObjects.push_back(tr);

  tr = new Triangle(Vector3(-14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, -10.0f), Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.95f, 0.0f));
  tr->setTexture(&planeTexture, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f);
  sceneObjects.push_back(tr);
}

Scene::~Scene()
{
  for (SCENE_OBJECTS::iterator obj = sceneObjects.begin(); obj != sceneObjects.end(); ++obj)
    delete *obj;
}

Color Scene::tracePixel(const int x, const int y, const int aan) const
{
  assert(x >= 0);
  assert(y >= 0);
  assert(aan > 0);
  assert(fabs(camera.fov) > VERY_SMALL_NUMBER);

  if (x < 0 || y < 0 || aan <= 0 || fabs(camera.fov) <= VERY_SMALL_NUMBER)
    return Color(0.0f, 0.0f, 0.0f);

  const int maxReflections = 7;
  Color aaPixelColor = { 0.0f, 0.0f, 0.0f };

  const float flx = float(x) - screenWidth / 2.0f;
  const float fly = float(y) - screenHeight / 2.0f;
  const float screenPlaneZ = float(screenWidth) / 2.0f / tanf(camera.fov / 2.0f);

  for (int aax = 0; aax < aan; aax++)
  for (int aay = 0; aay < aan; aay++)
  {
    Color colorMul = { 1.0f, 1.0f, 1.0f };
    Vector3 randDir = Vector3::randomInsideSphere(0.5f);
    Color pixelColor = { 0.0f, 0.0f, 0.0f };
    Vector3 origin = camera.eye;
    Vector3 ray(flx + float(aax) / aan,
                fly + float(aay) / aan,
                screenPlaneZ);
    ray = camera.view * ray;

    for (int refl = 0; refl < maxReflections; ++refl)
    {
      float minDistance = FLT_MAX;
      SceneObject* hitObject = NULL;
      Vector3 drop, norm, reflect;
      Material dropMaterial;

      for (SCENE_OBJECTS::const_iterator obj = sceneObjects.begin();
        obj != sceneObjects.end();
        ++obj)
      {
        Vector3 curDrop, curNorm, curReflect;
        Material curDropMaterial;
        float curDist;

        if ((*obj)->trace(origin, ray, &curDrop, &curNorm, &curReflect, &curDist, &curDropMaterial)
          &&
          curDist < minDistance)
        {
          minDistance = curDist;
          drop = curDrop;
          norm = curNorm;
          reflect = curReflect;
          dropMaterial = curDropMaterial;
          hitObject = *obj;
        }
      }

      if (hitObject)
      {
        float rayLen = ray.length();
        float normLen = norm.length();
        float reflectLen = reflect.length();
        float sumLightPower = 0.0f;
        float sumSpecPower = 0.0f;
        Color sumLightColor = { 0.0f, 0.0f, 0.0f };
        Color sumSpecColor = { 0.0f, 0.0f, 0.0f };

        for (SCENE_LIGHTS::const_iterator lt = sceneLights.begin(); lt != sceneLights.end(); ++lt)
        {
          Vector3 dropToLight = (*lt)->origin - drop;
          bool inShadow = false;

          for (SCENE_OBJECTS::const_iterator obj = sceneObjects.begin();
            obj != sceneObjects.end();
            ++obj)
          {
            Vector3 curDrop, curNorm, curReflect;
            Color curColor;

            dropToLight.normalize();
            dropToLight += randDir * 0.02f;

            if ((*obj)->trace(drop, dropToLight, NULL, NULL, NULL, NULL, NULL))
            {
              inShadow = true;
              break;
            }
          }

          if (!inShadow)
          {
            float dropToLightLen = dropToLight.length();

            float a = dropToLightLen * normLen;
            float lightPower = (a > VERY_SMALL_NUMBER) ? dropToLight * norm / a : 0.0f;

            if (lightPower > VERY_SMALL_NUMBER)
            {
              sumLightColor += (*lt)->color * lightPower;
              sumLightPower += lightPower;
            }

            a = dropToLightLen * reflectLen;
            float specPower = (a > VERY_SMALL_NUMBER) ? dropToLight * reflect / a : 0.0f;

            if (specPower > VERY_SMALL_NUMBER)
            {
              sumSpecColor = (sumSpecColor * sumSpecPower + (*lt)->color * specPower) / (sumSpecPower + specPower);
              sumSpecPower = fmax(sumSpecPower, specPower);
            }
          }
        }

        int ltCnt = sceneLights.size();
        if (ltCnt)
        {
          sumLightPower /= float(ltCnt);
          sumLightColor /= float(ltCnt);
        }
        float reflectivity = dropMaterial.reflectivity;
        Color & color = dropMaterial.color;
        Material::Type type = dropMaterial.type;

        float a = diffLightPower + sumLightPower;
        if (fabs(a) > VERY_SMALL_NUMBER)
          sumLightColor = (diffLightColor * diffLightPower + sumLightColor * sumLightPower) / a;
        if (sumLightPower >= 0)
          sumLightPower = diffLightPower + sqrt(sumLightPower) * (1 - diffLightPower);
        sumSpecPower = pow(sumSpecPower, 30.0f) * reflectivity;

        Color finColor;
        if (type == Material::mtDielectric)
        {
          float a = rayLen * normLen;
          float dropAngleCos = (a > VERY_SMALL_NUMBER) ? clamp(ray * -norm / a, 0.0f, 1.0f) : 0.0f;
          float finRefl = 0.2f + 0.8f * pow(1.0f - dropAngleCos, 3.0f);

          finColor = colorMul * (1.0f - finRefl) * color * sumLightPower * sumLightColor;
          finColor = finColor * (1.0f - sumSpecPower) + sumSpecColor * sumSpecPower;

          colorMul *= finRefl;
        }
        else
        {
          float finRefl = 0.8f;

          finColor = colorMul * (1.0f - finRefl) * color * sumLightPower * sumLightColor;
          finColor = finColor * (1.0f - sumSpecPower) + sumSpecColor * sumSpecPower;

          colorMul *= color * finRefl;
        }

        pixelColor += finColor;
        pixelColor.clamp();

        if (colorMul.r < 0.01f && colorMul.g < 0.01f && colorMul.b < 0.01f)
          break;

        origin = drop;
        ray = reflect.normalized() + randDir * (1.0f - reflectivity);
      }
      else
      {
        pixelColor = pixelColor + skybox.getTexelColor(ray) * colorMul * mainLightColor;
        pixelColor.clamp();
        break;
      }
    }
    aaPixelColor = aaPixelColor + pixelColor;
  }
  aaPixelColor = aaPixelColor / float(aan * aan);

  return aaPixelColor;
}

