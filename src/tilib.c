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
 * Top-level compression & decompression functions.
 * 
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#include <stdlib.h>
#include <memory.h>

#include "../include/tilib.h"
#include "../include/color.h"
#include "../include/daub97.h"
#include "../include/butterworth.h"
#include "../include/extend.h"
#include "../include/spiht.h"
#include "../include/split.h"
#include "../include/errcodes.h"

#define HDRSIZE    (22)
#define DEF_SCALES (5)

#define DEF_LUM (90)
#define DEF_CB  (5)
#define DEF_CR  (5)

#define ALIGN(x_, y_) ((x_) & ((1 << y_) - 1) ? ((x_) & ~((1 << y_) - 1)) + (1 << y_) : (x_))

#define MAX(x_, y_) (x_ > y_ ? x_ : y_)
#define MIN(x_, y_) (x_ < y_ ? x_ : y_)

#define WRITE_BYTE(buf_, data_, offs_) (buf_[offs_ + 0] = (unsigned char) ((data_ >> 0) & 0xff))

#define WRITE_WORD(buf_, data_, offs_) (buf_[offs_ + 0] = (unsigned char) ((data_ >> 8) & 0xff),\
                                        buf_[offs_ + 1] = (unsigned char) ((data_ >> 0) & 0xff))

#define WRITE_DWORD(buf_, data_, offs_) (buf_[offs_ + 0] = (unsigned char) ((data_ >> 24) & 0xff),\
                                         buf_[offs_ + 1] = (unsigned char) ((data_ >> 16) & 0xff),\
                                         buf_[offs_ + 2] = (unsigned char) ((data_ >>  8) & 0xff),\
                                         buf_[offs_ + 3] = (unsigned char) ((data_ >>  0) & 0xff))

#define READ_BYTE(buf_, data_, offs_) (data_ = (int) (buf_[offs_ + 0]))

#define READ_WORD(buf_, data_, offs_) (data_ = (int) ((buf_[offs_ + 0] << 8) |\
                                                      (buf_[offs_ + 1] << 0)))

#define READ_DWORD(buf_, data_, offs_) (data_ = (int) ((buf_[offs_ + 0] << 24) |\
                                                       (buf_[offs_ + 1] << 16) |\
                                                       (buf_[offs_ + 2] << 8)  |\
                                                       (buf_[offs_ + 3] << 0)))

static unsigned char check_sum(unsigned char *buf, int len);

static unsigned char check_sum(unsigned char *buf, int len)
{
  unsigned char s1 = 1;
  unsigned char s2 = 0;

  if (len == 0) return 1;

  do {
    s1 = (unsigned char) ((s1 + *buf++) % 13);
    s2 = (unsigned char) ((s1 + s2) % 13);
  } while (--len != 0);

  return (unsigned char) ((s2 << 4) + s1);
}

