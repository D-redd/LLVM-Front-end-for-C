int diffProd(int a1, int b1, int a2, int b2)
{
    return a1 * b1 - a2 * b2;
}

int mdet3call(int a11, int a12, int a13, 
         int a21, int a22, int a23, 
         int a31, int a32, int a33)
{
    int d1,d2,d3;
    d1 = diffProd(a22,a33,a23,a32);
    d2 = diffProd(a21,a33,a23,a31);
    d3 = diffProd(a21,a32,a22,a31);
    
   return a11 * d1 - a12 * d2 + a13 * d3;
}

int square(int n) { return n * n; }

int power5(int n) { return n * square(square(n) - 1) - 1; }

int checkZero(int a, int b) { return (a + b) * (a - b) - (square(a) - square(b)); }
