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
 * Implementation of SPIHT algorithm.
 *
 * References:
 *
 * Amir Said, William A. Pearlman, "A New Fast and Efficient Image
 * Codec Based on Set Partitioning in Hierarchical Trees", IEEE Trans.
 * on Circuits and Systems for Video Technology, Vol. 6, June 1996.
 *
 * Shapiro, J. M. "Embedded image coding using zerotrees of wavelet
 * coefficients", IEEE Transactions on Signal Processing,
 * Vol. 41, No. 12 (1993), p. 3445-3462.
 *
 * C. Valens, "Embedded Zerotree Wavelet Encoding", 1999
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#include <stdlib.h>
#include "../include/spiht.h"
#include "../include/ari.h"
#include "../include/nodelist.h"
#include "../include/bitio.h"
#include "../include/errcodes.h"

#define ABS(value) (value >= 0 ? value : - value)

#define TYPE_S (0)
#define TYPE_A (1)
#define TYPE_B (2)

static int InitialThreshold(double **dwt,
                            int rows,
                            int cols);

static void ResetDWT(double **dwt,
                     int rows,
                     int cols);


static int IsZerotree(double **dwt,
                      int rows,
                      int cols,
                      int levels,
                      int threshold,
                      Node *node);

static int IsValidNodeA(int rows,
                        int cols,
                        int levels,
                        Node *node);

static int IsValidNodeB(int rows,
                        int cols,
                        int levels,
                        Node *node);

static void ChangeNodeType(Node *node);

static int GetNodeOffspring(int rows,
                            int cols,
                            int levels,
                            Node *node,
                            Node *offspring);

static int IsNodeSignificant(double **dwt,
                             int rows,
                             int cols,
                             int levels,
                             int threshold,
                             int node_type,
                             Node *node);

static void InitCoefficient(double **dwt,
                            int threshold,
                            int sign,
                            Node *node);

static int SPIHTInit(int rows,
                     int cols,
                     int levels,
                     NodeList *LIP,
                     NodeList *LIS);

static int SPIHTEncodeSignificancePass(double **dwt,
                                       int rows,
                                       int cols,
                                       int levels,
                                       int threshold,
                                       NodeList *LIP,
                                       NodeList *LSP,
                                       NodeList *LIS,
                                       BitStream *bit_stream,
                                       ArithCoder *arith_coder);

static int SPIHTEncodeRefinementPass(double **dwt,
                                     int threshold,
                                     NodeList *LSP,
                                     BitStream *bit_stream,
                                     ArithCoder *arith_coder);

static int SPIHTDecodeSignificancePass(double **dwt,
                                       int rows,
                                       int cols,
                                       int levels,
                                       int threshold,
                                       NodeList *LIP,
                                       NodeList *LSP,
                                       NodeList *LIS,
                                       BitStream *bit_stream,
                                       ArithCoder *arith_coder);

static int SPIHTDecodeRefinementPass(double **dwt,
                                     int threshold,
                                     NodeList *LSP,
                                     BitStream *bit_stream,
                                     ArithCoder *arith_coder);

static int InitialThreshold(double **dwt,
                            int rows,
                            int cols)
{
  int temp, bits;
  int row, col;

  temp = bits = 0;

  for (row = 0; row < rows; row++)
    for (col = 0; col < cols; col++)
      if (ABS(dwt[row][col]) > temp) temp = (int) ABS(dwt[row][col]);

  if (temp == 0) return 0;

  while (temp != 0) {
    temp >>= 1;
    bits++;
  }

  return (1 << (bits - 1));
}

static void ResetDWT(double **dwt,
                     int rows,
                     int cols)
{
  int row, col;

  for (row = 0; row < rows; row++)
    for (col = 0; col < cols; col++)
      dwt[row][col] = 0;
}

