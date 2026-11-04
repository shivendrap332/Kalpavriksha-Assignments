#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_POOL_SIZE 1000000L
#define INITIAL_CAPACITY 4

typedef enum { FREE_BLOCK = 0, ALLOCATED_BLOCK = 1 } BlockStatus;
typedef enum { FIRST_FIT = 1, BEST_FIT = 2, WORST_FIT = 3 } Strategy;

typedef struct {
    size_t start;       
    size_t size;        
    int id;             
    BlockStatus status;
} Block;

typedef struct {
    unsigned char *pool;
    size_t pool_size;
    Block *blocks;      
    size_t count;
    size_t capacity;
    int next_id;
    Strategy strategy;
} Manager;



static int read_long(const char *prompt, long *out)
{
    char line[128];
    char *end;
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!fgets(line, sizeof line, stdin))
            return 0; 
        errno = 0;
        long v = strtol(line, &end, 10);
        while (*end == ' ' || *end == '\t') end++;
        if (errno == ERANGE || end == line || (*end != '\n' && *end != '\0')) {
            printf("Invalid input. Please enter a whole number.\n");
            continue;
        }
        *out = v;
        return 1;
    }
}


static int ensure_capacity(Manager *m, size_t needed)
{
    if (needed <= m->capacity)
        return 1;
    size_t new_cap = m->capacity ? m->capacity * 2 : INITIAL_CAPACITY;
    while (new_cap < needed)
        new_cap *= 2;
    Block *tmp = realloc(m->blocks, new_cap * sizeof *tmp);
    if (!tmp)
        return 0;
    m->blocks = tmp;
    m->capacity = new_cap;
    return 1;
}

static void insert_block(Manager *m, size_t idx, Block b)
{
    
    memmove(&m->blocks[idx + 1], &m->blocks[idx],
            (m->count - idx) * sizeof(Block));
    m->blocks[idx] = b;
    m->count++;
}

static void remove_block(Manager *m, size_t idx)
{
    memmove(&m->blocks[idx], &m->blocks[idx + 1],
            (m->count - idx - 1) * sizeof(Block));
    m->count--;
}

static int find_index_by_id(const Manager *m, int id)
{
    for (size_t i = 0; i < m->count; i++)
        if (m->blocks[i].status == ALLOCATED_BLOCK && m->blocks[i].id == id)
            return (int)i;
    return -1;
}


static void merge_free_blocks(Manager *m)
{
    size_t w = 0;
    for (size_t i = 0; i < m->count; i++) {
        if (w > 0 && m->blocks[i].status == FREE_BLOCK &&
            m->blocks[w - 1].status == FREE_BLOCK) {
            m->blocks[w - 1].size += m->blocks[i].size;
        } else {
            m->blocks[w++] = m->blocks[i];
        }
    }
    m->count = w;
}


static int validate_map(const Manager *m)
{
    size_t expected = 0;
    for (size_t i = 0; i < m->count; i++) {
        const Block *b = &m->blocks[i];
        if (b->start != expected || b->size == 0)
            return 0;
        if (b->status == ALLOCATED_BLOCK && b->id <= 0)
            return 0;
        if (i > 0 && b->status == FREE_BLOCK &&
            m->blocks[i - 1].status == FREE_BLOCK)
            return 0;
        expected += b->size;
    }
    return expected == m->pool_size;
}



static int first_fit(const Manager *m, size_t size)
{
    for (size_t i = 0; i < m->count; i++)
        if (m->blocks[i].status == FREE_BLOCK && m->blocks[i].size >= size)
            return (int)i;
    return -1;
}

static int best_fit(const Manager *m, size_t size)
{
    int best = -1;
    for (size_t i = 0; i < m->count; i++) {
        if (m->blocks[i].status == FREE_BLOCK && m->blocks[i].size >= size &&
            (best < 0 || m->blocks[i].size < m->blocks[best].size))
            best = (int)i;
    }
    return best;
}

static int worst_fit(const Manager *m, size_t size)
{
    int worst = -1;
    for (size_t i = 0; i < m->count; i++) {
        if (m->blocks[i].status == FREE_BLOCK && m->blocks[i].size >= size &&
            (worst < 0 || m->blocks[i].size > m->blocks[worst].size))
            worst = (int)i;
    }
    return worst;
}

static int find_free_block(const Manager *m, size_t size)
{
    switch (m->strategy) {
    case BEST_FIT:  return best_fit(m, size);
    case WORST_FIT: return worst_fit(m, size);
    default:        return first_fit(m, size);
    }
}

static const char *strategy_name(Strategy s)
{
    return s == BEST_FIT ? "Best Fit" : s == WORST_FIT ? "Worst Fit" : "First Fit";
}



static size_t total_free(const Manager *m)
{
    size_t t = 0;
    for (size_t i = 0; i < m->count; i++)
        if (m->blocks[i].status == FREE_BLOCK) t += m->blocks[i].size;
    return t;
}

static size_t largest_free(const Manager *m)
{
    size_t big = 0;
    for (size_t i = 0; i < m->count; i++)
        if (m->blocks[i].status == FREE_BLOCK && m->blocks[i].size > big)
            big = m->blocks[i].size;
    return big;
}

