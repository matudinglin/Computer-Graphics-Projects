# OVERVIEW
CSCI 420 assignment 3 by Yao Lin

Complete **all** basic requirements and **15 points** extra requirements.

Open Source on github: https://github.com/matudinglin/Computer-Graphics-Projects

# REQUIRMENTS
## Basic Requirements Check List
- *Triangle intersection (20 points)*
- Sphere intersection (20 points)
- Triangle Phong shading (15 points)
- Sphere Phong shading (15 points)
- Shadows rays (15 points)
- Still images (15 points)

## Extra Credits Check List
- **Recursive reflection (5 pts)**
  - Use formula: **(1 - ks) * localPhongColor + ks * colorOfReflectedRay**.
  - Can be turn On/Off using global variable: ***const boolrecursiveReflectionOn***.
  - Change max number of reflections using global variable: ***const int maxReflection***.
- **Good antialiasing (5 pts)**
  - Anti-aliaing by using Supersampling method: Multi-Sample Anti-Aliasing (MSAA).
  - Multiple locations are sampled within every pixel, and each of those samples is fully rendered and combined with the others to produce the pixel that is ultimately displayed.
  - Can be turn On/Off using global variable: ***const bool antialiasingOn***.
  - Change anti-aliaing sample rate using global variable: ***const int sampleRate***.
- **Soft shadows (5 pts)**
  - Sample every point light source into multiple point light source to simulate a sizeable light source. (Another way is generating multiple shadow rays to one point light source).
  - Can be turn On/Off using global variable: ***const bool softShadowsOn***.
  - Change the approximate accuracy of soft shadow using variable: ***int subLightPerPointLight***.

# RESULTS
## Basic Requirements Results
Render all 5 scenes to satisfy the basic requirements.
- ```./RESULTS/spheres-result.jpg```
- ```./RESULTS/SIGGRAPH-result.jpg```
- ```./RESULTS/table-result.jpg```
- ```./RESULTS/test1.jpg```
- ```./RESULTS/test2.jpg```

## Extra Credits Results
Render 5 extra scenes to show features for extra credits.
- ```./RESULTS/table-result-antialiasingOn10x10.jpg```
  - table scene with antialiasing feature.
- ```./RESULTS/table-result-softShadowOn100.jpg```
  - table scene with soft shadow feature.
- ```./RESULTS/table-result-softShadowOn100-recursiveReflectionOn3-antialiasingOn2x2.jpg```
  - table scene with antialiasing, soft shadow and recursive reflection features.
- ```./RESULTS/spheres-result-recursiveReflectionOn5.jpg```
  - spheres scene with recursive reflection feature.
- ```./RESULTS/SIGGRAPH-result-softShadowOn100-recursiveReflectionOn3-antialiasingOn2x2.jpg```
  - SIGGRAPH scene with antialiasing, soft shadow and recursive reflection features.


# IMPLEMENTATION
## Structures & helper Functions
- struct Vec3d
  - A OpenCV::vec3d like struct.
  - 3 double value vector which supports most vector operation, including addition, subtraction, multiplication, clamping and normalization etc.
- vector-related functions
  - dot(const Vec3d& v1, const Vec3d& v2)
  - cross(const Vec3d& v1, const Vec3d& v2)
  - clamp(double& val)
- struct Ray
  - store basic operation and porperties of a ray
  - including position and direction
  - including two memebr functions use to get hit position with a sphere and a triangle.

## Triangle Intersection 
- Member function in struct Ray:
  ```
    bool getHitPosition(const Triangle& t, Vec3d &hitPos)
    {
      compute triangle normal for ray-plane intersection;
      if (ray is parallel with the plane) 
        return false;
      use formula to compute hit time;
      if (hit piont is backward) 
        return false;
      use formula to compute hit point;
      if (hit point is inside triangle) 
        return true;
      else 
        return false;
    }
  ```
- hitTime = (dot(normal, trianlge_vertex) - dot(normal, ray_position)) / dot(normal, ray_direction);
- hitPosition = ray_position + ray_direction * hitTime;

## Sphere Intersection 
- Member function in struct Ray:
  ```
    bool getHitPosition(const Sphere& s, Vec3d &hitPos)
    {
      compute a, b, c and delta value for solving binary equation;
      if (ray is parallel with the plane / no solution for euqation) 
        return false;
      use formula to compute hit time;
      if (hit piont is backward) 
        return false;
      use formula to compute hit point;
      if (hit point is inside sphere) 
        return true;
      else 
        return false;
    }
  ```
