#include <iostream>
#include <cstdint>

#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>

#include "Bitmap.h"
#include "geometry.h"

using namespace std;

struct Plane
{
    Vec3f center, pl_normal, color;

    Plane(Vec3f c, Vec3f p, Vec3f cl): center(c), pl_normal(p), color(cl) {}

    bool intersect(Vec3f point, Vec3f dir, float &dist, Vec3f &normal) {
        Vec3f to_plane = center - point;

        float prod1 = dir * pl_normal, prod2 = to_plane * pl_normal;
        if (fabs(prod1) < 0.001) return false;
        dist = prod2 / prod1;

        normal = pl_normal;

        return dist > 0;
    }
};

struct Sphere
{
    Vec3f center, color;
    float radius;

    Sphere(Vec3f c, float r, Vec3f cl): center(c), radius(r), color(cl) {}

    bool intersect(Vec3f point, Vec3f dir, float &dist, Vec3f &normal) {
        Vec3f to_sphere = center - point;

        float AB = dir * to_sphere;
        if (AB < 0.01) return false;

        float CD = radius;
        float CB = sqrt(to_sphere * to_sphere - AB * AB);
        if (CB > CD) return false;
        float DB = sqrt(CD * CD - CB * CB);
        float AD = AB - DB;
        dist = AD;
        normal = (point + dir * dist - center).normalize();

        return true;
    }
};

Plane planes[] = {
    {Vec3f(-1, -1, 0), Vec3f(0, 1, -0.125).normalize(), Vec3f(0, 0, 0)},
    {Vec3f(-1, -1, 0), Vec3f(1, 0, -0.125).normalize(), Vec3f(0, 0, 0)},
};

Sphere spheres[] = {
    {Vec3f(0, 0, 2), 1, Vec3f(1.0, 1.0, 1.0)},
    {Vec3f(-.4, 2, 2), .6, Vec3f(0.6,  0.3, 0.1)},
};

bool trace(Vec3f point, Vec3f dir, float &dist, Vec3f &normal, Vec3f &color)
{
    float cur_dist;
    Vec3f cur_normal;
    dist = 1e9;
    bool hit = false;

    for (int i = 0; i < sizeof(planes) / sizeof(Plane); ++i) {
        if (planes[i].intersect(point, dir, cur_dist, cur_normal) && cur_dist < dist) {
            dist = cur_dist;
            color = planes[i].color;
            normal = cur_normal;
            hit = true;
        }
    }

    for (int i = 0; i < sizeof(spheres) / sizeof(Sphere); ++i) {
        if (spheres[i].intersect(point, dir, cur_dist, cur_normal) && cur_dist < dist) {
            dist = cur_dist;
            color = spheres[i].color;
            normal = cur_normal;
            hit = true;
        }
    }

    return hit;
}

Vec3f light(2, 2, 0);

Vec3f get_color(Vec3f point, Vec3f dir, int d)
{
    Vec3f color(.3, .3, 1), cur_color, normal;
    float dist;

    if (trace(point, dir, dist, normal, cur_color))
    {
        color = cur_color;

        point = point + dir * dist + normal * 0.01;
        dir = dir - normal * (normal * dir) * 2;
        Vec3f ldir = light - point;
        Vec3f normal_;
        float light_intensity = 10 / (ldir * ldir);
        float light_dist = ldir.norm();
        float lightness = 0.2, specular = 0;
        ldir = ldir.normalize();

        bool hit = trace(point, ldir, dist, normal_, cur_color);
        if(!hit || dist > light_dist) {
            lightness += max(0.f, ldir * normal) * light_intensity;
            specular += pow(max(0.f, dir * ldir), 30) * light_intensity;
        }

        color = color * lightness + Vec3f(1, 1, 1) * specular;

        if (d)
        {
            color = color * .6 + get_color(point, dir, d - 1) * .4;
        }
    }


    return color;
}

int main(int argc, const char** argv)
{
	std::unordered_map<std::string, std::string> cmdLineParams;

	for(int i=0; i<argc; i++)
	{
		std::string key(argv[i]);

		if(key.size() > 0 && key[0]=='-')
		{
			if(i != argc-1) // not last argument
			{
				cmdLineParams[key] = argv[i+1];
				i++;
			}
			else
				cmdLineParams[key] = "";
		}
	}

	std::string outFilePath = "zout.bmp";
	if(cmdLineParams.find("-out") != cmdLineParams.end())
		outFilePath = cmdLineParams["-out"];

	int sceneId = 1;
	if(cmdLineParams.find("-scene") != cmdLineParams.end())
		sceneId = atoi(cmdLineParams["-scene"].c_str());

    if (sceneId != 1) return 0;

    int h = 512, w = 512;
	std::vector<uint32_t> image(h * w);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            Vec3f point(0, 0, -1), dir(Vec3f(i/256. - 1, j/256. - 1, 1).normalize());
            Vec3f color = get_color(point, dir, 2);

            for (int k = 0; k < 3; ++k) {
                image[i * h + j] = (image[i * h + j] << 8) + int(min(1.f, color[2 - k]) * 255);
            }
        }
    }

	SaveBMP(outFilePath.c_str(), image.data(), h, w);

	std::cout << "end." << std::endl;
	return 0;
}
