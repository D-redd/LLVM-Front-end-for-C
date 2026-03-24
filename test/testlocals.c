int Taylor_1_div_x_v2(int x) {
    int z;
    z = x - 1;
    return 1 - z + z*z - z*z*z;
   }

int mdet3(int a11, int a12, int a13, 
         int a21, int a22, int a23, 
         int a31, int a32, int a33)
{
    int d1,d2,d3;
    d1 = a22 * a33 - a23 * a32;
    d2 = a21 * a33 - a23 * a31;
    d3 = a21 * a32 - a22 * a31;
    
   return a11 * d1 - a12 * d2 + a13 * d3;
}

int fibo7()
{
    int a, b;
    a = 1; b = 1;
    a = a + b; b = a + b;
    a = a + b; b = a + b;
    return a + b;
}

int deepExp2(int x, int y, int z, int t, int q, int r)
{
    int a, b, c;
    a = x - y;
    b = z - t;
    c = q - r;
   return x*1-(y*2-(z*3-(t*4-(q*11-(r*12-(a*13-(b*14-(c*21-(x*22-(y*23-(z*24-(q*31-(r*32-(a*33-b*34))))))))))))));
}
