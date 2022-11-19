/*
CSCI 420
Assignment 3 Raytracer

Name: Yao Lin
*/

#include <windows.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdio.h>
#include <string>

#include "opencv2/core/core.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs/imgcodecs.hpp"

#define MAX_TRIANGLES 2000
#define MAX_SPHERES 10
#define MAX_LIGHTS 10

char* filename = 0;

//different display modes
#define MODE_DISPLAY 1
#define MODE_JPEG 2
int mode = MODE_DISPLAY;

//you may want to make these smaller for debugging purposes
#define WIDTH 640
#define HEIGHT 480

//the field of view of the camera
#define FOV 60.0
const double PI = 3.1415926;

unsigned char buffer[HEIGHT][WIDTH][3];

// OpenCV like Vec3d
struct Vec3d
{
	double x, y, z;

	Vec3d() = default;
	Vec3d(double _x, double _y, double _z) :x(_x), y(_y), z(_z) {};
	~Vec3d() = default;

	Vec3d operator-() const { return Vec3d(-x, -y, -z); }
	Vec3d operator-(const Vec3d& v) const { return Vec3d(x - v.x, y - v.y, z - v.z); }
	Vec3d& operator-=(const Vec3d& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}
	Vec3d operator+(const Vec3d& v) const { return Vec3d(x + v.x, y + v.y, z + v.z); }
	Vec3d& operator+=(const Vec3d& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}
	Vec3d operator*(double c) const { return Vec3d(c * x, c * y, c * z); }
	Vec3d& operator*=(double c)
	{
		x *= c;
		y *= c;
		z *= c;
		return *this;
	}
	Vec3d operator/(double c) const { return Vec3d(x / c, y / c, z / c); }
	Vec3d& operator/=(double c)
	{
		x /= c;
		y /= c;
		z /= c;
		return *this;
	}

	double length() const
	{
		double temp = x * x + y * y + z * z;
		return sqrt(temp);
	}

	Vec3d& normalized()
	{
		*this /= length();
		return *this;
	}
};
double dot(const Vec3d& v1, const Vec3d& v2){ return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
Vec3d cross(const Vec3d& v1, const Vec3d& v2)
{
	Vec3d v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

struct Vertex
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double normal[3];
	double shininess;
};

typedef struct Triangle
{
	struct Vertex v[3];
} Triangle;

typedef struct Sphere
{
	double position[3];
	double color_diffuse[3];
	double color_specular[3];
	double shininess;
	double radius;
} Sphere;

typedef struct Light
{
	double position[3];
	double color[3];
} Light;


struct Ray
{
	Vec3d pos;
	Vec3d dir; //normalized

	Ray() = default;
	Ray(const Vec3d& _pos, const Vec3d& _dir) : pos(_pos), dir(_dir) {};
	// check ray & sphere intersection
	bool getHitPosition(Vec3d &hit, const Sphere& s)
	{
		double a = 1;
		double b = 2 * (dir.x * (pos.x - s.position[0]) + 
						dir.y * (pos.y - s.position[1]) + 
						dir.z * (pos.z - s.position[2]));
		double c = pow(pos.x - s.position[0], 2) + 
				   pow(pos.y - s.position[1], 2) + 
				   pow(pos.z - s.position[2], 2) - 
			       pow(s.radius, 2);
		double t1 = (-b + sqrt(b * b - 4 * a * c)) / 2;
		double t2 = (-b - sqrt(b * b - 4 * a * c)) / 2;

		// no hit
		if (t1 <= 0 && t2 <= 0)
		{
			return false;
		}

		// hit
		double hitTime;
		if (t1 > t2 && t2 > 0) 
			hitTime = t2;
		else 
			hitTime = t1;
		hit = pos + dir * hitTime;
		return true;
	}
	// check ray & triangle intersection
	bool getHitPosition(Vec3d &hit, const Triangle& t)
	{
		// Normal for ray-plane intersection = (B-A)x(C-A) (and you must normalize this vector)
		Vec3d A(t.v[0].position[0], t.v[0].position[1], t.v[0].position[2]);
		Vec3d B(t.v[1].position[0], t.v[1].position[1], t.v[1].position[2]);
		Vec3d C(t.v[2].position[0], t.v[2].position[1], t.v[2].position[2]);
		Vec3d N = cross(B - A, C - A); 
		N.normalized();
		double hitTime = dot(A - pos, N) / dot(dir, N);

		// no hit the plane
		if (hitTime <= 0) return false;

		// hit the plane
		hit = pos + dir * hitTime;
		if (dot(cross(B - A, hit - A), N) < 0 ||
			dot(cross(B - A, hit - A), N) < 0 ||
			dot(cross(B - A, hit - A), N) < 0) return false;
		return true;
	}
};


Triangle triangles[MAX_TRIANGLES];
Sphere spheres[MAX_SPHERES];
Light lights[MAX_LIGHTS];
double ambient_light[3];

int num_triangles = 0;
int num_spheres = 0;
int num_lights = 0;

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b);
void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);

