// assign1.cpp : Defines the entry point for the console application.
//

/*
  CSCI 420 Computer Graphics
  Assignment 1: Height Fields
  Yao Lin
*/

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "CImg-2.3.5\CImg.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace cimg_library;

int g_iMenuId;

int g_vMousePos[2] = { 0, 0 };
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

typedef enum { MY_POINT, MY_LINE, MY_TRIANGLE, MY_LINE_TRIANGLE, MY_COLORED_TRIANGLE, MY_TEXTURE_TRIANGLE } DISPLAYTYPE;
DISPLAYTYPE displayType = MY_POINT;

typedef enum {GRAYSCALE, RGBSCALE} COLORSTATE;
COLORSTATE colorState = RGBSCALE;

/* State of the world */
float g_vLandRotate[3] = { 0.0, 0.0, 0.0 };
float g_vLandTranslate[3] = { 0.0, 0.0, 0.0 };
float g_vLandScale[3] = { 1.0, 1.0, 1.0 };

CImg<unsigned char>* g_pHeightData;
CImg<unsigned char>* coloredImage;

CImg<unsigned char>* textureImage;
static GLuint texName;

/* Window properties*/
int windowWidth = 1280;
int windowHeight = 720;
string windowTitle = "CSCI 420 assignment 1 - Yao Lin";

/* Vertex vector */
vector<GLfloat> pointVertex, pointColor;
vector<GLfloat> lineVertex, lineColor;
vector<GLfloat> triangleVertex, triangleColor;
vector<GLfloat> darkLineColor;
vector<GLfloat> coloredTriangleColor;
vector<GLfloat> textureVertex, textureCoor;
bool coloredImageLoaded = false;
bool textureImageLoaded = false;

GLfloat mirrorTransfrom[] = { -1,0,0,0,
							   0,1,0,0,
							   0,0,1,0,
							   0,0,0,1 };

/* Write a screenshot to the specified filename */
void saveScreenshot(char* filename)
{
	int i, j;

	if (filename == NULL)
		return;

	/* Allocate a picture buffer */
	CImg<unsigned char> in(640, 480, 1, 3, 0);

	printf("File to save to: %s\n", filename);

	for (i = 479; i >= 0; i--) {
		glReadPixels(0, 479 - i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
			in.data());
	}

	if (in.save_jpeg(filename))
		printf("File saved Successfully\n");
	else
		printf("Error in Saving\n");

}

