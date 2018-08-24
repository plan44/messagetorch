// Configuration
// =============

// Number of LEDs around the tube. One too much looks better (italic text look)
// than one to few (backwards leaning text look)
// Higher number = diameter of the torch gets larger
const uint16_t ledsPerLevel = 11; // Original: 13, smaller tube 11, high density small 17

// Number of "windings" of the LED strip around (or within) the tube
// Higher number = torch gets taller
const uint16_t levels = 21; // original 18, smaller tube 21, high density small 7

// set to true if you wound the torch clockwise (as seen from top). Note that
// this reverses the entire animation (in contrast to mirrorText, which only
// mirrors text).
const bool reversedX = false;

// set to true if you wound the torch from top down
const bool reversedY = false;

// set to true if every other row in the LED matrix is ordered backwards.
// This mode is useful for WS2812 modules which have e.g. 16x16 LEDs on one
// flexible PCB. On these modules, the data line starts in the lower left
// corner, goes right for row 0, then left in row 1, right in row 2 etc.
const bool alternatingX = false;
// set to true if your WS2812 chain runs up (or up/down, with alternatingX set) the "torch",
// for example if you want to do a wide display out of multiple 16x16 arrays
const bool swapXY = false;




// Set this to true if you wound the LED strip clockwise, starting at the bottom of the
// tube, when looking onto the tube from the top. The default winding direction
// for versions of MessageTorch which did not have this setting was 0, which
// means LED strip was usually wound counter clock wise from bottom to top.
// Note: this setting only reverses the direction of text rendering - the torch
//   animation itself is not affected
const bool mirrorText = false;

// Set this to the LED type in use.
// - ws2811_brg is WS2811 driver chip wired to LEDs in B,R,G order
// - ws2812 is WS2812 LED chip in standard order for this chip: G,R,B
#define LED_TYPE p44_ws2812::ws2812


// define this to 1 to disable Cheerlights part of the code (to save memory)
#define NO_CHEERLIGHT 1

// define this to 1 to disable digitalSTROM part of the code (to save memory)
//#define NO_DIGITALSTROM 1


/*
 * Spark Core library to control WS2812 based RGB LED devices
 * using SPI to create bitstream.
 * Future plan is to use DMA to feed the SPI, so WS2812 bitstream
 * can be produced without CPU load and without blocking IRQs
 *
 * (c) 2014-2018 by luz@plan44.ch (GPG: 1CC60B3A)
 * Licensed as open source under the terms of the MIT License
 * (see LICENSE.TXT)
 */


// Declaration (would go to .h file once library is separated)
// ===========================================================

class p44_ws2812 {

public:
  typedef enum {
    ws2811_brg,
    ws2812
  } LedType;

private:
  typedef struct {
    unsigned int red:5;
    unsigned int green:5;
    unsigned int blue:5;
  } __attribute((packed)) RGBPixel;

  LedType ledType; // the LED type
  uint16_t numLeds; // number of LEDs
  uint16_t ledsPerPixel; // number of LEDs per pixel
  uint16_t numPixels; // number of pixels
  RGBPixel *pixelBufferP; // the pixel buffer
  uint16_t pixelsPerRow; // number of pixels per row
  uint16_t numRows; // number of rows
  bool xReversed; // even (0,2,4...) rows go backwards, or all if not alternating
  bool yReversed; // Y reversed
  bool alternating; // direction changes after every row
  bool swapXY; // swap X and Y

public:
  /// create driver for a WS2812 LED chain
  /// @param aLedType type of LEDs
  /// @param aNumLeds number of physical LEDs in the chain (not necessarily pixels, when aLedsPerPixel>1)
  /// @param aPixelsPerRow number of consecutive LEDs in the WS2812 chain that build a row (usually x direction, y if swapXY was set)
  /// @param aXReversed X direction is reversed
  /// @param aAlternating X direction is reversed in first row, normal in second, reversed in third etc..
  /// @param aSwapXY X and Y reversed (for up/down wiring)
  /// @param aYReversed Y direction is reversed
  /// @param aLedsPerPixel number of consecutive LEDS to be treated as a single pixel
  p44_ws2812(LedType aLedType, uint16_t aNumLeds, uint16_t aPixelsPerRow=0, bool aXReversed=false, bool aAlternating=false, bool aSwapXY=false, bool aYReversed=false, uint16_t aLedsPerPixel=1);

  /// destructor
  ~p44_ws2812();

  /// begin using the driver
  void begin();

  /// transfer RGB values to LED chain
  /// @note this must be called to update the actual LEDs after modifying RGB values
  /// with setColor() and/or setColorDimmed()
  void show();

  /// set color of one LED
  /// @param aRed intensity of red component, 0..255
  /// @param aGreen intensity of green component, 0..255
  /// @param aBlue intensity of blue component, 0..255
  void setColorXY(uint16_t aX, uint16_t aY, byte aRed, byte aGreen, byte aBlue);
  void setColor(uint16_t aLedNumber, byte aRed, byte aGreen, byte aBlue);

  /// set color of one LED, scaled by a visible brightness (non-linear) factor
  /// @param aRed intensity of red component, 0..255
  /// @param aGreen intensity of green component, 0..255
  /// @param aBlue intensity of blue component, 0..255
  /// @param aBrightness brightness, will be converted non-linear to PWM duty cycle for uniform brightness scale, 0..255
  void setColorDimmedXY(uint16_t aX, uint16_t aY, byte aRed, byte aGreen, byte aBlue, byte aBrightness);
  void setColorDimmed(uint16_t aLedNumber, byte aRed, byte aGreen, byte aBlue, byte aBrightness);

  /// get current color of LED
  /// @param aRed set to intensity of red component, 0..255
  /// @param aGreen set to intensity of green component, 0..255
  /// @param aBlue set to intensity of blue component, 0..255
  /// @note for LEDs set with setColorDimmed(), this returns the scaled down RGB values,
  ///   not the original r,g,b parameters. Note also that internal brightness resolution is 5 bits only.
  void getColorXY(uint16_t aX, uint16_t aY, byte &aRed, byte &aGreen, byte &aBlue);
  void getColor(uint16_t aLedNumber, byte &aRed, byte &aGreen, byte &aBlue);

  /// @return number of pixels
  int getNumPixels();
  /// @return number of Pixels in X direction
  uint16_t getSizeX();
  /// @return number of Pixels in Y direction
  uint16_t getSizeY();

private:

  uint16_t ledIndexFromXY(uint16_t aX, uint16_t aY);


};



// Implementation (would go to .cpp file once library is separated)
// ================================================================

static const uint8_t pwmTable[32] = {0, 1, 1, 2, 3, 4, 6, 7, 9, 10, 13, 15, 18, 21, 24, 28, 33, 38, 44, 50, 58, 67, 77, 88, 101, 115, 132, 150, 172, 196, 224, 255};

p44_ws2812::p44_ws2812(LedType aLedType, uint16_t aNumLeds, uint16_t aPixelsPerRow, bool aXReversed, bool aAlternating, bool aSwapXY, bool aYReversed, uint16_t aLedsPerPixel)
{
  numLeds = aNumLeds; // raw number of LEDs
  ledsPerPixel = aLedsPerPixel;
  if (ledsPerPixel<1) ledsPerPixel=1;
  numPixels = numLeds/ledsPerPixel;
  ledType = aLedType;
  if (aPixelsPerRow==0) {
    pixelsPerRow = numPixels; // single row
    numRows = 1;
  }
  else {
    pixelsPerRow = aPixelsPerRow; // set row size
    numRows = (numPixels-1)/pixelsPerRow+1; // calculate number of (full or partial) rows
  }
  xReversed = aXReversed;
  alternating = aAlternating;
  swapXY = aSwapXY;
  yReversed = aYReversed;
  // allocate the buffer
  if((pixelBufferP = new RGBPixel[numPixels])!=NULL) {
    memset(pixelBufferP, 0, sizeof(RGBPixel)*numPixels); // all LEDs off
  }
}

p44_ws2812::~p44_ws2812()
{
  // free the buffer
  if (pixelBufferP) delete pixelBufferP;
}


int p44_ws2812::getNumPixels()
{
  return numPixels;
}


uint16_t p44_ws2812::getSizeX()
{
  return swapXY ? numRows : pixelsPerRow;
}


uint16_t p44_ws2812::getSizeY()
{
  return swapXY ? pixelsPerRow : numRows;
}




void p44_ws2812::begin()
{
  // begin using the driver
  SPI.begin();
  switch (ledType) {
    case ws2811_brg:
      SPI.setClockDivider(SPI_CLOCK_DIV16); // WS2811: System clock is 72MHz, we need 4.5MHz for SPI
      break;
    case ws2812:
      SPI.setClockDivider(SPI_CLOCK_DIV8); // WS2812: System clock is 72MHz, we need 9MHz for SPI
      break;
  }
  SPI.setBitOrder(MSBFIRST); // MSB first for easier scope reading :-)
  SPI.transfer(0); // make sure SPI line starts low (Note: SPI line remains at level of last sent bit, fortunately)
}

void p44_ws2812::show()
{
  // Note: on the spark core, system IRQs might happen which exceed 50uS
  // causing WS2812 chips to reset in midst of data stream.
  // Thus, until we can send via DMA, we need to disable IRQs while sending
  __disable_irq();
  switch(ledType) {
    case ws2811_brg: {
      // transfer RGB values to LED chain
      for (uint16_t i=0; i<numPixels; i++) {
        RGBPixel *pixP = &(pixelBufferP[i]);
        byte b;
        for (uint16_t r=0; r<ledsPerPixel; r++) {
          // Order of PWM data for WS2811 LEDs usually is BRG
          // - blue
          b = pwmTable[pixP->blue];
          for (byte j=0; j<8; j++) {
            SPI.transfer(b & 0x80 ? 0x7C : 0x40);
            b = b << 1;
          }
          // - red
          b = pwmTable[pixP->red];
          for (byte j=0; j<8; j++) {
            SPI.transfer(b & 0x80 ? 0x7C : 0x40);
            b = b << 1;
          }
          // - green
          b = pwmTable[pixP->green];
          for (byte j=0; j<8; j++) {
            SPI.transfer(b & 0x80 ? 0x7C : 0x40);
            b = b << 1;
          }
        }
      }
    }
    case ws2812: {
      // transfer RGB values to LED chain
      for (uint16_t i=0; i<numPixels; i++) {
        RGBPixel *pixP = &(pixelBufferP[i]);
        byte b;
        for (uint16_t r=0; r<ledsPerPixel; r++) {
          // Order of PWM data for WS2812 LEDs is G-R-B
          // - green
          b = pwmTable[pixP->green];
          for (byte j=0; j<8; j++) {
            SPI.transfer(b & 0x80 ? 0x7E : 0x70);
            b = b << 1;
          }
          // - red
          b = pwmTable[pixP->red];
          for (byte j=0; j<8; j++) {
            SPI.transfer(b & 0x80 ? 0x7E : 0x70);
            b = b << 1;
          }
          // - blue
          b = pwmTable[pixP->blue];
          for (byte j=0; j<8; j++) {
            SPI.transfer(b & 0x80 ? 0x7E : 0x70);
            b = b << 1;
          }
        }
      }
    }
  } // switch
  __enable_irq();
}


uint16_t p44_ws2812::ledIndexFromXY(uint16_t aX, uint16_t aY)
{
  if (swapXY) { uint16_t tmp = aY; aY = aX; aX = tmp; }
  if (yReversed) { aY = numRows-1-aY; }
  uint16_t ledindex = aY*pixelsPerRow;
  bool reversed = xReversed;
  if (alternating) {
    if (aY & 0x1) reversed = !reversed;
  }
  if (reversed) {
    ledindex += (pixelsPerRow-1-aX);
  }
  else {
    ledindex += aX;
  }
  return ledindex;
}


void p44_ws2812::setColorXY(uint16_t aX, uint16_t aY, byte aRed, byte aGreen, byte aBlue)
{
  uint16_t ledindex = ledIndexFromXY(aX,aY);
  if (ledindex>=numPixels) return;
  RGBPixel *pixP = &(pixelBufferP[ledindex]);
  // linear brightness is stored with 5bit precision only
  pixP->red = aRed>>3;
  pixP->green = aGreen>>3;
  pixP->blue = aBlue>>3;
}


void p44_ws2812::setColor(uint16_t aLedNumber, byte aRed, byte aGreen, byte aBlue)
{
  int y = aLedNumber / getSizeX();
  int x = aLedNumber % getSizeX();
  setColorXY(x, y, aRed, aGreen, aBlue);
}