Vec3d computeColor(const Ray &ray, )
{

}

Ray generateRay(double x, double y)
{
	// The aspect ratio of an image is the ratio of its width to its height
	double aspectRatio = (double)WIDTH / (double)HEIGHT;
	double fov = tan(FOV / 2 * PI / 180.0);
	// convert coordinate space to [0, 1]
	x = (x + 0.5) / (double)WIDTH; 
	y = (y + 0.5) / (double)HEIGHT;
	// convert coordinate space to [-1, 1]
	x = 2.0 * x - 1;
	y = 2.0 * y - 1;
	// convert coordinate space to image space, assume z = -1
	x = x * fov * aspectRatio;
	y = y * fov;
	// generate ray
	Vec3d pos(0.0, 0.0, -1.0);
	Vec3d xy(x, y, 0.0);
	Vec3d dir = xy - pos;
	return Ray(pos, dir);
}

void draw_scene()
{
	unsigned int x, y;
	for (x = 0; x < WIDTH; x++)
	{
		glPointSize(2.0);
		glBegin(GL_POINTS);
		for (y = 0; y < HEIGHT; y++)
		{
			plot_pixel(x, y, x % 256, y % 256, (x + y) % 256);
		}
		glEnd();
		glFlush();
	}
	printf("Done!\n"); fflush(stdout);
}

void plot_pixel_display(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	glColor3f(((double)r) / 256.f, ((double)g) / 256.f, ((double)b) / 256.f);
	glVertex2i(x, y);
}

void plot_pixel_jpeg(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	buffer[HEIGHT - y - 1][x][0] = r;
	buffer[HEIGHT - y - 1][x][1] = g;
	buffer[HEIGHT - y - 1][x][2] = b;
}

void plot_pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	plot_pixel_display(x, y, r, g, b);
	if (mode == MODE_JPEG)
		plot_pixel_jpeg(x, y, r, g, b);
}

void save_jpg()
{
	if (filename == NULL)
		return;

	// Allocate a picture buffer // 
	cv::Mat3b bufferBGR = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3); //rows, cols, 3-channel 8-bit.
	printf("File to save to: %s\n", filename);

	// unsigned char buffer[HEIGHT][WIDTH][3];
	for (int r = 0; r < HEIGHT; r++) {
		for (int c = 0; c < WIDTH; c++) {
			for (int chan = 0; chan < 3; chan++) {
				unsigned char red = buffer[r][c][0];
				unsigned char green = buffer[r][c][1];
				unsigned char blue = buffer[r][c][2];
				bufferBGR.at<cv::Vec3b>(r, c) = cv::Vec3b(blue, green, red);
			}
		}
	}
	if (cv::imwrite(filename, bufferBGR)) {
		printf("File saved Successfully\n");
	}
	else {
		printf("Error in Saving\n");
	}
}

void parse_check(char* expected, char* found)
{
	if (stricmp(expected, found))
	{
		char error[100];
		printf("Expected '%s ' found '%s '\n", expected, found);
		printf("Parse error, abnormal abortion\n");
		exit(0);
	}

}