int TiCompress(unsigned char *image,
               unsigned char *stream,
               int img_width,
               int img_height,
               int wavelet,
               int img_type,
               int desired_size,
               int *actual_size,
               int lum_ratio,
               int cb_ratio,
               int cr_ratio,
               int scales)
{
  int align_width, align_height;
  int width_bits, height_bits, temp;
  int lum_size, cb_size, cr_size;
  int lum_actual, cb_actual, cr_actual;
  int result;
  unsigned char *image_buf, *stream_buf;
  unsigned char *src, *dst, *end;
  double *dwt_data;

  if (image == NULL || stream == NULL) return BAD_PARAMS;
  if (img_width <= 0 || img_height <= 0) return BAD_PARAMS;
  if (img_width > 16383 || img_height > 16383) return BAD_PARAMS;
  if (wavelet != BUTTERWORTH && wavelet != DAUB97) return BAD_PARAMS;
  if (img_type != GRAYSCALE && img_type != TRUECOLOR) return BAD_PARAMS;
  if (img_type == GRAYSCALE && desired_size < HDRSIZE + 2) return BAD_PARAMS;
  if (img_type == TRUECOLOR && desired_size < HDRSIZE + 6) return BAD_PARAMS;
  if (lum_ratio * cb_ratio * cr_ratio == 0 && (lum_ratio != 0 || cb_ratio != 0 || cr_ratio != 0)) return BAD_PARAMS;
  if (lum_ratio + cb_ratio + cr_ratio != 0 && lum_ratio + cb_ratio + cr_ratio != 100) return BAD_PARAMS;
  if (scales < 0) return BAD_PARAMS;

  *actual_size = 0;

  dwt_data = NULL;
  image_buf = NULL;
  stream_buf = NULL;

  if (scales == 0) {

    temp = img_width;
    width_bits = 0;

    while ((temp & 1) != 1) {
      temp >>= 1;
      width_bits++;
    }

    temp = img_height;
    height_bits = 0;

    while ((temp & 1) != 1) {
      temp >>= 1;
      height_bits++;
    }

    scales = MAX(DEF_SCALES, MIN(width_bits, height_bits));
  }

  align_width = ALIGN(img_width, scales);
  align_height = ALIGN(img_height, scales);

  dwt_data = (double *) malloc(align_width * align_height * sizeof(double));

  if (dwt_data == NULL) {
    result = MEMORY_ERROR;
    goto error;
  }

  if (img_type == GRAYSCALE) {

    ExtendImage(image, dwt_data, img_height, img_width, align_height, align_width);

    if (wavelet == BUTTERWORTH)
      result = ButterworthAnalysis2D(dwt_data, align_width, align_height, scales);
    else
      result = Daub97Analysis2D(dwt_data, align_height, align_width, scales);

    if (result != OK) goto error;

    result = SPIHTEncodeDWT(dwt_data, align_height, align_width, scales, stream + HDRSIZE, desired_size - HDRSIZE, actual_size);

    if (result != OK && result != BUFFER_FULL) goto error;

    WRITE_BYTE(stream, 0x54, 0);
    WRITE_BYTE(stream, 0x69, 1);

    WRITE_WORD(stream, img_width, 2);
    WRITE_WORD(stream, img_height, 4);

    WRITE_BYTE(stream, scales, 6);
    WRITE_BYTE(stream, img_type, 7);
    WRITE_BYTE(stream, (wavelet == BUTTERWORTH ? 0 : 1), 8);

    WRITE_DWORD(stream, *actual_size, 9);
    WRITE_DWORD(stream, 0, 13);
    WRITE_DWORD(stream, 0, 17);

    WRITE_BYTE(stream, check_sum(stream, HDRSIZE - 1), 21);

    *actual_size += HDRSIZE;

    result = OK;

  } else {

    if (lum_ratio == 0) {

      cr_size = MAX(2, ((desired_size - HDRSIZE) * DEF_CR / 100) - 4);
      cb_size = MAX(2, ((desired_size - HDRSIZE) * DEF_CB / 100) - 4);
      lum_size = (desired_size - HDRSIZE) - cr_size - cb_size;

    } else {

      cr_size = MAX(2, ((desired_size - HDRSIZE) * cr_ratio / 100) - 4);
      cb_size = MAX(2, ((desired_size - HDRSIZE) * cb_ratio / 100) - 4);
      lum_size = (desired_size - HDRSIZE) - cr_size - cb_size;
    }

    image_buf = (unsigned char *) malloc(img_width * img_height);
    stream_buf = (unsigned char *) malloc((desired_size - HDRSIZE));

    if (image_buf == NULL || stream_buf == NULL) {
      result = MEMORY_ERROR;
      goto error;
    }

    ConvertRGBToYCbCr(image, img_width * img_height * 3);

    src = image;
    dst = image_buf;
    end = image + img_width * img_height * 3;

    while (src < end) {
      *dst = *src;
      dst++; src += 3;
    }

    ExtendImage(image_buf, dwt_data, img_height, img_width, align_height, align_width);

    if (wavelet == BUTTERWORTH)
      result = ButterworthAnalysis2D(dwt_data, align_width, align_height, scales);
    else 
      result = Daub97Analysis2D(dwt_data, align_height, align_width, scales);

    if (result != OK) goto error;

    result = SPIHTEncodeDWT(dwt_data, align_height, align_width, scales, stream_buf, lum_size, &lum_actual);

    if (result != OK && result != BUFFER_FULL) goto error;

    src = image + 1;
    dst = image_buf;
    end = image + img_width * img_height * 3;

    while (src < end) {
      *dst = *src;
      dst++; src += 3;
    }

    ExtendImage(image_buf, dwt_data, img_height, img_width, align_height, align_width);

    if (wavelet == BUTTERWORTH)
      result = ButterworthAnalysis2D(dwt_data, align_width, align_height, scales);
    else
      result = Daub97Analysis2D(dwt_data, align_height, align_width, scales);

    if (result != OK) goto error;

    result = SPIHTEncodeDWT(dwt_data, align_height, align_width, scales, stream_buf + lum_actual, cb_size, &cb_actual);

    if (result != OK && result != BUFFER_FULL) goto error;

    src = image + 2;
    dst = image_buf;
    end = image + img_width * img_height * 3;

    while (src < end) {
      *dst = *src;
      dst++; src += 3;
    }

    ExtendImage(image_buf, dwt_data, img_height, img_width, align_height, align_width);

    if (wavelet == BUTTERWORTH)
      result = ButterworthAnalysis2D(dwt_data, align_width, align_height, scales);
    else
      result = Daub97Analysis2D(dwt_data, align_height, align_width, scales);

    if (result != OK) goto error;

    result = SPIHTEncodeDWT(dwt_data, align_height, align_width, scales, stream_buf + lum_actual + cb_actual, cr_size, &cr_actual);

    if (result != OK && result != BUFFER_FULL) goto error;

    MergeChannels(stream + HDRSIZE, stream_buf, stream_buf + lum_actual, stream_buf + lum_actual + cb_actual, lum_actual, cb_actual, cr_actual);

    WRITE_BYTE(stream, 0x54, 0);
    WRITE_BYTE(stream, 0x69, 1);

    WRITE_WORD(stream, img_width, 2);
    WRITE_WORD(stream, img_height, 4);

    WRITE_BYTE(stream, scales, 6);
    WRITE_BYTE(stream, img_type, 7);
    WRITE_BYTE(stream, (wavelet == BUTTERWORTH ? 0 : 1) , 8);

    WRITE_DWORD(stream, lum_actual, 9);
    WRITE_DWORD(stream, cb_actual, 13);
    WRITE_DWORD(stream, cr_actual, 17);

    WRITE_BYTE(stream, check_sum(stream, HDRSIZE - 1), 21);

    *actual_size = lum_actual + cb_actual + cr_actual + HDRSIZE;

    result = OK;
  }

  error:

  free(dwt_data);
  free(image_buf);
  free(stream_buf);

  return result;
}