void p44_ws2812::setColorDimmedXY(uint16_t aX, uint16_t aY, byte aRed, byte aGreen, byte aBlue, byte aBrightness)
{
  setColorXY(aX, aY, (aRed*aBrightness)>>8, (aGreen*aBrightness)>>8, (aBlue*aBrightness)>>8);
}


void p44_ws2812::setColorDimmed(uint16_t aLedNumber, byte aRed, byte aGreen, byte aBlue, byte aBrightness)
{
  int y = aLedNumber / getSizeX();
  int x = aLedNumber % getSizeX();
  setColorDimmedXY(x, y, aRed, aGreen, aBlue, aBrightness);
}


void p44_ws2812::getColor(uint16_t aLedNumber, byte &aRed, byte &aGreen, byte &aBlue)
{
  int y = aLedNumber / getSizeX();
  int x = aLedNumber % getSizeX();
  getColorXY(x, y, aRed, aGreen, aBlue);
}


void p44_ws2812::getColorXY(uint16_t aX, uint16_t aY, byte &aRed, byte &aGreen, byte &aBlue)
{
  uint16_t ledindex = ledIndexFromXY(aX,aY);
  if (ledindex>=numPixels) return;
  RGBPixel *pixP = &(pixelBufferP[ledindex]);
  // linear brightness is stored with 5bit precision only
  aRed = pixP->red<<3;
  aGreen = pixP->green<<3;
  aBlue = pixP->blue<<3;
}



// Utilities
// =========


typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} RGBColor;


uint16_t random(uint16_t aMinOrMax, uint16_t aMax = 0)
{
  if (aMax==0) {
    aMax = aMinOrMax;
    aMinOrMax = 0;
  }
  uint32_t r = aMinOrMax;
  aMax = aMax - aMinOrMax + 1;
  r += rand() % aMax;
  return r;
}


inline void reduce(byte &aByte, byte aAmount, byte aMin = 0)
{
  int r = aByte-aAmount;
  if (r<aMin)
    aByte = aMin;
  else
    aByte = (byte)r;
}


