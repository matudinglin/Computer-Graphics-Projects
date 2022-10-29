// assign2.cpp : Defines the entry point for the console application.
//

/*
	CSCI 420 Computer Graphics
	Assignment 2: Roller Coasters
	Yao Lin
*/

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iostream>
#include <GL/glu.h>
#include <GL/glut.h>
#include <time.h>

#include "opencv2/core/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"

#include "OBJ_Loader.h"

using namespace std;

/* Loaded textures */
cv::Mat3b leftImage, groundImage, frontImage, rightImage, skyImage, backImage;

/* Window properties*/
int windowWidth = 1280;
int windowHeight = 720;
string windowTitle = "CSCI 420 assignment 2 - Yao Lin";
int menuID;

/* State of the world */
float g_vLandRotate[3] = { 0.0, 0.0, 0.0 };
float g_vLandTranslate[3] = { 0.0, 0.0, 0.0 };
float g_vLandScale[3] = { 1.0, 1.0, 1.0 };

/* Mouse Control */
int g_vMousePos[2] = { 0, 0 };
int g_iLeftMouseButton = 0; 
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;
typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

/* Structs */
struct Vec3f
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	Vec3f(GLfloat x1, GLfloat y1, GLfloat z1)
	{
		x = x1; y = y1; z = z1;
	}
	Vec3f() {};
	Vec3f operator+(const Vec3f& b)
	{
		Vec3f p;
		p.x = x + b.x;
		p.y = y + b.y;
		p.z = z + b.z;
		return p;
	}
	Vec3f operator-(const Vec3f& b)
	{
		Vec3f p;
		p.x = x - b.x;
		p.y = y - b.y;
		p.z = z - b.z;
		return p;
	}
	Vec3f operator-()
	{
		Vec3f p;
		p.x = -x;
		p.y = -y;
		p.z = -z;
		return p;
	}
	Vec3f operator*(float c)
	{
		Vec3f p;
		p.x = c * x;
		p.y = c * y;
		p.z = c * z;
		return p;
	}
	bool operator==(const Vec3f& b)
	{
		if (x == b.x && y == b.y && z == b.z)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	Vec3f normalize()
	{
		GLfloat length = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
		x = x / length;
		y = y / length;
		z = z / length;
		return *this;
	}
};
struct Vec2f
{
	GLfloat u;
	GLfloat v;
	Vec2f(GLfloat u1, GLfloat v1)
	{
		u = u1; v = v1;
	}
};
struct Spline {
	int numControlPoints;
	struct Vec3f *points;
};
Vec3f cross(const Vec3f& a, const Vec3f& b)
{
	Vec3f p;
	p.x = a.y * b.z - a.z * b.y;
	p.y = a.z * b.x - a.x * b.z;
	p.z = a.x * b.y - a.y * b.x;
	return p.normalize();
}
Vec3f dot(const Vec3f& a, const Vec3f& b)
{
	Vec3f p;
	return p;
}

/* texutres */
GLuint g_textures[6];
const GLfloat g_border = 256;
vector<Vec3f> g_envGVertex, g_envSVertex, g_envLVertex, g_envRVertex, g_envFVertex, g_envBVertex;
vector<Vec2f> g_envGTextureVertex, g_envSTextureVertex, g_envLTextureVertex, g_envRTextureVertex, g_envFTextureVertex, g_envBTextureVertex;

/* splines and rails */
Spline* g_Splines;
int g_iNumOfSplines;
const GLfloat g_splineStep = 0.005f;
const GLfloat g_S = 0.5f;
const GLfloat g_railSize = 0.015f, g_railWidth = 0.12f;
vector<Vec3f> g_splineVertex, g_splineTangent, g_splineNormal, g_splineBinormal, g_splineColor;
vector<Vec3f> g_lRailVertex, g_lRailColor, g_lRailNormal;
vector<Vec3f> g_rRailVertex, g_rRailColor, g_rRailNormal;
vector<Vec3f> g_mRailVertex, g_mRailColor, g_mRailNormal;
vector<Vec3f> g_sRailVertex, g_sRailColor, g_sRailNormal;

/* objects */
vector<Vec3f> g_objVertex, g_objColor, g_objNormal;

/* Camera control */
enum CameraState{CAMERA_FREE, CAMERA_FIXED};
CameraState g_cameraState = CAMERA_FREE;
bool g_isRunning = false;
GLfloat g_cameraPos = 0;
Vec3f velocity;
vector<GLfloat> g_lookAt = { 0,0,-0.5,0,0,0,0,1,0 };

/* Frame rate control */
int g_prevTime = time(nullptr);
int g_crntTime = 0.0;
int g_frameCounter = 0;
int g_FPS = 60;

int loadSplines(char *argv) {
	char *cName = (char *)malloc(128 * sizeof(char));
	FILE *fileList;
	FILE *fileSpline;
	int iType, i = 0, j, iLength;

	/* load the track file */
	fileList = fopen(argv, "r");
	if (fileList == NULL) {
		printf ("can't open file\n");
		exit(1);
	}
  
	/* stores the number of splines in a global variable */
	fscanf(fileList, "%d", &g_iNumOfSplines);
	//printf("%d\n", g_iNumOfSplines);
	g_Splines = (struct Spline *)malloc(g_iNumOfSplines * sizeof(struct Spline));

	/* reads through the spline files */
	for (j = 0; j < g_iNumOfSplines; j++) {
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL) {
			printf ("can't open file\n");
			exit(1);
		}

		/* gets length for spline file */
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		/* allocate memory for all the points */
		g_Splines[j].points = (struct Vec3f *)malloc(iLength * sizeof(struct Vec3f));
		g_Splines[j].numControlPoints = iLength;

		/* saves the data to the struct */
		while (fscanf(fileSpline, "%f %f %f", 
			&g_Splines[j].points[i].x, 
			&g_Splines[j].points[i].y, 
			&g_Splines[j].points[i].z) != EOF) {
			i++;
		}
	}

	free(cName);

	return g_iNumOfSplines;
}

