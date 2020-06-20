#ifndef COMPRESS_H
#define COMPRESS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>

#include "struct.h"

int set_value(char *filename, struct node *node[256]);
struct node *build_tree(struct node *node[256], int num_nodes);
void search_tree(struct node *n, struct c_byte c_byte[256], int depth);
FILE *write_dictionary(char *dir, struct c_byte c_byte[256]);
void destroy_tree(struct node *n);
void write_compressed(char *file_in, FILE *file_out, struct c_byte c_byte[256]);
struct node *node_init(int byte);
int two_smallest(struct node *node[256], struct node *smallest[2]);
void print_bits(struct c_byte *c_byte);

#endif /* COMPRESS_H */