- hitTime = (-b + sqrt(delta)) / 2 & hitTime = (-b - sqrt(delta)) / 2
- hitPosition = ray_position + ray_direction * hitTime;


## Triangle Phong Shading
- Using function overloading to implement Phong Shading feature:
  ```
    bool getHitPosition(const Triangle& t, Vec3d &hitPos)
    {
      use barycentric coordinates to interpolate normal;
      compute diffuse parameters;
      compute specular parameters;
      compute Phong Shading;
      return color;
    }
  ```
- Compute barycentric value ***alpha, beta, gamma*** by computing the area of triangles and dividing.
- compute phong shading color using formula in assignment detials: ***I =
lightColor * (kd * (L dot N) + ks * (R dot V) ^ α)***

## Sphere Phong Shading
- Using function overloading to implement Phong Shading feature
  ```
    bool getHitPosition(const Sphere& s, Vec3d &hitPos)
    {
      compute diffuse parameters;
      compute specular parameters;
      compute Phong Shading;
      return color;
    }
  ```
- compute phong shading color using formula in assignment detials: ***I =
lightColor * (kd * (L dot N) + ks * (R dot V) ^ α)***

## Shadows Rays
- Determine if the point is in shadow by launching a shadow ray to each of the lights. If the point is in shadow, set its color to black.
- In function ***computeTrianglesIntersection()***, which compute ray with all triangles, determine shadow and phong shading:
  ```
    for each light source
      generate a shadow ray from hit point;
      for each triangles and spheres
        if (shadow ray hits object) 
          set color to black;
      if (doesn't hit any object) 
        use Phong Shading;
  ```
- Check if the shadow ray hits an object before hit the lights.

## Antialiasing
- ***draw_scene()*** function will call ***computePixelColor(x, y)*** function for every pixel in image with size WIDTH * HEIGHT. After adding the ambient light color and clamping, ***draw_scene()*** function call the OpenGL function to draw the pixel.
- In ***computePixelColor(x, y)*** function, there is a if statement to determine if the antialiasing feature will be turn ON/OFF:
  ```
    color computePixelColor(x, y)
    {
      if (antialiasingOn)
      {
        int samplePerPixel = sampleRate * sampleRate;
        generate samplePerPixel rays;
        accumulate all color from rays;
        return color / samplePerPixel;
      }
      else
      {
        generate one ray;
        compute the color from this ray;
        return the color;
      }
    }
  ```
- ***generateRay(double x, double y)*** function will take image coordinates and return the generated ray:
  ```
    Ray generateRay(x, y)
    {
      convert coordinate space to [0, 1];
      convert coordinate space to [-1, 1];
      convert coordinate space to image space, assuming z = -1;
      generate the ray;
      return ray;
    }
  ```
- ***computeRayColor(const Ray &ray)*** function will compute the color from this ray and return the final color:
  ```
    Vec3d computeRayColor(const Ray &ray)
    {
      computeSpheresIntersection(ray, color, hitPos);
      computeTrianglesIntersection(ray, color, hitPos);
      if (no hit) 
        return background color;
      else
        return color;
    }
  ```

## Recursive Reflection
- In ***computePixelColor(double x, double y)*** function, we compute the color from ray, if recursive reflection is ON, it will call ***recursivelyComputeRayColor(const Ray& ray, int reflectTime)*** to compute ray color.
- ***recursivelyComputeRayColor(const Ray& ray, int reflectTime)*** takes ray and the reflection time, then return the color:
  ```
    recursivelyComputeRayColor(const Ray& ray, int reflectTime)
    {
      if (reflectTime > maxReflection) return color;
      computeSpheresIntersection(ray, color, hitPos);
      computeTrianglesIntersection(ray, color, hitPos);
      compute ks and reflectRay;
      recursively call function and compute color;
    }
  ```
- Set a reflect rate to control the color influence from the reflection.

## Soft Shadows
- If soft shadow feature is ON, it will randomly generate ***subLightPerPointLight*** for every point light source to simulate soft shadow:
  ```
    for each original light source
    {
      for (int j = 0; j < subLightPerPointLight; ++j)
      {
        subLight.position = random sample based on original light source; 
        subLight.color = original color / subLightPerPointLight;
      }
    }
  ```
- Then doing the normal shadow ray to every new subLight to compute the shadow.