void parse_doubles(FILE* file, char* check, double p[3])
{
	char str[100];
	fscanf(file, "%s", str);
	parse_check(check, str);
	fscanf(file, "%lf %lf %lf", &p[0], &p[1], &p[2]);
	printf("%s %lf %lf %lf\n", check, p[0], p[1], p[2]);
}

void parse_rad(FILE* file, double* r)
{
	char str[100];
	fscanf(file, "%s", str);
	parse_check("rad:", str);
	fscanf(file, "%lf", r);
	printf("rad: %f\n", *r);
}

void parse_shi(FILE* file, double* shi)
{
	char s[100];
	fscanf(file, "%s", s);
	parse_check("shi:", s);
	fscanf(file, "%lf", shi);
	printf("shi: %f\n", *shi);
}

int loadScene(char* argv)
{
	FILE* file = fopen(argv, "r");
	int number_of_objects;
	char type[50];
	int i;
	Triangle t;
	Sphere s;
	Light l;
	fscanf(file, "%i", &number_of_objects);

	printf("number of objects: %i\n", number_of_objects);
	char str[200];

	parse_doubles(file, "amb:", ambient_light);

	for (i = 0; i < number_of_objects; i++)
	{
		fscanf(file, "%s\n", type);
		printf("%s\n", type);
		if (stricmp(type, "triangle") == 0)
		{

			printf("found triangle\n");
			int j;

			for (j = 0; j < 3; j++)
			{
				parse_doubles(file, "pos:", t.v[j].position);
				parse_doubles(file, "nor:", t.v[j].normal);
				parse_doubles(file, "dif:", t.v[j].color_diffuse);
				parse_doubles(file, "spe:", t.v[j].color_specular);
				parse_shi(file, &t.v[j].shininess);
			}

			if (num_triangles == MAX_TRIANGLES)
			{
				printf("too many triangles, you should increase MAX_TRIANGLES!\n");
				exit(0);
			}
			triangles[num_triangles++] = t;
		}
		else if (stricmp(type, "sphere") == 0)
		{
			printf("found sphere\n");

			parse_doubles(file, "pos:", s.position);
			parse_rad(file, &s.radius);
			parse_doubles(file, "dif:", s.color_diffuse);
			parse_doubles(file, "spe:", s.color_specular);
			parse_shi(file, &s.shininess);

			if (num_spheres == MAX_SPHERES)
			{
				printf("too many spheres, you should increase MAX_SPHERES!\n");
				exit(0);
			}
			spheres[num_spheres++] = s;
		}
		else if (stricmp(type, "light") == 0)
		{
			printf("found light\n");
			parse_doubles(file, "pos:", l.position);
			parse_doubles(file, "col:", l.color);

			if (num_lights == MAX_LIGHTS)
			{
				printf("too many lights, you should increase MAX_LIGHTS!\n");
				exit(0);
			}
			lights[num_lights++] = l;
		}
		else
		{
			printf("unknown type in scene description:\n%s\n", type);
			exit(0);
		}
	}
	return 0;
}

void display()
{

}

void init()
{
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, WIDTH, 0, HEIGHT, 1, -1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void idle()
{
	//hack to make it only draw once
	static int once = 0;
	if (!once)
	{
		draw_scene();
		if (mode == MODE_JPEG)
			save_jpg();
	}
	once = 1;
}

int main(int argc, char** argv)
{
	if (argc < 2 || argc > 3)
	{
		printf("usage: %s <scenefile> [jpegname]\n", argv[0]);
		exit(0);
	}
	if (argc == 3)
	{
		mode = MODE_JPEG;
		filename = argv[2];
	}
	else if (argc == 2)
		mode = MODE_DISPLAY;

	glutInit(&argc, argv);
	loadScene(argv[1]);

	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WIDTH, HEIGHT);
	int window = glutCreateWindow("Ray Tracer");
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	init();
	glutMainLoop();
}