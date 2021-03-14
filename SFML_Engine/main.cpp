#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <math.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

#define DEBUG true

struct Vec3d
{
	float x, y, z;
};

struct Triangle
{
	Vec3d p[3];
	float col;
};

struct Mesh
{
	std::vector<Triangle> tris;

	bool LoadFromObjectFile(std::string sFilename)
	{
		std::ifstream f(sFilename);
		if (!f.is_open())
			return false;

		// Local cache of verts
		std::vector<Vec3d> verts;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);
			std::cout << line << "\n";
			std::stringstream s;
			s << line;
			char junk;

			if (line[0] == 'v')
			{
				Vec3d v;
				s >> junk >> v.x >> v.y >> v.z;

				verts.push_back(v);
			}
			else

				if (line[0] == 'f')
				{
					int f[3]{ 0, 0, 0 };

					s >> junk >> f[0] >> f[1] >> f[2];
					tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
				}
		}
		return true;
	}
};

struct Mat4x4
{
	float m[4][4] = { 0 };
};

class Game
{
private:
	Mesh cube;
	Mat4x4 matProj;
	Vec3d vCamera;

	int screenWidth = 1280;
	int screenHeight = 720;
	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(screenWidth, screenHeight), "SFML works!");

	std::vector<sf::ConvexShape> shape;
	sf::Clock clock;

	float fTheta = 0;

	void MultiplyMatrix(Vec3d& i, Vec3d& o, Mat4x4& m)
	{
		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

		if (w != 0.0f)
		{
			o.x /= w; o.y /= w; o.z /= w;
		}
	}

public:
	void Tick()
	{
		sf::Event ev;
		while (window.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
				window.close();
		}

		Mat4x4 matRotZ, matRotX;
		fTheta = 1.0f * clock.getElapsedTime().asSeconds();

		// Rotation Z
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sinf(fTheta);
		matRotZ.m[1][1] = cosf(fTheta);
		matRotZ.m[2][2] = 1;
		matRotZ.m[3][3] = 1;

		// Rotation X
		matRotX.m[0][0] = 1;
		matRotX.m[1][1] = cosf(fTheta * 0.5f);
		matRotX.m[1][2] = sinf(fTheta * 0.5f);
		matRotX.m[2][1] = -sinf(fTheta * 0.5f);
		matRotX.m[2][2] = cosf(fTheta * 0.5f);
		matRotX.m[3][3] = 1;

		float z = sin(clock.getElapsedTime().asSeconds()) * 3 + 10;

		std::vector<Triangle> vecTriangleToRaster;

		for (Triangle tri : cube.tris)
		{
			Triangle triProjected, triTranslated, triRotatedZ, triRotatedZX = {};

			// Rotate in Z-Axis
			MultiplyMatrix(tri.p[0], triRotatedZ.p[0], matRotZ);
			MultiplyMatrix(tri.p[1], triRotatedZ.p[1], matRotZ);
			MultiplyMatrix(tri.p[2], triRotatedZ.p[2], matRotZ);

			// Rotate in X-Axis
			MultiplyMatrix(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
			MultiplyMatrix(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
			MultiplyMatrix(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

			triTranslated = triRotatedZX;
			triTranslated.p[0].z = triRotatedZX.p[0].z + z;
			triTranslated.p[1].z = triRotatedZX.p[1].z + z;
			triTranslated.p[2].z = triRotatedZX.p[2].z + z;

			Vec3d normal, line1, line2;
			line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
			line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
			line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

			line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
			line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
			line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z * line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;

			float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			normal.x /= l; normal.y /= l; normal.z /= l;

			if (normal.x * (triTranslated.p[0].x - vCamera.x) +
				normal.y * (triTranslated.p[0].y - vCamera.y) +
				normal.z * (triTranslated.p[0].z - vCamera.z) < 0)
			{
				Vec3d dirLight = { 0.0f, 0.0f, -1.0f };
				float l = sqrtf(dirLight.x * dirLight.x + dirLight.y * dirLight.y + dirLight.z * dirLight.z);
				dirLight.x /= l; dirLight.y /= l; dirLight.z /= l;

				float dp = normal.x * dirLight.x + normal.y * dirLight.y + normal.z * dirLight.z;
				triProjected.col = dp;
				MultiplyMatrix(triTranslated.p[0], triProjected.p[0], matProj);
				MultiplyMatrix(triTranslated.p[1], triProjected.p[1], matProj);
				MultiplyMatrix(triTranslated.p[2], triProjected.p[2], matProj);

				triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
				triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
				triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

				triProjected.p[0].x *= 0.5f * screenWidth;
				triProjected.p[0].y *= 0.5f * screenHeight;
				triProjected.p[1].x *= 0.5f * screenWidth;
				triProjected.p[1].y *= 0.5f * screenHeight;
				triProjected.p[2].x *= 0.5f * screenWidth;
				triProjected.p[2].y *= 0.5f * screenHeight;

				vecTriangleToRaster.push_back(triProjected);
			}

			sort(vecTriangleToRaster.begin(), vecTriangleToRaster.end(), [](Triangle& t1, Triangle& t2)
				{
					float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
					float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
					return z1 > z2;
				});

			for (auto& triProjected : vecTriangleToRaster)
			{
				this->shape.push_back(DrawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y, triProjected.col * 225));
			}
		}

		window.clear();
		for (unsigned int i = 0; i < shape.size(); i++)
		{
			window.draw(shape[i]);
		}
		shape.clear();
		window.display();
	}

	sf::ConvexShape DrawTriangle(float x, float y, float x2, float y2, float x3, float y3, float lightStrength)
	{
		sf::ConvexShape convex;
		convex.setPointCount(3);
		convex.setPoint(2, sf::Vector2f(x, y));
		convex.setPoint(1, sf::Vector2f(x2, y2));
		convex.setPoint(0, sf::Vector2f(x3, y3));
		convex.setFillColor(sf::Color(lightStrength, lightStrength, lightStrength));

		if (DEBUG)
		{
			convex.setOutlineThickness(1);
			convex.setOutlineColor(sf::Color(0, 0, 0));
		}

		return convex;
	}

	void Start()
	{
		cube.LoadFromObjectFile("ship.obj");

		float fNear = 0.1f;
		float fFar = 1000.0f;
		float fFov = 90.0f;
		float fAspectRatio = 720.0f / 1280.0f;
		float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

		matProj.m[0][0] = fAspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][3] = 0.0f;
	}
};

int main()
{
	Game g;
	g.Start();
	while (true)
	{
		g.Tick();
	}

	return 0;
}
