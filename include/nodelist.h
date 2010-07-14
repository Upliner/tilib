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
 * An implementation of double linked list data structure for SPIHT.
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#ifndef NODELIST_H
#define NODELIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Node
{
  short row;
  short col;
  struct _Node *prev;
  struct _Node *next;
} Node;

typedef struct
{
  Node *start;
  Node *end;
} NodeList;

NodeList *AllocNodeList();
void FreeNodeList(NodeList *list);
Node *AllocNode();
void FreeNode(Node *node);
int AppendNode(NodeList *list, short row, short col);
int RemoveNode(NodeList *list, Node *node);
int MoveNode(NodeList *list1, NodeList *list2, Node *node);

#ifdef __cplusplus
}
#endif

#endif /* NODELIST_H */
