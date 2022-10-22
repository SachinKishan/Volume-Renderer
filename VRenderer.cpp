// VRenderer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#pragma comment(lib,"libtiff.lib")
#include <iostream>
#include "lodepng.h"
#include "ImageHeader.h"
#include "Sphere.h"
#include "Box.h"
#include "Matrix.h"
//#include "tiffio.h"


vec3 minPos(-3, -3, -3);
vec3 maxPos(3, 3, 3);

std::unique_ptr<Grid> grid = std::make_unique<Grid>(minPos, maxPos);

const vec3 background_color(0.572f, 0.772f, 0.921f);
float evalDensity(std::unique_ptr<Grid> grid, const point3& p)
{
	vec3 gridSize = grid->bounds[1] - grid->bounds[0];
	vec3 pLocal = (p - grid->bounds[0]) / gridSize[0];
	vec3 pVoxel = pLocal * grid->dimension;

	int xi = static_cast<int>(std::floor(pVoxel.x()));
	int yi = static_cast<int>(std::floor(pVoxel.y()));
	int zi = static_cast<int>(std::floor(pVoxel.z()));

	// nearest neighbor
	return grid->density[(zi * grid->dimension + yi) * grid->dimension + xi];
}

vec3 integrate(ray r, const std::vector<std::unique_ptr<Object>>& objects)
{
	const Object* hit_object = nullptr;

	//first find if the ray actually hits an object
	IsectData isect;
	for (const auto& object : objects) {
		IsectData isect_object;
		if (object->intersect(r, isect_object)) {
			hit_object = object.get();
			isect = isect_object;
		}
	}
	
	//if ray hits nothing, pixel is given background color
	if (!hit_object)
		return background_color;
	
	//properties for forward raymarching to use
	float step_size = 0.1;//step size for moving in the object when we make a hit


	float absorption = 0;/*sigma - absorption coefficient
	transmittance is the amount of light that passed through after being absorbed
	transmittance ranges from 0 to 1
	absorption ranges from 0 to 1
	1=absorption+transmittance
	*/

	float scattering = 1;// light that gets scattered to the eye
	float density = 1;//assuming the object is uniform density sphere
	//T= exp^(-distance * sigma) where T is the amount of light that transmits
	//calculate number of steps and a more accurate step size
	int ns = std::ceil((isect.t1 - isect.t0) / step_size);
	step_size = (isect.t1 - isect.t0) / ns;

	//vec3 light_dir{ 0, -1, 0 };
	vec3 light_dir(-0.315798, 0.719361, 0.618702);
	vec3 light_color{ 1.3, 0.3, 0.9 };
	IsectData isect_vol;

	float transparency = 1;  //initialize transmission to 1 (fully transparent medium) 
	vec3 result;  //initialize volumetric sphere color to 0
	for (int n = 0; n < ns; ++n) 
	{
		float t = isect.t0 + step_size * (n + 0.5);//take the middle of step
		vec3 sample_pos = r.orig + t * r.dir;//take that position of the ray
		//float d = evalDensity(std::move(grid), sample_pos);

		// compute sample transmission
		float sample_attenuation = exp(-step_size * (scattering + absorption));//compute how much light actually passes
		transparency *= sample_attenuation;//multiply each time a step is gathered to show much from the background is being transmitted

		//light in scattering from external light source. 
		ray lightray(sample_pos, light_dir);
		
		if (hit_object->intersect(lightray, isect_vol) && isect_vol.inside) {
			float light_attenuation = exp(-density * isect_vol.t1 * (scattering + absorption));
			result += transparency * light_color * light_attenuation * scattering * density * step_size;
		}
		else;
			//std::cerr << "oops\n";
	}
	
	// combine background color and volumetric sphere color
	return background_color*transparency + result;

}



vec3 traceScene(ray r, Sphere *sphere)//using a basic sphere that 'emits' light
{
	double t0, t1;
	IsectData iData;
	if (sphere->intersect(r,iData))
	{
		//std::cout << t0 << " " << t1 << std::endl;
		vec3 p1 = r.origin() + r.direction() * iData.t0;
		vec3 p2 = r.origin() + r.direction() * iData.t1;

		float distance = (p2 - p1).length();
		float transmission = exp(-distance * sphere->sigma);
		return background_color * transmission + sphere->scatter * (1 - transmission);

	}
	else return background_color;
}