static int IsZerotree(double **dwt,
                      int rows,
                      int cols,
                      int levels,
                      int threshold,
                      Node *node)
{
  int min_row, max_row, min_col, max_col;
  int row, col;

  row = ABS(node->row);
  col = ABS(node->col);

  if (row < rows >> levels && col < cols >> levels) return INTERNAL_ERROR;

  if (row >= rows >> 1 || col >= cols >> 1) return INTERNAL_ERROR;

  min_row = row << 1;
  max_row = (row + 1) << 1;
  min_col = col << 1;
  max_col = (col + 1) << 1;

  while (max_row <= rows && max_col <= cols) {
    for (row = min_row; row < max_row; row++)
      for (col = min_col; col < max_col; col++)
        if (ABS(dwt[row][col]) >= threshold) return FALSE;

    min_row <<= 1;
    max_row <<= 1;
    min_col <<= 1;
    max_col <<= 1;
  }

  return TRUE;
}

static int IsValidNodeA(int rows,
                        int cols,
                        int levels,
                        Node *node)
{
  int row, col;

  row = ABS(node->row);
  col = ABS(node->col);

  if (row < rows >> levels && col < cols >> levels) return FALSE;

  if (row >= rows >> 1 || col >= cols >> 1) return FALSE;

  return TRUE;
}

static int IsValidNodeB(int rows,
                        int cols,
                        int levels,
                        Node *node)
{
  int row, col;

  row = ABS(node->row);
  col = ABS(node->col);

  if (row < rows >> levels && col < cols >> levels) return FALSE;

  if (row >= rows >> 2 || col >= cols >> 2) return FALSE;

  return TRUE;
}

static void ChangeNodeType(Node *node)
{
  node->row = - node->row;
  node->col = - node->col;
}

static int GetNodeOffspring(int rows,
                            int cols,
                            int levels,
                            Node *node,
                            Node *offspring)
{
  int row, col;

  row = ABS(node->row);
  col = ABS(node->col);

  if (IsValidNodeA(rows, cols, levels, node) == FALSE) return INTERNAL_ERROR;

  offspring[0].row = row << 1;
  offspring[0].col = col << 1;
  offspring[0].next = NULL;
  offspring[0].prev = NULL;

  offspring[1].row = row << 1;
  offspring[1].col = (col << 1) + 1;
  offspring[1].next = NULL;
  offspring[1].prev = NULL;

  offspring[2].row = (row << 1) + 1;
  offspring[2].col = col << 1;
  offspring[2].next = NULL;
  offspring[2].prev = NULL;

  offspring[3].row = (row << 1) + 1;
  offspring[3].col = (col << 1) + 1;
  offspring[3].next = NULL;
  offspring[3].prev = NULL;

  return OK;
}

static int IsNodeSignificant(double **dwt,
                             int rows,
                             int cols,
                             int levels,
                             int threshold,
                             int node_type,
                             Node *node)
{
  Node offspring[4];
  int row, col;
  int result;
  int index;

  row = ABS(node->row);
  col = ABS(node->col);

  if (node_type == TYPE_S) {

    if (ABS(dwt[row][col]) >= threshold) return TRUE;
    else return FALSE;
  }
  
  if (node_type == TYPE_A) {

    result = IsZerotree(dwt, rows, cols, levels, threshold, node);

    switch (result) {

      case TRUE: return FALSE;

      case FALSE: return TRUE;

      default: return result;
    };
  }

  if (node_type == TYPE_B) {

    result = GetNodeOffspring(rows, cols, levels, node, offspring);

    if (result != OK) return result;

    for (index = 0; index < 4; index++) {

      result = IsZerotree(dwt, rows, cols, levels, threshold, &offspring[index]);

      switch (result) {

        case TRUE: break;

        case FALSE: return TRUE;

        default: return result;
      };
    }

    return FALSE;
  }

  return INTERNAL_ERROR;
}

