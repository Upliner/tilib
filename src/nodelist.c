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

#include <stdlib.h>
#include "../include/nodelist.h"
#include "../include/errcodes.h"

NodeList *AllocNodeList()
{
  NodeList *list;

  list = (NodeList *) malloc(sizeof(NodeList));

  if (list == NULL) return NULL;

  list->start = NULL;
  list->end = NULL;

  return list;
}

void FreeNodeList(NodeList *list)
{
  Node *next;
  Node *current;

  if (list == NULL) return;

  current = list->start;

  while (current != NULL) {
    next = current->next; 
    free(current);
    current = next;
  }

  free(list);
}

Node *AllocNode()
{
  Node *node;

  node = (Node *) malloc(sizeof(Node));

  if (node == NULL) return NULL;

  node->row = 0;
  node->col = 0;
  node->prev = NULL;
  node->next = NULL;

  return node;
}

void FreeNode(Node *node)
{
  free(node);
}

int AppendNode(NodeList *list, short row, short col)
{
  Node *node;

  node = AllocNode();

  if (node == NULL) return MEMORY_ERROR;

  if (list == NULL) return INTERNAL_ERROR;

  node->row = row;
  node->col = col;
  node->next = NULL;
  node->prev = list->end;

  if (list->end != NULL) {
    list->end->next = node;
    list->end = node;
  } else {
    list->start = node;
    list->end = node;
  }

  return OK;
}

int RemoveNode(NodeList *list, Node *node)
{
  if (list == NULL || node == NULL) return INTERNAL_ERROR;

  if (list->start == NULL || list->end == NULL) return INTERNAL_ERROR;

  if (node->prev != NULL) node->prev->next = node->next;
  else list->start = node->next;

  if (node->next != NULL) node->next->prev = node->prev;
  else list->end = node->prev;

  free(node);

  return OK;
}

int MoveNode(NodeList *list1, NodeList *list2, Node *node)
{
  int result;

  if (list1 == NULL || list2 == NULL || node == NULL) return INTERNAL_ERROR;

  result = AppendNode(list2, node->row, node->col);

  if (result != OK) return result;

  result = RemoveNode(list1, node);

  if (result != OK) return result;

  return OK;
}