/* Write a screenshot to the specified filename */
void saveScreenshot(char *filename)
{
	if (filename == NULL)
		return;

	// Allocate a picture buffer // 
	cv::Mat3b bufferRGB = cv::Mat::zeros(480, 640, CV_8UC3); //rows, cols, 3-channel 8-bit.
	printf("File to save to: %s\n", filename);

	//use fast 4-byte alignment (default anyway) if possible
	glPixelStorei(GL_PACK_ALIGNMENT, (bufferRGB.step & 3) ? 1 : 4);
	//set length of one complete row in destination data (doesn't need to equal img.cols)
	glPixelStorei(GL_PACK_ROW_LENGTH, bufferRGB.step / bufferRGB.elemSize());
	glReadPixels(0, 0, bufferRGB.cols, bufferRGB.rows, GL_RGB, GL_UNSIGNED_BYTE, bufferRGB.data);
	//flip to account for GL 0,0 at lower left
	cv::flip(bufferRGB, bufferRGB, 0);
	//convert RGB to BGR
	cv::Mat3b bufferBGR(bufferRGB.rows, bufferRGB.cols, CV_8UC3);
	cv::Mat3b out[] = { bufferBGR };
	// rgb[0] -> bgr[2], rgba[1] -> bgr[1], rgb[2] -> bgr[0]
	int from_to[] = { 0,2, 1,1, 2,0 };
	mixChannels(&bufferRGB, 1, out, 1, from_to, 3);

	if (cv::imwrite(filename, bufferBGR)) {
		printf("File saved Successfully\n");
	}
	else {
		printf("Error in Saving\n");
	}
}

/* Function to get a pixel value. Use like PIC_PIXEL macro. 
Note: OpenCV images are in channel order BGR. 
This means that:
chan = 0 returns BLUE, 
chan = 1 returns GREEN, 
chan = 2 returns RED. 

OpenCV help:
Access number of rows of image (height): image.rows;
Access number of columns of image (width): image.cols;
Pixel 0,0 is the upper left corner. Byte order for 3-channel images is BGR. */
unsigned char getPixelValue(cv::Mat3b& image, int x, int y, int chan)
{
	return image.at<cv::Vec3b>(y, x)[chan];
}

/* Function that does nothing but demonstrates looping through image coordinates.*/
void loopImage(cv::Mat3b& image)
{
	for (int r = 0; r < image.rows; r++) { // y-coordinate
		for (int c = 0; c < image.cols; c++) { // x-coordinate
			for (int channel = 0; channel < 3; channel++) {
				// DO SOMETHING... example usage
				// unsigned char blue = getPixelValue(image, c, r, 0);
				// unsigned char green = getPixelValue(image, c, r, 1); 
				// unsigned char red = getPixelValue(image, c, r, 2); 
			}
		}
	}
}

/* Read an image into memory.
Set argument displayOn to true to make sure images are loaded correctly.
One image loaded, set to false so it doesn't interfere with OpenGL window.*/
int readImage(char *filename, cv::Mat3b& image, bool displayOn)
{

	std::cout << "  - Reading image: " << filename << std::endl;
	image = cv::imread(filename);
	if (!image.data) // Check for invalid input                    
	{
		std::cout << "Could not open or find the image." << std::endl;
		return 1;
	}

	if (displayOn)
	{
		cv::imshow("TestWindow", image);
		cv::waitKey(0); // Press any key to enter. 
	}
	return 0;
}