void readVertex()
{
	GLfloat width = g_pHeightData->width();
	GLfloat height = g_pHeightData->height();

	for (int w = 0; w < width - 1; ++w)
	{
		for (int h = 0; h < height - 1; ++h)
		{
			GLfloat x, y, z, x1, y1, z1, x2, y2, z2, x3, y3, z3;
			GLfloat r, g, b, r1, g1, b1, r2, g2, b2, r3, g3, b3;

			x = w;
			y = h;

			x1 = w + 1;
			y1 = h;

			x2 = w;
			y2 = h + 1;

			x3 = w + 1;
			y3 = h + 1;

			if (colorState == GRAYSCALE)
			{
				z = r = g = b = float(*g_pHeightData->data(w, h, 0)) / 255.0f;
				z1 = r1 = g1 = b1 = float(*g_pHeightData->data(w + 1, h, 0)) / 255.0f;
				z2 = r2 = g2 = b2 = float(*g_pHeightData->data(w, h + 1, 0)) / 255.0f;
				z3 = r3 = g3 = b3 = float(*g_pHeightData->data(w + 1, h + 1, 0)) / 255.0f;
			}
			else if (colorState == RGBSCALE)
			{
				r = float(*g_pHeightData->data(w, h, 0, 0)) / 255.0f;
				g = float(*g_pHeightData->data(w, h, 0, 1)) / 255.0f;
				b = float(*g_pHeightData->data(w, h, 0, 2)) / 255.0f;
				z = 0.299 * r + 0.587 * g + 0.114 * b;

				r1 = float(*g_pHeightData->data(w + 1, h, 0, 0)) / 255.0f;
				g1 = float(*g_pHeightData->data(w + 1, h, 0, 1)) / 255.0f;
				b1 = float(*g_pHeightData->data(w + 1, h, 0, 2)) / 255.0f;
				z1 = 0.299 * r1 + 0.587 * g1 + 0.114 * b1;

				r2 = float(*g_pHeightData->data(w, h + 1, 0, 0)) / 255.0f;
				g2 = float(*g_pHeightData->data(w, h + 1, 0, 1)) / 255.0f;
				b2 = float(*g_pHeightData->data(w, h + 1, 0, 2)) / 255.0f;
				z2 = 0.299 * r2 + 0.587 * g2 + 0.114 * b2;

				r3 = float(*g_pHeightData->data(w + 1, h + 1, 0, 0)) / 255.0f;
				g3 = float(*g_pHeightData->data(w + 1, h + 1, 0, 1)) / 255.0f;
				b3 = float(*g_pHeightData->data(w + 1, h + 1, 0, 2)) / 255.0f;
				z3 = 0.299 * r3 + 0.587 * g3 + 0.114 * b3;
			}

			// Read points
			pointVertex.push_back(x);
			pointVertex.push_back(y);
			pointVertex.push_back(z);

			pointColor.push_back(r);
			pointColor.push_back(g);
			pointColor.push_back(b);

			// Read lines
			// 1
			lineVertex.push_back(x);
			lineVertex.push_back(y);
			lineVertex.push_back(z);
			lineColor.push_back(r);
			lineColor.push_back(g);
			lineColor.push_back(b);

			lineVertex.push_back(x1);
			lineVertex.push_back(y1);
			lineVertex.push_back(z1);
			lineColor.push_back(r1);
			lineColor.push_back(g1);
			lineColor.push_back(b1);

			// 2
			lineVertex.push_back(x);
			lineVertex.push_back(y);
			lineVertex.push_back(z);
			lineColor.push_back(r);
			lineColor.push_back(g);
			lineColor.push_back(b);

			lineVertex.push_back(x2);
			lineVertex.push_back(y2);
			lineVertex.push_back(z2);
			lineColor.push_back(r2);
			lineColor.push_back(g2);
			lineColor.push_back(b2);

			// 3
			lineVertex.push_back(x1);
			lineVertex.push_back(y1);
			lineVertex.push_back(z1);
			lineColor.push_back(r1);
			lineColor.push_back(g1);
			lineColor.push_back(b1);

			lineVertex.push_back(x2);
			lineVertex.push_back(y2);
			lineVertex.push_back(z2);
			lineColor.push_back(r2);
			lineColor.push_back(g2);
			lineColor.push_back(b2);

			// 4
			lineVertex.push_back(x1);
			lineVertex.push_back(y1);
			lineVertex.push_back(z1);
			lineColor.push_back(r1);
			lineColor.push_back(g1);
			lineColor.push_back(b1);

			lineVertex.push_back(x3);
			lineVertex.push_back(y3);
			lineVertex.push_back(z3);
			lineColor.push_back(r3);
			lineColor.push_back(g3);
			lineColor.push_back(b3);

			// 5
			lineVertex.push_back(x2);
			lineVertex.push_back(y2);
			lineVertex.push_back(z2);
			lineColor.push_back(r2);
			lineColor.push_back(g2);
			lineColor.push_back(b2);

			lineVertex.push_back(x3);
			lineVertex.push_back(y3);
			lineVertex.push_back(z3);
			lineColor.push_back(r3);
			lineColor.push_back(g3);
			lineColor.push_back(b3);

			// Read triangles
			// 1
			triangleVertex.push_back(x);
			triangleVertex.push_back(y);
			triangleVertex.push_back(z);
			triangleColor.push_back(r);
			triangleColor.push_back(g);
			triangleColor.push_back(b);

			triangleVertex.push_back(x1);
			triangleVertex.push_back(y1);
			triangleVertex.push_back(z1);
			triangleColor.push_back(r1);
			triangleColor.push_back(g1);
			triangleColor.push_back(b1);

			triangleVertex.push_back(x2);
			triangleVertex.push_back(y2);
			triangleVertex.push_back(z2);
			triangleColor.push_back(r2);
			triangleColor.push_back(g2);
			triangleColor.push_back(b2);

			// 2
			triangleVertex.push_back(x1);
			triangleVertex.push_back(y1);
			triangleVertex.push_back(z1);
			triangleColor.push_back(r1);
			triangleColor.push_back(g1);
			triangleColor.push_back(b1);

			triangleVertex.push_back(x2);
			triangleVertex.push_back(y2);
			triangleVertex.push_back(z2);
			triangleColor.push_back(r2);
			triangleColor.push_back(g2);
			triangleColor.push_back(b2);

			triangleVertex.push_back(x3);
			triangleVertex.push_back(y3);
			triangleVertex.push_back(z3);
			triangleColor.push_back(r3);
			triangleColor.push_back(g3);
			triangleColor.push_back(b3);

			// Read colored lines
			if (colorState == GRAYSCALE)
			{

				// 1
				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z);

				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z1);

				// 2
				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z);

				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z2);

				// 3
				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z1);

				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z2);

				// 4                    
				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z3);

				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z2);

				// 5                    
				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z1);

				darkLineColor.push_back(0);
				darkLineColor.push_back(0);
				darkLineColor.push_back(z3);
			}

			// Read colored triangle color
			if (colorState == GRAYSCALE && coloredImageLoaded)
			{
				GLfloat _r, _g, _b, _r1, _g1, _b1, _r2, _g2, _b2, _r3, _g3, _b3;

				_r = float(*coloredImage->data(w, h, 0, 0)) / 255.0f;
				_g = float(*coloredImage->data(w, h, 0, 1)) / 255.0f;
				_b = float(*coloredImage->data(w, h, 0, 2)) / 255.0f;

				_r1 = float(*coloredImage->data(w + 1, h, 0, 0)) / 255.0f;
				_g1 = float(*coloredImage->data(w + 1, h, 0, 1)) / 255.0f;
				_b1 = float(*coloredImage->data(w + 1, h, 0, 2)) / 255.0f;

				_r2 = float(*coloredImage->data(w, h + 1, 0, 0)) / 255.0f;
				_g2 = float(*coloredImage->data(w, h + 1, 0, 1)) / 255.0f;
				_b2 = float(*coloredImage->data(w, h + 1, 0, 2)) / 255.0f;

				_r3 = float(*coloredImage->data(w + 1, h + 1, 0, 0)) / 255.0f;
				_g3 = float(*coloredImage->data(w + 1, h + 1, 0, 1)) / 255.0f;
				_b3 = float(*coloredImage->data(w + 1, h + 1, 0, 2)) / 255.0f;

				// 1
				coloredTriangleColor.push_back(z *  _r);
				coloredTriangleColor.push_back(z *  _g);
				coloredTriangleColor.push_back(z *  _b);
													
				coloredTriangleColor.push_back(z1 * _r1);
				coloredTriangleColor.push_back(z1 * _g1);
				coloredTriangleColor.push_back(z1 * _b1);
													
				coloredTriangleColor.push_back(z2 * _r2);
				coloredTriangleColor.push_back(z2 * _g2);
				coloredTriangleColor.push_back(z2 * _b2);
													
				// 2								
				coloredTriangleColor.push_back(z1 * _r1);
				coloredTriangleColor.push_back(z1 * _g1);
				coloredTriangleColor.push_back(z1 * _b1);
													
				coloredTriangleColor.push_back(z2 * _r2);
				coloredTriangleColor.push_back(z2 * _g2);
				coloredTriangleColor.push_back(z2 * _b2);
													
				coloredTriangleColor.push_back(z3 * _r3);
				coloredTriangleColor.push_back(z3 * _g3);
				coloredTriangleColor.push_back(z3 * _b3);
			}

			// Read texture Vertex
			if (colorState == GRAYSCALE && textureImageLoaded)
			{
				textureVertex.push_back(x);
				textureVertex.push_back(y);
				textureVertex.push_back(z);

				textureVertex.push_back(x1);
				textureVertex.push_back(y1);
				textureVertex.push_back(z1);

				textureVertex.push_back(x3);
				textureVertex.push_back(y3);
				textureVertex.push_back(z3);

				textureVertex.push_back(x2);
				textureVertex.push_back(y2);
				textureVertex.push_back(z2);
			}
			
			// Read texture coordinates
			if (colorState == GRAYSCALE && textureImageLoaded)
			{
				textureCoor.push_back(x /(width-1));
				textureCoor.push_back(y /(height - 1));

				textureCoor.push_back(x1 / (width - 1));
				textureCoor.push_back(y1 / (height - 1));

				textureCoor.push_back(x3 / (width - 1));
				textureCoor.push_back(y3 / (height - 1));

				textureCoor.push_back(x2 / (width - 1));
				textureCoor.push_back(y2 / (height - 1));
			}
		}
	}
}


