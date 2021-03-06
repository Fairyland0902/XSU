#ifndef _XSU_LIST_H
#define _XSU_LIST_H

#define LIST_POISON1 (void*)0x10101010
#define LIST_POISON2 (void*)0x20202020

struct list_head {
    struct list_head* prev;
    struct list_head* next;
};

#define LIST_HEAD_INIT(name) \
    {                        \
        &(name), &(name)     \
    }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr)  \
    do {                     \
        (ptr)->next = (ptr); \
        (ptr)->prev = (ptr); \
    } while (0)

static inline void __list_add(struct list_head* new, struct list_head* prev, struct list_head* next)
{
    new->next = next;
    new->prev = prev;
    prev->next = new;
    next->prev = new;
}

static inline void list_add(struct list_head* new, struct list_head* head)
{
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head* new, struct list_head* head)
{
    __list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head* prev, struct list_head* next)
{
    prev->next = next;
    next->prev = prev;
}

static inline void list_del(struct list_head* entry)
{
    __list_del(entry->prev, entry->next);
    entry->prev = LIST_POISON1;
    entry->next = LIST_POISON2;
}

static inline void list_del_init(struct list_head* entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head* entry, struct list_head* head)
{
    __list_del(entry->prev, entry->next);
    list_add(entry, head);
}

static inline void list_move_tail(struct list_head* entry, struct list_head* head)
{
    __list_del(entry->prev, entry->next);
    list_add_tail(entry, head);
}

static inline unsigned int list_empty(struct list_head* head) { return head->next == head; }

#define list_entry(ptr, type, member) container_of(ptr, type, member)
#define list_for_each(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)
#define list_for_each_safe(pos, n, head) for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

struct hlist_head {
    struct hlist_node* first;
};

struct hlist_node {
    struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT \
    {                   \
        .first = NULL   \
    }
#define HLIST_HEAD(name) struct hlist_head name = { .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
#define INIT_HLIST_NODE(ptr) ((ptr)->next = NULL, (ptr)->pprev = NULL)

#endif
