#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "MathFunctions/MathFunctions.h"

int main (int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stdout,"Usage: %s number\n",argv[0]);
    return 1;
  }

  double inputValue = atof(argv[1]);
  double outputValue = mysqrt(inputValue);
  fprintf(stdout,"The square root of %g is %g\n",
          inputValue, outputValue);
  return 0;
}