void envGenerator()
{
	/* env texture coordinates */
	// ground
	g_envGVertex.emplace_back(-g_border, -g_border, -g_border);
	g_envGVertex.emplace_back(g_border, -g_border, -g_border);
	g_envGVertex.emplace_back(g_border, -g_border, g_border);
	g_envGVertex.emplace_back(-g_border, -g_border, g_border);
	// sky
	g_envSVertex.emplace_back(-g_border, g_border, -g_border);
	g_envSVertex.emplace_back(g_border, g_border, -g_border);
	g_envSVertex.emplace_back(g_border, g_border, g_border);
	g_envSVertex.emplace_back(-g_border, g_border, g_border);
	// left
	g_envLVertex.emplace_back(-g_border, -g_border, -g_border);
	g_envLVertex.emplace_back(-g_border, g_border, -g_border);
	g_envLVertex.emplace_back(-g_border, g_border, g_border);
	g_envLVertex.emplace_back(-g_border, -g_border, g_border);
	// right
	g_envRVertex.emplace_back(g_border, -g_border, -g_border);
	g_envRVertex.emplace_back(g_border, g_border, -g_border);
	g_envRVertex.emplace_back(g_border, g_border, g_border);
	g_envRVertex.emplace_back(g_border, -g_border, g_border);
	// front
	g_envFVertex.emplace_back(-g_border, -g_border, -g_border);
	g_envFVertex.emplace_back(g_border, -g_border, -g_border);
	g_envFVertex.emplace_back(g_border, g_border, -g_border);
	g_envFVertex.emplace_back(-g_border, g_border, -g_border);
	// back
	g_envBVertex.emplace_back(-g_border, -g_border, g_border);
	g_envBVertex.emplace_back(g_border, -g_border, g_border);
	g_envBVertex.emplace_back(g_border, g_border, g_border);
	g_envBVertex.emplace_back(-g_border, g_border, g_border);

	/* texture mapping coordinates */
	// ground
	g_envGTextureVertex.emplace_back(0, 1);
	g_envGTextureVertex.emplace_back(1, 1);
	g_envGTextureVertex.emplace_back(1, 0);
	g_envGTextureVertex.emplace_back(0, 0);
	// sky
	g_envSTextureVertex.emplace_back(0, 0);
	g_envSTextureVertex.emplace_back(1, 0);
	g_envSTextureVertex.emplace_back(1, 1);
	g_envSTextureVertex.emplace_back(0, 1);
	// left
	g_envLTextureVertex.emplace_back(0, 1);
	g_envLTextureVertex.emplace_back(0, 0);
	g_envLTextureVertex.emplace_back(1, 0);
	g_envLTextureVertex.emplace_back(1, 1);
	// right
	g_envRTextureVertex.emplace_back(1, 1);
	g_envRTextureVertex.emplace_back(1, 0);
	g_envRTextureVertex.emplace_back(0, 0);
	g_envRTextureVertex.emplace_back(0, 1);
	// front
	g_envFTextureVertex.emplace_back(1, 1);
	g_envFTextureVertex.emplace_back(0, 1);
	g_envFTextureVertex.emplace_back(0, 0);
	g_envFTextureVertex.emplace_back(1, 0);
	// back
	g_envBTextureVertex.emplace_back(0, 1);
	g_envBTextureVertex.emplace_back(1, 1);
	g_envBTextureVertex.emplace_back(1, 0);
	g_envBTextureVertex.emplace_back(0, 0);

	// Initial textures
	// enable texture features
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	// generate 6 textures
	glGenTextures(6, g_textures);

	glBindTexture(GL_TEXTURE_2D, g_textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, groundImage.rows, groundImage.cols,
		0, GL_RGB, GL_UNSIGNED_BYTE, groundImage.data);

	glBindTexture(GL_TEXTURE_2D, g_textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, skyImage.rows, skyImage.cols,
		0, GL_RGB, GL_UNSIGNED_BYTE, skyImage.data);

	glBindTexture(GL_TEXTURE_2D, g_textures[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, leftImage.rows, leftImage.cols,
		0, GL_RGB, GL_UNSIGNED_BYTE, leftImage.data);

	glBindTexture(GL_TEXTURE_2D, g_textures[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rightImage.rows, rightImage.cols,
		0, GL_RGB, GL_UNSIGNED_BYTE, rightImage.data);

	glBindTexture(GL_TEXTURE_2D, g_textures[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frontImage.rows, frontImage.cols,
		0, GL_RGB, GL_UNSIGNED_BYTE, frontImage.data);

	glBindTexture(GL_TEXTURE_2D, g_textures[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, backImage.rows, backImage.cols,
		0, GL_RGB, GL_UNSIGNED_BYTE, backImage.data);
}

void splineGenerator()
{
	// Generate spline points
	for (int i = 0; i < g_iNumOfSplines; ++i)
	{
		int numControlPoints = g_Splines[i].numControlPoints;
		for (int j = 0; j < numControlPoints - 3; ++j)
		{
			// Brute force 
			Vec3f cp0 = g_Splines[i].points[j];
			Vec3f cp1 = g_Splines[i].points[j + 1];
			Vec3f cp2 = g_Splines[i].points[j + 2];
			Vec3f cp3 = g_Splines[i].points[j + 3];

			for (float u = 0; u < 1.0f; u += g_splineStep)
			{
				Vec3f point;
				GLfloat uu = u * u, uuu = u * u * u;

				point.x = g_S * ((2 * cp1.x) + (-cp0.x + cp2.x) * u + (2 * cp0.x - 5 * cp1.x + 4 * cp2.x - cp3.x) * uu + (-cp0.x + 3 * cp1.x - 3 * cp2.x + cp3.x) * uuu);
				point.y = g_S * ((2 * cp1.y) + (-cp0.y + cp2.y) * u + (2 * cp0.y - 5 * cp1.y + 4 * cp2.y - cp3.y) * uu + (-cp0.y + 3 * cp1.y - 3 * cp2.y + cp3.y) * uuu);
				point.z = g_S * ((2 * cp1.z) + (-cp0.z + cp2.z) * u + (2 * cp0.z - 5 * cp1.z + 4 * cp2.z - cp3.z) * uu + (-cp0.z + 3 * cp1.z - 3 * cp2.z + cp3.z) * uuu);
				g_splineVertex.push_back(point);

				Vec3f color(1.0, 1.0, 1.0);
				g_splineColor.push_back(color);
			}
		}
	}

	// Generate spline rail
	for (int i = 0; i < g_splineVertex.size() - 3; ++i) {

		Vec3f cp0 = g_splineVertex[i];
		Vec3f cp1 = g_splineVertex[i + 1];
		Vec3f cp2 = g_splineVertex[i + 2];
		Vec3f cp3 = g_splineVertex[i + 3];

		// Compute tangent
		const GLfloat u = g_splineStep, uu = u * u;
		Vec3f t, n, b;
		t.x = g_S * ((-cp0.x + cp2.x) + (2 * cp0.x - 5 * cp1.x + 4 * cp2.x - cp3.x) * (2 * u) + (-cp0.x + 3 * cp1.x - 3 * cp2.x + cp3.x) * (3 *uu));
		t.y = g_S * ((-cp0.y + cp2.y) + (2 * cp0.y - 5 * cp1.y + 4 * cp2.y - cp3.y) * (2 * u) + (-cp0.y + 3 * cp1.y - 3 * cp2.y + cp3.y) * (3 *uu));
		t.z = g_S * ((-cp0.z + cp2.z) + (2 * cp0.z - 5 * cp1.z + 4 * cp2.z - cp3.z) * (2 * u) + (-cp0.z + 3 * cp1.z - 3 * cp2.z + cp3.z) * (3 *uu));
		t.normalize();
		g_splineTangent.push_back(t);
		// Compute normal ---- MAY HAVE PROBLEM WHEN tangent.y IS UPSIDE DOWN
		Vec3f yaxis(0, 1, 0);
		if (t == yaxis) yaxis = Vec3f(0, 0, 1);
		b = cross(t, yaxis);
		g_splineBinormal.push_back(b);
		// Compute binormal
		n = cross(b,t);
		g_splineNormal.push_back(n);



		Vec3f p1, p2;
		// Generate rail
		Vec3f offset = Vec3f(g_railWidth * (b.x - n.x), 0.0f, g_railWidth * (b.z - n.z));
		// left rail
		p1 = cp0 + offset;
		p2 = cp3 + offset;

		Vec3f v0, v1, v2, v3, v4, v5, v6, v7;
		v0 = { p1.x + g_railSize * (-n.x + b.x), p1.y + g_railSize * (-n.y + b.y), p1.z + g_railSize * (-n.z + b.z) };
		v1 = { p1.x + g_railSize * (n.x + b.x), p1.y + g_railSize * (n.y + b.y), p1.z + g_railSize * (n.z + b.z) };
		v2 = { p1.x + g_railSize * (n.x - b.x), p1.y + g_railSize * (n.y - b.y), p1.z + g_railSize * (n.z - b.z) };
		v3 = { p1.x + g_railSize * (-n.x - b.x), p1.y + g_railSize * (-n.y - b.y), p1.z + g_railSize * (-n.z - b.z) };
		v4 = { p2.x + g_railSize * (-n.x + b.x), p2.y + g_railSize * (-n.y + b.y), p2.z + g_railSize * (-n.z + b.z) };
		v5 = { p2.x + g_railSize * (n.x + b.x), p2.y + g_railSize * (n.y + b.y), p2.z + g_railSize * (n.z + b.z) };
		v6 = { p2.x + g_railSize * (n.x - b.x), p2.y + g_railSize * (n.y - b.y), p2.z + g_railSize * (n.z - b.z) };
		v7 = { p2.x + g_railSize * (-n.x - b.x), p2.y + g_railSize * (-n.y - b.y), p2.z + g_railSize * (-n.z - b.z) };
		
		// front
		g_lRailVertex.push_back(v2); g_lRailVertex.push_back(v1); g_lRailVertex.push_back(v3); g_lRailVertex.push_back(v3); g_lRailVertex.push_back(v1); g_lRailVertex.push_back(v0);
		// left
		g_lRailVertex.push_back(v6); g_lRailVertex.push_back(v2); g_lRailVertex.push_back(v7); g_lRailVertex.push_back(v7); g_lRailVertex.push_back(v2); g_lRailVertex.push_back(v3);
		// right
		g_lRailVertex.push_back(v1); g_lRailVertex.push_back(v5); g_lRailVertex.push_back(v0); g_lRailVertex.push_back(v0); g_lRailVertex.push_back(v5); g_lRailVertex.push_back(v4);
		// up
		g_lRailVertex.push_back(v6); g_lRailVertex.push_back(v5); g_lRailVertex.push_back(v2); g_lRailVertex.push_back(v2); g_lRailVertex.push_back(v5); g_lRailVertex.push_back(v1);
		// down
		g_lRailVertex.push_back(v7); g_lRailVertex.push_back(v4); g_lRailVertex.push_back(v3); g_lRailVertex.push_back(v3); g_lRailVertex.push_back(v4); g_lRailVertex.push_back(v0);
		// back
		g_lRailVertex.push_back(v6); g_lRailVertex.push_back(v5); g_lRailVertex.push_back(v7); g_lRailVertex.push_back(v7); g_lRailVertex.push_back(v5); g_lRailVertex.push_back(v4);
		

		// right rail
		p1 = cp0 - offset;
		p2 = cp3 - offset;
		v0 = { p1.x + g_railSize * (-n.x + b.x), p1.y + g_railSize * (-n.y + b.y), p1.z + g_railSize * (-n.z + b.z) };
		v1 = { p1.x + g_railSize * (n.x + b.x), p1.y + g_railSize * (n.y + b.y), p1.z + g_railSize * (n.z + b.z) };
		v2 = { p1.x + g_railSize * (n.x - b.x), p1.y + g_railSize * (n.y - b.y), p1.z + g_railSize * (n.z - b.z) };
		v3 = { p1.x + g_railSize * (-n.x - b.x), p1.y + g_railSize * (-n.y - b.y), p1.z + g_railSize * (-n.z - b.z) };
		v4 = { p2.x + g_railSize * (-n.x + b.x), p2.y + g_railSize * (-n.y + b.y), p2.z + g_railSize * (-n.z + b.z) };
		v5 = { p2.x + g_railSize * (n.x + b.x), p2.y + g_railSize * (n.y + b.y), p2.z + g_railSize * (n.z + b.z) };
		v6 = { p2.x + g_railSize * (n.x - b.x), p2.y + g_railSize * (n.y - b.y), p2.z + g_railSize * (n.z - b.z) };
		v7 = { p2.x + g_railSize * (-n.x - b.x), p2.y + g_railSize * (-n.y - b.y), p2.z + g_railSize * (-n.z - b.z) };

		// front
		g_rRailVertex.push_back(v2); g_rRailVertex.push_back(v1); g_rRailVertex.push_back(v3); g_rRailVertex.push_back(v3); g_rRailVertex.push_back(v1); g_rRailVertex.push_back(v0);
		// left
		g_rRailVertex.push_back(v6); g_rRailVertex.push_back(v2); g_rRailVertex.push_back(v7); g_rRailVertex.push_back(v7); g_rRailVertex.push_back(v2); g_rRailVertex.push_back(v3);
		// right
		g_rRailVertex.push_back(v1); g_rRailVertex.push_back(v5); g_rRailVertex.push_back(v0); g_rRailVertex.push_back(v0); g_rRailVertex.push_back(v5); g_rRailVertex.push_back(v4);
		// up
		g_rRailVertex.push_back(v6); g_rRailVertex.push_back(v5); g_rRailVertex.push_back(v2); g_rRailVertex.push_back(v2); g_rRailVertex.push_back(v5); g_rRailVertex.push_back(v1);
		// down
		g_rRailVertex.push_back(v7); g_rRailVertex.push_back(v4); g_rRailVertex.push_back(v3); g_rRailVertex.push_back(v3); g_rRailVertex.push_back(v4); g_rRailVertex.push_back(v0);
		// back
		g_rRailVertex.push_back(v6); g_rRailVertex.push_back(v5); g_rRailVertex.push_back(v7); g_rRailVertex.push_back(v7); g_rRailVertex.push_back(v5); g_rRailVertex.push_back(v4);

		// mid
		if(i % 5 == 0)
		{
			p1 = cp0 + offset;
			p2 = cp3 - offset;
		
			v0 = { p1.x + g_railSize * (-n.x + b.x), p1.y + g_railSize * (-n.y + b.y), p1.z + g_railSize * (-n.z + b.z) };
			v1 = { p1.x + g_railSize * (n.x + b.x), p1.y + g_railSize * (n.y + b.y), p1.z + g_railSize * (n.z + b.z) };
			v2 = { p1.x + g_railSize * (n.x - b.x), p1.y + g_railSize * (n.y - b.y), p1.z + g_railSize * (n.z - b.z) };
			v3 = { p1.x + g_railSize * (-n.x - b.x), p1.y + g_railSize * (-n.y - b.y), p1.z + g_railSize * (-n.z - b.z) };
			v4 = { p2.x + g_railSize * (-n.x + b.x), p2.y + g_railSize * (-n.y + b.y), p2.z + g_railSize * (-n.z + b.z) };
			v5 = { p2.x + g_railSize * (n.x + b.x), p2.y + g_railSize * (n.y + b.y), p2.z + g_railSize * (n.z + b.z) };
			v6 = { p2.x + g_railSize * (n.x - b.x), p2.y + g_railSize * (n.y - b.y), p2.z + g_railSize * (n.z - b.z) };
			v7 = { p2.x + g_railSize * (-n.x - b.x), p2.y + g_railSize * (-n.y - b.y), p2.z + g_railSize * (-n.z - b.z) };

			// front
			g_mRailVertex.push_back(v2); g_mRailVertex.push_back(v1); g_mRailVertex.push_back(v3); g_mRailVertex.push_back(v3); g_mRailVertex.push_back(v1); g_mRailVertex.push_back(v0);
			// left
			g_mRailVertex.push_back(v6); g_mRailVertex.push_back(v2); g_mRailVertex.push_back(v7); g_mRailVertex.push_back(v7); g_mRailVertex.push_back(v2); g_mRailVertex.push_back(v3);
			// right
			g_mRailVertex.push_back(v1); g_mRailVertex.push_back(v5); g_mRailVertex.push_back(v0); g_mRailVertex.push_back(v0); g_mRailVertex.push_back(v5); g_mRailVertex.push_back(v4);
			// up
			g_mRailVertex.push_back(v6); g_mRailVertex.push_back(v5); g_mRailVertex.push_back(v2); g_mRailVertex.push_back(v2); g_mRailVertex.push_back(v5); g_mRailVertex.push_back(v1);
			// down
			g_mRailVertex.push_back(v7); g_mRailVertex.push_back(v4); g_mRailVertex.push_back(v3); g_mRailVertex.push_back(v3); g_mRailVertex.push_back(v4); g_mRailVertex.push_back(v0);
			// back
			g_mRailVertex.push_back(v6); g_mRailVertex.push_back(v5); g_mRailVertex.push_back(v7); g_mRailVertex.push_back(v7); g_mRailVertex.push_back(v5); g_mRailVertex.push_back(v4);
		}

		// supporter
		if (i % 200 == 0)
		{
			p1 = cp0 + offset;
			p2 = Vec3f(p1.x, -g_border, p1.z);

			v0 = { p1.x + g_railSize * (-n.x + b.x), p1.y + g_railSize * (-n.y + b.y), p1.z + g_railSize * (-n.z + b.z) };
			v1 = { p1.x + g_railSize * (n.x + b.x), p1.y + g_railSize * (n.y + b.y), p1.z + g_railSize * (n.z + b.z) };
			v2 = { p1.x + g_railSize * (n.x - b.x), p1.y + g_railSize * (n.y - b.y), p1.z + g_railSize * (n.z - b.z) };
			v3 = { p1.x + g_railSize * (-n.x - b.x), p1.y + g_railSize * (-n.y - b.y), p1.z + g_railSize * (-n.z - b.z) };
			v4 = { p2.x + g_railSize * (-n.x + b.x), p2.y + g_railSize * (-n.y + b.y), p2.z + g_railSize * (-n.z + b.z) };
			v5 = { p2.x + g_railSize * (n.x + b.x), p2.y + g_railSize * (n.y + b.y), p2.z + g_railSize * (n.z + b.z) };
			v6 = { p2.x + g_railSize * (n.x - b.x), p2.y + g_railSize * (n.y - b.y), p2.z + g_railSize * (n.z - b.z) };
			v7 = { p2.x + g_railSize * (-n.x - b.x), p2.y + g_railSize * (-n.y - b.y), p2.z + g_railSize * (-n.z - b.z) };

			// front
			g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v1); g_sRailVertex.push_back(v3); g_sRailVertex.push_back(v3); g_sRailVertex.push_back(v1); g_sRailVertex.push_back(v0);
			// left
			g_sRailVertex.push_back(v6); g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v3);
			// right
			g_sRailVertex.push_back(v1); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v0); g_sRailVertex.push_back(v0); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v4);
			// up
			g_sRailVertex.push_back(v6); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v1);
			// down
			g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v4); g_sRailVertex.push_back(v3); g_sRailVertex.push_back(v3); g_sRailVertex.push_back(v4); g_sRailVertex.push_back(v0);
			// back
			g_sRailVertex.push_back(v6); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v4);

			p1 = cp0 - offset;
			p2 = Vec3f(p1.x, -g_border, p1.z);

			v0 = { p1.x + g_railSize * (-n.x + b.x), p1.y + g_railSize * (-n.y + b.y), p1.z + g_railSize * (-n.z + b.z) };
			v1 = { p1.x + g_railSize * (n.x + b.x), p1.y + g_railSize * (n.y + b.y), p1.z + g_railSize * (n.z + b.z) };
			v2 = { p1.x + g_railSize * (n.x - b.x), p1.y + g_railSize * (n.y - b.y), p1.z + g_railSize * (n.z - b.z) };
			v3 = { p1.x + g_railSize * (-n.x - b.x), p1.y + g_railSize * (-n.y - b.y), p1.z + g_railSize * (-n.z - b.z) };
			v4 = { p2.x + g_railSize * (-n.x + b.x), p2.y + g_railSize * (-n.y + b.y), p2.z + g_railSize * (-n.z + b.z) };
			v5 = { p2.x + g_railSize * (n.x + b.x), p2.y + g_railSize * (n.y + b.y), p2.z + g_railSize * (n.z + b.z) };
			v6 = { p2.x + g_railSize * (n.x - b.x), p2.y + g_railSize * (n.y - b.y), p2.z + g_railSize * (n.z - b.z) };
			v7 = { p2.x + g_railSize * (-n.x - b.x), p2.y + g_railSize * (-n.y - b.y), p2.z + g_railSize * (-n.z - b.z) };

			// front
			g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v1); g_sRailVertex.push_back(v3); g_sRailVertex.push_back(v3); g_sRailVertex.push_back(v1); g_sRailVertex.push_back(v0);
			// left
			g_sRailVertex.push_back(v6); g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v3);
			// right
			g_sRailVertex.push_back(v1); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v0); g_sRailVertex.push_back(v0); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v4);
			// up
			g_sRailVertex.push_back(v6); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v2); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v1);
			// down
			g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v4); g_sRailVertex.push_back(v3); g_sRailVertex.push_back(v3); g_sRailVertex.push_back(v4); g_sRailVertex.push_back(v0);
			// back
			g_sRailVertex.push_back(v6); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v7); g_sRailVertex.push_back(v5); g_sRailVertex.push_back(v4);
		}

		// compute color
		for (int j = 0; j < 36; j++) {
			Vec3f colors(1.0, 1.0, 0.0);
			g_lRailColor.push_back(colors);
			g_rRailColor.push_back(colors);
			g_mRailColor.push_back(colors);
			g_sRailColor.push_back(colors);
		}

		// compute normal
		Vec3f ln0, ln1, ln2, ln3, ln4, ln5, ln6, ln7;
		ln0 = -n + b - t; ln0.normalize();
		ln1 = n + b - t; ln1.normalize();
		ln2 = n - b - t; ln2.normalize();
		ln3 = - n - b - t; ln3.normalize();
		ln4 = -n + b + t; ln4.normalize();
		ln5 = n + b + t; ln5.normalize();
		ln6 = n - b + t; ln6.normalize();
		ln7 = -n - b + t; ln7.normalize();

		// left
		g_lRailNormal.push_back(ln2); g_lRailNormal.push_back(ln1); g_lRailNormal.push_back(ln3); g_lRailNormal.push_back(ln3); g_lRailNormal.push_back(ln1); g_lRailNormal.push_back(ln0);
		g_lRailNormal.push_back(ln6); g_lRailNormal.push_back(ln2); g_lRailNormal.push_back(ln7); g_lRailNormal.push_back(ln7); g_lRailNormal.push_back(ln2); g_lRailNormal.push_back(ln3);
		g_lRailNormal.push_back(ln1); g_lRailNormal.push_back(ln5); g_lRailNormal.push_back(ln0); g_lRailNormal.push_back(ln0); g_lRailNormal.push_back(ln5); g_lRailNormal.push_back(ln4);
		g_lRailNormal.push_back(ln6); g_lRailNormal.push_back(ln5); g_lRailNormal.push_back(ln2); g_lRailNormal.push_back(ln2); g_lRailNormal.push_back(ln5); g_lRailNormal.push_back(ln1);
		g_lRailNormal.push_back(ln7); g_lRailNormal.push_back(ln4); g_lRailNormal.push_back(ln3); g_lRailNormal.push_back(ln3); g_lRailNormal.push_back(ln4); g_lRailNormal.push_back(ln0);
		g_lRailNormal.push_back(ln6); g_lRailNormal.push_back(ln5); g_lRailNormal.push_back(ln7); g_lRailNormal.push_back(ln7); g_lRailNormal.push_back(ln5); g_lRailNormal.push_back(ln4);
		
		// right
		g_rRailNormal.push_back(ln2); g_rRailNormal.push_back(ln1); g_rRailNormal.push_back(ln3); g_rRailNormal.push_back(ln3); g_rRailNormal.push_back(ln1); g_rRailNormal.push_back(ln0);
		g_rRailNormal.push_back(ln6); g_rRailNormal.push_back(ln2); g_rRailNormal.push_back(ln7); g_rRailNormal.push_back(ln7); g_rRailNormal.push_back(ln2); g_rRailNormal.push_back(ln3);
		g_rRailNormal.push_back(ln1); g_rRailNormal.push_back(ln5); g_rRailNormal.push_back(ln0); g_rRailNormal.push_back(ln0); g_rRailNormal.push_back(ln5); g_rRailNormal.push_back(ln4);
		g_rRailNormal.push_back(ln6); g_rRailNormal.push_back(ln5); g_rRailNormal.push_back(ln2); g_rRailNormal.push_back(ln2); g_rRailNormal.push_back(ln5); g_rRailNormal.push_back(ln1);
		g_rRailNormal.push_back(ln7); g_rRailNormal.push_back(ln4); g_rRailNormal.push_back(ln3); g_rRailNormal.push_back(ln3); g_rRailNormal.push_back(ln4); g_rRailNormal.push_back(ln0);
		g_rRailNormal.push_back(ln6); g_rRailNormal.push_back(ln5); g_rRailNormal.push_back(ln7); g_rRailNormal.push_back(ln7); g_rRailNormal.push_back(ln5); g_rRailNormal.push_back(ln4);

		// mid
		g_mRailNormal.push_back(ln2); g_mRailNormal.push_back(ln1); g_mRailNormal.push_back(ln3); g_mRailNormal.push_back(ln3); g_mRailNormal.push_back(ln1); g_mRailNormal.push_back(ln0);
		g_mRailNormal.push_back(ln6); g_mRailNormal.push_back(ln2); g_mRailNormal.push_back(ln7); g_mRailNormal.push_back(ln7); g_mRailNormal.push_back(ln2); g_mRailNormal.push_back(ln3);
		g_mRailNormal.push_back(ln1); g_mRailNormal.push_back(ln5); g_mRailNormal.push_back(ln0); g_mRailNormal.push_back(ln0); g_mRailNormal.push_back(ln5); g_mRailNormal.push_back(ln4);
		g_mRailNormal.push_back(ln6); g_mRailNormal.push_back(ln5); g_mRailNormal.push_back(ln2); g_mRailNormal.push_back(ln2); g_mRailNormal.push_back(ln5); g_mRailNormal.push_back(ln1);
		g_mRailNormal.push_back(ln7); g_mRailNormal.push_back(ln4); g_mRailNormal.push_back(ln3); g_mRailNormal.push_back(ln3); g_mRailNormal.push_back(ln4); g_mRailNormal.push_back(ln0);
		g_mRailNormal.push_back(ln6); g_mRailNormal.push_back(ln5); g_mRailNormal.push_back(ln7); g_mRailNormal.push_back(ln7); g_mRailNormal.push_back(ln5); g_mRailNormal.push_back(ln4);

		// supporter
		g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln1); g_sRailNormal.push_back(ln3); g_sRailNormal.push_back(ln3); g_sRailNormal.push_back(ln1); g_sRailNormal.push_back(ln0);
		g_sRailNormal.push_back(ln6); g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln3);
		g_sRailNormal.push_back(ln1); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln0); g_sRailNormal.push_back(ln0); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln4);
		g_sRailNormal.push_back(ln6); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln1);
		g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln4); g_sRailNormal.push_back(ln3); g_sRailNormal.push_back(ln3); g_sRailNormal.push_back(ln4); g_sRailNormal.push_back(ln0);
		g_sRailNormal.push_back(ln6); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln4);
		g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln1); g_sRailNormal.push_back(ln3); g_sRailNormal.push_back(ln3); g_sRailNormal.push_back(ln1); g_sRailNormal.push_back(ln0);
		g_sRailNormal.push_back(ln6); g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln3);
		g_sRailNormal.push_back(ln1); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln0); g_sRailNormal.push_back(ln0); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln4);
		g_sRailNormal.push_back(ln6); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln2); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln1);
		g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln4); g_sRailNormal.push_back(ln3); g_sRailNormal.push_back(ln3); g_sRailNormal.push_back(ln4); g_sRailNormal.push_back(ln0);
		g_sRailNormal.push_back(ln6); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln7); g_sRailNormal.push_back(ln5); g_sRailNormal.push_back(ln4);

}
}

