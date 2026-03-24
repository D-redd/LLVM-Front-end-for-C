int circleArea100(int radius)
{
	return 314 * radius * radius;
}

int mac(int a, int b, int c)
{
	return a + b * c;
}

int f(int a, int b, int c) {
   return b * b - 4 * a * c;
   }

int pnd(int n1, int n2, int p, int Multiplier) {
   return Multiplier * (n1 * p + n2 * (100 - p));
   }



int rcubed(int return2) { return return2 * return2 * return2; }

int volume(int pi, int r, int height) {
   return pi * (r * r) * height; 
   }



int Taylor_1_div_x(int x) {
   return 1
      -(x-1)
      +(x-1)*(x-1)
      -(x-1)*(x-1)*(x-1) ; 
   }

int xx(int a, int b, int c, int d) {
    return (d-a)*0-((a-b)*10+((b-c)*20-(c-d)*30)) ; 
    }

int deepExp(int x, int y, int z, int t)
{
   return x*1-(y*2-(z*3-(t*4-(x*11-(y*12-(z*13-(t*14-(x*21-(y*22-(z*23-(t*24-(x*31-(y*32-(z*33-t*34))))))))))))));
}



int scalarProd_3d(int x1, int y1, int z1, int x2, int y2, int z2)
{
   return x1*x2 + y1*y2 + z1*z2;
}

int point_cross_diff(int x1, int y1, int z1, int x2, int y2, int z2)
{
   return y1 * z2 - z1 * y2;
}

int det3(int a11, int a12, int a13, 
         int a21, int a22, int a23, 
         int a31, int a32, int a33)
{
   return a11 * (a22 * a33 - a23 * a32)
        - a12 * (a21 * a33 - a23 * a31)
        + a13 * (a21 * a32 - a22 * a31)
        ;
}
