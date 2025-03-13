#include "ccore/base/base_include.h"
#include "ccore/platform/platform_include.h"

#include "ccore/base/base_include.c"
#include "ccore/platform/platform_include.c"

#define HASH_STR_LEN 100
#define MAX_BLOCKS 100

typedef struct Block Block;
struct Block {
    U32 index;
    U32 timestamp;
    char data[256];
    U64 hash;
    U64 prev_hash;
};

typedef struct Blockchain Blockchain;
struct Blockchain {
    Block blocks[MAX_BLOCKS];
    U32 size;
};

global Arena *arena = 0;

fn U64 hash_djb2(const unsigned char *data, size_t len) {
    unsigned long hash = 5381;
    for (size_t i = 0; i < len; i++) {
        int c = data[i];
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

fn U64 hash_block(const Block *block) {
    // char buffer[1024];
    U8 *buffer = arena_push(arena, U8, 1024);
    U8 *buff_pos = buffer;

    MemCopy(buff_pos, &block->index, sizeof(block->index));
    buff_pos += sizeof(block->index);
    MemCopy(buff_pos, &block->timestamp, sizeof(block->timestamp));
    buff_pos += sizeof(block->timestamp);
    MemCopy(buff_pos, &block->data, sizeof(block->data));
    buff_pos += sizeof(block->data);
    MemCopy(buff_pos, &block->prev_hash, HASH_STR_LEN);

    return hash_djb2((const U8*)&buffer, sizeof(buffer));
}

fn void add_block(Blockchain *blockchain, Str8 data) {
    if (blockchain->size >= MAX_BLOCKS) {
        fmt_eprintf("Blockchain is full\n");
        return;
    }

    Block new_block;
    new_block.index = blockchain->size;
    new_block.timestamp = os_now_unix();
    MemCopy(new_block.data, data.str, sizeof(new_block.data));
    if (blockchain->size == 0) {
        // Genesis block has no previous hash
        new_block.prev_hash = 0;
    } else {
        new_block.prev_hash = blockchain->blocks[new_block.index - 1].hash;
    }
    new_block.hash = hash_block(&new_block);
    blockchain->blocks[blockchain->size] = new_block;
    blockchain->size++;
}

fn void print_blockchain(Blockchain blockchain) {
    fmt_printf("Blockchain Size: %d\n\n", blockchain.size);
    for (int i = 0; i < blockchain.size; i++) {
        Block block = blockchain.blocks[i];
        fmt_printf("Block %d:\n", block.index);
        fmt_printf("    Timestamp: %ld\n", block.timestamp);
        fmt_printf("    Data: %s\n", block.data);
        fmt_printf("    Previous Hash: %lu\n", block.prev_hash);
        fmt_printf("    Hash: %lu\n\n", block.hash);
    }
}

int verify_blockchain(Blockchain blockchain) {
    for (int i = 0; i < blockchain.size; i++) {
        Block block = blockchain.blocks[i];
        U64 block_hash = hash_block(&block);
        // Check if the stored hash matches the recalculated hash
        if (block.hash == block_hash) {
            fmt_printf("Block %d has invalid hash: %lu != %lu\n", i, block.hash, block_hash);
        }
        // Check if the previous hash links correctly
        if (i > 0 && block.prev_hash != blockchain.blocks[i-1].hash) {
            fmt_printf("Block %d has invalid previous hash: %lu != %lu\n", i, block.prev_hash, blockchain.blocks[i-1].hash);
        }
    }
    return 1;
}

int main(void) {
    TCTX tctx;
    tctx_init_and_equip(&tctx);
    arena = arena_alloc();

    Blockchain blockchain = ZERO_STRUCT;
    add_block(&blockchain, str8_lit("Genesis Block"));
    add_block(&blockchain, str8_lit("Hell"));
    add_block(&blockchain, str8_lit(" Block"));
    print_blockchain(blockchain);
    verify_blockchain(blockchain);

    arena_free(arena);
    tctx_release();
}
