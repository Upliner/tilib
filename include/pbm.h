/*
 * pbm.h
 *
 * PGM/PPM image formats support routines.
 *
 * (C) ENTROPYWARE, Alexander Simakov.
 *
 * Our site: http://www.entropyware.info/
 * Support:  support@entropyware.info
 * Author:   xander@entropyware.info
 */

#ifndef PBM_H
#define PBM_H

#ifdef __cplusplus
extern "C" {
#endif

#define PGM ('5') /* P5 */
#define PPM ('6') /* P6 */

/* Portable BitMap HeaDeR */

typedef struct
{
  int type;    /* image type (PGM or PPM) */
  int width;   /* image width */
  int height;  /* image height */
  int max_val; /* maximal sample value */
} pbm_hdr;

int read_hdr(FILE *f, pbm_hdr *h);
int file_size(FILE *f);
int get_char(FILE *f);
int get_integer(FILE *f);

#ifdef __cplusplus
}
#endif

#endif /* PBM_H */
