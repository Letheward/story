// simple max count dynamic array

typedef struct {
    String* data;
    u64     count;
    u64     allocated;
} LanguageTable;

void language_table_add(LanguageTable* table, String s) {
    u64 wanted = table->count + 1;
    assert(wanted < table->allocated);
    table->data[table->count] = s;
    table->count = wanted;
}

// linear search
u8 language_table_get_index(LanguageTable* table, String s, u64* index_out) {
    *index_out = 0;
    for (u64 i = 0; i < table->count; i++) {
        if (string_equal(s, table->data[i])) {
            *index_out = i;
            return 1;
        }
    }
    return 0; 
}
