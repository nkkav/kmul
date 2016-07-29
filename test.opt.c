// test.opt.c
#include <stdio.h>
#include <stdlib.h>

long kmul_s32_p_23 (long x)
{
  long t0;
  long t1;
  long t2;
  long t3;
  long t4;
  long t5;
  long t6;
  long t7;
  long t8;
  long t9;
  long t10;
  long t11;
  long t12;
  long t13;
  long t14;
  long t15;
  long y;
  t0 = x;
  t1 = t0 << 1;
  t2 = t1 + x;
  t3 = t2 << 3;
  t4 = t3 - x;
  y = t4;
  return (y);
}

int main(int argc, char *argv[]) 
{
  int a, b;
  a = atoi(argv[1]);
  b = kmul_s32_p_23(a);
  printf("b = %d\n", b);
  return b;
}