static void InitCoefficient(double **dwt,
                            int threshold,
                            int sign,
                            Node *node)
{
  dwt[node->row][node->col] = (sign ?  - threshold - (threshold >> 1) : threshold + (threshold >> 1));
}

static int SPIHTInit(int rows,
                     int cols,
                     int levels,
                     NodeList *LIP,
                     NodeList *LIS)
{
  int max_row, max_col;
  int row, col;
  int result;

  max_row = rows >> (levels - 1);
  max_col = cols >> (levels - 1);

  for (row = 0; row < max_row; row++) {
    for (col = 0; col < max_col; col++) {

      if ((result = AppendNode(LIP, row, col)) != OK) return result;

      if (IsValidNodeA(rows, cols, levels, LIP->end) == TRUE)
      if ((result = AppendNode(LIS, row, col)) != OK) return result;
    }
  }

  return OK;
}

static int SPIHTEncodeSignificancePass(double **dwt,
                                       int rows,
                                       int cols,
                                       int levels,
                                       int threshold,
                                       NodeList *LIP,
                                       NodeList *LSP,
                                       NodeList *LIS,
                                       BitStream *bit_stream,
                                       ArithCoder *arith_coder)
{
  int result1, result2, result3, result4, index;
  Node offspring[4];
  Node *node, *next;

  node = LIP->start;

  while (node != NULL) {

    next = node->next;

    result1 = IsNodeSignificant(dwt, rows, cols, levels, threshold, TYPE_S, node);

    if (result1 == TRUE) {

      if ((result2 = EncodeSymbol(arith_coder, bit_stream, 1)) != OK) return result2;
      UpdateModel(arith_coder, 1);

      if ((result2 = EncodeSymbol(arith_coder, bit_stream, (dwt[node->row][node->col] > 0 ? 0 : 1))) != OK) return result2;
      UpdateModel(arith_coder, (dwt[node->row][node->col] > 0 ? 0 : 1));

      if ((result2 = MoveNode(LIP, LSP, node)) != OK) return result2;

    } else if (result1 == FALSE) {

      if ((result2 = EncodeSymbol(arith_coder, bit_stream, 0)) != OK) return result2;
      UpdateModel(arith_coder, 0);

    } else return result1;

    node = next;
  }

  node = LIS->start;

  while (node != NULL) {

    next = node->next;

    if (node->row > 0 || node->col > 0) {

      result1 = IsNodeSignificant(dwt, rows, cols, levels, threshold, TYPE_A, node);

      if (result1 == TRUE) {

        if ((result2 = EncodeSymbol(arith_coder, bit_stream, 1)) != OK) return result2;
        UpdateModel(arith_coder, 1);

        if ((result2 = GetNodeOffspring(rows, cols, levels, node, offspring)) != OK) return result2;

        for (index = 0; index < 4; index++) {

          result3 = IsNodeSignificant(dwt, rows, cols, levels, threshold, TYPE_S, &offspring[index]);

          if (result3 == TRUE) {

            if ((result4 = EncodeSymbol(arith_coder, bit_stream, 1)) != OK) return result4;
            UpdateModel(arith_coder, 1);

            if ((result4 = EncodeSymbol(arith_coder, bit_stream, (dwt[offspring[index].row][offspring[index].col]  > 0 ? 0 : 1))) != OK) return result4;
            UpdateModel(arith_coder, (dwt[offspring[index].row][offspring[index].col]  > 0 ? 0 : 1));

            if ((result4 = AppendNode(LSP, offspring[index].row, offspring[index].col)) != OK) return result4;

          } else if (result3 == FALSE) {

            if ((result4 = EncodeSymbol(arith_coder, bit_stream, 0)) != OK) return result4;
            UpdateModel(arith_coder, 0);

            if ((result4 = AppendNode(LIP, offspring[index].row, offspring[index].col)) != OK) return result4;

          } else return result3;

        }

        if (IsValidNodeB(rows, cols, levels, node) == TRUE) {

          ChangeNodeType(node);

          if ((result3 = MoveNode(LIS, LIS, node)) != OK) return result3;

        } else {

          if ((result3 = RemoveNode(LIS, node)) != OK) return result3;
        }

      } else if (result1 == FALSE) {

        if ((result2 = EncodeSymbol(arith_coder, bit_stream, 0)) != OK) return result2;
        UpdateModel(arith_coder, 0);

      } else return result1;

    } else {

      result1 = IsNodeSignificant(dwt, rows, cols, levels, threshold, TYPE_B, node);

      if (result1 == TRUE) {

        if ((result2 = EncodeSymbol(arith_coder, bit_stream, 1)) != OK) return result2;
        UpdateModel(arith_coder, 1);

        if ((result2 = GetNodeOffspring(rows, cols, levels, node, offspring)) != OK) return result2;

        for (index = 0; index < 4; index++) {

          if ((result3 = AppendNode(LIS, offspring[index].row, offspring[index].col)) != OK) return result3;

        }

        next = node->next;

        if ((result3 = RemoveNode(LIS, node)) != OK) return result3;

      } else if (result1 == FALSE) {

        if ((result2 = EncodeSymbol(arith_coder, bit_stream, 0)) != OK) return result2;
        UpdateModel(arith_coder, 0);

      } else return result1;

    }

    node = next;
  }

  return OK;
}

