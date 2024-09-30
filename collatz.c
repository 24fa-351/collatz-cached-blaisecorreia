#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LRU 1
#define LFU 2

typedef struct CacheNode {
    int number;
    int steps;
    struct CacheNode* prev;
    struct CacheNode* next;
} CacheNode;

typedef struct LFUCacheNode {
    int number;
    int steps;
    int frequency;
    struct LFUCacheNode* next;
} LFUCacheNode;
typedef struct {
    int type;  
    int size; 
    int count; 

    CacheNode* head;
    CacheNode* tail;
    LFUCacheNode* lfu_head;

} Cache;





int collatz_steps(int n) {
    int steps = 0;
    while (n != 1) {
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
        steps++;
    }
    return steps;
}



CacheNode* create_lru_node(int number, int steps) {
    CacheNode* new_node = (CacheNode*)malloc(sizeof(CacheNode));
    new_node->number = number;
    new_node->steps = steps;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}



void move_to_front(Cache* cache, CacheNode* node) {
    if (node == cache->head) return; 
    

    
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (node == cache->tail) cache->tail = node->prev;
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) cache->head->prev = node;
    cache->head = node;
    if (!cache->tail) cache->tail = node;
}


void add_to_lru_cache(Cache* cache, int number, int steps) {
    CacheNode* new_node = create_lru_node(number, steps);
    new_node->next = cache->head;
    if (cache->head) cache->head->prev = new_node;
    cache->head = new_node;

    if (!cache->tail) cache->tail = new_node;
    if (cache->count == cache->size) {
        CacheNode* old_tail = cache->tail;
        cache->tail = old_tail->prev;
        if (cache->tail) cache->tail->next = NULL;
        free(old_tail);
    } else {
        cache->count++;
    }
}


LFUCacheNode* create_lfu_node(int number, int steps) {
    LFUCacheNode* new_node = (LFUCacheNode*)malloc(sizeof(LFUCacheNode));
    new_node->number = number;
    new_node->steps = steps;
    new_node->frequency = 1;
    new_node->next = NULL;
    return new_node;
}


void add_to_lfu_cache(Cache* cache, int number, int steps) {
    LFUCacheNode* new_node = create_lfu_node(number, steps);
    if (!cache->lfu_head) {
        cache->lfu_head = new_node;
    } else {
        LFUCacheNode* current = cache->lfu_head;
        LFUCacheNode* prev = NULL;
        while (current && current->frequency <= new_node->frequency) {
            prev = current;
            current = current->next;
        }

        if (prev) {
            prev->next = new_node;
        } else {
            cache->lfu_head = new_node;
        }
        new_node->next = current;
    }

    cache->count++;
    if (cache->count > cache->size) {
        LFUCacheNode* current = cache->lfu_head;
        cache->lfu_head = current->next;
        free(current);
        cache->count--;
    }
}


int check_lru_cache(Cache* cache, int number) {
    CacheNode* current = cache->head;
    while (current) {
        if (current->number == number) {
            move_to_front(cache, current);
            return current->steps;
        }
        current = current->next;
    }
    return -1;
}

int check_lfu_cache(Cache* cache, int number) {
    LFUCacheNode* current = cache->lfu_head;
    while (current) {
        if (current->number == number) {
            current->frequency++; 
            return current->steps;
        }
        current = current->next;
    }
    return -1; 
}


int collatz_with_cache(Cache* cache, int number) {
    int steps = -1;
    if (cache->type == LRU) {
        steps = check_lru_cache(cache, number);
    } else if (cache->type == LFU) {
        steps = check_lfu_cache(cache, number);
    }

    if (steps == -1) {
        steps = collatz_steps(number);
        if (cache->type == LRU) {
            add_to_lru_cache(cache, number, steps);
        } else if (cache->type == LFU) {
            add_to_lfu_cache(cache, number, steps);
        }
    }

    return steps;
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Usage: %s N MIN MAX policy cache_size\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int MIN = atoi(argv[2]);
    int MAX = atoi(argv[3]);
    char* policy_str = argv[4];
    int cache_size = atoi(argv[5]);

    Cache cache;
    cache.size = cache_size;
    cache.count = 0;

    if (strcmp(policy_str, "none") == 0) {
        cache.type = 0;
    } else if (strcmp(policy_str, "LRU") == 0) {
        cache.type = LRU;
        cache.head = NULL;
        cache.tail = NULL;
    } else if (strcmp(policy_str, "LFU") == 0) {
        cache.type = LFU;
        cache.lfu_head = NULL;
    } else {
        printf("Unknown cache policy. Use 'none', 'LRU', or 'LFU'.\n");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        int number = MIN + rand() % (MAX - MIN + 1);
        int steps = collatz_with_cache(&cache, number);
        printf("Number: %d -> Steps: %d\n", number, steps);
    }

    return 0;
}
