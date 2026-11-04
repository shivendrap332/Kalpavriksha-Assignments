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
