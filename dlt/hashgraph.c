// Resources:
// - https://www.youtube.com/watch?v=wgwYU1Zr9Tg
// - https://www.hashgraph.com/wp-content/uploads/2024/09/SWIRLDS-TR-2016-01.pdf

#define LOG_LEVEL LogLevel_Debug

#include "ccore/base/base_include.h"
#include "ccore/platform/platform_include.h"

#include "ccore/base/base_include.c"
#include "ccore/platform/platform_include.c"

typedef struct Event Event;
struct Event
{
    char *sig;
    time_t timestamp;
    char payload[100];
    char* self_parent;    // Hash of parent event from same node
    char* other_parent;   // Hash of parent event from another node
};

typedef struct Node Node;
struct Node
{
    char *name;
    char *pub_key;
    Event events[10];
    int events_count;
    Node* next;
};

typedef struct Hashgraph Hashgraph;
struct Hashgraph
{
    Node nodes[10];
    U32 nodes_count;
};

global Arena *arena = 0;

fn U64 hash_djb2(const U8 *data, size_t len, U64 seed)
{
    U64 hash = seed > 0 ? seed : 5381;
    for (size_t i = 0; i < len; i++) {
        int c = data[i];
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

void add_event(Node* node, const U8 *payload, char* other_parent)
{
    log_debug_printf("- Added Event '%s' to Node '%s'\n", payload, node->name);
    Event *event = &node->events[node->events_count];
    event->timestamp = os_now_unix();
    MemCopy(event->payload, payload, 100);

    if (node->events_count > 0) {
        Event* last_event = &node->events[node->events_count - 1];
        event->self_parent = (U8 *)hash_djb2((const U8*)&last_event, sizeof(*last_event), 0);
    } else {
        event->self_parent = NULL;
    }

    if (other_parent) {
        MemCopy(&event->other_parent, &other_parent, sizeof(other_parent));
    } else { event->other_parent = NULL; }

    event->sig = (U8*)hash_djb2((const U8*)event, sizeof(*event), (U64)node->pub_key);

    node->events_count++;
}

fn Node* create_node(Hashgraph *hashgraph, Str8 name, Str8 pub_key)
{
    log_debug_printf("Created node: %s\n", name.str);
    Node* node = &hashgraph->nodes[hashgraph->nodes_count];
    MemCopy(&node->name, &name.str, name.size + 1);
    MemCopy(&node->pub_key, &pub_key.str, pub_key.size + 1);
    MemZeroArray(node->events);
    add_event(node, "Genesises Event", NULL);
    node->next = NULL;
    hashgraph->nodes_count++;
    return node;
}

void node_gossip(Node* from_node, Node* to_node)
{
    if (from_node->events_count == 0) return;

    U64 other_hash = 0;
    {
        Event* from_last_event = &from_node->events[from_node->events_count - 1];
        other_hash = hash_djb2((const U8*)&from_last_event, sizeof(*from_last_event), 0);
    }
    add_event(to_node, "Received gossip", (U8*)other_hash);

    fmt_printf("+ %s gossiped to %s\n", from_node->name, to_node->name);
}

int main(void)
{
    TCTX tctx;
    tctx_init_and_equip(&tctx);
    arena = arena_alloc();

    Hashgraph hashgraph = ZERO_STRUCT;

    Node *node1 = create_node(&hashgraph, str8_lit("Node 1"), str8_lit("1234567890"));
    Node *node2 = create_node(&hashgraph, str8_lit("Node 2"), str8_lit("1234234344"));

    add_event(node1, "Data 1", NULL);
    add_event(node1, "Data 2", NULL);
    node_gossip(node1, node2);
    add_event(node2, "hello 2", NULL);
    node_gossip(node2, node1);

    fmt_printf("\n");
    for (U32 i = 0; i < hashgraph.nodes_count; i++)
    {
        Node node = hashgraph.nodes[i];
        fmt_printf("%s\n", node.name);
        for (U32 i = 0; i < node.events_count; i++)
        {
            Event *event = &node.events[i];
            char *head_symbol = "┣";
            char *child_symbol = "┃";
            if (i == node.events_count - 1) {
                head_symbol = "┗";
                child_symbol = " ";
            }
            fmt_printf(" %s━ Event %d:\n",             head_symbol, i);
            fmt_printf(" %s      Signature: %lu\n",    child_symbol, event->sig);
            fmt_printf(" %s      Timestamp: %ld\n",    child_symbol, event->timestamp);
            fmt_printf(" %s      Payload: %s\n",       child_symbol, event->payload);
            fmt_printf(" %s      Self Parent: %lu\n",  child_symbol, event->self_parent);
            fmt_printf(" %s      Other Parent: %lu\n", child_symbol, event->other_parent);
        }
        fmt_printf("\n");
    }

    arena_free(arena);
    tctx_release();
}
