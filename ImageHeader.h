#pragma once
#include <cassert>
#include <cstring>
#include <fstream>
#include <utility>

class Image
{
public:
    // Rgb structure, i.e. a pixel 
    struct Rgb
    {
        Rgb() : r(0), g(0), b(0) {}
        Rgb(float c) : r(c), g(c), b(c) {}
        Rgb(float _r, float _g, float _b) : r(_r), g(_g), b(_b) {}
        bool operator != (const Rgb& c) const
        {
            return c.r != r || c.g != g || c.b != b;
        }
        Rgb& operator *= (const Rgb& rgb)
        {
            r *= rgb.r, g *= rgb.g, b *= rgb.b; return *this;
        }
        Rgb& operator += (const Rgb& rgb)
        {
            r += rgb.r, g += rgb.g, b += rgb.b; return *this;
        }
        friend float& operator += (float& f, const Rgb rgb)
        {
            f += (rgb.r + rgb.g + rgb.b) / 3.f; return f;
        }
        float r, g, b;
    };

    unsigned int w, h; // Image resolution 
    Rgb* pixels; // 1D array of pixels 
    static const Rgb kBlack, kWhite, kRed, kGreen, kBlue; // Preset colors 

    Image() : w(0), h(0), pixels(NULL) { /* empty image */ }
    Image(const unsigned int& _w, const unsigned int& _h, const Rgb& c = kBlack) : w(_w), h(_h), pixels(NULL)//implicit constructor call
    {
        pixels = new Rgb[w * h];
        for (int i = 0; i < w * h; ++i)
            pixels[i] = c;
    }
    // copy constructor
    Image(const Image& img) : w(img.w), h(img.h), pixels(nullptr)
    {
        pixels = new Rgb[w * h];
        memcpy(pixels, img.pixels, sizeof(Rgb) * w * h);
    }
    Image& operator = (Image&& img)
    {
        if (this != &img) {
            if (pixels != nullptr) delete[] pixels;
            w = img.w, h = img.h;
            pixels = img.pixels;
            img.pixels = nullptr;
            img.w = img.h = 0;
        }
        return *this;
    }
    Image& operator /= (const float& div)
    {
        float invDiv = 1 / div;
        for (int i = 0; i < w * h; ++i)
            pixels[i] *= invDiv;

        return *this;
    }
    const Rgb& operator [] (const unsigned int& i) const
    {
        return pixels[i];
    }
    Rgb& operator [] (const unsigned int& i)
    {
        return pixels[i];
    }
    Rgb& operator () (const unsigned& x, const unsigned int& y) const
    {
        assert(x < w&& y < h);
        return pixels[y * w + x];
    }
    Image operator + (const Image& img)
    {
        Image tmp(*this);
        // add pixels to each other
        for (int i = 0; i < w * h; ++i)
            tmp[i] += img[i];

        return tmp;
    }
    Image& operator += (const Image& img)
    {
        for (int i = 0; i < w * h; ++i) pixels[i] += img[i];
        return *this;
    }
    Image& operator *= (const Rgb& rgb)
    {
        for (int i = 0; i < w * h; ++i) pixels[i] *= rgb;
        return *this;
    }
    friend Image operator * (const Rgb& rgb, const Image& img)
    {
        Image tmp(img);
        tmp *= rgb;
        return tmp;
    }
    static Image circshift(const Image& img, const std::pair<int, int>& shift)
    {
        Image tmp(img.w, img.h);
        int w = img.w, h = img.h;
        for (int j = 0; j < h; ++j) {
            int jmod = (j + shift.second) % h;
            for (int i = 0; i < w; ++i) {
                int imod = (i + shift.first) % w;
                tmp[jmod * w + imod] = img[j * w + i];
            }
        }

        return tmp;
    }
    ~Image()
    {
        if (pixels != NULL) delete[] pixels;
    }

};

const Image::Rgb Image::kBlack = Image::Rgb(0);
const Image::Rgb Image::kWhite = Image::Rgb(1);
const Image::Rgb Image::kRed = Image::Rgb(1, 0, 0);
const Image::Rgb Image::kGreen = Image::Rgb(0, 1, 0);
const Image::Rgb Image::kBlue = Image::Rgb(0, 0, 1);

Image readPPM(const char* filename)
{
    std::ifstream ifs;
    ifs.open(filename, std::ios::binary);
    // need to spec. binary mode for Windows users
    Image src;
    try {
        if (ifs.fail()) {
            throw("Can't open input file");
        }
        std::string header;
        int w, h, b;
        ifs >> header;
        if (strcmp(header.c_str(), "P6") != 0) throw("Can't read input file");
        ifs >> w >> h >> b;
        src.w = w;
        src.h = h;
        src.pixels = new Image::Rgb[w * h]; // this is throw an exception if bad_alloc 
        ifs.ignore(256, '\n'); // skip empty lines in necessary until we get to the binary data 
        unsigned char pix[3]; // read each pixel one by one and convert bytes to floats 
        for (int i = 0; i < w * h; ++i) {
            ifs.read(reinterpret_cast<char*>(pix), 3);
            src.pixels[i].r = pix[0] / 255.f;
            src.pixels[i].g = pix[1] / 255.f;
            src.pixels[i].b = pix[2] / 255.f;
        }
        ifs.close();
    }
    catch (const char* err) {
        fprintf(stderr, "%s\n", err);
        ifs.close();
    }

    return src;
}

void savePPM(const Image& img, const char* filename)
{
    if (img.w == 0 || img.h == 0) { fprintf(stderr, "Can't save an empty image\n"); return; }
    std::ofstream ofs;
    try {
        ofs.open(filename, std::ios::binary); // need to spec. binary mode for Windows users 
        if (ofs.fail()) throw("Can't open output file");
        ofs << "P6\n" << img.w << " " << img.h << "\n255\n";
        unsigned char r, g, b;
        // loop over each pixel in the image, clamp and convert to byte format
        for (int i = 0; i < img.w * img.h; ++i) {
            r = static_cast<unsigned char>(std::min(1.f, img.pixels[i].r) * 255);
            g = static_cast<unsigned char>(std::min(1.f, img.pixels[i].g) * 255);
            b = static_cast<unsigned char>(std::min(1.f, img.pixels[i].b) * 255);
            ofs << r << g << b;
        }
        ofs.close();
    }
    catch (const char* err) {
        fprintf(stderr, "%s\n", err);
        ofs.close();
    }
}