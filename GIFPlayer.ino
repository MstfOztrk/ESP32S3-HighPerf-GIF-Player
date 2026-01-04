/// <summary>
/// LOCKED 30 FPS ESP32-S3 GIF PLAYER.
/// - Limits framerate to 30 FPS to prevent "too fast" playback.
/// - Reduces tearing by syncing frame timing.
/// - Retains Retro Scanline aesthetic.
/// </summary>
#include <SPI.h>
#include <FS.h>
#include <LittleFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <AnimatedGIF.h>

// --- AYARLAR ---
#define TARGET_FPS 60               // Sabit FPS Hedefi
#define FRAME_DELAY (1000 / TARGET_FPS) // Kare başına düşen milisaniye (33ms)

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

  // RETRO SCANLINE: Tek satırları atla (Hız + Estetik)
  if (y & 1) return;

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

void Die(const char *msg)
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

  // Hardware SPI Başlat
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  tft.begin(80000000);
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);

  if (!LittleFS.begin(true)) Die("FS ERROR");

  File f = LittleFS.open(GIF_PATH, "r");
  if (!f) Die("NO FILE");

  gifSize = f.size();
  
  gifData = (uint8_t *)ps_malloc(gifSize);
  if (!gifData) gifData = (uint8_t *)malloc(gifSize);
  if (!gifData) Die("NO RAM");

  f.read(gifData, gifSize);
  f.close();

  gif.begin(LITTLE_ENDIAN_PIXELS);

  if (!gif.open(gifData, gifSize, GIFDraw)) Die("GIF ERR");
  
  gif.setDrawType(GIF_DRAW_RAW);
  gif.allocFrameBuf(GIFAlloc);
}

void loop()
{
  // Başlangıç zamanını tut
  uint32_t startParams = millis();
  
  int delayMs = 0; 
  int result = gif.playFrame(false, &delayMs);
  
  if (result == 0)
  {
    gif.reset();
  }

  // İşlem süresini hesapla
  uint32_t processingTime = millis() - startParams;
  
  // Eğer işlem hedef süreden (33ms) kısa sürdüyse, aradaki fark kadar bekle
  if (processingTime < FRAME_DELAY)
  {
    delay(FRAME_DELAY - processingTime);
  }
  
  // Eğer işlem zaten 33ms'den uzun sürdüyse bekleme yapma (Drop frame olmasın diye devam et)
}