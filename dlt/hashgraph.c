// Resources:
// - https://www.youtube.com/watch?v=wgwYU1Zr9Tg
// - https://www.hashgraph.com/wp-content/uploads/2024/09/SWIRLDS-TR-2016-01.pdf

#include "ccore/base/base_include.h"
#include "ccore/platform/platform_include.h"

#include "ccore/base/base_include.c"
#include "ccore/platform/platform_include.c"

#define HASH_STR_LEN 100
#define MAX_BLOCKS 100

typedef struct Event Event;
struct Event {
    char *creator;
    time_t timestamp;
    char payload[100];
    char* self_parent;    // Hash of parent event from same node
    char* other_parent;   // Hash of parent event from another node
};

typedef struct Node Node;
struct Node {
    char *name;
    Event events[10];
    int event_count;
    Node* next;
};

typedef struct Hashgraph Hashgraph;
struct Hashgraph {
    Node nodes[10];
    U32 nodes_count;
};

global Arena *arena = 0;

fn U64 hash_djb2(const U8 *data, size_t len) {
    U64 hash = 5381;
    for (size_t i = 0; i < len; i++) {
        int c = data[i];
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

void add_event(Node* node, const U8 *payload, char* self_parent, char* other_parent) {
    Event *event = &node->events[node->event_count];
    MemCopy(&event->creator, &node->name, sizeof(node->name));
    event->timestamp = os_now_unix();
    MemCopy(event->payload, payload, 100);

    if (self_parent) {
        MemCopy(&event->self_parent, &self_parent, sizeof(self_parent));
    } else { event->self_parent = NULL;}

    if (other_parent) {
        MemCopy(&event->other_parent, &other_parent, sizeof(other_parent));
    } else { event->other_parent = NULL; }

    node->event_count++;
}

fn Node* create_node(Hashgraph *hashgraph, Str8 name)
{
    Node* node = &hashgraph->nodes[hashgraph->nodes_count];
    MemCopy(&node->name, &name.str, name.size + 1);
    MemZeroArray(node->events);
    node->next = NULL;
    hashgraph->nodes_count++;
    return node;
}

void gossip(Node* from_node, Node* to_node) {
    if (from_node->event_count == 0) return;

    // Get the latest event from sender
    Event* latest = &from_node->events[from_node->event_count - 1];
    Event* old_latest = &from_node->events[from_node->event_count - 2];
    U64 self_hash = from_node->event_count > 1 ?
                     hash_djb2((const U8*)old_latest, sizeof(*old_latest)) : 0;
    U64 other_hash = hash_djb2((const U8*)latest, sizeof(*latest));

    // Create new event at receiving node
    add_event(to_node, "Received gossip", (U8*)&self_hash, (U8*)&other_hash);

    fmt_printf("%s gossiped to %s\n", from_node->name, to_node->name);
}

int main(void)
{
    TCTX tctx;
    tctx_init_and_equip(&tctx);
    arena = arena_alloc();

    Hashgraph hashgraph = ZERO_STRUCT;

    Node *node1 = create_node(&hashgraph, str8_lit("Node 1"));
    Node *node2 = create_node(&hashgraph, str8_lit("Node 2"));

    add_event(node1, "hello", NULL, NULL);
    add_event(node1, "hello", NULL, NULL);
    // add_event(node2, "hello 2", NULL, NULL);
    gossip(node1, node2);
    add_event(node1, "hello", NULL, NULL);
    gossip(node1, node2);

    fmt_printf("\n");
    for (U32 i = 0; i < hashgraph.nodes_count; i++) {
        Node node = hashgraph.nodes[i];
        fmt_printf("%s\n", node.name);
        for (U32 i = 0; i < node.event_count; i++) {
            Event *event = &node.events[i];
            char *head_symbol = "┣";
            char *child_symbol = "┃";
            if (i == node.event_count - 1) {
                head_symbol = "┗";
                child_symbol = " ";
            }
            fmt_printf(" %s━ Event %d:\n",             head_symbol, i);
            fmt_printf(" %s      Creator: %s\n",       child_symbol, event->creator);
            fmt_printf(" %s      Timestamp: %ld\n",    child_symbol, event->timestamp);
            fmt_printf(" %s      Payload: %s\n",       child_symbol, event->payload);
            fmt_printf(" %s      Self Parent: %lu\n",  child_symbol, event->self_parent);
            fmt_printf(" %s      Other Parent: %lu\n", child_symbol, event->other_parent);
        }
        fmt_printf("\n");
    }

    // fmt_printf("\nAlice's events:\n");
    // for (int i = 0; i < node1->event_count; i++) {
    //     fmt_printf("Event %d: %s by %s\n", i, node1->events[i].payload, node1->events[i].creator);
    // }

    // fmt_printf("\nBob's events:\n");
    // for (int i = 0; i < node2->event_count; i++) {
    //     fmt_printf("Event %d: %s by %s\n", i, node2->events[i].payload, node2->events[i].creator);
    // }

    arena_free(arena);
    tctx_release();
}