static int SPIHTEncodeRefinementPass(double **dwt,
                                     int threshold,
                                     NodeList *LSP,
                                     BitStream *bit_stream,
                                     ArithCoder *arith_coder)
{
  Node *node;
  int result;

  node = LSP->start;

  if (threshold > 0) {

    while (node != NULL) {

      if ((result = EncodeSymbol(arith_coder, bit_stream, (((int) ABS(dwt[node->row][node->col])) & threshold ? 1 : 0))) != OK) return result;
      UpdateModel(arith_coder, ((((int) ABS(dwt[node->row][node->col])) & threshold) ? 1 : 0));
      node = node->next;
    }
  }

  return OK;
}

static int SPIHTDecodeSignificancePass(double **dwt,
                                       int rows,
                                       int cols,
                                       int levels,
                                       int threshold,
                                       NodeList *LIP,
                                       NodeList *LSP,
                                       NodeList *LIS,
                                       BitStream *bit_stream,
                                       ArithCoder *arith_coder)
{
  int result1, result2, result3, result4, index;
  Node offspring[4];
  Node *node, *next;
  int bit;

  node = LIP->start;

  while (node != NULL) {

    next = node->next;

    if ((result1 = DecodeSymbol(arith_coder, bit_stream, &bit)) != OK) return result1;
    UpdateModel(arith_coder, bit);

    if (bit == 1) {

      if ((result2 = DecodeSymbol(arith_coder, bit_stream, &bit)) != OK) return result2;
      UpdateModel(arith_coder, bit);

      InitCoefficient(dwt, threshold, bit, node);

      if ((result2 = MoveNode(LIP, LSP, node)) != OK) return result2;
    }

    node = next;
  }

  node = LIS->start;

  while (node != NULL) {

    next = node->next;

    if (node->row > 0 || node->col > 0) {

      if ((result1 = DecodeSymbol(arith_coder, bit_stream, &bit)) != OK) return result1;
      UpdateModel(arith_coder, bit);

      if (bit == 1) {

        if ((result2 = GetNodeOffspring(rows, cols, levels, node, offspring)) != OK) return result2;

        for (index = 0; index < 4; index++) {

          if ((result3 = DecodeSymbol(arith_coder, bit_stream, &bit)) != OK) return result3;
          UpdateModel(arith_coder, bit);

          if (bit == 1) {

            if ((result4 = DecodeSymbol(arith_coder, bit_stream, &bit)) != OK) return result4;
            UpdateModel(arith_coder, bit);

            InitCoefficient(dwt, threshold, bit, &offspring[index]);

            if ((result4 = AppendNode(LSP, offspring[index].row, offspring[index].col)) != OK) return result4;

          } else {

            if ((result4 = AppendNode(LIP, offspring[index].row, offspring[index].col)) != OK) return result4;

          }

        }

        if (IsValidNodeB(rows, cols, levels, node) == TRUE) {

          ChangeNodeType(node);

          if ((result3 = MoveNode(LIS, LIS, node)) != OK) return result3;

        } else {

          if ((result3 = RemoveNode(LIS, node)) != OK) return result3;

        }

      }

    } else {

      if ((result1 = DecodeSymbol(arith_coder, bit_stream, &bit)) != OK) return result1;
      UpdateModel(arith_coder, bit);

      if (bit == 1) {

        if ((result2 = GetNodeOffspring(rows, cols, levels, node, offspring)) != OK) return result2;

        for (index = 0; index < 4; index++) {

          if ((result3 = AppendNode(LIS, offspring[index].row, offspring[index].col)) != OK) return result3;

        }

        next = node->next;

        if ((result3 = RemoveNode(LIS, node)) != OK) return result3;

      }

    }

    node = next;
  }

  return OK;
}

