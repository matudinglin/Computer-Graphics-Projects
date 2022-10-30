# OVERVIEW
CSCI 420 assignment2 by Yao Lin

Complete **all** basic requirements, animation requirements and **24 points** extra requirements.

Open Source on github: https://github.com/matudinglin/Computer-Graphics-Projects

# USAGE
- Press <kbd> 1 </kbd> to reset camera, Press <kbd> 2 </kbd> to run or stop.
- Use <kbd> Mouse </kbd> to rotate
- Use <kbd> Shift </kbd>, <kbd> Ctrl </kbd> and <kbd> Alt </kbd>  to change view.
- Use <kbd> Esc </kbd> to quit.

# REQUIRMENTS
## Basic Requirements Check List
- Complete all levels
  - Level 1: use brute force to properly render Catmull-Rom splines to represent track.
  - Level 2 & 3: render a texture-mapped ground and sky.
  - Level 4: Move the camera at a reasonable speed in a continuous path and orientation along the coaster.
  - Level 5: Render a rail cross-section.
- Render the coaster in an interesting manner.
- Run at interactive frame rates
  - Use a frame rate counter to show FPS of the program.
  - Run at 60 FPS steadily.
- Code & video & README file

## Animation Requirements Check List
- video clip recorded by OBS

## Extra Credits Check List
- Render a T-shaped rail cross-section.
- Render double rail.
- Make track circular and close it with C1 continuity.
- Add OpenGL lighting to make your coaster look more realistic
- Draw crossbars and the support structure of the rail.
- Draw splines using recursive subdivision instead of using brute force.
- Load and render a Stanford Bunny into the scene.

# IMPLEMENTATION
## Helper functions
- struct Vec3f: 3 float value vector which supports most vector operation, including addition, subtraction, multiplication and normalization etc.
- struct Vec2f: 2 float value vector which supports most vector operation.

## Rendering the Environment
- Load texture image using readImage() function and store them as cv::Mat3b, convert color space from BGR to RGB. 
- Use envGenerator() to compute coordinates for texture mapping
  - Store the coordinates of skybox cube into g_envVertex
  - Store the coordinates of texture into g_envTextureVertex
- Setup texture parameters and use vertex array for textrue mapping
  ```
    glBindTexture(GL_TEXTURE_2D, g_textures[1]);
    glVertexPointer(3, GL_FLOAT, 0, g_envSVertex.data());
    glTexCoordPointer(2, GL_FLOAT, 0,g_envSTextureVertex.data());
    glDrawArrays(GL_QUADS, 0, g_envSVertex.size());
  ```

## Compute Spline Points
- Use splineGenerator() to compute spline points.
- Generate spline points using two methods.
  - Brute force
    - compute based on formula in slides.
  - Subdivide 
    - recursively call subDivide() function to generate spline points.
- Store spline points into g_splineVertex array.

## Compute Tangents, Normals and Binormals
- Use railGenerator() to compute tangent, normal and binormal of each points.
  - Compute tangent using the formula from slides. 
  - Compute binormal by compute the cross product between tangent and (0, 0, 1).
  - Compute normal by compute the cross product between tangent and binormal.
- Store them into g_splineTangent, g_splineNormal, g_splineBinormal respectively.

## Generate Rails
- Use railGenerator() to generate T-shaped rail cross-section, double rail and rail support structure.
  - Compute the left and right rails by applying an offset to the points, then compute 8 vertices of the rectangle that construct the rails. T-shaped rail cross-section is achieved by adding another rail which have an offset and different size from the original rails.
  - Compute the middle crossbars by applying an offset to the points and compute them with some intervals.
  - Compute the rail support structure by making one points into the ground y value, and compute them with some intervals.
- Store them into g_RailVertex respectively.

## Update Camera
- Use glutTimerFunc() function to call idleFunc() in 60FPS to make the animation more smooth and realistic.
- idleFunc() function will call updateCamera() function with current running position 
- updateCamera() will update the lookAt vector based on the given current position
  ```
    Vec3f eye = g_splineVertex[pos] + g_splineNormal[pos] * 0.3f;
    Vec3f center = eye + g_splineTangent[pos];
    Vec3f up = g_splineNormal[pos];

    g_lookAt = { eye.x, eye.y, eye.z,
        center.x, center.y, center.z,
        up.x, up.y, up.z };
  ```

## Load Stanford Bunny and Tyra into scene
-  OBJ Loader library is from https://github.com/Bly7/OBJ-Loader.
-  Use OBJ Loader to load .obj file into program.
-  Then call objGenerator() function to store the vertex, color and normal into vectors for rendering
-  Display the object in the front of the screen when running the roller coaster, simulating the minecart.

## Lighting
- Light is applying to rails and object
- Setup lighting parameters, such as ambient light, diffusive light, specular light and light position
- The normals of the rails are computed in railGenerator() function by adding the tangent, the normal and the binormal of the points.