int TiCheckHeader(unsigned char *stream,
                  int *img_width,
                  int *img_height,
                  int *img_type,
                  int stream_size)
{
  if (stream_size < HDRSIZE) return DAMAGED_HEADER;

  if (check_sum(stream, HDRSIZE - 1) != stream[21]) return DAMAGED_HEADER;

  READ_WORD(stream, *img_width, 2);
  READ_WORD(stream, *img_height, 4);
  READ_BYTE(stream, *img_type, 7);

  return OK;
}

int TiDecompress(unsigned char *stream,
                 unsigned char *image,
                 int img_width,
                 int img_height,
                 int img_type,
                 int stream_size)
{
  int scales, lum_size, cb_size, cr_size;
  int lum_actual, cb_actual, cr_actual;
  int align_width, align_height, wavelet = 0;
  int result;
  unsigned char *image_buf, *stream_buf;
  unsigned char *src, *dst, *end;
  double *dwt_data;

  if (image == NULL || stream == NULL) return BAD_PARAMS;
  if (img_width <= 0 || img_height <= 0) return BAD_PARAMS;
  if (img_width > 16383 || img_height > 16383) return BAD_PARAMS;
  if (img_type != GRAYSCALE && img_type != TRUECOLOR) return BAD_PARAMS;
  if (img_type == GRAYSCALE && stream_size < HDRSIZE + 2) return DAMAGED_HEADER;
  if (img_type == TRUECOLOR && stream_size < HDRSIZE + 6) return DAMAGED_HEADER;

  if (check_sum(stream, HDRSIZE - 1) != stream[21]) return DAMAGED_HEADER;

  READ_BYTE(stream, scales, 6);
  READ_BYTE(stream, wavelet, 8);

  READ_DWORD(stream, lum_size, 9);
  READ_DWORD(stream, cb_size, 13);
  READ_DWORD(stream, cr_size, 17);

  dwt_data = NULL;
  image_buf = NULL;
  stream_buf = NULL;

  align_width = ALIGN(img_width, scales);
  align_height = ALIGN(img_height, scales);

  dwt_data = (double *) malloc(align_width * align_height * sizeof(double));

  if (dwt_data == NULL) {
    result = MEMORY_ERROR;
    goto error;
  }

  if (img_type == GRAYSCALE) {

    result = SPIHTDecodeDWT(dwt_data, align_height, align_width, scales, stream + HDRSIZE, stream_size - HDRSIZE);

    if (result != OK && result != BUFFER_EMPTY) goto error;

    if (wavelet == BUTTERWORTH)
      result = ButterworthSynthesis2D(dwt_data, align_width, align_height, scales);
    else
      result = Daub97Synthesis2D(dwt_data, align_height, align_width, scales);

    if (result != OK) goto error;

    ExtractImage(dwt_data, image, align_height, align_width, img_height, img_width);

    result = OK;

  } else {

    image_buf = (unsigned char *) malloc(img_width * img_height);
    stream_buf = (unsigned char *) malloc(lum_size + cb_size + cr_size);

    if (image_buf == NULL || stream_buf == NULL) {
      result = MEMORY_ERROR;
      goto error;
    }

    SplitChannels(stream + HDRSIZE, stream_buf, stream_buf + lum_size, stream_buf + lum_size + cb_size,
    stream_size - HDRSIZE, lum_size, cb_size, cr_size, &lum_actual, &cb_actual, &cr_actual);

    if (lum_actual < 2) {
      memset(dwt_data, 0, align_width * align_height * sizeof(double));
      result = OK;
    } else {
      result = SPIHTDecodeDWT(dwt_data, align_height, align_width, scales, stream_buf, lum_actual);
    }

    if (result != OK && result != BUFFER_EMPTY) goto error;

    if (wavelet == BUTTERWORTH)
      result = ButterworthSynthesis2D(dwt_data, align_width, align_height, scales);
    else
      result = Daub97Synthesis2D(dwt_data, align_height, align_width, scales);

    if (result != OK) goto error;

    ExtractImage(dwt_data, image_buf, align_height, align_width, img_height, img_width);

    src = image_buf;
    dst = image;
    end = image_buf + img_width * img_height;

    while (src < end) {
      *dst = *src;
       src++; dst += 3;
    }

    if (cb_actual < 2) {
      memset(dwt_data, 0, align_width * align_height * sizeof(double));
      result = OK;
    } else {
      result = SPIHTDecodeDWT(dwt_data, align_height, align_width, scales, stream_buf + lum_size, cb_actual);
    }

    if (result != OK && result != BUFFER_EMPTY) goto error;

    if (wavelet == BUTTERWORTH)
      result = ButterworthSynthesis2D(dwt_data, align_width, align_height, scales);
    else
      result = Daub97Synthesis2D(dwt_data, align_height, align_width, scales);

    if (result != OK) goto error;

    ExtractImage(dwt_data, image_buf, align_height, align_width, img_height, img_width);

    src = image_buf;
    dst = image + 1;
    end = image_buf + img_width * img_height;

    while (src < end) {
      *dst = *src;
       src++; dst += 3;
    }

    if (cr_actual < 2) {
      memset(dwt_data, 0, align_width * align_height * sizeof(double));
      result = OK;
    } else {
      result = SPIHTDecodeDWT(dwt_data, align_height, align_width, scales, stream_buf + lum_size + cb_size, cr_actual);
    }

    if (result != OK && result != BUFFER_EMPTY) goto error;

    if (wavelet == BUTTERWORTH)
      result = ButterworthSynthesis2D(dwt_data, align_width, align_height, scales);
    else
      result = Daub97Synthesis2D(dwt_data, align_height, align_width, scales);

    if (result != OK) goto error;

    ExtractImage(dwt_data, image_buf, align_height, align_width, img_height, img_width);

    src = image_buf;
    dst = image + 2;
    end = image_buf + img_width * img_height;

    while (src < end) {
      *dst = *src;
       src++; dst += 3;
    }

    ConvertYCbCrToRGB(image, img_width * img_height * 3);

    result = OK;
  }

  error:

  free(dwt_data);
  free(image_buf);
  free(stream_buf);

  return result;
}