void myinit()
{
	/* setup gl view here */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	// enable vertex and color array for rendering
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
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
	//read vertex data 
	readVertex();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPointSize(1);
	gluLookAt(0.0, 0.0, 1.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);

	// mouse control
	glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
	glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
	glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
	glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

	// adjust view
	glMultMatrixf(mirrorTransfrom);
	glRotatef(180, 0, 0, 1);
	glScalef(1.6f, 1.6f, 1);
	glScalef(1.0f/g_pHeightData->width(), 1.0f/g_pHeightData->height(), 0.7);
	glTranslatef(-g_pHeightData->width()/2, -g_pHeightData->height()/2, 0);

	switch (displayType)
	{
	case MY_POINT:
		glVertexPointer(3, GL_FLOAT, 0, pointVertex.data());
		glColorPointer(3, GL_FLOAT, 0, pointColor.data());
		glDrawArrays(GL_POINTS, 0, pointVertex.size() / 3);
		break;

	case MY_LINE:
		glVertexPointer(3, GL_FLOAT, 0, lineVertex.data());
		glColorPointer(3, GL_FLOAT, 0, lineColor.data());
		glDrawArrays(GL_LINES, 0, lineVertex.size() / 3);
		break;

	case MY_TRIANGLE:
		glVertexPointer(3, GL_FLOAT, 0, triangleVertex.data());
		glColorPointer(3, GL_FLOAT, 0, triangleColor.data());
		glDrawArrays(GL_TRIANGLES, 0, triangleVertex.size() / 3);
		break;

	case MY_LINE_TRIANGLE:
		glVertexPointer(3, GL_FLOAT, 0, lineVertex.data());
		glColorPointer(3, GL_FLOAT, 0, darkLineColor.data());
		glDrawArrays(GL_LINES, 0, lineVertex.size() / 3);
		glPolygonOffset(1.0, 1.0);
		glVertexPointer(3, GL_FLOAT, 0, triangleVertex.data());
		glColorPointer(3, GL_FLOAT, 0, triangleColor.data());
		glDrawArrays(GL_TRIANGLES, 0, triangleVertex.size() / 3);
		break;

	case MY_COLORED_TRIANGLE:
		glVertexPointer(3, GL_FLOAT, 0, triangleVertex.data());
		glColorPointer(3, GL_FLOAT, 0, coloredTriangleColor.data());
		glDrawArrays(GL_TRIANGLES, 0, triangleVertex.size() / 3);
		break;

	case MY_TEXTURE_TRIANGLE:
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_COLOR_ARRAY);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glVertexPointer(3, GL_FLOAT, 0, textureVertex.data());
		glTexCoordPointer(2, GL_FLOAT, 0, textureCoor.data());
		glDrawArrays(GL_QUADS, 0, textureVertex.size() / 3);

		glEnable(GL_COLOR_ARRAY);
		glDisable(GL_TEXTURE_2D);
		break;
	}

	glutSwapBuffers(); // double buffer flush
}