inline void increase(byte &aByte, byte aAmount, byte aMax = 255)
{
  int r = aByte+aAmount;
  if (r>aMax)
    aByte = aMax;
  else
    aByte = (byte)r;
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
void wheel(byte WheelPos, byte &red, byte &green, byte &blue) {
  if(WheelPos < 85) {
    red = WheelPos * 3;
    green = 255 - WheelPos * 3;
    blue = 0;
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    red = 255 - WheelPos * 3;
    green = 0;
    blue = WheelPos * 3;
  } else {
    WheelPos -= 170;
    red = 0;
    green = WheelPos * 3;
    blue = 255 - WheelPos * 3;
  }
}


int hexToInt(char aHex)
{
  if (aHex<'0') return 0;
  aHex -= '0';
  if (aHex>9) aHex -= 7;
  if (aHex>15) return 0;
  return aHex;
}


void webColorToRGB(String aWebColor, RGBColor &aColor)
{
  int cc;
  int i=0;
  if (aWebColor.length()==6) {
    // RRGGBB
    cc = hexToInt(aWebColor[i++]);
    aColor.r = (cc<<4) + hexToInt(aWebColor[i++]);
    cc = hexToInt(aWebColor[i++]);
    aColor.g = (cc<<4) + hexToInt(aWebColor[i++]);
    cc = hexToInt(aWebColor[i++]);
    aColor.b = (cc<<4) + hexToInt(aWebColor[i++]);
  }
  else if (aWebColor.length()==3) {
    // RGB
    cc = hexToInt(aWebColor[i++]);
    aColor.r = (cc<<4) + cc;
    cc = hexToInt(aWebColor[i++]);
    aColor.g = (cc<<4) + cc;
    cc = hexToInt(aWebColor[i++]);
    aColor.b = (cc<<4) + cc;
  }
}


// Simple 7 pixel height dot matrix font
// =====================================
// Note: the font is derived from a monospaced 7*5 pixel font, but has been adjusted a bit
//       to get rendered proportionally (variable character width, e.g. "!" has width 1, whereas "m" has 7)
//       In the fontGlyphs table below, every char has a number of pixel colums it consists of, and then the
//       actual column values encoded as a string.

typedef struct {
  uint8_t width;
  const char *cols;
} glyph_t;

const int numGlyphs = 102; // 96 ASCII 0x20..0x7F plus 6 ÄÖÜäöü
const int rowsPerGlyph = 7;
const int glyphSpacing = 2;

static const glyph_t fontGlyphs[numGlyphs] = {
  { 5, "\x00\x00\x00\x00\x00" },  //   0x20 (0)
  { 1, "\x5f" },                  // ! 0x21 (1)
  { 3, "\x03\x00\x03" },          // " 0x22 (2)
  { 5, "\x28\x7c\x28\x7c\x28" },  // # 0x23 (3)
  { 5, "\x24\x2a\x7f\x2a\x12" },  // $ 0x24 (4)
  { 5, "\x4c\x2c\x10\x68\x64" },  // % 0x25 (5)
  { 5, "\x30\x4e\x55\x22\x40" },  // & 0x26 (6)
  { 1, "\x01" },                  // ' 0x27 (7)
  { 3, "\x1c\x22\x41" },          // ( 0x28 (8)
  { 3, "\x41\x22\x1c" },          // ) 0x29 (9)
  { 5, "\x01\x03\x01\x03\x01" },  // * 0x2A (10)
  { 5, "\x08\x08\x3e\x08\x08" },  // + 0x2B (11)
  { 2, "\x50\x30" },              // , 0x2C (12)
  { 5, "\x08\x08\x08\x08\x08" },  // - 0x2D (13)
  { 2, "\x60\x60" },              // . 0x2E (14)
  { 5, "\x40\x20\x10\x08\x04" },  // / 0x2F (15)

  { 5, "\x3e\x51\x49\x45\x3e" },  // 0 0x30 (0)
  { 3, "\x42\x7f\x40" },          // 1 0x31 (1)
  { 5, "\x62\x51\x49\x49\x46" },  // 2 0x32 (2)
  { 5, "\x22\x41\x49\x49\x36" },  // 3 0x33 (3)
  { 5, "\x0c\x0a\x09\x7f\x08" },  // 4 0x34 (4)
  { 5, "\x4f\x49\x49\x49\x31" },  // 5 0x35 (5)
  { 5, "\x3e\x49\x49\x49\x32" },  // 6 0x36 (6)
  { 5, "\x03\x01\x71\x09\x07" },  // 7 0x37 (7)
  { 5, "\x36\x49\x49\x49\x36" },  // 8 0x38 (8)
  { 5, "\x26\x49\x49\x49\x3e" },  // 9 0x39 (9)
  { 2, "\x66\x66" },              // : 0x3A (10)
  { 2, "\x56\x36" },              // ; 0x3B (11)
  { 4, "\x08\x14\x22\x41" },      // < 0x3C (12)
  { 4, "\x24\x24\x24\x24" },      // = 0x3D (13)
  { 4, "\x41\x22\x14\x08" },      // > 0x3E (14)
  { 5, "\x02\x01\x59\x09\x06" },  // ? 0x3F (15)

  { 5, "\x3e\x41\x5d\x55\x5e" },  // @ 0x40 (0)
  { 5, "\x7c\x0a\x09\x0a\x7c" },  // A 0x41 (1)
  { 5, "\x7f\x49\x49\x49\x36" },  // B 0x42 (2)
  { 5, "\x3e\x41\x41\x41\x22" },  // C 0x43 (3)
  { 5, "\x7f\x41\x41\x22\x1c" },  // D 0x44 (4)
  { 5, "\x7f\x49\x49\x41\x41" },  // E 0x45 (5)
  { 5, "\x7f\x09\x09\x01\x01" },  // F 0x46 (6)
  { 5, "\x3e\x41\x49\x49\x7a" },  // G 0x47 (7)
  { 5, "\x7f\x08\x08\x08\x7f" },  // H 0x48 (8)
  { 3, "\x41\x7f\x41" },          // I 0x49 (9)
  { 5, "\x30\x40\x40\x40\x3f" },  // J 0x4A (10)
  { 5, "\x7f\x08\x0c\x12\x61" },  // K 0x4B (11)
  { 5, "\x7f\x40\x40\x40\x40" },  // L 0x4C (12)
  { 7, "\x7f\x02\x04\x0c\x04\x02\x7f" },  // M 0x4D (13)
  { 5, "\x7f\x02\x04\x08\x7f" },  // N 0x4E (14)
  { 5, "\x3e\x41\x41\x41\x3e" },  // O 0x4F (15)

  { 5, "\x7f\x09\x09\x09\x06" },  // P 0x50 (0)
  { 5, "\x3e\x41\x51\x61\x7e" },  // Q 0x51 (1)
  { 5, "\x7f\x09\x09\x09\x76" },  // R 0x52 (2)
  { 5, "\x26\x49\x49\x49\x32" },  // S 0x53 (3)
  { 5, "\x01\x01\x7f\x01\x01" },  // T 0x54 (4)
  { 5, "\x3f\x40\x40\x40\x3f" },  // U 0x55 (5)
  { 5, "\x1f\x20\x40\x20\x1f" },  // V 0x56 (6)
  { 5, "\x7f\x40\x38\x40\x7f" },  // W 0x57 (7)
  { 5, "\x63\x14\x08\x14\x63" },  // X 0x58 (8)
  { 5, "\x03\x04\x78\x04\x03" },  // Y 0x59 (9)
  { 5, "\x61\x51\x49\x45\x43" },  // Z 0x5A (10)
  { 3, "\x7f\x41\x41" },          // [ 0x5B (11)
  { 5, "\x04\x08\x10\x20\x40" },  // \ 0x5C (12)
  { 3, "\x41\x41\x7f" },          // ] 0x5D (13)
  { 4, "\x04\x02\x01\x02" },      // ^ 0x5E (14)
  { 5, "\x40\x40\x40\x40\x40" },  // _ 0x5F (15)

  { 2, "\x01\x02" },              // ` 0x60 (0)
  { 5, "\x20\x54\x54\x54\x78" },  // a 0x61 (1)
  { 5, "\x7f\x44\x44\x44\x38" },  // b 0x62 (2)
  { 5, "\x38\x44\x44\x44\x08" },  // c 0x63 (3)
  { 5, "\x38\x44\x44\x44\x7f" },  // d 0x64 (4)
  { 5, "\x38\x54\x54\x54\x18" },  // e 0x65 (5)
  { 5, "\x08\x7e\x09\x09\x02" },  // f 0x66 (6)
  { 5, "\x48\x54\x54\x54\x38" },  // g 0x67 (7)
  { 5, "\x7f\x08\x08\x08\x70" },  // h 0x68 (8)
  { 3, "\x48\x7a\x40" },          // i 0x69 (9)
  { 5, "\x20\x40\x40\x48\x3a" },  // j 0x6A (10)
  { 4, "\x7f\x10\x28\x44" },      // k 0x6B (11)
  { 3, "\x3f\x40\x40" },          // l 0x6C (12)
  { 7, "\x7c\x04\x04\x38\x04\x04\x78" },  // m 0x6D (13)
  { 5, "\x7c\x04\x04\x04\x78" },  // n 0x6E (14)
  { 5, "\x38\x44\x44\x44\x38" },  // o 0x6F (15)

  { 5, "\x7c\x14\x14\x14\x08" },  // p 0x70 (0)
  { 5, "\x08\x14\x14\x7c\x40" },  // q 0x71 (1)
  { 5, "\x7c\x04\x04\x04\x08" },  // r 0x72 (2)
  { 5, "\x48\x54\x54\x54\x24" },  // s 0x73 (3)
  { 5, "\x04\x04\x7f\x44\x44" },  // t 0x74 (4)
  { 5, "\x3c\x40\x40\x40\x7c" },  // u 0x75 (5)
  { 5, "\x1c\x20\x40\x20\x1c" },  // v 0x76 (6)
  { 7, "\x7c\x40\x40\x38\x40\x40\x7c" },  // w 0x77 (7)
  { 5, "\x44\x28\x10\x28\x44" },  // x 0x78 (8)
  { 5, "\x0c\x50\x50\x50\x3c" },  // y 0x79 (9)
  { 5, "\x44\x64\x54\x4c\x44" },  // z 0x7A (10)
  { 3, "\x08\x36\x41" },          // { 0x7B (11)
  { 1, "\x7f" },                  // | 0x7C (12)
  { 3, "\x41\x36\x08" },          // } 0x7D (13)
  { 4, "\x04\x02\x04\x08" },      // ~ 0x7E (14)
  { 5, "\x7F\x41\x41\x41\x7F" },  //   0x7F (15)

  { 5, "\x7D\x0a\x09\x0a\x7D" },  // Ä 0x41 (1)
  { 5, "\x3F\x41\x41\x41\x3F" },  // Ö 0x4F (15)
  { 5, "\x3D\x40\x40\x40\x3D" },  // Ü 0x55 (5)
  { 5, "\x20\x55\x54\x55\x78" },  // ä 0x61 (1)
  { 5, "\x38\x45\x44\x45\x38" },  // ö 0x6F (15)
  { 5, "\x3c\x41\x40\x41\x7c" },  // ü 0x75 (5)
};


// Main program, torch simulation
// ==============================

// moved defining constants for number of LEDs to top of file

const uint16_t numLeds = ledsPerLevel*levels; // total number of LEDs

p44_ws2812 leds(LED_TYPE, numLeds, swapXY ? levels : ledsPerLevel, reversedX, alternatingX, swapXY, reversedY, 1); // create WS281x driver

// global parameters

enum {
  mode_off = 0,
  mode_torch = 1, // torch
  mode_colorcycle = 2, // moving color cycle
  mode_lamp = 3, // lamp
  mode_testpattern = 4, // test pattern
};

byte mode = mode_torch; // main operation mode
int brightness = 255; // overall brightness
byte fade_base = 140; // crossfading base brightness level

// text params

int text_intensity = 255; // intensity of last column of text (where text appears)
int cycles_per_px = 5;
int text_repeats = 15; // text displays until faded down to almost zero
int fade_per_repeat = 15; // how much to fade down per repeat
int text_base_line = 8;
byte red_text = 0;
byte green_text = 255;
byte blue_text = 180;


// clock parameters

int clock_interval = 0; // 15*60; // by default, show clock every 15 mins (0=never)
int clock_zone = 2; // UTC+2 = CEST = Central European Summer Time
char clock_fmt[30] = "%k:%M"; // use format specifiers from strftime, see e.g. http://linux.die.net/man/3/strftime. %k:%M is 24h hour/minute clock

// torch parameters

uint16_t cycle_wait = 1; // 0..255

byte flame_min = 100; // 0..255
byte flame_max = 220; // 0..255

byte random_spark_probability = 2; // 0..100
byte spark_min = 200; // 0..255
byte spark_max = 255; // 0..255

byte spark_tfr = 40; // 0..256 how much energy is transferred up for a spark per cycle
uint16_t spark_cap = 200; // 0..255: spark cells: how much energy is retained from previous cycle

uint16_t up_rad = 40; // up radiation
uint16_t side_rad = 35; // sidewards radiation
uint16_t heat_cap = 0; // 0..255: passive cells: how much energy is retained from previous cycle

byte red_bg = 0;
byte green_bg = 0;
byte blue_bg = 0;
byte red_bias = 10;
byte green_bias = 0;
byte blue_bias = 0;
int red_energy = 180;
int green_energy = 145;
int blue_energy = 0;

byte upside_down = 0; // if set, flame (or rather: drop) animation is upside down. Text remains as-is


// lamp mode params

byte lamp_red = 220;
byte lamp_green = 220;
byte lamp_blue = 200;


#if !NO_CHEERLIGHT

// cheerlight params
uint8_t cheer_brightness = 100; // initial brightness
uint8_t cheer_fade_cycles = 30; // fade cheer color one brightness step every 30 cycles

#endif


// Cloud API
// =========

// this function automagically gets called upon a matching POST request
int handleParams(String command)
{
  //look for the matching argument "coffee" <-- max of 64 characters long
  int p = 0;
  while (p<(int)command.length()) {
    int i = command.indexOf(',',p);
    if (i<0) i = command.length();
    int j = command.indexOf('=',p);
    if (j<0) break;
    String key = command.substring(p,j);
    String value = command.substring(j+1,i);
    int val = value.toInt();
    // global params
    if (key=="wait")
      cycle_wait = val;
    else if (key=="mode")
      mode = val;
    else if (key=="brightness")
      brightness = val;
    else if (key=="fade_base")
      fade_base = val;
    #if !NO_CHEERLIGHT
    // cheerlight params
    else if (key=="cheer_brightness")
      cheer_brightness = val;
    else if (key=="cheer_fade_cycles")
      cheer_fade_cycles = val;
    #endif
    // simple lamp params
    else if (key=="lamp_red")
      lamp_red = val;
    else if (key=="lamp_green")
      lamp_green = val;
    else if (key=="lamp_blue")
      lamp_blue = val;
    // text color params
    else if (key=="red_text")
      red_text = val;
    else if (key=="green_text")
      green_text = val;
    else if (key=="blue_text")
      blue_text = val;
    // text params
    else if (key=="cycles_per_px")
      cycles_per_px = val;
    else if (key=="text_repeats")
      text_repeats = val;
    else if (key=="text_base_line")
      text_base_line = val;
    else if (key=="fade_per_repeat")
      fade_per_repeat = val;
    else if (key=="text_intensity")
      text_intensity = val;
    // clock display params
    else if (key=="clock_interval")
      clock_interval = val;
    else if (key=="clock_fmt")
      value.toCharArray(clock_fmt, 30);
    else if (key=="clock_zone")
      clock_zone = val;
    // torch color params
    else if (key=="red_bg")
      red_bg = val;
    else if (key=="green_bg")
      green_bg = val;
    else if (key=="blue_bg")
      blue_bg = val;
    else if (key=="red_bias")
      red_bias = val;
    else if (key=="green_bias")
      green_bias = val;
    else if (key=="blue_bias")
      blue_bias = val;
    else if (key=="red_energy")
      red_energy = val;
    else if (key=="green_energy")
      green_energy = val;
    else if (key=="blue_energy")
      blue_energy = val;
    // torch params
    else if (key=="spark_prob") {
      random_spark_probability = val;
      resetEnergy();
    }
    else if (key=="spark_cap")
      spark_cap = val;
    else if (key=="spark_tfr")
      spark_tfr = val;
    else if (key=="side_rad")
      side_rad = val;
    else if (key=="up_rad")
      up_rad = val;
    else if (key=="heat_cap")
      heat_cap = val;
    else if (key=="flame_min")
      flame_min = val;
    else if (key=="flame_max")
      flame_max = val;
    else if (key=="spark_min")
      spark_min = val;
    else if (key=="spark_max")
      spark_max = val;
    else if (key=="upside_down")
      upside_down = val;
    p = i+1;
  }
  return 1;
}


#if !NO_DIGITALSTROM

const int VDSD_API_VERSION=2;

// this function automagically gets called upon a matching POST request
int handleVdsd(String command)
{
  String cmd = command;
  String value;
  bool hasValue = false;
  int j = command.indexOf('=');
  if (j>=0) {
    hasValue = true;
    String c = command.substring(0,j);
    cmd = c;
    String v = command.substring(j+1);
    value = v;
  }
  if (cmd=="version") {
    // API version
    return VDSD_API_VERSION;
  }
  else if (cmd=="config") {
    // 0xssiibboo, ss=# of sensors, ii=# of binary inputs, bb=# of buttons, oo=# output type (0=none, 1=on/off, 2=RGB)
    return 0x00000002; // RGB output
  }
  else if (cmd=="brightness") {
    // primary output is brightness
    if (hasValue) {
      brightness = value.toInt();
    }
    else {
      return brightness;
    }
  }
  else if (cmd=="state") {
    // state is: 0xmmrrggbb, where mm=mode, rr/gg/bb = RGB for RGB modes or bb=brightness for non-RBG
    if (hasValue) {
      uint32_t v = value.toInt();
      // get mode
      mode = (v>>24) & 0xFF;
      if (mode==mode_lamp) {
        // RGB Lamp
        lamp_red = (v>>16) & 0xFF;
        lamp_green = (v>>8) & 0xFF;
        lamp_blue = v & 0xFF;
        brightness = 0xFF;
      }
      else {
        // colored modes, only set overall brightness
        brightness = v & 0xFF;
      }
    }
    else {
      if (mode==mode_lamp) {
        // RGB Lamp
        return
          (mode<<24) |
          (lamp_red<<16) |
          (lamp_green<<8) |
          lamp_blue;
      }
      else {
        // only brightness
        return
          (mode<<24) |
          (brightness & 0xFF);
      }
    }
  }
  return 0;
}

#endif


// text layer
// ==========

// text layer, but only one strip around the tube (ledsPerLevel) with the height of the characters (rowsPerGlyph)
const int textPixels = ledsPerLevel*rowsPerGlyph;
byte textLayer[textPixels];
String text;

int textPixelOffset;
int textCycleCount;
int repeatCount;


// this function automagically gets called upon a matching POST request
int newMessage(String aText)
{
  // URL decode
  text = "";
  int i = 0;
  char c;
  while (i<(int)aText.length()) {
    if (aText[i]=='%') {
      if ((int)aText.length()<=i+2) break; // end of text
      // get hex
      c = (hexToInt(aText[i+1])<<4) + hexToInt(aText[i+2]);
      i += 2;
    }
    // Ä = C3 84
    // Ö = C3 96
    // Ü = C3 9C
    // ä = C3 A4
    // ö = C3 B6
    // ü = C3 BC
    else if (aText[i]==0xC3) {
      if ((int)aText.length()<=i+1) break; // end of text
      switch (aText[i+1]) {
        case 0x84: c = 0x80; break; // Ä
        case 0x96: c = 0x81; break; // Ö
        case 0x9C: c = 0x82; break; // Ü
        case 0xA4: c = 0x83; break; // ä
        case 0xB6: c = 0x84; break; // ö
        case 0xBC: c = 0x85; break; // ü
        default: c = 0x7F; break; // unknown
      }
      i += 1;
    }
    else {
      c = aText[i];
    }
    // put to output string
    text += String(c);
    i++;
  }
  // initiate display of new text
  textPixelOffset = -ledsPerLevel;
  textCycleCount = 0;
  repeatCount = 0;
  return 1;
}


void resetText()
{
  for(int i=0; i<textPixels; i++) {
    textLayer[i] = 0;
  }
}


void crossFade(byte aFader, byte aValue, byte &aOutputA, byte &aOutputB)
{
  byte baseBrightness = (aValue*fade_base)>>8;
  byte varBrightness = aValue-baseBrightness;
  byte fade = (varBrightness*aFader)>>8;
  aOutputB = baseBrightness+fade;
  aOutputA = baseBrightness+(varBrightness-fade);
}


int glyphIndexForChar(const char aChar)
{
  int i = aChar-0x20;
  if (i<0 || i>=numGlyphs) i = 95; // ASCII 0x7F-0x20
  return i;
}


void renderText()
{
  // fade between rows
  byte maxBright = text_intensity-repeatCount*fade_per_repeat;
  byte thisBright, nextBright;
  crossFade(255*textCycleCount/cycles_per_px, maxBright, thisBright, nextBright);
  // generate vertical rows
  int activeCols = ledsPerLevel-2;
  // calculate text length in pixels
  int totalTextPixels = 0;
  int textLen = (int)text.length();
  for (int i=0; i<textLen; i++) {
    // sum up width of individual chars
    totalTextPixels += fontGlyphs[glyphIndexForChar(text[i])].width + glyphSpacing;
  }
  for (int x=0; x<ledsPerLevel; x++) {
    uint8_t column = 0;
    // determine font column
    if (x<activeCols) {
      int colPixelOffset = textPixelOffset + x;
      if (colPixelOffset>=0) {
        // visible column
        // - calculate character index
        int charIndex = 0;
        int glyphOffset = colPixelOffset;
        const glyph_t *glyphP = NULL;
        while (charIndex<textLen) {
          glyphP = &fontGlyphs[glyphIndexForChar(text[charIndex])];
          int cw = glyphP->width + glyphSpacing;
          if (glyphOffset<cw) break; // found char
          glyphOffset -= cw;
          charIndex++;
        }
        // now we have
        // - glyphP = the glyph,
        // - glyphOffset=column offset within that glyph (but might address a spacing column not stored in font table)
        if (charIndex<textLen) {
          // is a column of a visible char
          if (glyphOffset<glyphP->width) {
            // fetch glyph column
            column = glyphP->cols[glyphOffset];
          }
        }
      }
    }
    // now render columns
    for (int glyphRow=0; glyphRow<rowsPerGlyph; glyphRow++) {
      int i;
      int leftstep;
      if (mirrorText) {
        i = (glyphRow+1)*ledsPerLevel - 1 - x; // LED index, x-direction mirrored
        leftstep = 1;
      }
      else {
        i = glyphRow*ledsPerLevel + x; // LED index
        leftstep = -1;
      }
      if (glyphRow < rowsPerGlyph) {
        if (column & (0x40>>glyphRow)) {
          textLayer[i] = thisBright;
          // also adjust pixel left to this one
          if (x>0) {
            increase(textLayer[i+leftstep], nextBright, maxBright);
          }
          continue;
        }
      }
      textLayer[i] = 0; // no text
    }
  }
  // increment
  textCycleCount++;
  if (textCycleCount>=cycles_per_px) {
    textCycleCount = 0;
    textPixelOffset++;
    if (textPixelOffset>totalTextPixels) {
      // text shown, check for repeats
      repeatCount++;
      if (text_repeats!=0 && repeatCount>=text_repeats) {
        // done
        text = ""; // remove text
      }
      else {
        // show again
        textPixelOffset = -ledsPerLevel;
        textCycleCount = 0;
      }
    }
  }
}



// torch mode
// ==========

byte currentEnergy[numLeds]; // current energy level
byte nextEnergy[numLeds]; // next energy level
byte energyMode[numLeds]; // mode how energy is calculated for this point

enum {
  torch_passive = 0, // just environment, glow from nearby radiation
  torch_nop = 1, // no processing
  torch_spark = 2, // slowly looses energy, moves up
  torch_spark_temp = 3, // a spark still getting energy from the level below
};



void resetEnergy()
{
  for (int i=0; i<numLeds; i++) {
    currentEnergy[i] = 0;
    nextEnergy[i] = 0;
    energyMode[i] = torch_passive;
  }
}




void calcNextEnergy()
{
  int i = 0;
  for (int y=0; y<levels; y++) {
    for (int x=0; x<ledsPerLevel; x++) {
      byte e = currentEnergy[i];
      byte m = energyMode[i];
      switch (m) {
        case torch_spark: {
          // loose transfer up energy as long as the is any
          reduce(e, spark_tfr);
          // cell above is temp spark, sucking up energy from this cell until empty
          if (y<levels-1) {
            energyMode[i+ledsPerLevel] = torch_spark_temp;
          }
          break;
        }
        case torch_spark_temp: {
          // just getting some energy from below
          byte e2 = currentEnergy[i-ledsPerLevel];
          if (e2<spark_tfr) {
            // cell below is exhausted, becomes passive
            energyMode[i-ledsPerLevel] = torch_passive;
            // gobble up rest of energy
            increase(e, e2);
            // loose some overall energy
            e = ((int)e*spark_cap)>>8;
            // this cell becomes active spark
            energyMode[i] = torch_spark;
          }
          else {
            increase(e, spark_tfr);
          }
          break;
        }
        case torch_passive: {
          e = ((int)e*heat_cap)>>8;
          increase(e, ((((int)currentEnergy[i-1]+(int)currentEnergy[i+1])*side_rad)>>9) + (((int)currentEnergy[i-ledsPerLevel]*up_rad)>>8));
        }
        default:
          break;
      }
      nextEnergy[i++] = e;
    }
  }
}


const uint8_t energymap[32] = {0, 64, 96, 112, 128, 144, 152, 160, 168, 176, 184, 184, 192, 200, 200, 208, 208, 216, 216, 224, 224, 224, 232, 232, 232, 240, 240, 240, 240, 248, 248, 248};

void calcNextColors()
{
  int textStart = text_base_line*ledsPerLevel;
  int textEnd = textStart+rowsPerGlyph*ledsPerLevel;
  for (int i=0; i<numLeds; i++) {
    if (i>=textStart && i<textEnd && textLayer[i-textStart]>0) {
      // overlay with text color
      leds.setColorDimmed(i, red_text, green_text, blue_text, (brightness*textLayer[i-textStart])>>8);
    }
    else {
      int ei; // index into energy calculation buffer
      if (upside_down)
        ei = numLeds-i;
      else
        ei = i;
      uint16_t e = nextEnergy[ei];
      currentEnergy[ei] = e;
      if (e>250)
        leds.setColorDimmed(i, 170, 170, e, brightness); // blueish extra-bright spark
      else {
        if (e>0) {
          // energy to brightness is non-linear
          byte eb = energymap[e>>3];
          byte r = red_bias;
          byte g = green_bias;
          byte b = blue_bias;
          increase(r, (eb*red_energy)>>8);
          increase(g, (eb*green_energy)>>8);
          increase(b, (eb*blue_energy)>>8);
          leds.setColorDimmed(i, r, g, b, brightness);
        }
        else {
          // background, no energy
          leds.setColorDimmed(i, red_bg, green_bg, blue_bg, brightness);
        }
      }
    }
  }
}


void injectRandom()
{
  // random flame energy at bottom row
  for (int i=0; i<ledsPerLevel; i++) {
    currentEnergy[i] = random(flame_min, flame_max);
    energyMode[i] = torch_nop;
  }
  // random sparks at second row
  for (int i=ledsPerLevel; i<2*ledsPerLevel; i++) {
    if (energyMode[i]!=torch_spark && random(100)<random_spark_probability) {
      currentEnergy[i] = random(spark_min, spark_max);
      energyMode[i] = torch_spark;
    }
  }
}


#if !NO_CHEERLIGHT

// Cheerlights interface
// =====================
// see cheerlights.com
// Code partly from https://github.com/ls6/spark-core-cheerlights licensed under MIT license

TCPClient cheerLightsAPI;
String responseLine;
unsigned long nextPoll = 0;
uint8_t cheer_red = 0;
uint8_t cheer_green = 0;
uint8_t cheer_blue = 0;
uint8_t cheer_bright = 0;
uint8_t cheer_fade_cnt = 0;


void processCheerColor(String colorName)
{
  uint8_t red, green, blue;

  if (colorName == "purple") {
    red = 128; green = 0; blue = 128;
  } else if (colorName == "red") {
    red = 255; green = 0; blue = 0;
  } else if (colorName == "green") {
    red = 0; green = 255; blue = 0;
  } else if (colorName == "blue") {
    red = 0; green = 0; blue = 255;
  } else if (colorName == "cyan") {
    red = 0; green = 255; blue = 255;
  } else if (colorName == "white") {
    red = 255; green = 255; blue = 255;
  } else if (colorName == "warmwhite") {
    red = 253; green = 245; blue = 230;
  } else if (colorName == "magenta") {
    red = 255; green = 0; blue = 255;
  } else if (colorName == "yellow") {
    red = 255; green = 255; blue = 0;
  } else if (colorName == "orange") {
    red = 255; green = 165; blue = 0;
  } else if (colorName == "pink") {
    red = 255; green = 192; blue = 203;
  } else if (colorName == "oldlace") {
    red = 253; green = 245; blue = 230;
  }
  else {
    // unknown color, do nothing
  }
  // check if cheer color is different
  if (red!=cheer_red || green!=cheer_green || blue!=cheer_blue) {
    // initiate new cheer colored background sequence
    cheer_red = red;
    cheer_green = green;
    cheer_blue = blue;
    cheer_bright = cheer_brightness; // start with configured brightness
    cheer_fade_cnt = 0;
  }
}


void updateBackgroundWithCheerColor()
{
  if (cheer_bright>0) {
    red_bg = ((int)cheer_red*cheer_bright)>>8;
    green_bg = ((int)cheer_green*cheer_bright)>>8;
    blue_bg = ((int)cheer_blue*cheer_bright)>>8;
    // check fading
    cheer_fade_cnt++;
    if (cheer_fade_cnt>=cheer_fade_cycles) {
      cheer_fade_cnt = 0;
      cheer_bright--;
      // if we reached 0 now, turn off background now
      if (cheer_bright==0) {
        red_bg = 0;
        green_bg = 0;
        blue_bg = 0;
      }
    }
  }
}


void checkCheerlights()
{
  if (cheer_brightness>0) {
    // only poll if displaye is enabled (not zero brightness)
    if (nextPoll<=millis()) {
      nextPoll = millis()+60000;
      // in case previous request wasn't answered, close connection
      cheerLightsAPI.stop();
      // issue a new request
      if (cheerLightsAPI.connect("api.thingspeak.com", 80)) {
        cheerLightsAPI.println("GET /channels/1417/field/1/last.txt HTTP/1.0");
        cheerLightsAPI.println();
      }
      responseLine = "";
    }
    if (cheerLightsAPI.available()) {
      char ch = cheerLightsAPI.read();
      responseLine += ch;
      // check for end of line (LF)
      if (ch==0x0A) {
        if (responseLine.length() == 2) {
          // empty line (CRLF only)
          // now response body (color) follows
          String colorName = "";
          while (cheerLightsAPI.available()) {
            ch = cheerLightsAPI.read();
            colorName += ch;
          };
          processCheerColor(colorName);
          cheerLightsAPI.stop();
        };
        responseLine = ""; // next line
      }
    }
  }
}

#endif


// Main program
// ============

// Note: in system mode manual, there is no connectivity, and device must be programmed via USB
// SYSTEM_MODE(MANUAL);

void setup()
{
  resetEnergy();
  resetText();
  leds.begin();
  // remote control
  Spark.function("params", handleParams); // parameters
  Spark.function("message", newMessage); // text message display
  #if !NO_DIGITALSTROM
  Spark.function("vdsd", handleVdsd); // virtual digitalstrom device interface
  #endif
}



byte cnt = 0;

void loop()
{
  #if !NO_CHEERLIGHT
  // check cheerlights
  checkCheerlights();
  updateBackgroundWithCheerColor();
  #endif
  // check clock display
  if (clock_interval>0) {
    time_t now = Time.now(); // UTC
    now += clock_zone*3600; // add seconds east (=ahead) of UTC
    struct tm *loc;
    loc = localtime(&now);
    int secOfHour = loc->tm_min*60 + loc->tm_sec;
    if (secOfHour % clock_interval == 0) {
       // seconds of hour evenly dividable by clock_interval -> display time now
       char timeString[30];
       strftime(timeString, 30, clock_fmt, loc);
       newMessage(timeString);
    }
  }

  // render the text
  renderText();
  int textStart = text_base_line*ledsPerLevel;
  int textEnd = textStart+rowsPerGlyph*ledsPerLevel;
  switch (mode) {
    case mode_off: {
      // off
      for(int i=0; i<leds.getNumPixels(); i++) {
        leds.setColor(i, 0, 0, 0);
      }
      break;
    }
    case mode_lamp: {
      // just single color lamp + text display
      for (int i=0; i<leds.getNumPixels(); i++) {
        if (i>=textStart && i<textEnd && textLayer[i-textStart]>0) {
          leds.setColorDimmed(i, red_text, green_text, blue_text, (textLayer[i-textStart]*brightness)>>8);
        }
        else {
          leds.setColorDimmed(i, lamp_red, lamp_green, lamp_blue, brightness);
        }
      }
      break;
    }
    case mode_torch: {
      // torch animation + text display + cheerlight background
      injectRandom();
      calcNextEnergy();
      calcNextColors();
      break;
    }
    case mode_colorcycle: {
      // simple color wheel animation
      cnt++;
      byte r,g,b;
      for(int i=0; i<leds.getNumPixels(); i++) {
        wheel(((i * 256 / leds.getNumPixels()) + cnt) & 255, r, g, b);
        if (i>=textStart && i<textEnd && textLayer[i-textStart]>0) {
          leds.setColorDimmed(i, r, g, b, (textLayer[i-textStart]*brightness)>>8);
        }
        else {
          leds.setColorDimmed(i, r, g, b, brightness>>1); // only half brightness for full area color
        }
      }
      break;
    }
    case mode_testpattern: {
      // test pattern
      for (int i=0; i<leds.getNumPixels(); i++) {
        int y = i / ledsPerLevel; // intensity
        int x = i % ledsPerLevel; // color
        byte r,g,b;
        wheel(x*10, r, g, b);
        leds.setColorDimmed(i, r, g, b, 60+y*9);
      }
      break;
    }
  }
  // transmit colors to the leds
  leds.show();
  // wait
  delay(cycle_wait); // latch & reset needs 50 microseconds pause, at least.
}
