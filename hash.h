/********************************************************************************
 * hash.c: Provides different hash function
 * Code samples & inspiration taken from:
 *  - https://github.com/nothings/stb/blob/master/stb_ds.h by Sean Barrett
 *  - https://web.archive.org/web/20071102041545/http://www.concentric.net/~Ttwang/tech/inthash.htm
        by Thomas Wang
 *  - http://www.cse.yorku.ca/~oz/hash.html
 *  - https://burtleburtle.net/bob/hash/integer.html
 * -----------------------------------------------------------------------------
 * TODO: Add more hash functions
 *   - https://en.wikipedia.org/wiki/List_of_hash_functions
 *   - https://en.wikipedia.org/wiki/Comparison_of_cryptographic_hash_functions
 *   - https://en.wikipedia.org/wiki/NIST_hash_function_competition#cite_note-71
********************************************************************************/

#include <stddef.h>
#include <assert.h>

#define HASH_SIZE_T_BITS           ((sizeof (size_t)) * 8)
#define HASH_ROTATE_LEFT(val, n)   (((val) << (n)) | ((val) >> (HASH_SIZE_T_BITS - (n))))
#define HASH_ROTATE_RIGHT(val, n)  (((val) >> (n)) | ((val) << (HASH_SIZE_T_BITS - (n))))

size_t hash_stb6432shift(char *str, size_t seed) {
  size_t hash = seed;
  while (*str)
     hash = HASH_ROTATE_LEFT(hash, 9) + (unsigned char) *str++;

  hash ^= seed;
  hash = (~hash) + (hash << 18);
  hash ^= hash ^ HASH_ROTATE_RIGHT(hash,31);
  hash = hash * 21;
  hash ^= hash ^ HASH_ROTATE_RIGHT(hash,11);
  hash += (hash << 6);
  hash ^= HASH_ROTATE_RIGHT(hash,22);
  return hash+seed;
}

unsigned long hash_djbx33a(const char *s) {
    unsigned long key = 0;
    assert(s);
    while (*s) {
        key = key * 37 + *s++;
    }
    return key & 0x7FFFFFFF;
}

unsigned long hash_djb2(char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

static unsigned long hash_sdbm(str)
unsigned char *str;
{
    unsigned long hash = 0;
    int c;
    while ((c = *str++))
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}

unsigned long hash_2lose(char *str) {
	unsigned int hash = 0;
	int c;

	while ((c = *str++))
	    hash += c;

	return hash;
}

unsigned int hash_adler32(char* s)
{
    unsigned int a = 1;
    unsigned int b = 0;
    const unsigned int MODADLER = 65521;

    size_t i = 0;
    while (s[i] != '\0')
    {
        a = (a + s[i]) % MODADLER;
        b = (b + a) % MODADLER;
        i++;
    }
    return (b << 16) | a;
}

unsigned int hash_crc32(char* s)
{
    unsigned int crc = 0xffffffff;
    size_t i = 0;
    while (s[i] != '\0')
    {
        unsigned char byte = s[i];
        crc = crc ^ byte;
        for (unsigned char j = 8; j > 0; --j)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }

        i++;
    }
    return crc ^ 0xffffffff;
}

unsigned char hash_xor8(char* s)
{
    unsigned char hash = 0;
    size_t i = 0;
    while (s[i] != '\0')
    {
        hash = (hash + s[i]) & 0xff;
        i++;
    }
    return (((hash ^ 0xff) + 1) & 0xff);
}

unsigned int hash_int_mix(unsigned int a, unsigned int b, unsigned int c)
{
  a=a-b;  a=a-c;  a=a^(c >> 13);
  b=b-c;  b=b-a;  b=b^(a << 8);
  c=c-a;  c=c-b;  c=c^(b >> 13);
  a=a-b;  a=a-c;  a=a^(c >> 12);
  b=b-c;  b=b-a;  b=b^(a << 16);
  c=c-a;  c=c-b;  c=c^(b >> 5);
  a=a-b;  a=a-c;  a=a^(c >> 3);
  b=b-c;  b=b-a;  b=b^(a << 10);
  c=c-a;  c=c-b;  c=c^(b >> 15);
  return c;
}

unsigned int hash_int_jenkins(unsigned int a) {
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

unsigned int hash_int_wang32shift(unsigned int a) {
    a = ~a + (a << 15);           /*  a = (a << 15) - a - 1; */
    a = a ^ (a >> 12);
    a = a + (a << 2);
    a = a ^ (a >> 4);
    a = a * 2057;                 /*  a = (a + (a << 3)) + (a << 11); */
    a = a ^ (a >> 16);
    return a;
}

unsigned int hash_int_wang32shiftmult(unsigned int key) {
    key = (key ^ 61) ^ (key >> 16);
    key = key + (key << 3);
    key = key ^ (key >> 4);
    key = key * 0x27d4eb2d;
    key = key ^ (key >> 15);
    return key;
}

unsigned int hash_int_wang6432shift(long unsigned int key) {
  key = (~key) + (key << 18); // key = (key << 18) - key - 1;
  key = key ^ (key >> 31);
  key = key * 21; // key = (key + (key << 2)) + (key << 4);
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return (unsigned int) key;
}

long unsigned int hash_int_wang64shift(long unsigned int key) {
  key = (~key) + (key << 21); // key = (key << 21) - key - 1;
  key = key ^ (key >> 24);
  key = (key + (key << 3)) + (key << 8); // key * 265
  key = key ^ (key >> 14);
  key = (key + (key << 2)) + (key << 4); // key * 21
  key = key ^ (key >> 28);
  key = key + (key << 31);
  return key;
}
