/*
 * Test SANE glue code from saneglue.asm
 * NB: The purpose of this code is _not_ to perform rigorous
 * testing of the numerical correctness of SANE. That is
 * taken for a given. The sole purpose is to test the SANE
 * glue code (and, by extension, SANE patches such as fpcp).
 *
 * Does not test gethaltvector/sethaltvector (yet).
 *
 * Written in 1997 by Soenke Behrens.
 * This code is hereby placed into the Public Domain.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma lint -1

#include "sane.h"

#pragma optimize 0x0048 /* Catch errors in parameter passing */

void a_function(void);

int main (void)
{
   short i,j;
   DecForm convert;
   Decimal d;
   extended x,y,z;
   static char s[258];
   rounddir r;
   roundpre rp;
   environment e1, e2, e3;
   haltvector hv1, hv2;

  /*
   * Test whether we start with IEEE default environment set
   */

   s_getenvironment(&e2);
   if (e2 != IEEEDEFAULTENV)
     printf("SANE environment does not match default environment: %#.4x.\n",e2);

  /*
   * Test fabs
   */

   x = s_fabs(-3.275);
   if (x != 3.275)
     printf("s_fabs() test failed.\n");
   else
     printf("s_fabs() test successful.\n");

  /*
   * Test fneg
   */

   x = s_fneg(3.275);
   if (x != -3.275)
     printf("s_fneg() test failed.\n");
   else
     printf("s_fneg() test successful.\n");
  
  /*
   * Test num2dec and dec2num
   */
   convert.style = FLOATDECIMAL;
   convert.digits = 8;
   s_num2dec(&convert,3.1415926,&d);
   if (d.sgn != 0 || d.exp != -7 ||
       strncmp(d.sig.text,"31415926",d.sig.length) != 0)
     printf("s_num2dec() test failed.\n");
   else
     printf("s_num2dec() test successful.\n");
   x = s_dec2num(&d);

   if (fabs(x - 3.1415926) < 1E-15)
     printf("s_dec2num() test successful.\n");
   else
     printf("s_dec2num() test failed.\n");

  /*
   * Test dec2str and str2dec
   */
   convert.style = FLOATDECIMAL;
   convert.digits = 8;
   s_num2dec(&convert,2.7182818,&d);
   s_dec2str(&convert,&d,s);
   if (strcmp(s," 2.7182818e+0") != 0)
     printf("s_dec2str() test failed.\n");
   else
     printf("s_dec2str() test successful.\n");

   i = 1;
   s_str2dec(s,&i,&d,&j);
   if (j != 1)
   {
      fprintf (stderr,"s_str2dec() rejected input string.\n");
      exit (EXIT_FAILURE);
   }
   x = s_dec2num (&d);

   if (fabs(x - 2.7182818) < 1E-15)
     printf("s_str2dec() test successful.\n");
   else
     printf("s_str2dec() test failed.\n");

  /*
   * Test remainder
   */
   z = s_remainder (3.1415926,2.7182818,&i);
   if (fabs(z - 0.4233108) < 1E-15)
     printf("s_remainder() test successful.\n");
   else
     printf("s_remainder() test failed.\n"); 

  /*
   * Test sqrt
   */

   z = s_sqrt (2.0);
   if (fabs(z - 1.414213562373095048763788073) < 1E-15)
     printf("s_sqrt() test successful.\n");
   else
     printf("s_sqrt() test failed.\n");

  /*
   * Test rint
   */

   z = s_rint (3.1415926);
   if (z == 3.0)
     printf("s_rint() test successful.\n");
   else
     printf("s_rint() test failed.\n");

  /*
   * Test scalb
   */

   z = s_scalb (2,1.4142136);
   if (fabs(z - 5.6568544) < 1E-15)
     printf("s_scalb() test successful.\n");
   else
     printf("s_scalb() test failed.\n");

  /*
   * Test logb
   */

   z = s_logb (1.234e308);
   if (z == 1023.0)
     printf("s_logb() test successful.\n");
   else
     printf("s_logb() test failed.\n");

  /*
   * Test copysign
   */

   z = s_copysign(1.234,-5.678);
   if (z == 5.678)
     printf("s_copysign() test successful.\n");
   else
     printf("s_copysign() test failed.\n");

  /*
   * Test nextfloat
   */

   z = s_nextfloat(1.0,1.1);
   if (z == 1.00000011920928955078125)
     printf("s_nextfloat() test successful.\n");
   else
     printf("s_nextfloat() test failed.\n");

  /*
   * Test nextdouble
   */

   z = s_nextdouble(1.0,1.1);
   if (z == 1.000000000000000222044604925)
     printf("s_nextdouble() test successful.\n");
   else
     printf("s_nextdouble() test failed.\n");

  /*
   * Test nextextended
   */

   z = s_nextextended(1.0,1.1);
   /* I only have double constants, so testing this is a bit tricky */
   if (fabs(z - 1.000000000000000000108420217) < 1E-15)
     printf("s_nextextended() test successful.\n");
   else
     printf("s_nextextended() test failed.\n");

  /*
   * Test log2
   */

   z = s_log2(1.1);
   if (fabs(z - 0.1375035237499350248218483658) < 1E-15)
     printf("s_log2() test successful.\n");
   else
     printf("s_log2() test failed.\n");

  /*
   * Test log
   */

   z = s_log(1.1);
   if (fabs(z - 0.09531017980432494078943525193) < 1E-15)
     printf("s_log() test successful.\n");
   else
     printf("s_log() test failed.\n");

  /*
   * Test log1
   */

   z = s_log1(1.1);
   if (fabs(z - 0.7419373447293773548238092486) < 1E-15)
     printf("s_log1() test successful.\n");
   else
     printf("s_log1() test failed.\n");

  /*
   * Test exp2
   */

   z = s_exp2(1.1);
   if (fabs(z - 2.143546925072586460409712616) < 1E-15)
     printf("s_exp2() test successful.\n");
   else
     printf("s_exp2() test failed.\n");

  /*
   * Test exp
   */

   z = s_exp(1.1);
   if (fabs(z - 3.004166023946433378968498551) < 1E-15)
     printf("s_exp() test successful.\n");
   else
     printf("s_exp() test failed.\n");

  /*
   * Test exp1
   */

   z = s_exp1(1.1);
   if (fabs(z - 2.004166023946433378968498551) < 1E-15)
     printf("s_exp1() test successful.\n");
   else
     printf("s_exp1() test failed.\n");

  /*
   * Test power
   */

   z = s_power(1.1,2.2);
   if (fabs(z - 1.233286300554662750887657818) < 1E-15)
     printf("s_power() test successful.\n");
   else
     printf("s_power() test failed.\n");

  /*
   * Test ipower
   */

   z = s_ipower(1.12345,2);
   if (fabs(z - 1.26213990250000013411922628) < 1E-15)
     printf("s_ipower() test successful.\n");
   else
     printf("s_ipower() test failed.\n");

  /*
   * Test compound
   */

   z = s_compound(0.12,2.3);
   if (fabs(z - 1.297781121042242946086661681) < 1E-15)
     printf("s_compound() test successful.\n");
   else
     printf("s_compound() test failed.\n");

  /*
   * Test annuity
   */

   z = s_annuity(0.12,2.3);
   if (fabs(z - 1.91211699860898026910333708) < 1E-15)
     printf("s_annuity() test successful.\n");
   else
     printf("s_annuity() test failed.\n");

  /*
   * Test tan
   */

   z = s_tan(1.1);
   if (fabs(z - 1.96475965724865238239311982) < 1E-15)
     printf("s_tan() test successful.\n");
   else
     printf("s_tan() test failed.\n");

  /*
   * Test sin
   */

   z = s_sin(1.1);
   if (fabs(z - 0.8912073600614353802488142031) < 1E-15)
     printf("s_sin() test successful.\n");
   else
     printf("s_sin() test failed.\n");

  /*
   * Test cos
   */

   z = s_cos(1.1);
   if (fabs(z - 0.4535961214255773086276909284) < 1E-15)
     printf("s_cos() test successful.\n");
   else
     printf("s_cos() test failed.\n");

  /*
   * Test atan
   */

   z = s_atan(1.1);
   if (fabs(z - 0.8329812666744317456186258442) < 1E-15)
     printf("s_atan() test successful.\n");
   else
     printf("s_atan() test failed.\n");

  /*
   * Test randomx
   */

   x = 1.0;
   z = s_randomx(&x);
   if (z == 16807.0)
     printf("s_randomx() test successful.\n");
   else
     printf("s_randomx() test failed.\n");

  /*
   * Test classfloat
   */

   i = s_classfloat(-0.0);
   if (i == 0x03)
     printf("s_classfloat() test successful.\n");
   else
     printf("s_classfloat() test failed.\n");

  /*
   * Test classdouble
   */

   i = s_classdouble(-0.0);
   if (i == 0x03)
     printf("s_classdouble() test successful.\n");
   else
     printf("s_classdouble() test failed.\n");

  /*
   * Test classcomp
   */

   i = s_classcomp(-0.0);
   if (i == 0x03)
     printf("s_classcomp() test successful.\n");
   else
     printf("s_classcomp() test failed.\n");

  /*
   * Test classextended
   */

   i = s_classextended(-0.0);
   if (i == 0x03)
     printf("s_classextended() test successful.\n");
   else
     printf("s_classextended() test failed.\n");

  /*
   * Test signnum
   */

   if (s_signnum(-123.45) == 1 && s_signnum(123.45) == 0)
     printf("s_signnum() test successful.\n");
   else
     printf("s_signnum() test failed.\n");

  /*
   * Test relation
   */

   i = s_relation(1.23,s_nan(2));
   if (i == UNORDERED)
     printf("s_relation() test successful.\n");
   else
     printf("s_relation() test failed.\n");

  /*
   * For the following tests, clear all exceptions the previous
   * tests might have generated.
   */
   s_setenvironment(IEEEDEFAULTENV);

  /*
   * Test setexception and testexception
   */

    s_setexception(OVERFLOW | INEXACT, 1);
    if (s_testexception(INEXACT) && s_testexception(OVERFLOW))
      printf("s_setexception() and s_testexception() tests successful.\n");
    else
      printf("s_setexception() and s_testexception() tests failed.\n");
    /* Clear exceptions again */
    s_setexception(OVERFLOW | INEXACT, 0);

  /*
   * Test sethalt and testhalt
   */

    s_sethalt(OVERFLOW | INEXACT, 1);
    if (s_testhalt(INEXACT) && s_testhalt(OVERFLOW))
      printf("s_sethalt() and s_testhalt() tests successful.\n");
    else
      printf("s_sethalt() and s_testhalt() tests failed.\n");
    /* Clear halts again */
    s_sethalt(OVERFLOW | INEXACT, 0);

  /*
   * Test setround and getround
   */

   if ((r = s_getround()) != TONEAREST)
   {
     printf("Rounding direction is not default TONEAREST as required by SANE:\n");
     switch (r)
     {
       case TONEAREST:
          printf("Program error, please investigate.\n");
          break;
       case UPWARD:
          printf("Rounding direction is UPWARD.\n");
          break;
       case DOWNWARD:
          printf("Rounding direction is DOWNWARD.\n");
          break;
       case TOWARDZERO:
          printf("Rounding direction is TOWARDZERO.\n");
          break;
       default:
          printf("Undefined rounding direction: %d\n",r);
          break;
     }
   }
   s_setround(TOWARDZERO);
   if (s_getround() == TOWARDZERO)
     printf("s_setround() and s_getround() tests successful.\n");
   else
     printf("s_setround() and s_getround() tests failed.\n");
   /* Set back to earlier value */
   s_setround(r);

  /*
   * Test setprecision and getprecision
   */
   if ((rp = s_getprecision()) != EXTPRECISION)
   {
     printf("Rounding precision is not default \"extended\" as required by SANE:\n");
     switch (rp)
     {
       case EXTPRECISION:
          printf("Program error, please investigate.\n");
          break;
       case DBLPRECISION:
          printf("Rounding precision is \"double\".\n");
          break;
       case FLOATPRECISION:
          printf("Rounding precision is \"single\".\n");
          break;
       default:
          printf("Undefined rounding precision: %d\n",r);
          break;
     }
   }
   s_setprecision(FLOATPRECISION);
   if (s_getprecision() == FLOATPRECISION)
     printf("s_setprecision() and s_getprecision() tests successful.\n");
   else
     printf("s_setprecision() and s_getprecision() tests failed.\n");
   /* Set back to earlier value */
   s_setprecision(rp);

  /*
   * Test setenvironment and getenvironment
   */

   s_getenvironment(&e2);
   if (e2 != IEEEDEFAULTENV)
     printf("SANE environment does not match default environment: %#.4x.\n",e2);
   s_setenvironment(0x0A0A); /* nonsense value */
   s_getenvironment(&e1);
   if (e1 == 0x0A0A)
     printf("s_setenvironment() and s_getenvironment() tests successful.\n");
   else
     printf("s_setenvironment() and s_getenvironment() tests failed.\n");
   /* Now restore the original environment word */
   s_setenvironment(e2);

  /*
   * Test procentry and procexit
   */

  s_getenvironment(&e3);
  s_setenvironment(0x0A0A); /* nonsense value */
  s_procentry(&e2);
  s_getenvironment(&e1);
  if (e1 == IEEEDEFAULTENV)
    printf("s_procentry() test successful.\n");
  else
    printf("s_procentry test failed.\n");
  s_procexit(e2);
  s_getenvironment(&e1);
  if (e1 == 0x0A0A)
    printf("s_procexit() test successful.\n");
  else
    printf("s_procexit() test failed.\n");
  /* Set environment back to what it was */
  s_setenvironment(e3);

  /*
   * Test sethaltvector and gethaltvector
   */

   hv1 = s_gethaltvector();
   s_sethaltvector(a_function);
   hv2 = s_gethaltvector();
   if (hv2 == a_function)
     printf("s_sethaltvector() and s_gethaltvector() tests successful.\n");
   else
     printf("s_sethaltvector() and s_gethaltvector() tests failed.\n");
   /* Now restore original vector */
   s_sethaltvector(hv1);

  /*
   * Test pi
   */

   z = s_pi();
   if (fabs(z - 3.141592653589793238512808959) < 1E-15)
     printf("s_pi() test successful.\n");
   else
     printf("s_pi() test failed.\n");

  /*
   * Test nan
   */

   z = s_nan(5);
   sprintf(s,"%f",z);
   if (strcmp(s,"NAN(005)") == 0)
     printf("s_nan() test successful.\n");
   else
     printf("s_nan() test failed.\n");

  /*
   * Test inf
   */

   z = s_inf();
   sprintf(s,"%f",z);
   if (strcmp(s,"INF") == 0)
     printf("s_inf() test successful.\n");
   else
     printf("s_inf() test failed.\n");

   return (0);
}

void a_function(void)
{
  printf("Halt occured.\n");
  return;
}

/* End Of File */
