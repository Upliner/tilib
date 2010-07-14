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
 * Example of using TiLib for image coding (see readme.txt).
 * 
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "../include/pbm.h"
#include "../include/tilib.h"
#include "../include/errcodes.h"

#define MAX_LINE 1024

#define OPT_ENCODE      0
#define OPT_DECODE      1
#define OPT_INPUT       2
#define OPT_OUTPUT      3
#define OPT_SIZE        4
#define OPT_BUTTERWORTH 5
#define OPT_DAUBECHIES  6
#define OPT_LEVELS      7
#define OPT_HELP        8

int encode;
char infile[MAX_LINE];  /* Input file name */
char outfile[MAX_LINE]; /* Output file name */
int levels;             /* Number of transform levels */
int lum;                /* Bit budget for Y channel */
int cb;                 /* Bit budget for Cb channel */
int cr;                 /* Bit budget for Cr channel */
int size;               /* Desired encoded image size */
int filter;             /* Use Butterworth or Daubechies filter */

void usage()
{
  printf(
"TiCodec - lossy image compressor based on TiLib ver 0.9.1\n"
"Copyleft (C) 2004 Alexander Simakov, xander@entropyware.info\n"
"http://www.entropyware.info\n"
"Usage: ticodec [options]\n"
"-e, --encode: Encode image\n"
"-d, --decode: Decode image\n"
"-i, --input <filename>: Input file name\n"
"-o, --output <filename>: Output file name\n"
"-s, --size <num>: Desired encoded file size in bytes\n"
"-B, --butterworth: Use Butterworth wavelet transform\n"
"-D, --daubechies: Use Daubechies 9/7 wavelet transform (default)\n"
"-l, --levels <num>: Number of DWT transform levels (default = 5)\n"
"-y <num>: Bit budget (in %%) for Y channel (default = 90)\n"
"-b <num>: Bit budget (in %%) for Cb channel (default = 5)\n"
"-r <num>: Bit budget (in %%) for Cr channel (default = 5)\n"
"-h, --help: Show this help message\n"
"Note: Y%% + Cb%% + Cr%% must equals to 100%%\n"
"Examples:\n"
"ticodec -e -i foobar.ppm -o foobar.Ti -s 7777\n"
"ticodec -d -i somefile.Ti -o somefile.pgm\n"
"ticodec -e -i test.ppm -o test.Ti -s 10000 -B -l 9 -y 70 -b 20 -r 10\n");
  exit(1);
}

void validate_args(int argc, char **argv)
{
  int ed_flg, i_flg, o_flg, l_flg, s_flg, y_flg, b_flg, r_flg, BD_flg;
  struct option opts[] =
  {
    {"encode",      no_argument,       0, OPT_ENCODE},
    {"decode",      no_argument,       0, OPT_DECODE},
	{"input",       required_argument, 0, OPT_INPUT},
	{"output",      required_argument, 0, OPT_OUTPUT},
	{"size",        required_argument, 0, OPT_SIZE},
	{"butterworth", no_argument,       0, OPT_BUTTERWORTH},
	{"daubechies",  no_argument,       0, OPT_DAUBECHIES},
	{"levels",      required_argument, 0, OPT_LEVELS},
	{"help",        no_argument,       0, OPT_HELP},
    {0,             0,                 0, 0}
  };
  int opt;

  ed_flg = i_flg = o_flg = l_flg = s_flg = y_flg = b_flg = r_flg = BD_flg = 0;

  opterr = 0;

  while ((opt = getopt_long(argc, argv, "edi:o:s:BDl:y:b:r:", opts, NULL)) != -1)
  {
    switch (opt)
	{
      case 'e':
	  case OPT_ENCODE:
	  {
		if (ed_flg) usage();
		ed_flg = 1; encode = 1;
		break;
	  }
	  
	  case 'd':
	  case OPT_DECODE:
	  {
		if (ed_flg) usage();
		ed_flg = 1; encode = 0;
		break;
	  }
	  
	  case 'i':
	  case OPT_INPUT:
	  {
        if (i_flg) usage();
        i_flg = 1;
		snprintf(infile, sizeof(infile), "%s", optarg);  
		break;
	  }

	  case 'o':
	  case OPT_OUTPUT:
	  {
        if (o_flg) usage();
        o_flg = 1;
		snprintf(outfile, sizeof(outfile), "%s", optarg);  
		break;
	  }

      case 's':
	  case OPT_SIZE:
	  {
		if (s_flg) usage();
		s_flg = 1;
		size = atoi(optarg);
		break;
	  }

      case 'l':
	  case OPT_LEVELS:
	  {
		if (l_flg) usage();
		l_flg = 1;
		levels = atoi(optarg);
		break;
	  }

	  case 'B':
	  case OPT_BUTTERWORTH:
	  {
	    if (BD_flg) usage();
		BD_flg = 1;
		filter = BUTTERWORTH;
		break;
	  }	  

	  case 'D':
	  case OPT_DAUBECHIES:
	  {
	    if (BD_flg) usage();
		BD_flg = 1;
		filter = DAUB97;
		break;
	  }	  
	  
      case 'y':
	  {
		if (y_flg) usage();
		y_flg = 1;
		lum = atoi(optarg);
		break;
	  }

      case 'b':
	  {
		if (b_flg) usage();
		b_flg = 1;
		cb = atoi(optarg);
		break;
	  }
	  
      case 'r':
	  {
		if (r_flg) usage();
		r_flg = 1;
		cr = atoi(optarg);
		break;
	  }
	  
	  case ':':
	  case '?':
	  case 'h':
	  case OPT_HELP:
	  default:
	  {
	    usage();
		break;
	  }
	  }
  }

  if (optind < argc) usage();

  /* check options */

  if (ed_flg == 0 || i_flg == 0 || o_flg == 0) usage();

  if (encode == 1) {
    if (s_flg == 0) usage();
    if (size < 26) usage(); /* HDRSIZE + 2 + 2 + 2 */
    if ((y_flg + b_flg + r_flg) != 0 && (y_flg + b_flg + r_flg) != 3) usage();
    if (l_flg == 0) levels = 0;
    if (BD_flg == 0) filter = DAUB97;
    if (y_flg + b_flg + r_flg == 0) lum = cb = cr = 0;
    if ((y_flg + b_flg + r_flg == 3) && (lum <= 0 || cb <= 0 || cr <= 0)) usage();
    if ((y_flg + b_flg + r_flg == 3) && (lum + cb + cr != 100)) usage();
  } else {
    if (l_flg + s_flg + y_flg + b_flg + r_flg + BD_flg != 0) usage();
  }
}

