export void squareTest(uniform double re[], uniform double im[], uniform size_t n){
    foreach(i = 0 ... n){
        re[i] = re[i]*re[i];
        im[i] = im[i]*im[i];
    }
}

void multiply(uniform size_t n, uniform double aRe[], uniform double aIm[], uniform double bRe[], uniform double bIm[]){
    foreach(i =0 ... n){
        double reT = aRe[i];
        aRe[i] = aRe[i]*bRe[i] - aIm[i]*bIm[i];
        aIm[i] = reT*bIm[i] + aIm[i]*bRe[i];
    }
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