static void display_stats(const Manager *m)
{
    size_t alloc_blocks = 0, free_blocks = 0, alloc_bytes = 0, free_bytes = 0;
    for (size_t i = 0; i < m->count; i++) {
        if (m->blocks[i].status == ALLOCATED_BLOCK) {
            alloc_blocks++; alloc_bytes += m->blocks[i].size;
        } else {
            free_blocks++;  free_bytes += m->blocks[i].size;
        }
    }
    printf("Total memory: %zu bytes\n", m->pool_size);
    printf("Allocated: %zu bytes\n", alloc_bytes);
    printf("Free: %zu bytes\n", free_bytes);
    printf("Allocated blocks: %zu\n", alloc_blocks);
    printf("Free blocks: %zu\n", free_blocks);
    printf("Largest free block: %zu bytes\n", largest_free(m));
}

static void display_map(const Manager *m)
{
    printf("Memory Map (%zu bytes):\n", m->pool_size);
    for (size_t i = 0; i < m->count; i++) {
        const Block *b = &m->blocks[i];
        if (b->status == ALLOCATED_BLOCK)
            printf("[Allocated #%d: %zu bytes, Start: %zu]\n", b->id, b->size, b->start);
        else
            printf("[Free: %zu bytes, Start: %zu]\n", b->size, b->start);
    }
}


static void report_fragmentation(const Manager *m, size_t requested)
{
    size_t tf = total_free(m);
    if (tf < requested)
        return; 
    printf("External fragmentation detected: %zu bytes requested, %zu bytes free in total,\n"
           "but the largest single free block is only %zu bytes.\n"
           "Free blocks:", requested, tf, largest_free(m));
    int first = 1;
    for (size_t i = 0; i < m->count; i++) {
        if (m->blocks[i].status == FREE_BLOCK) {
            printf("%s %zu (at %zu)", first ? "" : ",", m->blocks[i].size, m->blocks[i].start);
            first = 0;
        }
    }
    printf(" bytes\n");
}


static void split_and_allocate(Manager *m, size_t idx, size_t size, int id)
{
    Block *b = &m->blocks[idx];
    size_t remainder = b->size - size;
    size_t old_start = b->start;
    b->size = size;
    b->id = id;
    b->status = ALLOCATED_BLOCK;
    if (remainder > 0) {
        Block rest = { old_start + size, remainder, 0, FREE_BLOCK };
        insert_block(m, idx + 1, rest);
    }
}

static void allocate_memory(Manager *m, long req)
{
    if (req <= 0 || (size_t)req > m->pool_size) {
        printf("Invalid size: must be between 1 and %zu.\n", m->pool_size);
        return;
    }
    size_t size = (size_t)req;
    int idx = find_free_block(m, size);
    if (idx < 0) {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        report_fragmentation(m, size);
        return;
    }
    if (!ensure_capacity(m, m->count + 1)) {
        printf("Allocation failed: could not grow block metadata.\n");
        return;
    }
    int id = m->next_id++;
    size_t start = m->blocks[idx].start;
    split_and_allocate(m, (size_t)idx, size, id);
    memset(m->pool + start, id & 0xFF, size); 
    printf("Allocation successful. ID: %d, Start: %zu, Size: %zu\n", id, start, size);
}

static void release_memory(Manager *m, long id)
{
    if (id <= 0 || id >= m->next_id) {
        printf("Release failed: unknown allocation ID %ld.\n", id);
        return;
    }
    int idx = find_index_by_id(m, (int)id);
    if (idx < 0) {
        printf("Release failed: allocation #%ld is already free.\n", id);
        return;
    }
    m->blocks[idx].status = FREE_BLOCK;
    m->blocks[idx].id = 0;
    merge_free_blocks(m);
    printf("Allocation released.\n");
}

