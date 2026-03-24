#include <stdio.h>
#include <string.h>

int pi100();
int e1000();
int ref17k();

void testBasicFuncs(void)
{
	// Test simple expressions without arguments
	printf("pi100() : %d\n", pi100());
	printf("e1000() : %d\n", e1000());
	printf("ref17k() : %d\n", ref17k());
}

#if TEST_ARGS == 1
int circleArea100(int radius);
int mac(int a, int b, int c);
int f(int a, int b, int c);
int pnd(int n1, int n2, int p, int Multiplier);
int rcubed(int r);
int volume(int pi, int r, int height);
int Taylor_1_div_x(int x);
int xx(int a, int b, int c, int d);
int scalarProd_3d(int x1, int y1, int z1, int x2, int y2, int z2);
int deepExp(int x, int y, int z, int t);
int point_cross_diff(int x1, int y1, int z1, int x2, int y2, int z2);
int det3(int a11, int a12, int a13, int a21, int a22, int a23, int a31, int a32, int a33);

void testExpressionFuncs(void)
{
	// Test simple expressions with arguments
	printf("circleArea100(3) : %d\n", circleArea100(3));
	printf("mac(-4,2,7) : %d\n", mac(-4,2,7));
	printf("f(-4,2,5) : %d\n", f(-4,2,5));
	printf("pnd(3,8,40,-3) : %d\n", pnd(3,8,40,-3));

	// Test MUL instruction restrictions
	printf("rcubed(-29) : %d\n", rcubed(-29)) ;
	printf("volume(3142,5,10): %d\n", volume(3142,5,10)) ;

	// Test complex expressions
	printf("Taylor_1_div_x(4): %d\n", Taylor_1_div_x(4)) ;
	printf("xx(4,2,5,-1) : %d\n", xx(4,2,5,-1)) ;
	printf("deepExp(-10,-14,-12,-18) : %d\n", deepExp(-10,-14,-12,-18)) ;

	// Test more than 4 parameters
	printf("scalarProd_3d(3,4,5,-5,-4,3) : %d\n", scalarProd_3d(3,4,5,-5,-4,3)) ;
	printf("point_cross_diff(3,4,5,-5,-4,3) : %d\n", point_cross_diff(3,4,5,-5,-4,3)) ;
	printf("det3(1,1,-2,1,-2,1,-2,1,1) : %d\n", det3(1,1,-2,1,-2,1,-2,1,1));
}
#endif

#if TEST_LOCALS == 1
int Taylor_1_div_x_v2(int x);
int mdet3(int a11, int a12, int a13, int a21, int a22, int a23, int a31, int a32, int a33);
int fibo7();
int deepExp2(int x, int y, int z, int t, int q, int r);

void testLocalVarFuncs(void)
{
	printf("Taylor_1_div_x_v2(4): %d\n", Taylor_1_div_x_v2(4)) ;
	printf("mdet3(1,1,-2,1,-2,1,-2,1,1) : %d\n", mdet3(1,1,-2,1,-2,1,-2,1,1)) ;
	printf("fibo7() : %d\n", fibo7()) ;
	printf("deepExp2(-10,-14,-12,-18,10,12) : %d\n", deepExp2(-10,-14,-12,-18,10,12)) ;
}
#endif

#if TEST_CALLS == 1
int mdet3call(int a11, int a12, int a13, int a21, int a22, int a23, int a31, int a32, int a33);
int power5(int n);
int checkZero(int a, int b);
void testFunctionCalls(void)
{
	printf("mdet3call(1,1,-2,1,-2,1,-2,1,1) : %d\n", mdet3call(1,1,-2,1,-2,1,-2,1,1)) ;
	printf("power5(-2) : %d\n", power5(-2)) ;
	printf("checkZero(4,-3) : %d\n", checkZero(4,-3)) ;
}
#endif

int main(int argc, char ** argv)
{
	testBasicFuncs();
#if TEST_ARGS == 1
	testExpressionFuncs();
#endif
#if TEST_LOCALS == 1
	testLocalVarFuncs();
#endif
#if TEST_CALLS == 1
	testFunctionCalls();
#endif
	return 0;
}
