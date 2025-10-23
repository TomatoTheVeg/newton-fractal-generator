#include <iostream>
#include <fstream>
#include <cmath>

#define WIDTH 100
#define LENGTH 100
#define MAX_ITER 50
#define MIN_STEP2 0.0001f

typedef struct Point 
{
    float re;
    float im;
    short approaches;
} Point;

struct RGB{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

struct RGB colorDamening(struct RGB color, float brightness){
    return {color.red*brightness, color.green*brightness, color.blue*brightness};
}

inline float len2(const Point* a, const Point* b) {
    float dr = a->re - b->re;
    float di = a->im - b->im;
    return dr*dr + di*di;
}

short nearestRoot(Point* point, Point roots[], short n){
    int nearestRoot = -1;
    float minDistance = __FLT32_MAX__;
    for(int i = 0;i<n;++i){
        float temp = len2(&roots[i],point);
        if(temp<minDistance){
            minDistance = temp;
            nearestRoot = i+1;
        }
    }
    return nearestRoot;
}

Point muliply(Point point1, Point point2){
    return {point1.re*point2.re - point1.im*point2.im, point1.re*point2.im + point1.im*point2.re};
}
Point conjugate(Point point){
    return {point.re, -point.im, 0};
}

Point divide(Point point1, Point point2){
    Point temp = muliply(point1, conjugate(point2));
    float len = point2.re*point2.re + point2.im*point2.im;
    temp.re/= len;
    temp.im/=len;
    return temp;
}

Point pointPow(Point base, int exp) {
    Point result = {1.0f, 0.0f, 0};  
    while (exp > 0) {
        if (exp & 1)                 
            result = muliply(result, base);
        base = muliply(base, base);  
        exp >>= 1;                   
    }
    return result;
}

Point func(Point x, int power){
    Point answer = pointPow(x, power);
    --answer.re;
    return answer;
}

Point derivative(Point point, int power){
    Point answer = pointPow(point, power-1);
    answer.re*=power;
    answer.im*=power;
    return answer;
}

void approx(int power, Point* point, Point roots[]){
    int counter = 0;
    bool landed = false;
    Point temp = {point->re, point->im, 0};
    short nR= 0;
    while(counter <=MAX_ITER){
        ++counter;
        Point der = derivative(temp, power);
        if(der.re == 0&& der.im == 0){
            break;
        }
        Point fraction = divide(func(temp, power), der);
        temp.re = temp.re - fraction.re;
        temp.im = temp.im - fraction.im;
        //nR = nearestRoot(temp, roots, power);
    }
    nR = nearestRoot(&temp, roots, power);
    point->approaches = nR;
}

void printPoint(Point point){
    printf("\tRe: %f   Im: %f |||", point.re, point.im, point.approaches);
}

int main() {
    int N = 5;
    Point* roots = new Point[N];
    for(int i = 0;i < N; ++i){
        roots[i].re = std::cos(2*M_PI*i/N);
        roots[i].im = std::sin(2*M_PI*i/N);
        printf("Root %d:\n\tRe: %f\n\tIm: %f \n", i, roots[i].re, roots[i].im);
    }
    struct RGB rootColors[9]= {
        {0,0,0},            // Black
        {255,   0,   0},  // Red
        {  0, 255,   0},  // Green
        {  0,   0, 255},  // Blue
        {255, 255,   0},  // Yellow
        {255, 165,   0},  // Orange
        {128,   0, 128},  // Purple
        {  0, 255, 255},  // Cyan
        {255, 192, 203},  // Pink
    };
    Point test = {1, 1, 0};
    //printPoint(test);
    approx(N, &test, roots);
    //printPoint(test);
    
    int width = 3000;
    int height = 3000;
    Point **pic = new Point*[height];
    for(int i = 0;i<height;++i){
        pic[i] = new Point[width];
    }

    for(int i =0; i<height;++i){
        for(int j = 0; j<width;++j){
            pic[i][j] = {((float)j/width -0.5f)*2, ((float)i/height -0.5f)*2, 0};
        }
    }

    for(int i =0; i<height;++i){
        for(int j = 0; j<width;++j){
            approx(N, &pic[i][j], roots);
            //printf(" %d  ", pic[i][j].approaches);
        }
        printf("\n\n");
    }


    //Write a ppm
    std::ofstream out("newton_fractal.ppm", std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file.\n";
        return 1;
    }
    out<< "P6\n" << width<<" "<<height<<"\n255\n";

    for(int i =0; i<height;++i){
        for(int j = 0; j<width;++j){
            out.write(reinterpret_cast<const char*>(rootColors + pic[i][j].approaches), sizeof(struct RGB));
            //printf(" %d  ", pic[i][j].approaches);
        }
        printf("\n\n");
    }
    out.close();
    printf("Done");
    return 0;
}