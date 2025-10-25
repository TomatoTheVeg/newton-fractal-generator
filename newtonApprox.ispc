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
        size_t y = floor(i*invWidth);
        size_t x = i - y*width;
        re[i] = (x * invWidth -0.5)*4;
        im[i] = (y * invHeight -0.5)*4;
    }
}

inline void multiplySingle(double &re1, double &im1, double re2, double im2){
    double reT1 = re1;
    double reT2 = re2;
    re1 = reT1*reT2 - im1*im2;
    im1 = reT1*im2 + im1*reT2;
}

inline void pointPowSingle(double* re, double* im, uniform uint16 expo){
    double reB = 1.0;
    double imB = 0.0;
    double baseRe = *re;
    double baseIm = *im;
    while (expo > 0) {
        if (expo & 1){
            multiplySingle(reB, imB, baseRe, baseIm);
        }                 
        multiplySingle(baseRe, baseIm, baseRe, baseIm); 
        expo >>= 1;                   
    }
    *re = reB;
    *im = imB; 
}

inline double len2(double re1, double im1, double re2, double im2){
    return (re1-re2)*(re1-re2) + (im1-im2)*(im1-im2);
}


inline void mulConst(double &re, double &im, double con){
    re *= con;
    im *= con;
}

inline void complexInverse(double* re, double* im){
    double delta = 1e-16;
    double len = (*re)*(*re) + (*im)*(*im) + delta;
    len = 1/len;

    *re *= len;
    *im *= len;

    *im *= -1;
}

inline void complexSum(double *re1, double *im1, double re2, double im2){
    *re1 += re2;
    *im1 += im2;
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

inline double minDistToRoots(double re, double im,
                             uniform double* reRoot, uniform double* imRoot,
                             uniform uint16 power) {
    double minLen = len2(re, im, reRoot[0], imRoot[0]);
    for (int j = 1; j < power; ++j) {
        double currLen = len2(re, im, reRoot[j], imRoot[j]);
        if (currLen < minLen) minLen = currLen;
    }
    return minLen;
}

inline void nearestRoot(double re, double im, 
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
}

task void approxRow(uniform size_t start, uniform size_t end, 
                    uniform double reRoot[], uniform double imRoot[],
                    uniform uint16 power,
                    uniform double re[], uniform double im[],
                    uniform uint8 r[], uniform uint8 g[], uniform uint8 b[],
                    uniform uint16 maxIterations, uniform double minDiff){
    uniform double invPower = 1.0/power;
    uniform float invMaxIter = 1.0/maxIterations;

    foreach(i = start ... end){
        uint32 counter = 0;
        
        
        for(uint16 iter = 0; iter<maxIterations; ++iter){
            ++counter;
            double minLen = minDistToRoots(re[i], im[i], reRoot, imRoot, power);
            if(minLen < minDiff){
                break;
            }
            double reT = re[i];
            double imT = im[i];

            pointPowSingle(re + i, im + i, power-1); //basically derivaive
            complexInverse(re+ i, im + i);
        
            mulConst(reT, imT, power-1);
            complexSum(re+i, im+i, reT, imT);
            //mulConst(re[i], im[i], invPower);
            re[i] *= invPower;
            im[i] *= invPower;
        }
            nearestRoot(re[i], im[i], reRoot, imRoot, power, r+i, g+i, b+i);
            r[i] = round(r[i] * (1-counter*invMaxIter));
            g[i] = round(g[i] * (1-counter*invMaxIter));
            b[i] = round(b[i] * (1-counter*invMaxIter));
    }
}

export void approxISPC(uniform size_t width, uniform size_t height,
                    uniform double reRoot[], uniform double imRoot[],
                    uniform uint16 power,
                    uniform double re[], uniform double im[],
                    uniform uint8 r[], uniform uint8 g[], uniform uint8 b[],
                    uniform uint16 maxIterations, uniform double minDiff){
    calculateRoots(power, reRoot, imRoot);
    fillEmptyPoints(width, height, re, im);
    uniform size_t len = height*width;
    
    for(uniform int i = 0; i < height; ++i){
        launch[1] approxRow(width*i, width*(i+1), reRoot, imRoot, power, re, im, r, g, b, maxIterations, minDiff);
    }
    //pointPowSingle(re, im, power, reBuff, imBuff);
    sync;

}