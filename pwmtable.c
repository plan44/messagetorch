// trivial utility to calculate brightness to PWM tables

#include <stdio.h>
#include <math.h>

int main() {
  const int brightMax = 31;
  const float logSkal = 4;      // exponent: 1 = lin, 2 = quadratic, 3 = cubic, ...  (float)
  const int pwmMin = 0;
  const int pwmMax = 255;

  // calculate pwm value (0..255): exponential scale of bright
  printf("const uint8_t pwmtable[%d] = {", brightMax+1);
  int first = 1;
  for (int bright=0; bright<=brightMax; bright++) {
    int pwmOut = pwmMin+round((pwmMax-pwmMin)*((exp((bright*logSkal)/brightMax)-1)/(exp(logSkal)-1)));
    if (!first) printf(", ");
    first = 0;
    printf("%d",pwmOut);
  }
  printf("};\n");
}