void menufunc(int value)
{
	switch (value)
	{
	case 0:
		exit(0);
		break;
	}
}

void doIdle()
{
	/* do some stuff... */

	/* make the screen update */
	glutPostRedisplay();
}

/* converts mouse drags into information about
rotation/translation/scaling */
void mousedrag(int x, int y)
{
	int vMouseDelta[2] = { x - g_vMousePos[0], y - g_vMousePos[1] };

	switch (g_ControlState)
	{
	case TRANSLATE:
		if (g_iLeftMouseButton)
		{
			g_vLandTranslate[0] += vMouseDelta[0] * 0.01;
			g_vLandTranslate[1] -= vMouseDelta[1] * 0.01;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandTranslate[2] += vMouseDelta[1] * 0.01;
		}
		break;
	case ROTATE:
		if (g_iLeftMouseButton)
		{
			g_vLandRotate[0] += vMouseDelta[1];
			g_vLandRotate[1] += vMouseDelta[0];
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandRotate[2] += vMouseDelta[1];
		}
		break;
	case SCALE:
		if (g_iLeftMouseButton)
		{
			g_vLandScale[0] *= 1.0 + vMouseDelta[0] * 0.01;
			g_vLandScale[1] *= 1.0 - vMouseDelta[1] * 0.01;
		}
		if (g_iMiddleMouseButton)
		{
			g_vLandScale[2] *= 1.0 - vMouseDelta[1] * 0.01;
		}
		break;
	}
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		g_iLeftMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_MIDDLE_BUTTON:
		g_iMiddleMouseButton = (state == GLUT_DOWN);
		break;
	case GLUT_RIGHT_BUTTON:
		g_iRightMouseButton = (state == GLUT_DOWN);
		break;
	}

	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		g_ControlState = TRANSLATE;
		break;
	case GLUT_ACTIVE_SHIFT:
		g_ControlState = SCALE;
		break;
	default:
		g_ControlState = ROTATE;
		break;
	}

	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '1':
		displayType = MY_POINT;
		break;
	case '2':
		displayType = MY_LINE;
		break;
	case '3':
		displayType = MY_TRIANGLE;
		break;
	case '4':
		if (colorState == GRAYSCALE)
			displayType = MY_LINE_TRIANGLE;
		break;
	case '5':
		if (colorState == GRAYSCALE && coloredImageLoaded)
			displayType = MY_COLORED_TRIANGLE;
		break;
	case '6':
		if (colorState == GRAYSCALE && textureImageLoaded)
			displayType = MY_TEXTURE_TRIANGLE;
		break;
	default:
		break;
	}
}

