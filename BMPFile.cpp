/*
   Copyright (c) 2014, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BMPFile.h"

typedef struct {
    union {
        uint16_t value;
        struct {
            unsigned r:5;
            unsigned g:6;
            unsigned b:5;
        } __attribute__((packed));
    } __attribute__((packed));
} __attribute__((packed)) Color565;

struct BitmapPixel24 {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} __attribute__((packed));

/*! Utility function to convert a 24-bit RGB value into a 16-bit RGB value. */
uint16_t BMPFile::rgb(uint32_t c) {
    uint8_t r = c >> 16;
    uint8_t g = c >> 8;
    uint8_t b = c;

    r = r >> 3;
    g = g >> 2;
    b = b >> 3;
    return ((r << 11) | (g << 5) | b);
}

/*! Utility function to convert three component colour values (R, G, B) into a 16-bit RGB value.*/
uint16_t BMPFile::rgb(uint8_t r, uint8_t g, uint8_t b) {
    r = r >> 3;
    g = g >> 2;
    b = b >> 3;
    return ((r << 11) | (g << 5) | b);
}

bool BMPFile::loadImageHeader() {
  _file->seek(0);
  if (_file->read((void *)&_header, sizeof(struct BitmapFileHeader)) != sizeof(struct BitmapFileHeader)) {
  	_file = NULL;
    _width = 0;
    _height = 0;
    return false;
  }

  if( _header.bfType != 0x4D42) return -1;

  if (_file->read((void *)&_info, sizeof(struct BitmapInfoHeader)) != sizeof(struct BitmapInfoHeader)) {
  	_file = NULL;
    _width = 0;
    _height = 0;
    return false;
  }

  if (_info.biBitCount < 16) {
    _file->seek(sizeof(struct BitmapFileHeader) + _info.biSize);
    _paletteSize = _info.biClrUsed;
    if (_paletteSize == 0) {
      _paletteSize = 1 << _info.biBitCount;
    }

    for (int i = 0; i < _paletteSize; i++) {
      _file->read((void *)&_palette[i], sizeof(struct BitmapPixel32));
    }
  }

  _file->seek(_header.bfBitmapOffset);
  _spos = _header.bfBitmapOffset;

  _width = _info.biWidth;
  _height = _info.biHeight;

  switch (_info.biBitCount) {
    case 1:
      _chunkSize = _width >> 3;
      break;
    case 4:
      _chunkSize = _width >> 1;
      break;
    case 8:
      _chunkSize = _width;
      break;
    case 16:
      _chunkSize = _width * 2;
      break;
    case 24:
      _chunkSize = _width * 3;
      break;
    case 32:
      _chunkSize = _width * 4;
      break;
  }

  _chunkSize = (_chunkSize + 3) & (~3);
  
  return true;
}

bool BMPFile::open(File &file)
{
	_file = &file;
	return loadImageHeader(); 
}

bool BMPFile::open(char* fileName)
{
	_file = new File();
	*_file = SD.open(fileName);
	return loadImageHeader();
}

void BMPFile::close(void)
{
	_file->close();
	_file = NULL;
}

void BMPFile::getScanLine(int16_t line, uint8_t *data) {
  uint32_t eof = _header.bfBitmapOffset + (getHeight() * _chunkSize);
  uint32_t pos = eof - ((line + 1) * _chunkSize);
  if (_spos != pos) {
    _file->seek(pos);
  }
  _file->read(data, _chunkSize);
  _spos += _chunkSize;
}

/*! Mix two colours together
 *  ========================
 *  Returns a new colour that is the mixing of the two provided colours.
 *
 *  Example:
 *
 *      unsigned int yellow = mix(Color::Red, Color::Green);
 */
uint16_t BMPFile::mix(uint16_t a, uint16_t b, uint8_t pct) {
    Color565 col_a;
    Color565 col_b;
    Color565 col_out;
    col_a.value = a;
    col_b.value = b;
    uint32_t temp;
    temp = (((int32_t)col_a.r * (255-pct)) / 255) + (((uint32_t)col_b.r * pct) / 255);
    col_out.r = temp;
    temp = (((int32_t)col_a.g * (255-pct)) / 255) + (((uint32_t)col_b.g * pct) / 255);
    col_out.g = temp;
    temp = (((int32_t)col_a.b * (255-pct)) / 255) + (((uint32_t)col_b.b * pct) / 255);
    col_out.b = temp;
    return col_out.value;
}


void BMPFile::drawIdx1(Adafruit_ST7735 *dev, int16_t x, int16_t y) {
  uint8_t data[_chunkSize];

  for (int32_t iy = 0; iy < getHeight(); iy++) {
    getScanLine(iy, data);
    for (uint32_t ix = 0; ix < getWidth(); ix++) {
      uint8_t mixIndex = data[ix >> 3];
      mixIndex = (mixIndex >> (7 - (ix % 8))) & 0x01;
      
      struct BitmapPixel32 *p = &_palette[mixIndex];
      uint16_t col = rgb(p->g, p->b, p->a);

      dev->drawPixel(x + ix, y + iy, col);

    }
  }
}

