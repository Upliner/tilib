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
 * Arithmetic coder for SPIHT.
 *
 * References:
 *
 * I.H. Witten, R.M. Neal, and J.H. Cleary, "Arithmetic coding for data
 * compression" CACM, vol. 30, pp. 520-540, Jun. 1987.
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#include <stdlib.h>
#include "../include/ari.h"
#include "../include/bitio.h"
#include "../include/errcodes.h"

ArithCoder *AllocArithCoder()
{
  return ((ArithCoder *) malloc(sizeof(ArithCoder)));
}

void FreeArithCoder(ArithCoder *arith_coder)
{
  free(arith_coder);
}

int BitPlusFolow(ArithCoder *arith_coder, BitStream *bit_stream, int bit)
{
  if ((WriteBit(bit_stream, bit)) == BUFFER_FULL) return BUFFER_FULL;

  while (arith_coder->underflow_bits > 0) {
    if ((WriteBit(bit_stream, 1 - bit)) == BUFFER_FULL) return BUFFER_FULL;
    arith_coder->underflow_bits--;
  }

  return OK;
}

void InitModel(ArithCoder *arith_coder)
{
  int i;

  for (i = 0; i <= ALPHA_SIZE; i++) arith_coder->cum_freq[i] = i;
}

void UpdateModel(ArithCoder *arith_coder, int symbol)
{
  int i;

  for (i = symbol + 1; i <= ALPHA_SIZE; i++) arith_coder->cum_freq[i]++;

  if (arith_coder->cum_freq[ALPHA_SIZE] >= MAX_FREQ) {

    arith_coder->cum_freq[0] = 0;

    for (i = 1; i <= ALPHA_SIZE; i++) {

      arith_coder->cum_freq[i] >>= 1;

      if (arith_coder->cum_freq[i] <= arith_coder->cum_freq[i - 1])
      arith_coder->cum_freq[i] = arith_coder->cum_freq[i - 1] + 1;
    }
  }
}

void InitEncoder(ArithCoder *arith_coder)
{
  arith_coder->low = 0;
  arith_coder->high = TOP_VALUE;
  arith_coder->underflow_bits = 0;
  arith_coder->value = 0;
}

int EncodeSymbol(ArithCoder *a, BitStream *b, int symbol)
{
  int range;

  range = a->high - a->low + 1;

  a->high = a->low + (range * a->cum_freq[symbol + 1]) / a->cum_freq[ALPHA_SIZE] - 1;
  a->low = a->low + (range * a->cum_freq[symbol]) / a->cum_freq[ALPHA_SIZE];

  for (;;) {

    if (a->high < HALF) {

      if ((BitPlusFolow(a, b, 0)) == BUFFER_FULL) return BUFFER_FULL;
    }
    else if (a->low >= HALF) {

      if ((BitPlusFolow(a, b, 1)) == BUFFER_FULL) return BUFFER_FULL;
      a->low -= HALF;
      a->high -= HALF;
    }
    else if (a->low >= FIRST_QTR && a->high < THIRD_QTR) {

      a->underflow_bits++;
      a->low -= FIRST_QTR;
      a->high -= FIRST_QTR;
    }
    else break;

    a->low = a->low << 1;
    a->high = (a->high << 1) + 1;
  }

  return OK;
}

int DoneEncoder(ArithCoder *arith_coder, BitStream *bit_stream)
{
  int i;

  for (i = 0; i < CODE_BITS; i++) {

    if (arith_coder->low >= HALF) {
      if ((BitPlusFolow(arith_coder, bit_stream, 1)) == BUFFER_FULL) return BUFFER_FULL;
      arith_coder->low -= HALF;
    } else {
      if ((BitPlusFolow(arith_coder, bit_stream, 0)) == BUFFER_FULL) return BUFFER_FULL;
    }

    arith_coder->low <<= 1;
  }

  return OK;
}

int InitDecoder(ArithCoder *arith_coder, BitStream *bit_stream)
{
  int i, bit;

  arith_coder->low = 0;
  arith_coder->high = TOP_VALUE;
  arith_coder->value = 0;

  for (i = 0; i < CODE_BITS; i++) {
    if ((ReadBit(bit_stream, &bit)) == BUFFER_EMPTY) return BUFFER_EMPTY;
    arith_coder->value = (arith_coder->value << 1) | bit;
  }

  return OK;
}

int DecodeSymbol(ArithCoder *a, BitStream *b, int *symbol)
{
  int range, cum, symb, bit;

  range = a->high - a->low + 1;

  cum = ((a->value - a->low + 1) * a->cum_freq[ALPHA_SIZE] - 1) / range;

  for (symb = ALPHA_SIZE - 1; a->cum_freq[symb] > cum; symb--);

  *symbol = symb;

  a->high = a->low + (range * a->cum_freq[symb + 1]) / a->cum_freq[ALPHA_SIZE] - 1;
  a->low = a->low + (range * a->cum_freq[symb]) / a->cum_freq[ALPHA_SIZE];

  for (;;) {

    if (a->high < HALF) {
      /* Nothing */
    }
    else if (a->low >= HALF) {

      a->value -= HALF;
      a->low -= HALF;
      a->high -= HALF;
    }
    else if (a->low >= FIRST_QTR && a->high < THIRD_QTR) {

      a->value -= FIRST_QTR;
      a->low -= FIRST_QTR;
      a->high -= FIRST_QTR;
    }
    else break;

    a->low = a->low << 1;
    a->high = (a->high << 1) + 1;

    if ((ReadBit(b, &bit)) == BUFFER_EMPTY) return BUFFER_EMPTY;

    a->value = (a->value << 1) | bit;
  }

  return OK;
}
