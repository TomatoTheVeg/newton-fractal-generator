#include "complexISPC.isph"

#define TWO_PI 6.28318530717958623199592693709

void calculateRoots(uniform int16 power, uniform double reRoot[], uniform double imRoot[]){
    uniform double invPower = 1.0/power;
    foreach (i = 0 ... power){
        reRoot[i] = cos(TWO_PI * i * invPower);
        imRoot[i] = sin(TWO_PI * i * invPower);
    }
}

void fillEmptyPoints(uniform size_t width, uniform size_t height, uniform double re[], uniform double im[]){
    uniform size_t len = width*height;
    uniform double invWidth = 1.0/width;
    uniform double invHeight = 1.0/height;
    foreach(i = 0 ... len){
        size_t y = floor(i*invHeight);
        size_t x = i - floor(i* invWidth)*width;
        re[i] = (x * invWidth -0.5)*2;
        im[i] = (y * invHeight -0.5)*2;
    }
}

export void approx_ispc(uniform size_t width, uniform size_t height,
                    uniform double reRoot[], uniform double imRoot[],
                    uniform uint16 power,
                    uniform double re[], uniform double im[],
                    uniform uint8 r[], uniform uint8 g[], uniform uint8 b[],
                    uniform uint16 maxIterations, uniform double minDiff){
    calculateRoots(power, reRoot, imRoot);
    fillEmptyPoints(width, height, re, im);
    uniform double *uniform reBuff = uniform new double[width*height];
    uniform double *uniform imBuff = uniform new double[width*height];
    uniform size_t len = height*width;
    /*
    foreach(i = 0 ... len){
        uint32 counter = 0;


        for(uint16 i = 0; i<maxIterations; ++i){
            ++counter;
        },
    }*/
    pointPow(len,re, im, power, reBuff, imBuff);
    
    foreach(i =0 ... len){
        re[i] = reBuff[i];
        im[i] = imBuff[i];
    }
    delete[] reBuff;
    delete[] imBuff;

}