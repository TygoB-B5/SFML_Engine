#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <math.h>

struct Vec3d
{
    float x, y, z;
};

struct Triangle
{
    Vec3d p[3];
};

struct Mesh
{
    std::vector<Triangle> tris;
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
    sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(1280, 720), "SFML works!");
    std::vector<sf::ConvexShape> shape;
    sf::Clock clock;

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
    void Start()
    {
        cube.tris = {

            // SOUTH
            { 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

            // EAST                                                      
            { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

            // NORTH                                                     
            { 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

            // WEST                                                      
            { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

            // TOP                                                       
            { 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

            // BOTTOM                                                    
            { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },

        };

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

        
        while (true)
        {
            OnTick();
        }
    }
private:

    float fTheta = 0;
	void OnTick()
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
            triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
            triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
            triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;


			MultiplyMatrix(triTranslated.p[0], triProjected.p[0], matProj);
			MultiplyMatrix(triTranslated.p[1], triProjected.p[1], matProj);
			MultiplyMatrix(triTranslated.p[2], triProjected.p[2], matProj);

			triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
			triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
			triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

			triProjected.p[0].x *= 0.5f * (float)1280;
			triProjected.p[0].y *= 0.5f * (float)720;
			triProjected.p[1].x *= 0.5f * (float)1280;
			triProjected.p[1].y *= 0.5f * (float)720;
			triProjected.p[2].x *= 0.5f * (float)1280;
			triProjected.p[2].y *= 0.5f * (float)720;
			this->shape.push_back(DrawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y));
		}

		window.clear();
		for (unsigned int i = 0; i < shape.size(); i++)
		{
			window.draw(shape[i]);
		}
		shape.clear();
		window.display();
	}


    int i = 225;
    int a = 0;
    sf::ConvexShape DrawTriangle(float x, float y, float x2, float y2, float x3, float y3)
    {
        if (a == 2)
        {
            i -= 20;
            if (i < 0)
                i == 225;

            a = 0;
        }
        a++;

        sf::ConvexShape convex;
        convex.setPointCount(3);
        convex.setPoint(0, sf::Vector2f(x, y));
        convex.setPoint(1, sf::Vector2f(x2, y2));
        convex.setPoint(2, sf::Vector2f(x3, y3));
        convex.setFillColor(sf::Color(i, i, i));

        printf((std::to_string(x2) + "\n").c_str());
        printf((std::to_string(y2) + "\n").c_str());
        return convex;
    }
};

int main()
{
    Game g;
    g.Start();

    return 0;
}
