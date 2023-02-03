
typedef u32 HashFunction(String s);

typedef struct {
    String key;      
    Scene  value;    
    u32    hash;
    u32    occupied; 
} HashTableEntry;

typedef struct {
    HashFunction*   hash_function;
    HashTableEntry* entries;
    u64             entry_count;
    u64             size;         // total allocated
    f64             load_factor;
} HashTable;

HashTable table_init(u64 size, f64 load_factor, HashFunction* f) {
    
    if (load_factor <= 0 || load_factor >= 1) load_factor = 0.7;
    
    // find min powers of 2 larger or equal to count
    {
        u64 base = 32;
        while (base < size) base *= 2;
        size = base;
    }
    
    return (HashTable) {
        .hash_function = f,
        .entries       = calloc(size, sizeof(HashTableEntry)),
        .entry_count   = 0,
        .size          = size,
        .load_factor   = load_factor,
    };
}

// todo: validate
u8 table_resize(HashTable* table) {
        
    u64 new_size = table->size * 2;
    if (new_size < table->size) return 0; // handle overflow

    HashTableEntry* new_entries = calloc(new_size, sizeof(HashTableEntry));
    if (!new_entries) return 0;
    
    // re-slot all old entries. todo: is this slow? 
    for (u64 i = 0; i < table->size; i++) {

        HashTableEntry* it = &table->entries[i];
        if (!it->occupied) continue;
        
        u32 hash  = it->hash;
        u64 index = hash & (new_size - 1);
        
        u64 probe_count = 1;
        while (new_entries[index].occupied) {
            
            HashTableEntry* entry = &new_entries[index];
            if (hash == entry->hash && string_equal(it->key, entry->key)) {
                goto next;
            }
            
            index = (index + probe_count) & (new_size - 1); // triangular probing
            probe_count++;

            if (probe_count >= new_size) return 0; // should not happen?
        }
        
        new_entries[index] = *it;
        next: continue;
    }
    
    free(table->entries);
    table->entries = new_entries;
    table->size    = new_size;

    return 1;
}

// todo: validate
HashTableEntry* table_put(HashTable* table, String key, Scene value) {
   
    if ((f64) (table->entry_count + 1) > (f64) table->size * table->load_factor) { 
        u8 ok = table_resize(table);
        if (!ok) return NULL;
    }
    
    u32 hash  = table->hash_function(key);
    u64 index = hash & (table->size - 1);
    
    u64 probe_count = 1;
    while (table->entries[index].occupied) {
        
        HashTableEntry* entry = &table->entries[index];
        if (hash == entry->hash && string_equal(key, entry->key)) {
            entry->value = value; // update value
            return entry;
        }
        
        index = (index + probe_count) & (table->size - 1); // triangular probing
        probe_count++;
        
        if (probe_count >= table->size) return NULL; // we've searched through all the entries
    }
    
    table->entries[index] = (HashTableEntry) { key, value, hash, 1 };
    table->entry_count++;

    return &table->entries[index];
}

// todo: validate
u8 table_get_index(HashTable* table, String key, u64* index_out) {

    *index_out = 0;

    u32 hash  = table->hash_function(key);
    u64 index = hash & (table->size - 1);
    
    u64 probe_count = 1;
    while (table->entries[index].occupied) {
        
        HashTableEntry* entry = &table->entries[index];
        if (hash == entry->hash) {
            if (string_equal(key, entry->key)) {
                *index_out = index;
                return 1;
            }
        }
        
        index = (index + probe_count) & (table->size - 1); // triangular probing
        probe_count++;
        
        if (probe_count >= table->size) return 0; // we've searched through all the entries, so we don't have it
    }
    
    return 0;
}

HashTableEntry* table_get_entry(HashTable* table, String key) {
    u64 index;
    u8 ok = table_get_index(table, key, &index);
    if (!ok) return NULL;
    return &table->entries[index];
}




/* ==== Hash Functions ==== */

u32 get_hash_djb2(String s) {
    
    u32 hash = 5381;
    for (u64 i = 0; i < s.count; i++) {
        hash += (hash << 5) + s.data[i];
    }
    
    return hash;
}

u32 get_hash_fnv1a(String s) {

    u32 hash = 0x811c9dc5;
    for (u64 i = 0; i < s.count; i++) {
        hash ^= (u32) s.data[i];
        hash *= 0x01000193;
    }
    
    return hash;
}
