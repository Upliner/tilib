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
 * Simple Bit I/O routines for SPIHT.
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#include <stdlib.h>
#include "../include/bitio.h"
#include "../include/errcodes.h"

BitStream *AllocBitStream()
{
  return ((BitStream *) malloc(sizeof(BitStream)));
}

void FreeBitStream(BitStream *bit_stream)
{
  free(bit_stream);
}

void InitWriteBits(BitStream *bit_stream)
{
  bit_stream->buffer_end = bit_stream->buffer + bit_stream->buffer_size;
  bit_stream->next_byte = bit_stream->buffer;
  bit_stream->bit_buffer = 0;
  bit_stream->mask = 0x80;
}

void InitReadBits(BitStream *bit_stream)
{
  bit_stream->buffer_end = bit_stream->buffer + bit_stream->buffer_size;
  bit_stream->next_byte = bit_stream->buffer;
  bit_stream->bit_buffer = 0;
  bit_stream->mask = 0;
}

int WriteBit(BitStream *bit_stream, int bit)
{
  if (bit_stream->next_byte >= bit_stream->buffer_end) return BUFFER_FULL;

  if (bit != 0) bit_stream->bit_buffer |= bit_stream->mask;

  bit_stream->mask >>= 1;

  if (bit_stream->mask == 0) {

    *bit_stream->next_byte++ = (unsigned char) bit_stream->bit_buffer;
    bit_stream->bit_buffer = 0;
    bit_stream->mask = 0x80;
  }

  return OK;
}

int ReadBit(BitStream *bit_stream, int *bit)
{
  *bit = 0;

  if (bit_stream->mask == 0) {

    if (bit_stream->next_byte >= bit_stream->buffer_end) return BUFFER_EMPTY;

    bit_stream->bit_buffer = *bit_stream->next_byte++;
    bit_stream->mask = 0x80;
  }

  if ((bit_stream->bit_buffer & bit_stream->mask) != 0) *bit = 1;

  bit_stream->mask >>= 1;

  return OK;
}

int FlushBits(BitStream *bit_stream)
{
  if (bit_stream == NULL) return INTERNAL_ERROR;

  if (bit_stream->next_byte >= bit_stream->buffer_end) return BUFFER_FULL;

  if (bit_stream->mask != 128) *bit_stream->next_byte++ = (unsigned char) bit_stream->bit_buffer;

  return OK;
}
