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
 * Just a few return codes for a library internal functions.
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#ifndef ERRCODES_H
#define ERRCODES_H

#ifdef __cplusplus
extern "C" {
#endif

#define OK             (0)
#define MEMORY_ERROR   (1)
#define INTERNAL_ERROR (2)
#define BUFFER_EMPTY   (3)
#define BUFFER_FULL    (4)
#define TRUE           (5)
#define FALSE          (6)
#define BAD_PARAMS     (7)
#define DAMAGED_HEADER (8)

#ifdef __cplusplus
}
#endif

#endif /* ERRCODES_H */
