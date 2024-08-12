#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct node {
   long data;
   struct node *next;
} node_t;

typedef struct linked_list {
   node_t *head;
} linked_list_t;

void
linked_list_init(linked_list_t *l)
{
   l->head = NULL;
}

int
linked_list_insert_head(linked_list_t *l, long data)
{
   node_t *n = calloc(1, sizeof(*n));
   if (n == NULL) {
      fprintf(stderr,
              "fatal: %s: allocation of %lu bytes failed\n",
              __func__, sizeof(*n));
      return (ENOMEM);
   }

   n->data = data;
   n->next = l->head;
   l->head = n;

   return (0);
}

void
linked_list_destroy(linked_list_t *l)
{
   for (node_t *p = l->head, *next = NULL; p != NULL; p = next) {
      next = p->next;
      free(p);
   }
   l->head = NULL;
}

void
linked_list_print(const linked_list_t *l)
{
   for (node_t *p = l->head; p != NULL; p = p->next) {
      if (p->next != NULL) {
         printf("%ld ", p->data);
      } else {
         printf("%ld\n", p->data);
      }
   }
}

void
linked_list_nodes_reverse(node_t *n)
{
   /* nothing to reverse */
   if (n == NULL || n->next == NULL)
      return;

   node_t *prev = NULL, *curr = n, *next = NULL;
   for (; curr != NULL; prev = curr, curr = next) {
      next = curr->next;
      curr->next = prev;
   }
}

node_t *
linked_list_chunk_last_element(node_t *n, unsigned long chunk_size)
{
   if (n == NULL)
      return NULL;

   unsigned long leftover = chunk_size - 1;
   node_t *p = n;
   while (p->next != NULL && leftover > 0) {
     p = p->next;
     leftover--; 
   }
   return (p);
}

void
linked_list_reverse_in_chunks(linked_list_t *l, long chunk_size)
{

   node_t *prev_chunk_tail = NULL;
   node_t *chunk_head = l->head;
   node_t *chunk_tail = linked_list_chunk_last_element(chunk_head, chunk_size);

   l->head = chunk_tail;

   while (chunk_head != NULL) {
      node_t *next_chunk_head = chunk_tail->next;

      chunk_tail->next = NULL;
      linked_list_nodes_reverse(chunk_head);

      if (prev_chunk_tail != NULL) {
         prev_chunk_tail->next = chunk_tail;
      }
      prev_chunk_tail = chunk_head;

      chunk_head = next_chunk_head;
      chunk_tail = linked_list_chunk_last_element(chunk_head, chunk_size);
   }
}

// XXX: restrict keyword
int
str_to_num(const char *s, unsigned long *n)
{
   errno = 0;
   char *strtol_end;
   unsigned long num = strtoul(s, &strtol_end, 0);
   if (s == strtol_end) {
      fprintf(stderr, "error: \"%s\" is not a number\n", s);
      return (-1);
   } else if (num == 0 && errno != 0) {
      fprintf(stderr,
              "error: strtol(\"%s\") returned error %d - can't parse string\n",
              s, errno);
      return (-1);
   }
   *n = num;
   return (0);
}

int
main(int c, char *v[])
{
   if (c < 2) {
      printf("usage: %s <chunk size> [<elements> ...]\n", v[0]);
      return (2);
   }

   unsigned long chunk_size = 0;
   int ret = str_to_num(v[1], &chunk_size);
   if (ret != 0) {
      return (2);
   }

   linked_list_t l;
   linked_list_init(&l);
   for (int i = 2; i < c; i++) {
      unsigned long data;
      ret = str_to_num(v[i], &data);
      if (ret != 0) {
         goto error;
      }
      linked_list_insert_head(&l, data);
   }

   printf("original list:\n\t");
   linked_list_print(&l);

   linked_list_reverse_in_chunks(&l, chunk_size);

   printf("%ld-chunk traversal:\n\t", chunk_size);
   linked_list_print(&l);

   linked_list_destroy(&l);
   return (0);
error:
   linked_list_destroy(&l);
   return (-1);
}
