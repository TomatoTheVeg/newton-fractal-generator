//#include "complexISPC.isph"

#define TWO_PI 6.28318530717958623199592693709
#define EPSILON 1e-12

struct RGB{
    uint8 red;
    uint8 green;
    uint8 blue;
};


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
        re[i] = (x * invWidth -0.5)*4;
        im[i] = (y * invHeight -0.5)*4;
    }
}

inline void multiplySingle(double* re1, double* im1, double* re2, double* im2){
    double reT1 = *re1;
    double reT2 = *re2;
    *re1 = reT1*reT2 - *(im1)*(*im2);
    *im1 = reT1*(*im2) + *(im1)*reT2;
}

inline void pointPowSingle(double* re, double* im, uniform uint16 expo, double* reB, double* imB){
    *reB = 1.0;
    *imB = 0.0;
    double reT = *re;
    double imT = *im;
    while (expo > 0) {
        if (expo & 1){
            multiplySingle(reB, imB, re, im);
        }                 
        multiplySingle(re, im, re, im); 
        expo >>= 1;                   
    }
    *re = reT;
    *im = imT;
}

inline double len2(double re1, double im1, double re2, double im2){
    return (re1-re2)*(re1-re2) + (im1-im2)*(im1-im2);
}


inline void mulConst(double* re, double* im, double con){
    *re *= con;
    *im *= con;
}

inline void complexInverse(double* re, double* im){
    double len = (*re)*(*re) + (*im)*(*im);
    len = 1/len;

    mulConst(re, im, len);

    *im *= -1;
}

inline void complexSum(double* re1, double* im1, double* re2, double* im2){
    *re1 += *re2;
    *im1 += *im2;
}

inline void writeColorFromIdx(uint16 idx3, uint8 *r, uint8 *g, uint8 *b) {
    switch (idx3) {
        case 0: *r = 255; *g =   0; *b =   0; break; // Red
        case 1: *r =   0; *g = 255; *b =   0; break; // Green
        case 2: *r =   0; *g =   0; *b = 255; break; // Blue
        case 3: *r = 255; *g = 255; *b =   0; break; // Yellow
        case 4: *r = 255; *g = 165; *b =   0; break; // Orange
        case 5: *r = 128; *g =   0; *b = 128; break; // Purple
        case 6: *r =   0; *g = 255; *b = 255; break; // Cyan
        default:*r = 255; *g = 192; *b = 203; break; // Pink
    }
}

inline double nearestRoot(double re, double im, 
                        uniform double* reRoot, uniform double* imRoot, uniform uint16 power,
                        uint8* r, uint8* g, uint8* b){
    double minLen = len2(re, im, reRoot[0], imRoot[0]);
    uint16 nearestRoot= 0;
    for(int j = 1; j<power; ++j){
        double currLen = len2(re, im, reRoot[j], imRoot[j]);
        if(minLen > currLen){
            minLen = currLen;
            nearestRoot = j;
        }
    }
    writeColorFromIdx(nearestRoot & 7, r, g, b);
    return minLen;
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
    uniform double invPower = 1.0/power;
    uniform float invMaxIter = 1.0/maxIterations;

    
    foreach(i = 0 ... len){
        uint32 counter = 0;
        
        
        for(uint16 iter = 0; iter<maxIterations; ++iter){
            ++counter;
            double minLen = nearestRoot(re[i], im[i], reRoot, imRoot, power, r+i, g+i, b+i);
            if(minLen < minDiff){
                break;
            }

            pointPowSingle(re + i, im + i, power-1, reBuff + i, imBuff + i); //basically derivaive
            if(abs(reBuff[i])<EPSILON && abs(imBuff[i])<EPSILON){
                r[i] = 0;
                g[i] = 0;
                b[i] = 0;
                break;
            }
            complexInverse(reBuff+ i, imBuff + i);
        
            mulConst(re + i, im + i, power-1);
            complexSum(re + i, im + i, reBuff + i, imBuff + i);
            mulConst(re + i, im + i, invPower);
        }

            r[i] = round(r[i] * (1-counter*invMaxIter));
            g[i] = round(g[i] * (1-counter*invMaxIter));
            b[i] = round(b[i] * (1-counter*invMaxIter));
    }
    
    //pointPowSingle(re, im, power, reBuff, imBuff);

    delete[] reBuff;
    delete[] imBuff;

}