#include "hash.h"

void ht_test(void) {
    assert(hash_stb6432shift("Hello", 1) == 7871771224773941061);
    assert(hash_djbx33a("Hello") == 140207504);
    assert(hash_djb2("Hello") == 210676686969);
    assert(hash_sdbm("Hello") == 5142962386210502930);
    assert(hash_2lose("Hello") == 500);
    assert(hash_adler32("Hello") == 93061621);
    assert(hash_crc32("Hello") == (unsigned int) -137262718);
    assert(hash_xor8("Hello") == 12);

    unsigned int num_mix = hash_int_mix(420, 12, 69);
    assert(num_mix == 1501094981);
    assert(hash_int_jenkins(num_mix) == 317653115);
    assert(hash_int_wang32shift(num_mix) == 1265322998);
    assert(hash_int_wang32shiftmult(num_mix) == 608512410);
    assert(hash_int_wang6432shift(num_mix) == 981695088);
    assert(hash_int_wang64shift(num_mix) == 5360857491066763507);
}

int main(void) {
    ht_test();
}