static int SPIHTDecodeRefinementPass(double **dwt,
                                     int threshold,
                                     NodeList *LSP,
                                     BitStream *bit_stream,
                                     ArithCoder *arith_coder)
{
  int result, coeff, bit;
  Node *node;

  node = LSP->start;

  if (threshold > 0) {

    while (node != NULL) {

      coeff = (int) dwt[node->row][node->col];

      if ((result = DecodeSymbol(arith_coder, bit_stream, &bit)) != OK) return result;
      UpdateModel(arith_coder, bit);

      if (coeff > 0) coeff -= threshold;
      else coeff += threshold;

      if (bit == 1) {
        if (coeff > 0) coeff += threshold;
        else coeff -= threshold;
      }

      if (coeff > 0) coeff += (threshold >> 1);
      else coeff -= (threshold >> 1);

      dwt[node->row][node->col] = coeff;

      node = node->next;
    }
  }

  return OK;
}

int SPIHTEncodeDWT(double *dwt_data,
                   int rows,
                   int cols,
                   int levels,
                   unsigned char *buffer,
                   int buffer_size,
                   int *stream_size)
{
  NodeList *LIP, *LSP, *LIS;
  BitStream *bit_stream;
  ArithCoder *arith_coder;
  int bits, temp, index;
  int result, threshold;
  double **dwt;
  int *cum_freq;

  LIP = LSP = LIS = NULL;
  bit_stream = NULL;
  arith_coder = NULL;
  dwt = NULL;
  cum_freq = NULL;

  *stream_size = 0;

  if (buffer_size < 2) {
    result = INTERNAL_ERROR;
    goto error;
  }

  LIP = AllocNodeList();
  LSP = AllocNodeList();
  LIS = AllocNodeList();

  bit_stream = AllocBitStream();
  arith_coder = AllocArithCoder();

  dwt = (double **) malloc(rows * sizeof(double *));
  cum_freq = (int *) malloc((ALPHA_SIZE + 1) * sizeof(int));

  if (LIP == NULL || LSP == NULL || LIS == NULL ||
      bit_stream == NULL || arith_coder == NULL ||
      dwt == NULL || cum_freq == NULL) {

    result = MEMORY_ERROR;
    goto error;
  }

  bit_stream->buffer = buffer + 1;
  bit_stream->buffer_size = buffer_size - 1;

  InitWriteBits(bit_stream);

  arith_coder->cum_freq = cum_freq;

  InitModel(arith_coder);
  InitEncoder(arith_coder);

  for (index = 0; index < rows; index++) dwt[index] = dwt_data + index * cols;

  threshold = InitialThreshold(dwt, rows, cols);

  temp = threshold;
  bits = 0;

  while (temp != 0) {
    temp >>= 1;
    bits++;
  }

  buffer[0] = (unsigned char) bits;

  result = SPIHTInit(rows, cols, levels, LIP, LIS);

  if (result != OK) goto error;

  while (threshold > 0) {

    result = SPIHTEncodeSignificancePass(dwt, rows, cols, levels, threshold, LIP, LSP, LIS, bit_stream, arith_coder);

    if (result != OK) goto error;

    result = SPIHTEncodeRefinementPass(dwt, threshold >> 1, LSP, bit_stream, arith_coder);

    if (result != OK) goto error;

    threshold >>= 1;
  }

  error:

  if (result == BUFFER_FULL || result == OK) {

    DoneEncoder(arith_coder, bit_stream);
    FlushBits(bit_stream);

    *stream_size = bit_stream->next_byte - bit_stream->buffer + 1;
  }

  free(cum_freq);
  free(dwt);

  FreeArithCoder(arith_coder);
  FreeBitStream(bit_stream);

  FreeNodeList(LIP);
  FreeNodeList(LSP);
  FreeNodeList(LIS);

  if (result == BUFFER_FULL) result = OK;

  return result;
}