#pragma region PNG
void encodeOneStep(const char* filename, std::vector<unsigned char>& image, unsigned width, unsigned height) {
	//Encode the image
	unsigned error = lodepng::encode(filename, image, width, height);

	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}
#pragma endregion

int main() {

	// World
	//TODO: Make this easier



	//ScaleMatrix scale(15);
	//scale.multVecMatrix(minPos, minPos);
	//scale.multVecMatrix(maxPos, maxPos);

	RotationMatrixX xRot(45);
	RotationMatrixY yRot(45);
	RotationMatrixZ zRot(45);

	//xRot.multVecMatrix(minPos,minPos);
	//xRot.multVecMatrix(maxPos,maxPos);
	//yRot.multVecMatrix(minPos, minPos);
	//yRot.multVecMatrix(maxPos,maxPos);
	//zRot.multVecMatrix(minPos,minPos);
	//zRot.multVecMatrix(maxPos,maxPos);

	// allocate memory to read the data from the cache file
	size_t numVoxels = grid->dimension * grid->dimension * grid->dimension;
	// feel free to use a unique_ptr if you want to (the sample program does)
	grid->density = new float[numVoxels];
	//std::ifstream cacheFile("./mrbrain-8bit/mrbrain-8bit035.tif", std::ios::binary);
	//TIFF* tif = TIFFOpen("./mrbrain-8bit/mrbrain-8bit035.tif", "r");
	// read the density values in memory
	//cacheFile.read((char*)grid->density, sizeof(float) * numVoxels);
	//if (!tif){std::cout << "file not found"; return 0;}
	//TIFFClose(tif);

	/*for (int i = 0; i < 127; i++)
	{
		std::cout << grid->density[i];
	}*/
	/*

	std::unique_ptr<Sphere> sph = std::make_unique<Sphere>(vec3(0, 0, -4), vec3(1.0, 1.0, 1.0), 1, 0.75f);
	std::vector<std::unique_ptr<Object>> world;


	world.push_back(std::move(grid));

	//generate image
	const char* filename = "out.png";
	std::vector<unsigned char> image;
	Image::Rgb color;
	const auto aspect_ratio = 1;
	const int image_width = 512;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	int total = image_width * image_height;

	// Camera
	auto viewport_height = 2.0;
	auto viewport_width = aspect_ratio * viewport_height;
	auto focal_length = 1.0;
	auto origin = point3(0, 0, 0);
	auto horizontal = vec3(viewport_width, 0, 0);
	auto vertical = vec3(0, viewport_height, 0);
	auto lower_left_corner = origin - horizontal / 2 - vertical / 2 - vec3(0, 0, focal_length);
	
	//lookat matrix
	Matrix44<float> look;
	look.lookat(vec3(-10,0,-25), vec3(0, 0, 0), vec3(0, 1, 0));

	//image resizing
	image.resize(image_width * image_height * 4);

	//ray tracing
	for (unsigned y = 0; y < image_height; y++)
	{
		for (unsigned x = 0; x < image_width; x++) {
			auto u = double(x) / (image_height - 1);
			auto v = double(y) / (image_width - 1);

			vec3 rayOrigin = origin;
			vec3 rayDir(lower_left_corner + u * horizontal + v * vertical - origin);
			rayDir[2] = -1;
			rayOrigin = look.multiplyVectorMatrix(rayOrigin);
			rayDir = look.multiplyVectorMatrix(rayDir);
			rayDir = rayDir - rayOrigin;

			rayDir = unit_vector(rayDir);

			ray r(rayOrigin,rayDir);
			vec3 col = integrate(r, world);

			//calculate color;
			color.r = col[0];
			color.g = col[1];
			color.b = col[2];
			//std::cout <<std::endl<< color.r<<" " << color.g << " " << color.b;

#pragma region Image File Color Translation
			//conversion to unsigned char
			unsigned char red = static_cast<unsigned char>(std::min(1.f, color.r) * 255);
			unsigned char g = static_cast<unsigned char>(std::min(1.f, color.g) * 255);
			unsigned char b = static_cast<unsigned char>(std::min(1.f, color.b) * 255);
			//allot color
			int val = 4 * image_width * y + 4 * x;
			image[val + 0] = red;
			image[val + 1] = g;
			image[val + 2] = b;
			image[val + 3] = 255;//alpha
#pragma endregion
		}
	}
	encodeOneStep(filename, image, image_width, image_height);*/
}