void updateCamera(GLfloat pos, GLfloat u)
{
	Vec3f eye = g_splineVertex[pos] + g_splineNormal[pos] * 0.3f;
	Vec3f center = eye + g_splineTangent[pos];
	Vec3f up = g_splineNormal[pos];

	g_lookAt = { eye.x, eye.y, eye.z,
		center.x, center.y, center.z,
		up.x, up.y, up.z };
}

void myinit()
{
	/* setup gl view here */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	// enable vertex and color array for rendering
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	splineGenerator();
	envGenerator();
}

void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Mouse control Transformation
	gluLookAt(g_lookAt[0], g_lookAt[1], g_lookAt[2], g_lookAt[3], g_lookAt[4], g_lookAt[5], g_lookAt[6], g_lookAt[7], g_lookAt[8]);
	glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
	glRotatef(g_vLandRotate[0], 1.0, 0.0, 0.0);
	glRotatef(g_vLandRotate[1], 0.0, 1.0, 0.0);
	glRotatef(g_vLandRotate[2], 0.0, 0.0, 1.0);
	glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);

	// Draw splines
	glVertexPointer(3, GL_FLOAT, 0, g_splineVertex.data());
	glColorPointer(3, GL_FLOAT, 0, g_splineColor.data());
	glDrawArrays(GL_LINE_STRIP, 0, g_splineVertex.size());

	// Draw ground and skybox
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, g_textures[0]);
	glVertexPointer(3, GL_FLOAT, 0, g_envGVertex.data());
	glTexCoordPointer(2, GL_FLOAT, 0, g_envGTextureVertex.data());
	glDrawArrays(GL_QUADS, 0, g_envGVertex.size());

	glBindTexture(GL_TEXTURE_2D, g_textures[1]);
	glVertexPointer(3, GL_FLOAT, 0, g_envSVertex.data());
	glTexCoordPointer(2, GL_FLOAT, 0, g_envSTextureVertex.data());
	glDrawArrays(GL_QUADS, 0, g_envSVertex.size());

	glBindTexture(GL_TEXTURE_2D, g_textures[2]);
	glVertexPointer(3, GL_FLOAT, 0, g_envLVertex.data());
	glTexCoordPointer(2, GL_FLOAT, 0, g_envLTextureVertex.data());
	glDrawArrays(GL_QUADS, 0, g_envLVertex.size());

	glBindTexture(GL_TEXTURE_2D, g_textures[3]);
	glVertexPointer(3, GL_FLOAT, 0, g_envRVertex.data());
	glTexCoordPointer(2, GL_FLOAT, 0, g_envRTextureVertex.data());
	glDrawArrays(GL_QUADS, 0, g_envRVertex.size());

	glBindTexture(GL_TEXTURE_2D, g_textures[4]);
	glVertexPointer(3, GL_FLOAT, 0, g_envFVertex.data());
	glTexCoordPointer(2, GL_FLOAT, 0, g_envFTextureVertex.data());
	glDrawArrays(GL_QUADS, 0, g_envFVertex.size());

	glBindTexture(GL_TEXTURE_2D, g_textures[5]);
	glVertexPointer(3, GL_FLOAT, 0, g_envBVertex.data());
	glTexCoordPointer(2, GL_FLOAT, 0, g_envBTextureVertex.data());
	glDrawArrays(GL_QUADS, 0, g_envBVertex.size());

	glDisable(GL_TEXTURE_2D);

	// Draw rails --- enable lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_FLAT);
	GLfloat light_position[] = { 0.0, -1.0, 0.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	GLfloat light_ambient1[] = { 0.0, 0.0, 1.0, 1.0 };
	GLfloat light_diffuse1[] = { 0.0, 0.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse1);
	
	// rail
	glVertexPointer(3, GL_FLOAT, 0, g_lRailVertex.data());
	glColorPointer(3, GL_FLOAT, 0, g_lRailColor.data());
	glNormalPointer(GL_FLOAT, 0, g_lRailNormal.data());
	glDrawArrays(GL_TRIANGLES, 0, g_lRailVertex.size());

	glVertexPointer(3, GL_FLOAT, 0, g_rRailVertex.data());
	glColorPointer(3, GL_FLOAT, 0, g_rRailColor.data());
	glNormalPointer(GL_FLOAT, 0, g_rRailNormal.data());
	glDrawArrays(GL_TRIANGLES, 0, g_rRailVertex.size());

	glVertexPointer(3, GL_FLOAT, 0, g_mRailVertex.data());
	glColorPointer(3, GL_FLOAT, 0, g_mRailColor.data());
	glNormalPointer(GL_FLOAT, 0, g_mRailNormal.data());
	glDrawArrays(GL_TRIANGLES, 0, g_mRailVertex.size());

	// supporter
	GLfloat light_ambient2[] = { 1.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse2[] = { 1.0, 0.0, 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse2);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient2);

	glVertexPointer(3, GL_FLOAT, 0, g_sRailVertex.data());
	glColorPointer(3, GL_FLOAT, 0, g_sRailColor.data());
	glNormalPointer(GL_FLOAT, 0, g_sRailNormal.data());
	glDrawArrays(GL_TRIANGLES, 0, g_sRailVertex.size());

	// minecart
	GLfloat light_ambient3[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse3[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse3);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient3);
	glLoadIdentity();
	glTranslatef(0.0f, -0.2f, -0.4f);
	glScalef(1.5, 1.5, 1.5);
	glVertexPointer(3, GL_FLOAT, 0, g_objVertex.data());
	glNormalPointer(GL_FLOAT, 0, g_objNormal.data());
	glDrawArrays(GL_TRIANGLES, 0, g_objVertex.size());

	glDisable(GL_LIGHTING);


	// Swap buffers
	glutSwapBuffers();

	// frame rate
	++g_frameCounter;
	g_crntTime = time(nullptr);
	if (g_crntTime - g_prevTime > 0)
	{
		windowTitle = "CSCI 420 assignment 2 - Yao Lin - FPS: " + to_string(g_frameCounter / (g_crntTime - g_prevTime));
		glutSetWindowTitle(windowTitle.c_str());
		g_frameCounter = 0;
		g_prevTime = g_crntTime;
	}

}

