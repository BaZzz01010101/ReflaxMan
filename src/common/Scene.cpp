#include "airly.h"

#include "Scene.h"
#include "Sphere.h"
#include "Triangle.h"


Scene::Scene(const wchar_t * exePath) 
{
  std::wstring skyboxTextureFileName = std::wstring(exePath) + L".\\textures\\skybox.tga";
  std::wstring planeTextureFileName = std::wstring(exePath) + L".\\textures\\himiya.tga";
  bool retVal = skybox.loadTexture(skyboxTextureFileName.c_str());
  assert(retVal);
  retVal = planeTexture.loadFromFile(planeTextureFileName.c_str());
  assert(retVal);

  camera = Camera(Vector3(7.427f, 3.494f, -3.773f), Vector3(6.5981f, 3.127f, -3.352f), Vector3(-0.320f, 0.930f, 0.180f), 1.05f);

  sceneLights.push_back(new OmniLight(Vector3(11.8e9f, 4.26e9f, 3.08e9f), 6.96e8f, Color(1.0f, 1.0f, 0.95f), 0.85f));
  //sceneLights.push_back(new OmniLight(Vector3(-1.26e9f, 11.8e9f, 1.08e9f), 6.96e8f, Color(1.0f, 0.5f, 0.5f), 0.2f));
  //sceneLights.push_back(new OmniLight(Vector3(11.8e9f, 4.26e9f, 3.08e9f), 6.96e9f, Color(1.0f, 1.0f, 0.95f), 0.85f));

  diffLightColor = { 0.95f, 0.95f, 1.0f };
  diffLightPower = 0.3f;

// set environment color to correct skybox texture depending on summary scene illumination
  envColor = diffLightColor * diffLightPower;
  for (SCENE_LIGHTS::const_iterator lt = sceneLights.begin(); lt != sceneLights.end(); ++lt)
    envColor += (*lt)->color * (*lt)->power;

  sceneObjects.push_back(new Sphere(Vector3(-1.25f, 1.5f, -0.25f), 1.5f, Material(Material::mtMetal, Color(1.0f, 1.0f, 1.0f), 1.0f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(0.15f, 1.0f, 1.75f), 1.0f, Material(Material::mtMetal, Color(1.0f, 1.0f, 1.0f), 0.9f, 0.0f)));

  sceneObjects.push_back(new Sphere(Vector3(-3.0f, 0.6f, -3.0f), 0.6f, Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.0f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(-0.5f, 0.5f, -2.5f), 0.5f, Material(Material::mtDielectric, Color(0.5f, 1.0f, 0.15f), 0.8f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(1.0f, 0.4f, -1.5f), 0.4f, Material(Material::mtDielectric, Color(0.0f, 0.5f, 1.0f), 0.99f, 0.0f)));

  sceneObjects.push_back(new Sphere(Vector3(1.8f, 0.4f, 0.1f), 0.4f, Material(Material::mtMetal, Color(1.0f, 0.65f, 0.45f), 0.99f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(1.7f, 0.5f, 1.9f), 0.5f, Material(Material::mtMetal, Color(1.0f, 0.90f, 0.60f), 0.8f, 0.0f)));
  sceneObjects.push_back(new Sphere(Vector3(0.6f, 0.6f, 4.2f), 0.6f, Material(Material::mtMetal, Color(0.9f, 0.9f, 0.9f), 0.1f, 0.0f)));

  Triangle* tr = NULL;

  tr = new Triangle(Vector3(-14.0f, 0.0f, -10.0f), Vector3(-14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, -10.0f), Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.9f, 0.0f));
  tr->setTexture(&planeTexture, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
  sceneObjects.push_back(tr);

  tr = new Triangle(Vector3(-14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, 10.0f), Vector3(14.0f, 0.0f, -10.0f), Material(Material::mtDielectric, Color(1.0f, 1.0f, 1.0f), 0.9f, 0.0f));
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
  assert(camera.fov > VERY_SMALL_NUMBER);
  assert(camera.fov < M_PI);

  if (x < 0 || y < 0 || aan <= 0 || camera.fov <= VERY_SMALL_NUMBER || camera.fov >= M_PI)
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

// going deep up to maxReflections
    for (int refl = 0; refl < maxReflections; ++refl)
    {
      float minDistance = FLT_MAX;
      SceneObject* hitObject = NULL;
      Vector3 drop, norm, reflect;
      Material dropMaterial;

// tracing intersections with all scene objects and select closest 
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

// if have intersection - proceed it
      if (hitObject)
      {
        float rayLen = ray.length();
        float normLen = norm.length();
        float reflectLen = reflect.length();
        Color sumLightColor = { 0.0f, 0.0f, 0.0f };
        Color sumSpecColor = { 0.0f, 0.0f, 0.0f };

// tracing each light source visibility
        for (SCENE_LIGHTS::const_iterator lt = sceneLights.begin(); lt != sceneLights.end(); ++lt)
        {
          OmniLight* light = *lt;

          Vector3 dropToLight = light->origin - drop;

          if (dropToLight * norm > VERY_SMALL_NUMBER)
          {
// make randomization within a radius of light source for smooth shadows
            float lightRadius = light->radius;
            Vector3 dropToLightShadowRand = dropToLight + randDir * lightRadius;
            bool inShadow = false;

// checking whether we are in the shadow of some scene object
            for (SCENE_OBJECTS::const_iterator obj = sceneObjects.begin();
              obj != sceneObjects.end();
              ++obj)
            {
              Vector3 curDrop, curNorm, curReflect;
              Color curColor;

              if (*obj != hitObject)
              {

                if ((*obj)->trace(drop, dropToLightShadowRand, NULL, NULL, NULL, NULL, NULL))
                {
                  inShadow = true;
                  break;
                }
              }
            }

// if we are not in the shadow - proceed illumination
            if (!inShadow)
            {
              float dropToLightLen = dropToLight.length();

// calc illumination from current light source
              Color & lightColor = light->color;
              float lightPower = light->power;
              float a = dropToLightLen * normLen;
              float lightDropCos = (a > VERY_SMALL_NUMBER) ? dropToLight * norm / a : 0.0f;

              if (lightPower > VERY_SMALL_NUMBER)
                sumLightColor += lightColor * lightDropCos * lightPower;

// calc specular reflection from current light source
              a = dropToLight.sqLength();
              float lightAngularRadiusSqCos = (a > VERY_SMALL_NUMBER) ? 1.0f - lightRadius * lightRadius / a : 0.0f;

              if (lightAngularRadiusSqCos > 0)
              {
                Vector3 dropToLightRand = dropToLight.normalized() + randDir * (1.0f - dropMaterial.reflectivity);
                a = dropToLightRand.length() * reflectLen;
                float reflectSpecularCos = (a > VERY_SMALL_NUMBER) ? dropToLightRand * reflect / a : 0.0f;
                reflectSpecularCos = clamp(reflectSpecularCos + (1.0f - sqrtf(lightAngularRadiusSqCos)), 0.0f, 1.0f);

                if (reflectSpecularCos > VERY_SMALL_NUMBER)
                {
                  float specPower = reflectSpecularCos;

                  if (lightRadius > VERY_SMALL_NUMBER)
                  {
                    specPower = pow(specPower, 1 + 3 * dropMaterial.reflectivity * dropToLightLen / lightRadius) * dropMaterial.reflectivity;
                    sumSpecColor = sumSpecColor + lightColor * specPower;
                  }
                }
              }
            }
          }
        }

        float reflectivity = dropMaterial.reflectivity;
        Color & color = dropMaterial.color;
        Material::Type type = dropMaterial.type;

        sumLightColor = diffLightColor * diffLightPower + sumLightColor;

        Color finColor;
        if (type == Material::mtDielectric)
        {
// for dielectric materials count reflectivity using rough approximation of the Fresnel curve
          float a = rayLen * normLen;
          float dropAngleCos = (a > VERY_SMALL_NUMBER) ? clamp(ray * -norm / a, 0.0f, 1.0f) : 0.0f;
          float refl = 0.2f + 0.8f * pow(1.0f - dropAngleCos, 3.0f);

          finColor = (1.0f - refl) * color * sumLightColor + sumSpecColor;
          finColor *= colorMul;
          
// multiply colorMul with counted reflectivity to reduce subsequent reflections impact
          colorMul *= refl;
        }
        else
        {
// for metal materials roughly set reflectivity equal to 0.8 according to Fresnel curve 
          float refl = 0.8f;

          finColor = (1.0f - refl) * color * sumLightColor + sumSpecColor;
          finColor *= colorMul;
          
// multiply colorMul with counted reflectivity and matherial color to reduce impact and colorize subsequent reflections
          colorMul *= color * refl;
        }

// summarize reflected colors
        pixelColor += finColor;
        pixelColor.clamp();

// exit if color multiplicator too low and counting of subsequent reflection have no sense
        if (colorMul.r < 0.01f && colorMul.g < 0.01f && colorMul.b < 0.01f)
          break;

// select reflection as new ray for tracing and randomize it depending on reflectivity of object
        origin = drop;
        ray = reflect.normalized() + randDir * (1.0f - reflectivity);
      }
      else // have no intersections
      {
        pixelColor = pixelColor + colorMul * skybox.getTexelColor(ray) * envColor;
        pixelColor.clamp();
        break;
      }
    }

// summarize supersampled colors
    aaPixelColor = aaPixelColor + pixelColor;
  }
  aaPixelColor = aaPixelColor / float(aan * aan);

  return aaPixelColor;
}

