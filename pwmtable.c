// trivial utility to calculate brightness to PWM tables

#include <stdio.h>
#include <math.h>

int main() {
  const int brightMax = 255;
  const float logSkal = 4;      // exponent: 1 = lin, 2 = quadratic, 3 = cubic, ...  (float)
  const int pwmMin = 0;
  const int pwmMax = 255;
  const int pwmStep = 1; // resolution for reverse table (PWM unit step size)
  const int brightScale = 1; // upscaling for brightness output in reverse table
  int pwmForBrightness[brightMax+1];

  // calculate pwm value (0..255): exponential scale of bright
  printf("const uint8_t pwmtable[%d] = {", brightMax+1);
  int first = 1;
  for (int bright=0; bright<=brightMax; bright++) {
    int pwmOut = pwmMin+round((pwmMax-pwmMin)*((exp((bright*logSkal)/brightMax)-1)/(exp(logSkal)-1)));
    pwmForBrightness[bright] = pwmOut;
    if (!first) printf(", ");
    first = 0;
    printf("%d",pwmOut);
  }
  printf("};\n");
  // also generate the inverse table
  printf("const uint8_t brightnesstable[%d] = {", (pwmMax-pwmMin+1)/pwmStep);
  first = 1;
  for (int pwmIn=pwmMin; pwmIn<=pwmMax; pwmIn+=pwmStep) {
    int bright = 0;
    for (bright=0; bright<=brightMax; bright++) {
      if (pwmForBrightness[bright]>=pwmIn) {
        break;
      }
    }
    if (!first) printf(", ");
    first = 0;
    printf("%d",bright*brightScale);
  }
  printf("};\n");
}