void CompressFile()
{
  FILE *in_file, *out_file;
  unsigned char *in_buf, *out_buf;
  int width, height, actual_size;
  int result;
  pbm_hdr h;

  in_buf = out_buf = NULL;

  /* open files */

  in_file = fopen(infile, "rb");
  out_file = fopen(outfile, "wb");

  if (in_file == NULL || out_file == NULL) {
    perror("fopen() failed");
    exit(1);
  }

  /* read image header */

  read_hdr(in_file, &h);

  width = h.width;
  height = h.height;

  /* allocate memory */

  in_buf = (unsigned char *) malloc(width * height * (h.type == PGM ? 1 : 3));
  out_buf = (unsigned char *) malloc(size);

  if (in_buf == NULL || out_buf == NULL) {
    perror("malloc() failed");
    exit(1);
  }

  /* read whole image into the buffer */

  fread(in_buf, 1, width * height * (h.type == PGM ? 1 : 3), in_file);

  /* compress it (with only one function call!) */

  result = TiCompress(in_buf, out_buf, width, height, filter, (h.type == PGM ? GRAYSCALE : TRUECOLOR),
  size, &actual_size, lum, cb, cr, levels);

  /* if something wrong ... */

  if (result != OK) {
    printf("TiCompress() failed: %d\n", result);
    exit(1);
  }

  /* store encoded stream to file */

  fwrite(out_buf, 1, actual_size, out_file);
}

void DecompressFile()
{
  FILE *in_file, *out_file;
  unsigned char *in_buf, *out_buf;
  int width, height, type, result, stream_size;

  in_buf = out_buf = NULL;

  /* open files */

  in_file = fopen(infile, "rb");
  out_file = fopen(outfile, "wb");

  if (in_file == NULL || out_file == NULL) {
    perror("fopen() failed");
    exit(1);
  }

  stream_size = file_size(in_file);

  /* allocate memory for a whole file */

  in_buf = (unsigned char *) malloc(stream_size);

  if (in_buf == NULL) {
    perror("malloc() failed");
    exit(1);
  }

  /* read it */

  fread(in_buf, 1, stream_size, in_file);

  /* check .Ti image header */

  result = TiCheckHeader(in_buf, &width, &height, &type, stream_size);

  /* ... damaged :( */

  if (result != OK) {
    printf("TiCheckHeader() failed: %d\n", result);
    exit(1);
  }

  /* allocate memory for a decoded image */

  out_buf = (unsigned char *) malloc(width * height * (type == GRAYSCALE ? 1 : 3));

  /* decode it! */

  result = TiDecompress(in_buf, out_buf, width, height, type, stream_size);

  /* if something wrong ... */

  if (result != OK) {
    printf("TiDecompress() failed: %d\n", result);
    exit(1);
  }

  /* write PGM of PPM header */

  fprintf(out_file, "P%s\n%d %d\n255\n", type == GRAYSCALE ? "5" : "6", width, height);

  /* store decoded image (PGM or PPM) to file */

  fwrite(out_buf, 1, width * height * (type == GRAYSCALE ? 1 : 3), out_file);
}

int main(int argc, char *argv[])
{
  validate_args(argc, argv);

  if (encode) CompressFile();
  else DecompressFile();

  return 0;
}
