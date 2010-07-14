/*
 * The TiLib: wavelet based lossy image compression library
 * Copyright (C) 1998-2004 Alexander Simakov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * QuikInfo:
 *
 * Color conversion routines (RGB <-> YCbCr).
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#define ROUND(x) (((x) < 0) ? (int) ((x) - 0.5) : (int) ((x) + 0.5))
#define FIX(x) ((x) < 0 ? 0 : ((x) > 255 ? 255 : (x)))

void ConvertRGBToYCbCr(unsigned char *buf, int n)
{
  double lum, cb, cr;
  double r, g, b;

  unsigned char *pn;

  pn = buf + n;

  while (buf < pn) {

    r = buf[0];
    g = buf[1];
    b = buf[2];

    lum = ROUND(0.299 * r + 0.587 * g + 0.114 *b);
    cb = ROUND((b - lum) / 1.772 + 127.5);
    cr = ROUND((r - lum) / 1.402 + 127.5);

    buf[0] = (unsigned char) FIX(lum);
    buf[1] = (unsigned char) FIX(cb);
    buf[2] = (unsigned char) FIX(cr);

    buf += 3;
  }
}

void ConvertYCbCrToRGB(unsigned char *buf, int n)
{
  double lum, cb, cr;
  double r, g, b;

  unsigned char *pn;

  pn = buf + n;

  while (buf < pn) {

    lum = buf[0];
    cb = buf[1] - 127.5;
    cr = buf[2] - 127.5;

    r = ROUND(lum + cr * 1.402);
    b = ROUND(lum + cb * 1.772);
    g = ROUND((lum - 0.114 * b - 0.299 * r) / 0.587);

    buf[0] = (unsigned char) FIX(r);
    buf[1] = (unsigned char) FIX(g);
    buf[2] = (unsigned char) FIX(b);

    buf += 3;
  }
}
