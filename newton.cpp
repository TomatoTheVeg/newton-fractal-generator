#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "newtonApprox.h"

#define MAX_ITER 50
#define MIN_STEP2 0.0001f

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

void writePPM(FrameBuff buff, const char* path){
    std::ofstream out("newton_fractal.ppm", std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file.\n";
        return;
    }
    out<< "P6\n" << buff.width<<" "<<buff.height<<"\n255\n";

    for(int i =0; i<buff.height;++i){
        for(int j = 0; j<buff.width;++j){
            //out.write(reinterpret_cast<const char*>(rootColors + pic[i][j].approaches), sizeof(struct RGB));
            //printf(" %d  ", pic[i][j].approaches);
        }
        printf("\n\n");
    }
    out.close();
}



int main() {
    int N = 3;
    size_t width = 5;
    size_t height= 5;
    Points points = Points(width, height);
    Roots roots = Roots(N);
    FrameBuff buff = FrameBuff(width, height);

    ispc::approx_ispc(width, height, roots.reRoots.data(), roots.imRoots.data(), N, points.re.data(), points.im.data(), buff.red.data(), buff.green.data(), buff.blue.data(), (unsigned short)MAX_ITER, MIN_STEP2);

    /*
    for(int i =0; i< N ; ++i){
        printf("Root %d: re: %f, im: %f\n", i, roots.reRoots.at(i), roots.imRoots.at(i));
    }
    
    */

    for(int i = 0; i< height; ++i){
        for(int j = 0; j< width; ++j){
            printf("re: %f, im: %f ", points.re.at(i*width + j), points.im.at(i*width + j));
        }
        printf("\n\n");
    }
        
    return 0;
}