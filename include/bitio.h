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

#ifndef BITIO_H
#define BITIO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  unsigned char *buffer;
  unsigned char *buffer_end;
  unsigned char *next_byte;

  int buffer_size;

  int bit_buffer;
  int mask;

} BitStream;

BitStream *AllocBitStream();
void FreeBitStream(BitStream *bit_stream);
void InitWriteBits(BitStream *bit_stream);
void InitReadBits(BitStream *bit_stream);
int WriteBit(BitStream *bit_stream, int bit);
int ReadBit(BitStream *bit_stream, int *bit);
int FlushBits(BitStream *bit_stream);

#ifdef __cplusplus
}
#endif

#endif /* BITIO_H */