void resetCamera()
{
	g_cameraPos = 0;
	g_vLandRotate[0] = 0.0; g_vLandRotate[1] = 0.0; g_vLandRotate[2] = 0.0;
	g_vLandTranslate[0] = 0.0; g_vLandTranslate[1] = 0.0; g_vLandTranslate[2] = 0.0;
	g_vLandScale[0] = 1.0; g_vLandScale[1] = 1.0; g_vLandScale[2] = 1.0;
}

void idleFunc(int)
{
	// make the screen update
	glutPostRedisplay();
	if (g_isRunning && g_cameraPos < g_splineVertex.size() - 3)
	{
		updateCamera(g_cameraPos, g_splineStep);
		// realistic speed control
		if (velocity.y < 0)
			g_cameraPos += 2;
		else
			g_cameraPos += 1;
	}
	glutTimerFunc(1000 / g_FPS, idleFunc, 0);
}

void reshapeFunc(int w, int h)
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

void menuFunc(int value)
{
	switch (value)
	{
	case 0:
		exit(0);
		break;
	}
}

void mouseDragFunc(int x, int y)
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

void mouseIdleFunc(int x, int y)
{
	g_vMousePos[0] = x;
	g_vMousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
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

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '1':
		g_cameraState = CAMERA_FREE;
		g_isRunning = false;
		resetCamera();
		break;
	case '2':
		g_cameraState = CAMERA_FIXED;
		g_isRunning = !g_isRunning;
		break;
	default:
		break;
	}
}

