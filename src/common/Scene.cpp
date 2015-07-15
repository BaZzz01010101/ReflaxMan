#include "trace_math.h"
#include "Scene.h"
#include "Sphere.h"
#include "Triangle.h"

Scene::Scene()
{
}

Scene::Scene(const Color & diffLightColor, float diffLightPower)
{
  this->diffLightColor = diffLightColor;
  this->diffLightPower = diffLightPower;
  envColor = diffLightColor * diffLightPower;
}

Scene::~Scene()
{
  for (SCENE_OBJECTS::iterator it = sceneObjects.begin(); it != sceneObjects.end(); ++it)
    delete *it;

  for (SCENE_LIGHTS::iterator it = sceneLights.begin(); it != sceneLights.end(); ++it)
    delete *it;

  for (SCENE_TEXTURES::iterator it = sceneTextures.begin(); it != sceneTextures.end(); ++it)
    delete *it;
}

Sphere * Scene::addSphere(const Vector3 & center, float radius, const Material & material)
{
  Sphere * sphere = new Sphere(center, radius, material);
  sceneObjects.push_back(sphere);
  return sphere;
}

Triangle * Scene::addTriangle(const Vector3 & v1, const Vector3 & v2, const Vector3 & v3, const Material & material)
{
  Triangle * triangle = new Triangle(v1, v2, v3, material);
  sceneObjects.push_back(triangle);
  return triangle;
}

OmniLight * Scene::addLight(const Vector3 & origin, float radius, const Color & color, float power)
{
  envColor += color * power;
  OmniLight * light = new OmniLight(origin, radius, color, power);
  sceneLights.push_back(light);
  return light;
}

Texture * Scene::addTexture(const char * fileName)
{
  Texture * texture = new Texture(fileName);
  sceneTextures.push_back(texture);
  return texture;
}

bool Scene::setSkyboxTexture(const char * fileName)
{
  return skybox.loadTexture(fileName);
}

Color Scene::trace(Vector3 origin, Vector3 ray, int reflNumber) const
{
  const Vector3 randDir = Vector3::randomInsideSphere(1.0f);
  Color mulColor = Color(1.0f, 1.0f, 1.0f);
  Color pixelColor = Color(0.0f, 0.0f, 0.0f);

  // going deep up to maxReflections
  for (int refl = 0; refl < reflNumber; ++refl)
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
      const float rayLen = ray.length();
      const float normLen = norm.length();
      const float reflectLen = reflect.length();
      Color sumLightColor = Color(0.0f, 0.0f, 0.0f);
      Color sumSpecColor = Color(0.0f, 0.0f, 0.0f);

      // tracing each light source visibility
      for (SCENE_LIGHTS::const_iterator lt = sceneLights.begin(); lt != sceneLights.end(); ++lt)
      {
        const OmniLight * light = *lt;
        const Vector3 dropToLight = light->origin - drop;

        // check only if drop point faced to light source
        if (dropToLight * norm > VERY_SMALL_NUMBER)
        {
          // make randomization within a radius of light source for smooth shadows
          const float lightRadius = light->radius;
          const Vector3 dropToLightShadowRand = dropToLight + randDir * lightRadius;
          bool inShadow = false;

          // checking whether we are in the shadow of some scene object
          for (SCENE_OBJECTS::const_iterator obj = sceneObjects.begin();
            obj != sceneObjects.end();
            ++obj)
          {
            if (*obj != hitObject && 
              (*obj)->trace(drop, dropToLightShadowRand, NULL, NULL, NULL, NULL, NULL))
            {
              inShadow = true;
              break;
            }
          }

          // if we are not in the shadow - proceed illumination
          if (!inShadow)
          {
            // calc illumination from current light source
            const float dropToLightLen = dropToLight.length();
            const Color & lightColor = light->color;
            const float lightPower = light->power;
            float a = dropToLightLen * normLen;
            const float lightDropCos = (a > VERY_SMALL_NUMBER) ? dropToLight * norm / a : 0.0f;

            if (lightPower > VERY_SMALL_NUMBER)
              sumLightColor += lightColor * lightDropCos * lightPower;

            // calc specular reflection from current light source
            a = dropToLight.sqLength();
            const float lightAngularRadiusSqCos = (a > VERY_SMALL_NUMBER) ? 1.0f - lightRadius * lightRadius / a : 0.0f;

            if (lightAngularRadiusSqCos > 0)
            {
              const Vector3 dropToLightRand = dropToLight.normalized() + randDir * (1.0f - dropMaterial.reflectivity);
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

      const float reflectivity = dropMaterial.reflectivity;
      const Color & color = dropMaterial.color;
      const Material::Type type = dropMaterial.type;

      sumLightColor = diffLightColor * diffLightPower + sumLightColor;

      Color finColor;
      if (type == Material::mtDielectric)
      {
        // for dielectric materials count reflectivity using rough approximation of the Fresnel curve
        const float a = rayLen * normLen;
        const float dropAngleCos = (a > VERY_SMALL_NUMBER) ? clamp(ray * -norm / a, 0.0f, 1.0f) : 0.0f;
        const float refl = 0.2f + 0.8f * pow(1.0f - dropAngleCos, 3.0f);

        finColor = (1.0f - refl) * color * sumLightColor + sumSpecColor;
        finColor *= mulColor;

        // multiply colorMul with counted reflectivity to reduce subsequent reflections impact
        mulColor *= refl;
      }
      else
      {
        // for metal materials roughly set reflectivity equal to 0.8 according to Fresnel curve 
        const float refl = 0.8f;

        finColor = (1.0f - refl) * color * sumLightColor + sumSpecColor;
        finColor *= mulColor;

        // multiply colorMul with counted reflectivity and matherial color to reduce impact and colorize subsequent reflections
        mulColor *= color * refl;
      }

      // summarize reflected colors
      pixelColor += finColor;
      pixelColor.clamp();

      // exit if color multiplicator too low and counting of subsequent reflection have no sense
      if (mulColor.r < 0.01f && mulColor.g < 0.01f && mulColor.b < 0.01f)
        break;

      // select reflection as new ray for tracing and randomize it depending on reflectivity of object
      origin = drop;
      ray = reflect.normalized() + randDir * (1.0f - reflectivity);
    }
    else // have no intersections
    {
      pixelColor = pixelColor + mulColor * skybox.getTexelColor(ray) * envColor;
      pixelColor.clamp();
      break;
    }
  }
  return pixelColor;
}