int SPIHTDecodeDWT(double *dwt_data,
                   int rows,
                   int cols,
                   int levels,
                   unsigned char *buffer,
                   int buffer_size)
{
  NodeList *LIP, *LSP, *LIS;
  BitStream *bit_stream;
  ArithCoder *arith_coder;
  int bits, index, result, threshold;
  double **dwt;
  int *cum_freq;

  LIP = LSP = LIS = NULL;
  bit_stream = NULL;
  arith_coder = NULL;
  dwt = NULL;
  cum_freq = NULL;

  if (buffer_size < 2) {
    result = INTERNAL_ERROR;
    goto error;
  }

  LIP = AllocNodeList();
  LSP = AllocNodeList();
  LIS = AllocNodeList();

  bit_stream = AllocBitStream();
  arith_coder = AllocArithCoder();

  dwt = (double **) malloc(rows * sizeof(double *));
  cum_freq = (int *) malloc((ALPHA_SIZE + 1) * sizeof(int));

  if (LIP == NULL || LSP == NULL || LIS == NULL ||
      bit_stream == NULL || arith_coder == NULL ||
      dwt == NULL || cum_freq == NULL) {

    result = MEMORY_ERROR;
    goto error;
  }

  for (index = 0; index < rows; index++) dwt[index] = dwt_data + index * cols;

  ResetDWT(dwt, rows, cols);

  bit_stream->buffer = buffer + 1;
  bit_stream->buffer_size = buffer_size - 1;

  InitReadBits(bit_stream);

  arith_coder->cum_freq = cum_freq;

  InitModel(arith_coder);
  result = InitDecoder(arith_coder, bit_stream);

  if (result != OK) goto error;

  bits = buffer[0];

  if (bits > 0) threshold = 1 << (bits - 1);
  else threshold = 0;

  result = SPIHTInit(rows, cols, levels, LIP, LIS);

  if (result != OK) goto error;

  while (threshold > 0) {

    result = SPIHTDecodeSignificancePass(dwt, rows, cols, levels, threshold, LIP, LSP, LIS, bit_stream, arith_coder);

    if (result != OK) goto error;

    result = SPIHTDecodeRefinementPass(dwt, threshold >> 1, LSP, bit_stream, arith_coder);

    if (result != OK) goto error;

    threshold >>= 1;
  }

  error:

  free(cum_freq);
  free(dwt);

  FreeArithCoder(arith_coder);
  FreeBitStream(bit_stream);

  FreeNodeList(LIP);
  FreeNodeList(LSP);
  FreeNodeList(LIS);

  if (result == BUFFER_EMPTY) result = OK;

  return result;
}