static void resize_allocation(Manager *m, long id, long req)
{
    if (id <= 0 || id >= m->next_id) {
        printf("Resize failed: unknown allocation ID %ld.\n", id);
        return;
    }
    int idx = find_index_by_id(m, (int)id);
    if (idx < 0) {
        printf("Resize failed: allocation #%ld is not currently allocated.\n", id);
        return;
    }
    if (req <= 0 || (size_t)req > m->pool_size) {
        printf("Invalid size: must be between 1 and %zu.\n", m->pool_size);
        return;
    }
    size_t new_size = (size_t)req;
    size_t i = (size_t)idx;
    size_t old_size = m->blocks[i].size;

    if (new_size == old_size) {
        printf("Allocation #%ld already has size %zu.\n", id, old_size);
        return;
    }

    
    if (new_size < old_size) {
        size_t diff = old_size - new_size;
        if (i + 1 < m->count && m->blocks[i + 1].status == FREE_BLOCK) {
            
            m->blocks[i + 1].start -= diff;
            m->blocks[i + 1].size += diff;
        } else {
            if (!ensure_capacity(m, m->count + 1)) {
                printf("Resize failed: could not grow block metadata.\n");
                return;
            }
            Block freed = { m->blocks[i].start + new_size, diff, 0, FREE_BLOCK };
            insert_block(m, i + 1, freed);
        }
        m->blocks[i].size = new_size;
        merge_free_blocks(m);
        printf("Allocation #%ld shrunk to %zu bytes (in place).\n", id, new_size);
        return;
    }

    
    size_t need = new_size - old_size;
    if (i + 1 < m->count && m->blocks[i + 1].status == FREE_BLOCK &&
        m->blocks[i + 1].size >= need) {
        size_t old_end = m->blocks[i].start + old_size;
        memset(m->pool + old_end, (int)(id & 0xFF), need);
        m->blocks[i].size = new_size;
        m->blocks[i + 1].start += need;
        m->blocks[i + 1].size -= need;
        if (m->blocks[i + 1].size == 0)
            remove_block(m, i + 1);
        printf("Allocation #%ld expanded to %zu bytes (in place).\n", id, new_size);
        return;
    }

    
    int dest = find_free_block(m, new_size);
    if (dest < 0) {
        printf("Resize failed: no free region can hold %zu bytes. "
               "Original allocation is unchanged.\n", new_size);
        report_fragmentation(m, new_size);
        return;
    }
    if (!ensure_capacity(m, m->count + 1)) {
        printf("Resize failed: could not grow block metadata. "
               "Original allocation is unchanged.\n");
        return;
    }
    size_t old_start = m->blocks[i].start;
    size_t dest_start = m->blocks[dest].start;
    memcpy(m->pool + dest_start, m->pool + old_start, old_size); 
    memset(m->pool + dest_start + old_size, (int)(id & 0xFF), need);
    split_and_allocate(m, (size_t)dest, new_size, (int)id);
    
    for (size_t k = 0; k < m->count; k++) {
        if (m->blocks[k].start == old_start && m->blocks[k].status == ALLOCATED_BLOCK) {
            m->blocks[k].status = FREE_BLOCK;
            m->blocks[k].id = 0;
            break;
        }
    }
    merge_free_blocks(m);
    printf("Allocation #%ld moved from %zu to %zu and resized to %zu bytes.\n",
           id, old_start, dest_start, new_size);
}



static int init_manager(Manager *m, size_t pool_size)
{
    m->pool = malloc(pool_size);
    m->blocks = malloc(INITIAL_CAPACITY * sizeof(Block));
    if (!m->pool || !m->blocks) {
        free(m->pool);
        free(m->blocks);
        m->pool = NULL;
        m->blocks = NULL;
        return 0;
    }
    memset(m->pool, 0, pool_size);
    m->pool_size = pool_size;
    m->capacity = INITIAL_CAPACITY;
    m->count = 1;
    m->blocks[0] = (Block){ 0, pool_size, 0, FREE_BLOCK };
    m->next_id = 1;
    m->strategy = FIRST_FIT;
    return 1;
}

static int choose_strategy(Manager *m)
{
    long c;
    for (;;) {
        if (!read_long("Select allocation strategy (1. First Fit, 2. Best Fit, 3. Worst Fit): ", &c))
            return 0;
        if (c >= 1 && c <= 3) {
            m->strategy = (Strategy)c;
            printf("Strategy set to %s.\n", strategy_name(m->strategy));
            return 1;
        }
        printf("Invalid choice.\n");
    }
}

static void print_menu(void)
{
    printf("\nMenu:\n"
           "1. Allocate memory\n"
           "2. Release memory\n"
           "3. Resize allocation\n"
           "4. Display memory map\n"
           "5. Display memory statistics\n"
           "6. Change allocation strategy\n"
           "7. Exit\n");
}

int main(void)
{
    Manager m = {0};
    long size;

    for (;;) {
        if (!read_long("Enter memory pool size: ", &size))
            return 0;
        if (size < 1 || size > MAX_POOL_SIZE) {
            printf("Invalid pool size: must be between 1 and %ld bytes.\n", MAX_POOL_SIZE);
            continue;
        }
        if (!init_manager(&m, (size_t)size)) {
            printf("Memory allocation for the pool failed.\n");
            return 1;
        }
        break;
    }
    if (!choose_strategy(&m))
        goto done;

    for (;;) {
        long choice, a, b;
        print_menu();
        if (!read_long("Choice: ", &choice))
            break;
        switch (choice) {
        case 1:
            if (read_long("Requested size: ", &a)) allocate_memory(&m, a);
            else goto done;
            break;
        case 2:
            if (read_long("Allocation ID: ", &a)) release_memory(&m, a);
            else goto done;
            break;
        case 3:
            if (read_long("Allocation ID: ", &a) && read_long("New size: ", &b))
                resize_allocation(&m, a, b);
            else goto done;
            break;
        case 4: display_map(&m); break;
        case 5: display_stats(&m); break;
        case 6: if (!choose_strategy(&m)) goto done; break;
        case 7: goto done;
        default: printf("Invalid choice. Enter a number from 1 to 7.\n");
        }
        if (!validate_map(&m)) {
            fprintf(stderr, "Internal error: memory map inconsistent!\n");
            break;
        }
    }
done:
    free(m.blocks);
    free(m.pool);
    printf("Goodbye.\n");
    return 0;
}
