
struct Complex { varying double re, im; };

extern void multiply(uniform size_t n, uniform double aRe[], uniform double aIm[], uniform double bRe[], uniform double bIm[]){
    foreach(i =0 ... n){
        double reT = aRe[i];
        aRe[i] = aRe[i]*bRe[i] - aIm[i]*bIm[i];
        aIm[i] = reT*bIm[i] + aIm[i]*bRe[i];
    }
}

inline Complex multiplySingle(double re1, double im1, double re2, double im2){
    Complex r;
    r.re = re1*re2 - im1*im2;
    r.im = re1*im2 + im1*re2;
    return r;
}

Complex pointPowSingle(double re, double im, uniform uint16 expo){
    double reB = 1.0;
    double imB = 0.0;
    while (expo > 0) {
        if (expo & 1){
            Complex c = multiplySingle(reB, imB, re, im);
            reB = c.re;
            imB = c.im;
        }                 
        Complex c = multiplySingle(re, im, re, im); 
        re = c.re;
        im = c.im;
        expo >>= 1;                   
    }
    Complex out;
    out.re = reB;
    out.im = imB;
    return out;
}

Complex mulConst(double re, double im, double con){
    Complex c;
    c.re = re*con;
    c.im = im*con;
    return c;
}


export void pointPow(uniform size_t n, uniform double re[], uniform double im[], uniform uint16 expo, uniform double reBuff[], uniform double imBuff[]) {
    foreach(i =0 ... n){
        reBuff[i] = 1;
        imBuff[i] = 0;
    }
    for(int j = 0; j<expo; ++j) {
        multiply(n, reBuff, imBuff, re, im);                 
    }
}