void BMPFile::drawIdx4(Adafruit_ST7735 *dev, int16_t x, int16_t y) {
  uint8_t data[_chunkSize];

  for (int32_t iy = 0; iy < getHeight(); iy++) {
    getScanLine(iy, data);
    for (uint32_t ix = 0; ix < getWidth(); ix++) {
      uint8_t mixIndex = data[ix >> 1];
      if (ix % 2 == 0)
      {
        mixIndex = (mixIndex >> 4) & 0x0F;
      }
      else
      {
        mixIndex = mixIndex & 0x0F;
      }
      struct BitmapPixel32 *p = &_palette[mixIndex];
      uint16_t col = rgb(p->g, p->b, p->a);

      dev->drawPixel(x + ix, y + iy, col);

    }
  }
}

void BMPFile::drawIdx(Adafruit_ST7735 *dev, int16_t x, int16_t y) {
  uint8_t data[_chunkSize];

  for (int32_t iy = 0; iy < getHeight(); iy++) {
    getScanLine(iy, data);
    for (uint32_t ix = 0; ix < getWidth(); ix++) {
      struct BitmapPixel32 *p = &_palette[data[ix]];
      uint16_t col = rgb(p->g, p->b, p->a);
      dev->drawPixel(x + ix, y + iy, col);
    }
  }

}

void BMPFile::draw565(Adafruit_ST7735 *dev, int16_t x, int16_t y) {
  uint8_t data[_chunkSize];

  for (int32_t iy = 0; iy < getHeight(); iy++) {
    getScanLine(iy, data);
    for (uint32_t ix = 0; ix < getWidth(); ix++) {
      uint32_t offset = ix * 2;
      uint16_t *p = (uint16_t *)(data + offset);
      dev->drawPixel(x + ix, y + iy, *p);
    }
  }
}

void BMPFile::drawRGB(Adafruit_ST7735 *dev, int16_t x, int16_t y) {
  uint8_t data[_chunkSize];

  for (int32_t iy = 0; iy < getHeight(); iy++) {
    getScanLine(iy, data);
    for (uint32_t ix = 0; ix < getWidth(); ix++) {
      uint32_t offset = ix * 3;
      struct BitmapPixel24 *p = (struct BitmapPixel24 *)(data + offset);
      uint16_t col = rgb(p->r, p->g, p->b);
      dev->drawPixel(x + ix, y + iy, col);
    }
  }
}

void BMPFile::drawRGBA(Adafruit_ST7735 *dev, int16_t x, int16_t y) {
  uint8_t data[_chunkSize];
  int rShift = 0;
  int gShift = 8;
  int bShift = 16;
  int aShift = 24;
  uint32_t rMask = 0x000000FF;
  uint32_t gMask = 0x0000FF00;
  uint32_t bMask = 0x00FF0000;
  uint32_t aMask = 0xFF000000;
  if (_info.biCompression == 3) {
    uint32_t t = _info.biMaskRed;
    rMask = _info.biMaskRed;
    rShift = 0;
    gShift = 0;
    bShift = 0;
    aShift = 0;
    while ((t & 1) == 0) {
      t >>= 1;
      rShift++;
    }
    t = _info.biMaskGreen;
    gMask = _info.biMaskGreen;
    while ((t & 1) == 0) {
      t >>= 1;
      gShift++;
    }
    t = _info.biMaskBlue;
    bMask = _info.biMaskBlue;
    while ((t & 1) == 0) {
      t >>= 1;
      bShift++;
    }
    t = _info.biMaskAlpha;
    aMask = _info.biMaskAlpha;
    while ((t & 1) == 0) {
      t >>= 1;
      aShift++;
    }
  }

  for (int32_t iy = 0; iy < getHeight(); iy++) {
    getScanLine(iy, data);
    for (uint32_t ix = 0; ix < getWidth(); ix++) {
      uint32_t offset = ix * 4;
      uint32_t red, green, blue, alpha;
      struct BitmapPixel32 *p = (struct BitmapPixel32 *)(data + offset);
      red = ((p->value & rMask) >> rShift);
      green = ((p->value & gMask) >> gShift);
      blue = ((p->value & bMask) >> bShift);
      alpha = ((p->value & aMask) >> aShift);
      int16_t fg = rgb(red, green, blue);
      dev->drawPixel(x + ix, y + iy, mix(_bgcolor, fg, alpha));

//      if (alpha == 255) {
//        dev->drawPixel(x + ix, y + iy, fg);
//      } else if (alpha > 0) {
//        dev->drawPixel(x + ix, y + iy, mix(ST7735_WHITE, fg, alpha));
//      }
    }
  }
}



void BMPFile::draw(Adafruit_ST7735 *dev, int16_t x, int16_t y) {
  if( _file == NULL ) return;
  switch (_info.biBitCount) {
    case 1:
      drawIdx1(dev, x, y);
      break;
    case 4:
      drawIdx4(dev, x, y);
      break;
    case 8:
      drawIdx(dev, x, y);
      break;
    case 16:
      draw565(dev, x, y);
      break;
    case 24:
      drawRGB(dev, x, y);
      break;
    case 32:
      drawRGBA(dev, x, y);
      break;
  }
}


