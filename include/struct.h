#ifndef STRUCT_H
#define STRUCT_H

#include <stdint.h>

struct node
{
        int16_t byte;
        uint64_t value;

        uint8_t bits[32];

        struct node *zero;
        struct node *one;
};

struct c_byte
{
        uint8_t *bits;
        uint8_t num_bits;
};

#endif /* STRUCT_H */