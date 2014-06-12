// test.opt.c
#include <stdio.h>
#include <stdlib.h>
signed int kmul_s32_p_23 (signed int x)
{
  signed int t0;
  signed int t1;
  signed int t2;
  signed int t3;
  signed int t4;
  signed int t5;
  signed int t6;
  signed int t7;
  signed int t8;
  signed int t9;
  signed int t10;
  signed int t11;
  signed int t12;
  signed int t13;
  signed int t14;
  signed int t15;
  signed int y;
  t0 = x;
  t1 = t0 << 1;
  t2 = t1 + x;
  t3 = t2 << 3;
  t4 = t3 - x;
  y = t4;
  return (y);
}

int main(int argc, char *argv[]) {
  int a, b;
  a = atoi(argv[1]);
  b = kmul_s32_p_23(a);
  printf("b = %d\n", b);
  return b;
}
