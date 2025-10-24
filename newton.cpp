#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "newtonApprox.h"

#define MAX_ITER 25
#define MIN_STEP2 0.0000001f

typedef struct Points 
{
    std::vector<double> re;
    std::vector<double> im;
    std::vector<short> approaches;
    std::vector<float> convSpeed;

    Points(size_t width, size_t height){
        re.resize(width*height);
        im.resize(width*height);
        approaches.resize(width*height);
        convSpeed.resize(width*height);
    }

} Points;

typedef struct Roots{
    std::vector<double> reRoots;
    std::vector<double> imRoots;

    Roots(short power){
        reRoots.resize(power);
        imRoots.resize(power);
    }
} Roots;

typedef struct FrameBuff{
    size_t width;
    size_t height;

    std::vector<unsigned char> red;
    std::vector<unsigned char> green;
    std::vector<unsigned char> blue;

    FrameBuff(size_t width, size_t height){
        this->width = width;
        this->height = height;
        red.resize(width*height);
        green.resize(width*height);
        blue.resize(width*height);
    }

} FrameBuff;

/*
void printPoint(Point point){
    printf("\tRe: %f   Im: %f |||", point.re, point.im, point.approaches);
}*/


void writePPM(const FrameBuff &fb, const std::string &filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out)
        throw std::runtime_error("Cannot open " + filename);

    // Write header (P6 = binary RGB)
    out << "P6\n" << fb.width << " " << fb.height << "\n255\n";

    // Interleave channels and write pixel data
    const size_t N = fb.width * fb.height;
    std::vector<unsigned char> row(3 * fb.width);

    for (size_t y = 0; y < fb.height; ++y) {
        for (size_t x = 0; x < fb.width; ++x) {
            size_t i = y * fb.width + x;
            row[3 * x + 0] = fb.red[i];
            row[3 * x + 1] = fb.green[i];
            row[3 * x + 2] = fb.blue[i];
        }
        out.write(reinterpret_cast<const char*>(row.data()), row.size());
    }
}


int main() {
    int N = 3;
    size_t width = 15000;
    size_t height= 15000;
    Points points = Points(width, height);
    Roots roots = Roots(N);
    FrameBuff buff = FrameBuff(width, height);

    ispc::approx_ispc(width, height, roots.reRoots.data(), roots.imRoots.data(), N, points.re.data(), points.im.data(), buff.red.data(), buff.green.data(), buff.blue.data(), (unsigned short)MAX_ITER, MIN_STEP2);


    writePPM(buff, "NEWTON.ppm");
    /*
    for(int i =0; i< N ; ++i){
        printf("Root %d: re: %f, im: %f\n", i, roots.reRoots.at(i), roots.imRoots.at(i));
    }
    
    */

    // for(int i = 0; i< height; ++i){
    //     for(int j = 0; j< width; ++j){
    //         printf("re: %f, im: %f ", points.re.at(i*width + j), points.im.at(i*width + j));
    //     }
    //     printf("\n\n");
    // }
        
    return 0;
}