void objGenerator(objl::Mesh mesh)
{
	for (int j = 0; j < mesh.Vertices.size(); j++)
	{
		Vec3f v, n, c;
		v = Vec3f(mesh.Vertices[j].Position.X, mesh.Vertices[j].Position.Y, mesh.Vertices[j].Position.Z);
		g_objVertex.push_back(v);
		n = Vec3f(mesh.Vertices[j].Normal.X, mesh.Vertices[j].Normal.Y, mesh.Vertices[j].Normal.Z);
		g_objNormal.push_back(n);
		c = Vec3f(1.0f, 0.0f, 0.0f);
		g_objColor.push_back(c);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc<2)
	{  
		printf ("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}
	// Load splines
	cout << "Loading " << loadSplines(argv[1]) <<  " splines..." << endl;
	
	// Load textures
	cout << "Loading texture..." << endl;
	readImage("textures/groundImage.jpg", groundImage, false);
	cv::cvtColor(groundImage, groundImage, CV_BGR2RGB);
	readImage("textures/skyImage.jpg", skyImage, false);
	cv::cvtColor(skyImage, skyImage, CV_BGR2RGB);
	readImage("textures/leftImage.jpg", leftImage, false);
	cv::cvtColor(leftImage, leftImage, CV_BGR2RGB);
	readImage("textures/rightImage.jpg", rightImage, false);
	cv::cvtColor(rightImage, rightImage, CV_BGR2RGB);
	readImage("textures/frontImage.jpg", frontImage, false);
	cv::cvtColor(frontImage, frontImage, CV_BGR2RGB);
	readImage("textures/backImage.jpg", backImage, false);
	cv::cvtColor(backImage, backImage, CV_BGR2RGB);

	// Load objects
	// objLoader is from https://github.com/Bly7/OBJ-Loader
	cout << "Loading object...";
	objl::Loader loader;
	loader.LoadFile("objects/bunny.obj");
	objGenerator(loader.LoadedMeshes[0]);


	// Initialize OpenGL
	cout << "Initializing OpenGL... " << endl;
	glutInit(&argc, (char**)argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);

	// Create Window
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle.c_str());

	// Set callbacks
	myinit();
	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	menuID = glutCreateMenu(menuFunc);
	glutSetMenu(menuID);
	glutAddMenuEntry("Quit", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutMotionFunc(mouseDragFunc);
	glutPassiveMotionFunc(mouseIdleFunc);
	glutMouseFunc(mouseButtonFunc);
	glutKeyboardFunc(keyboardFunc);
	//glutIdleFunc(idleFunc);
	glutTimerFunc(1000/g_FPS, idleFunc, 0);

	cout << "Enter Main Loop... " << endl;
	glutMainLoop();

	return 0;
}
