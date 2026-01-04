/// <summary>
/// FINAL PRODUCTION RELEASE
/// ------------------------
/// - SPI Speed: 60MHz (Stability fix)
/// - Sync: Adaptive (Reads GIF delay)
/// - Visual: Scanline Retro Mode (Configurable)
/// - Memory: Zero-Copy PSRAM
/// </summary>
#include <SPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <AnimatedGIF.h>

#define SPI_SPEED 60000000
#define RETRO_MODE 1

static const int PIN_MOSI = 11;
static const int PIN_MISO = 13;
static const int PIN_SCK  = 12;
static const int PIN_CS   = 10;
static const int PIN_DC   = 9;
static const int PIN_RST  = 8;
static const int PIN_BLK  = 7;

static const char* GIF_PATH = "/data.gif";

Adafruit_ILI9341 tft(PIN_CS, PIN_DC, PIN_RST);
AnimatedGIF gif;

uint8_t *gifData = NULL;
int32_t gifSize = 0;
uint16_t lineBuf[320];

void *GIFAlloc(uint32_t u32Size)
{
  void *p = ps_malloc(u32Size);
  if (!p) p = malloc(u32Size);
  return p;
}

void GIFFree(void *p)
{
  free(p);
}

void GIFDraw(GIFDRAW *pDraw)
{
  int x = pDraw->iX;
  int y = pDraw->iY + pDraw->y;
  int w = pDraw->iWidth;

  if (y >= tft.height() || x >= tft.width()) return;
  if (x + w > tft.width()) w = tft.width() - x;
  if (w <= 0) return;

#if RETRO_MODE
  if (y & 1) return;
#endif

  uint8_t *src = pDraw->pPixels;
  uint16_t *pal = pDraw->pPalette;

  tft.startWrite();

  if (pDraw->ucHasTransparency)
  {
    uint8_t tr = pDraw->ucTransparent;
    
    for (int i = 0; i < w; )
    {
      if (src[i] == tr)
      {
        i++;
        continue;
      }

      int run = 0;
      for (int k = i; k < w; k++)
      {
        if (src[k] == tr) break;
        lineBuf[run] = pal[src[k]];
        run++;
      }

      if (run > 0)
      {
        tft.setAddrWindow(x + i, y, run, 1);
        tft.writePixels(lineBuf, run, true, false);
        i += run;
      }
    }
  }
  else
  {
    for (int i = 0; i < w; i++)
    {
       lineBuf[i] = pal[src[i]];
    }
    tft.setAddrWindow(x, y, w, 1);
    tft.writePixels(lineBuf, w, true, false);
  }

  tft.endWrite();
}

void FatalError(const char *msg)
{
  tft.fillScreen(ILI9341_RED);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print(msg);
  while(1) {};
}

void setup()
{
  Serial.begin(115200);

  pinMode(PIN_BLK, OUTPUT);
  digitalWrite(PIN_BLK, HIGH);

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  tft.begin(SPI_SPEED);
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);

  if (!LittleFS.begin(true)) FatalError("FS FAIL");

  File f = LittleFS.open(GIF_PATH, "r");
  if (!f) FatalError("NO FILE");

  gifSize = f.size();
  
  gifData = (uint8_t *)ps_malloc(gifSize);
  if (!gifData) gifData = (uint8_t *)malloc(gifSize);
  if (!gifData) FatalError("NO RAM");

  f.read(gifData, gifSize);
  f.close();

  gif.begin(LITTLE_ENDIAN_PIXELS);

  if (!gif.open(gifData, gifSize, GIFDraw)) FatalError("GIF ERR");
  
  gif.setDrawType(GIF_DRAW_RAW);
  gif.allocFrameBuf(GIFAlloc);
}

void loop()
{
  uint32_t frameStart = millis();
  int frameDelay = 0; 

  int result = gif.playFrame(false, &frameDelay);
  
  if (result == 0)
  {
    gif.reset();
  }

  uint32_t workTime = millis() - frameStart;
  
  if (workTime < frameDelay)
  {
    delay(frameDelay - workTime);
  }
}