void reshape(int w, int h)
{
	// setup image size
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// setup camera
	glFrustum(-0.1, 0.1,
		-float(h) / (10.0 * float(w)),
		float(h) / (10.0 * float(w)), 0.03, 1000.0);
	// gluOrtho2D(-2.0, 2.0, 
	//     -2.0*float(h)/(float(w)), 
	//      2.0*float(h)/(float(w)));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("usage: %s heightfield.jpg\n", argv[0]);
		exit(1);
	}
	
	cout << "Loading heigt data..." << endl;
	g_pHeightData = new CImg<unsigned char>((char*)argv[1]);
	if (!g_pHeightData)
	{
		printf("error reading %s.\n", argv[1]);
		exit(1);
	}

	if (argc > 2)
	{
		cout << "Loading colored image..." << endl;
		coloredImage = new CImg<unsigned char>((char*)argv[2]);
		if (!coloredImage)
		{
			printf("error reading %s.\n", argv[2]);
			exit(1);
		}
		coloredImageLoaded = true;
	}

	if (argc > 3)
	{
		cout << "Loading texture image..." << endl;
		textureImage = new CImg<unsigned char>((char*)argv[3]);
		if (!textureImage)
		{
			printf("error reading %s.\n", argv[3]);
			exit(1);
		}
		textureImageLoaded = true;
	}

	cout << "Initializing OpenGL... " << endl;
	glutInit(&argc, (char**)argv);


	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle.c_str());

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	/* replace with any animate code */
	glutIdleFunc(doIdle);
	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);
	glutKeyboardFunc(keyboard);

	/* do initialization */
	myinit();

	cout << "Enter Main Loop... " << endl;
	glutMainLoop();
	return 0;
}