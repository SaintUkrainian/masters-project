#include <cstdint>

#define CHECK(expected, hash_nbytes) \
    if (memcmp(hash_code, (expected), (hash_nbytes)) == 0) { \
        printf("Success hashing"); \
    } else { \
        printf("Failed hashing!"); \
    }printf ("\n\n");\

void test();

void print(int data_len, uint8_t data[]);
