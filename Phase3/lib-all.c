/* Library file for Project III (all levels), CSCE 531 */

#include <stdio.h>

extern int a,b,c;

print_abc()
{
	printf("a = %d; b =  %d; c = %d\n",a,b,c);
}

extern int x;

print_x()
{
	printf("x = %d\n",x);
}


extern float z;

print_z()
{
	printf("z = %e\n",z);
}

print_oops()
{
	printf("oops!\n");
}

print_7()
{
	printf("7");
}

print_1()
{
	printf("1");
}

print_2()
{
	printf("2");
}

print_newline()
{
	printf("\n");
}

print_is()
{
	printf(" is ");
}

print_awesomest_nl()
{
	printf("awesomest\n");
}

print_outer()
{
	printf("OUTER: a = %d\n",a);
}

print_inner()
{
	printf("INNER: b = %d\n",b);
}

print_inner_inner()
{
	printf("INNER INNER: c = %d\n",c);
}

print_looping()
{
	printf("LOOPING: x = %d\n",x);
}

print_looping_looping()
{
	printf("LOOPING LOOPING: b = %d\n",b);
}

print_looping_looping_looping()
{
	printf("LOOPING LOOPING LOOPING: c = %d\n",c);
}

print_one()
{
	printf("a is one\n");
}

print_two()
{
	printf("a is two\n");
}

print_three()
{
	printf("a is three\n");
}

print_dunno()
{
	printf("dunno what a is\n");
}

print_just()
{
	printf("c just is\n");
}

print_isnt()
{
	printf("c isn't\n");
}

print_hello()
{
	printf("hello\n");
}

print_712()
{
	printf("712 ");
}

print_is_the()
{
	printf("is the ");
}

print_awesomest()
{
	printf("awesomest");
}

print_dots()
{
	printf("...\n");
}

prime()
{
	if (x == 36 || x == 6) return 0;
	else return 1;
}

process_prime()
{
	printf("process prime\n");
}

process_composite()
{
	printf("process composite\n");
}

print_nl()
{
	printf("\n");
}

print_dot()
{
	printf(".");
}

print_even()
{
	printf("E");
}

print_odd()
{
	printf("O");
}

print_j()
{
	printf("j");
}

print_i_j_values()
{
	extern int i,j;
	printf("i = %d; j = %d\n",i,j);
}
