/****************************************************************
*
*  math.h - math library
*
*  February 1989
*  Mike Westerfield
*
*  Copyright 1989, 1992
*  Byte Works, Inc.
*
****************************************************************/

#ifndef __math__
#define __math__

#define HUGE_VAL 1e5000

#ifndef __KeepNamespacePure__
   #define arctan(x) atan(x)
#   define isnan(x) _isnan(x)
#   define isinf(x) _isinf(x)
#endif

#ifdef __GNO__
int             _isnan(extended);
int             _isinf(extended);
#endif

double          acos(double);
double          asin(double);
double          atan(double);
double          cos(double);
double          cosh(double);
double          exp(double);
double          log(double);
double          log10(double);
double          sin(double);
double          sinh(double);
double          sqrt(double);
double          tan(double);
double          tanh(double);
double          atan2(double, double);
double          ceil(double);
double          fabs(double);
double          floor(double);
double          fmod(double, double);
double          frexp(double, int *);
double          ldexp(double, int);
double          modf(double, double *);
double          pow(double, double);

#endif
