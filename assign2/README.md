# Overview
CSCI 420 assignment1 by Yao Lin

Complete **all** basic requirements, **all** animation requirements and **all** extra requirements (except for interactive deform).

Open Source on github: https://github.com/matudinglin/Computer-Graphics-Projects

# Requirments Check List
## Basic Requirements Check List
- Press <kbd> 1 </kbd> switch to points mod, Press <kbd> 2 </kbd> switch to lines mod, Press <kbd> 3 </kbd> switch to triangles mod.
- Perspective view and z-buffer.
- Allow mouse control.
- Color the vertices using some smooth gradient.
- Screenshots for three required examples.
- Animation Requirements.
- README file.

## Animation Requirements Check List
- 20-30 seconds video clip

## Extra Credits Check List
- Support RGB input images.
- Render wireframe on top of solid triangles. Press <kbd> 4 </kbd> switch to this mod.
- Color the vertices based on color values taken from another image of equal size. Press <kbd> 5 </kbd> switch to this mod.
- Texturemap the surface with an arbitrary image. Press <kbd> 6 </kbd> switch to this mod.
- ~~Allow the user to interactively deform the landscape.~~

# Implementation
## Command Line Arguments
the format of command line arguments as follows,
    
    ./*.exe heightfield.jpg <colored-image.jpg> <texture.jpg>

The last two arguments are optional depends on whether need the colored triangle feature and texture feature.

## Core Functions
readVertex() is the core function, which read all the needed heightfield image, colored image with the same size and texture images into the array for future rendering.

It load point data into pointVertex and pointColor array, line data into lineVertex and lineColor array and so on.

## Draw Primitives
Define a enum, which is used to switch from different modes, there are 6 modes, including 3 mode required from basic requirments and 3 modes required from extra credits. As follows,

    typedef enum { MY_POINT, MY_LINE, MY_TRIANGLE, MY_LINE_TRIANGLE, MY_COLORED_TRIANGLE, MY_TEXTURE_TRIANGLE } DISPLAYTYPE;

Use OpenGL function glDrawArrays() to draw primitives, reduce the number of callback of opengl functions. E.g. Following code shows how to draw point height field.

    glVertexPointer(3, GL_FLOAT, 0, pointVertex.data());
    glColorPointer(3, GL_FLOAT, 0, pointColor.data());
    glDrawArrays(GL_POINTS, 0, pointVertex.size() / 3);

## Support color in input images
Define a enum to control the input image between RGB and Grayscale, as follows,

    typedef enum {GRAYSCALE, RGBSCALE} COLORSTATE;

Used the folling formula to compute the z value from RGB image
    
    z = 0.299 * r + 0.587 * g + 0.114 * b

For better performance, I output RGB image because it looks better than grayscale image

## Render wireframe on top of solid triangles
Use the following function to implement this features

    glPolygonOffset(1.0, 1.0);

## Color the vertices based on color values taken from another image of equal size
Read the equal size image and mixed the RGB from each pixel to the height filed image, example as below,

    _r = float(*coloredImage->data(w, h, 0, 0)) / 255.0f;
    _g = float(*coloredImage->data(w, h, 0, 1)) / 255.0f;
    _b = float(*coloredImage->data(w, h, 0, 2)) / 255.0f;

    coloredTriangleColor.push_back(z *  _r);
    coloredTriangleColor.push_back(z *  _g);
    coloredTriangleColor.push_back(z *  _b);

## Texturemap
Read the texture image and prepare for texture mapping

    // enable texture array for texture mapping
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureImage->width(),
		textureImage->height(), 0, GL_RGB, GL_UNSIGNED_BYTE,
		textureImage->data());

Texture mapping use glTexCoordPointer()

    	glEnable(GL_TEXTURE_2D);
		glDisable(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, textureVertex.data());
		glTexCoordPointer(2, GL_FLOAT, 0, textureCoor.data());
		glDrawArrays(GL_QUADS, 0, textureVertex.size() / 3);
		glEnable(GL_COLOR_ARRAY);
		glDisable(GL_TEXTURE_2D);