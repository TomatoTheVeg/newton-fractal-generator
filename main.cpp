#include <iostream>
#include <cmath>

#define WIDTH 100
#define LENGTH 100
#define MAX_ITER 20
#define MIN_STEP2 0.0001f

typedef struct Point 
{
    float re;
    float im;
    short approaches;
} Point;

float len2(Point* point1, Point* point2){
    return (point1->re-point2->re)*(point1->re-point2->re) + (point1->im-point2->im)*(point1->im-point2->im);
}

short nearestRoot(Point* point, Point roots[], short n){
    int nearestRoot = -1;
    float minDistance = __FLT32_MAX__;
    for(int i = 0;i<n;++i){
        float temp = len2(&roots[i],point);
        if(temp<minDistance){
            minDistance = temp;
            nearestRoot = i;
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

Point pointPow(Point point, int power){
    Point temp = point;
    for(int i = 1;i<power;++i){
        temp = muliply(temp, point);
    }
    return temp;
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
    Point* temp = new Point{point->re, point->im, 0};
    short nR= 0;
    while(counter <=MAX_ITER&&!landed){
        ++counter;
        Point fraction = divide(func(*temp, power), derivative(*temp, power));
        temp->re = temp->re - fraction.re;
        temp->im = temp->im - fraction.im;
        nR = nearestRoot(temp, roots, power);
        if(len2(&roots[nR], temp)<MIN_STEP2){
            landed = true;
        }
    }
    point->approaches = nR;
}

void printPoint(Point point){
    printf("\tRe: %f   Im: %f |||", point.re, point.im, point.approaches);
}

int main() {
    int N = 3;
    Point* roots = new Point[N];
    for(int i = 0;i < N; ++i){
        roots[i].re = std::cos(2*M_PI*i/N);
        roots[i].im = std::sin(2*M_PI*i/N);
        printf("Root %d:\n\tRe: %f\n\tIm: %f \n", i, roots[i].re, roots[i].im);
    }
    Point test = {1, 1, 0};
    //printPoint(test);
    approx(N, &test, roots);
    //printPoint(test);
    
    int width = 100;
    int height = 100;
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
            printf(" %d  ", pic[i][j].approaches);
        }
        printf("\n\n");
    }
    return 0;
}