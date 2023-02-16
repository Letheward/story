
/* ==== String: Basic ==== */

/* ---- Basic ---- */

String c_string_to_string(char* s) {
    return (String) { (u8*) s, (u64) strlen(s) }; 
}

// note: not safe, use with caution
String string_advance(String s, u64 pos) {
    return (String) {s.data + pos, s.count - pos};
}

// note: not safe, use with caution (also end is exclusive)
String string_view(String s, u64 start, u64 end) {
    return (String) {s.data + start, end - start};
}

// todo: speed
u8 string_equal(String a, String b) {
    if (a.count != b.count) return 0;
    if (a.data  == b.data ) return 1;
    for (u64 i = 0; i < a.count; i++) {
        if (a.data[i] != b.data[i]) return 0; 
    }
    return 1;
}

// split to a (by count) and b (rest)
u8 string_split(String in, u64 count, String* a_out, String* b_out) {
    
    *a_out = (String) {0};
    *b_out = (String) {0};
    
    if (count > in.count) return 0;

    *a_out = string_view(in, 0,     count);
    *b_out = string_view(in, count, in.count);
    
    return 1;
}

// split to a (rest) and b (by count)
// todo: is this slower? (extra branch here)
u8 string_split_from_end(String in, u64 count, String* a_out, String* b_out) {
    if (count > in.count) return 0;
    return string_split(in, in.count - count, a_out, b_out);
}

String string_eat(String* s, u64 count) {
    if (count > s->count) return (String) {0}; // todo: is this the behavior we want?
    u8* old = s->data;
    *s = string_advance(*s, count);
    return (String) { old, count };
}

String string_eat_from_end(String* s, u64 count) {
    if (count > s->count) return (String) {0}; // todo: is this the behavior we want?
    u64 pos = s->count - count;
    u8* out = s->data + pos;
    *s = string_view(*s, 0, pos);
    return (String) { out, count };
}

u8 string_contains_u8(String s, u8 c) {
    for (u64 i = 0; i < s.count; i++) {
        if (s.data[i] == c) return 1; 
    }
    return 0;
}

// todo: validate
u8 string_contains_any_u8(String a, String b) {
    for (u64 i = 0; i < a.count; i++) {
        for (u64 j = 0; j < b.count; j++) {
            if (a.data[i] == b.data[j]) return 1; 
        }
    }
    return 0;
}

// naive search for now
// todo: validate
u8 string_contains(String a, String b) {
    if (a.count < b.count) return 0;
    for (u64 i = 0; i < a.count - b.count + 1; i++) {
        for (u64 j = 0; j < b.count; j++) {
            if (a.data[i + j] != b.data[j]) goto next;
        }
        return 1;
        next: continue;
    }
    return 0;
}

u8 string_starts_with(String s, String match) {
    if (s.count < match.count) return 0;
    for (u64 i = 0; i < match.count; i++) {
        if (s.data[i] != match.data[i]) return 0;
    }
    return 1;
}

u8 string_starts_with_u8(String s, u8 c) {
    if (!s.count) return 0;
    return s.data[0] == c;
}

u8 string_ends_with(String s, String match) {
    if (s.count < match.count) return 0;
    u64 pos = s.count - match.count;
    for (u64 i = 0; i < match.count; i++) {
        if (s.data[pos + i] != match.data[i]) return 0;
    }
    return 1;
}

u8 string_ends_with_u8(String s, u8 c) {
    if (!s.count) return 0;
    return s.data[s.count - 1] == c;
}

// naive search for now
String string_find_u8(String s, u8 c) {
    for (u64 i = 0; i < s.count; i++) {
        if (s.data[i] == c) return (String) {s.data + i, s.count - i};        
    }
    return (String) {0};
}

// naive search for now
// a: "this is\t a string\n"
// b: " \t\r\n"       --- if match any single u8 in b then return position
String string_find_any_u8(String a, String b) {
    if (!a.count || !a.data) return (String) {0};
    for (u64 i = 0; i < a.count; i++) {
        for (u64 j = 0; j < b.count; j++) {
            if (a.data[i] == b.data[j]) return (String) {a.data + i, a.count - i};        
        }
    }
    return (String) {0};
}

// naive search for now
// todo: validate
String string_find(String a, String b) {
    if (!a.count || !b.count || !a.data || !b.data || (b.count > a.count)) return (String) {0};
    for (u64 i = 0; i < a.count - b.count + 1; i++) {
        for (u64 j = 0; j < b.count; j++) {
            if (a.data[i + j] != b.data[j]) goto next;
        }
        return (String) {a.data + i, a.count - i};
        next: continue;
    }
    return (String) {0};
}




/* ---- Trim ---- */

// todo: validate
String string_trim_any_u8_from_start(String s, String u) {
    for (u64 i = 0; i < s.count; i++) {
        if (!string_contains_u8(u, s.data[i])) return string_advance(s, i);
    }
    return (String) {0};
}

// todo: validate
String string_trim_any_u8_from_end(String s, String u) {
    for (u64 i = s.count; i > 0;) {
        i--;
        if (!string_contains_u8(u, s.data[i])) return (String) {s.data, i + 1};
    }
    return (String) {0};
}

// todo: inline these calls?
String string_trim_any_u8(String s, String u) {
    return string_trim_any_u8_from_start(string_trim_any_u8_from_end(s, u), u);
}

/*
    note: 
    the order of spaces affect performance,
    for example: " \n\r\t" will be faster on input data that doesn't have tabs
*/

String string_trim_spaces_from_start(String s) {
    return string_trim_any_u8_from_start(s, string(" \t\r\n"));    
}

String string_trim_spaces_from_end(String s) {
    return string_trim_any_u8_from_end(s, string(" \t\r\n"));    
}

// todo: inline these calls?
String string_trim_spaces(String s) {
    return string_trim_any_u8(s, string(" \t\r\n"));
}



/* ---- Parsing ---- */

// todo: validate
String string_eat_by_separator(String* s, String separator) {
    
    String found = string_find(*s, separator);
    if (!found.count) {
        String out = *s;
        *s = (String) {0};
        return out; 
    }
    
    String out = { s->data, found.data - s->data };
    *s = string_advance(*s, out.count + separator.count);

    return out;
}

// todo: should we make this the default?
// todo: validate
String string_eat_by_separator_excluding_empty(String* s, String separator) {
    
    u8* data  = NULL;     
    u64 count = 0;
    
    while (!count) {
        
        String found = string_find(*s, separator);
        if (!found.count) {
            String out = *s;
            *s = (String) {0};
            return out; 
        }
        
        data  = s->data;
        count = found.data - data;
    
        *s = string_advance(*s, count + separator.count);
    }
    
    return (String) { data, count };
}

// todo: validate
String string_eat_by_any_u8_separators(String* s, String separators) {

    *s = string_trim_any_u8_from_start(*s, separators);
    
    u64 count = 0;
    while (count < s->count) {
        if (string_contains_u8(separators, s->data[count])) break;
        count++; 
    }

    String out = { s->data, count };

    u64 to_skip = 0;
    for (u64 i = count; i < s->count; i++) {
        if (string_contains_u8(separators, s->data[i])) to_skip++;
        else break;
    }

    *s = string_advance(*s, count + to_skip);
    
    return out;
}

String string_eat_by_spaces(String* s) {
    return string_eat_by_any_u8_separators(s, string(" \t\r\n"));
}

String string_eat_line_excluding_empty(String* s) {
    return string_eat_by_any_u8_separators(s, string("\r\n"));
}

// todo: validate, refactor this using string_eat_by_any_u8_separators(), or something
// todo: this is not totally correct, since the last '\n' will not be seen as a new empty line
String string_eat_line(String* s) {

    u8 found = 0;

    u64 i = 0;
    while (i < s->count) {
        if (s->data[i] == '\n') {
            found = 1;
            break;
        }
        i++;
    }
    
    u8* data  = s->data;
    u64 count = i;
    
    if (!found) {
        *s = string_advance(*s, count);
        return (String) {data, count};
    } 

    u64 to_skip = 1;
    if (count && data[count - 1] == '\r') {
        count  -= 1;
        to_skip = 2; 
    }
    
    *s = string_advance(*s, count + to_skip);

    return (String) {data, count};
}

// todo: validate
u8 parse_u64(String s, u64* out) {

    if (!s.count) return 0;

    *out = 0;

    u64 result = 0;
    u64 digit_count = 0;

    for (u64 i = 0; i < s.count; i++) {
        
        u8 c = s.data[i];
        if (c == '_') {
            if (i != 0) continue;
            else        return 0;
        } else {
            if (c < '0' || c > '9') return 0;
        }
        
        result *= 10;
        result += c - '0';
        
        digit_count++;
        if (digit_count > 20) return 0; // todo: not handling all overflows
    }
    
    *out = result;
    
    return 1;
}






/* ---- Allocating ---- */

// todo: handle alloc failed
String string_copy(String s) {
    String out = { context.alloc(s.count), s.count };
    for (u64 i = 0; i < s.count; i++) out.data[i] = s.data[i]; // todo: speed
    return out;
}

// todo: validate, make this faster
Array(String) string_split_by_separator(String s, String separator) {

    u64 count = 0;
    for (String a = s; a.count;) {
        string_eat_by_separator(&a, separator);
        count++;
    }

    if (!count) return (Array(String)) {0};
    
    Array(String) out = { context.alloc(sizeof(String) * count), count }; // will still allocate if count is 1
    
    u64 acc = 0;
    for (String a = s; a.count;) {
        String chunk = string_eat_by_separator(&a, separator);
        out.data[acc] = chunk;
        acc++;
    }

    assert(acc == count);
    
    return out;
}

// todo: validate, make this faster
Array(String) string_split_by_separator_excluding_empty(String s, String separator) {

    u64 count = 0;
    for (String a = s; a.count;) {
        string_eat_by_separator_excluding_empty(&a, separator);
        count++;
    }

    if (!count) return (Array(String)) {0};
    
    Array(String) out = { context.alloc(sizeof(String) * count), count }; // will still allocate if count is 1
    
    u64 acc = 0;
    for (String a = s; a.count;) {
        String chunk = string_eat_by_separator_excluding_empty(&a, separator);
        out.data[acc] = chunk;
        acc++;
    }

    assert(acc == count);
    
    return out;
}

// todo: validate
Array(String) string_split_by_any_u8(String s, String separators) {
    
    u64 count = 0;
    for (String a = s; a.count;) {
        string_eat_by_any_u8_separators(&a, separators);
        count++;
    }

    if (!count) return (Array(String)) {0};
    
    Array(String) out = { context.alloc(sizeof(String) * count), count }; // will still allocate if count is 1
    
    u64 acc = 0;
    for (String a = s; a.count;) {
        String chunk = string_eat_by_any_u8_separators(&a, separators);
        out.data[acc] = chunk;
        acc++;
    }
    
    assert(acc == count);

    return out;
}

// note: the order of separators affects performance
Array(String) string_split_by_spaces(String in) {
    return string_split_by_any_u8(in, string(" \t\r\n"));
}

// note: unsafe, currently this is only a helper for string_join() and string_concat(), etc. 
// todo: better name
void append_buffer_by_string(u8* data, u64* acc, String s) {
    for (u64 i = 0; i < s.count; i++) data[*acc + i] = s.data[i];
    *acc += s.count;
}

// if we want to use varargs, just make a stack array
String string_concat(Array(String) in) {
    
    u64 count = 0;
    for (u64 i = 0; i < in.count; i++)  count += in.data[i].count;

    String out = { context.alloc(count), count };
    
    u64 acc = 0;
    for (u64 i = 0; i < in.count; i++) {
        append_buffer_by_string(out.data, &acc, in.data[i]);
    }

    assert(acc == count);
    
    return out;
}

// todo: cleanup
String string_join(Array(String) in, String separator, u8 end_with_separator) {
    
    if (in.count == 0) return (String) {0};
    if (in.count <  2) return in.data[0];

    u64 count = 0;
    for (u64 i = 0; i < in.count; i++) count += in.data[i].count;   
    count += separator.count * (end_with_separator ? in.count : (in.count - 1));
    
    String out = { context.alloc(count), count };

    u64 acc = 0;
    for (u64 i = 0; i < in.count - 1; i++) {
        append_buffer_by_string(out.data, &acc, in.data[i]);
        append_buffer_by_string(out.data, &acc, separator);
    }
    
    append_buffer_by_string(out.data, &acc, in.data[in.count - 1]);
    if (end_with_separator) append_buffer_by_string(out.data, &acc, separator);
    
    assert(acc == count);
   
    return out;
}

// todo: cleanup
String string_replace(String s, String a, String b) {

    void* (*old_alloc)(u64) = context.alloc; // ehh....
    context.alloc = temp_alloc;
    
    Array(String) chunks = string_split_by_separator(s, a);
   
    context.alloc = old_alloc;
    
    if (chunks.count < 2) return s;
    String result = string_join(chunks, b, 0);
    return result;
}




/* ==== Standard IO ==== */

// basic print
void print_string(String s) {
    fwrite(s.data, sizeof(u8), s.count, stdout);
}

// todo: not robust, need more testing, handle adjacent items (no space in between)
void print(String s, ...) {
    
    va_list args;
    va_start(args, s);
    
    for (u64 i = 0; i < s.count; i++) {

        u8 c = s.data[i];
        if (c == '@') {
            if (i + 1 < s.count && s.data[i + 1] == '@') { // short circuit 
                putchar('@');
                i++;
            } else {
                print_string(va_arg(args, String)); // not safe, but this is C varargs, what can you do 
            }
            continue;
        }

        putchar(c);
    }
    
    va_end(args);
}


void file_print_string(FILE* f, String s) {
    fwrite(s.data, sizeof(u8), s.count, f);
}

// todo: not robust, need more testing, handle adjacent items (no space in between)
void file_print(FILE* f, String s, ...) {
    
    va_list args;
    va_start(args, s);
    
    for (u64 i = 0; i < s.count; i++) {

        u8 c = s.data[i];
        if (c == '@') {
            if (i + 1 < s.count && s.data[i + 1] == '@') { // short circuit 
                fputc('@', f);
                i++;
            } else {
                file_print_string(f, va_arg(args, String)); // not safe, but this is C varargs, what can you do 
            }
            continue;
        }

        fputc(c, f);
    }
    
    va_end(args);
}

void file_print_quoted_string(FILE* f, String s) {

    fputc('"', f);
    
    for (u64 i = 0; i < s.count; i++) {
        
        u8 c = s.data[i];
        switch (c) {
            
            case '"':  fprintf(f, "\\\""); break;
            case '\\': fprintf(f, "\\\\"); break;
            case '\b': fprintf(f, "\\b");  break;
            case '\f': fprintf(f, "\\f");  break;
            case '\n': fprintf(f, "\\n");  break;
            case '\r': fprintf(f, "\\r");  break;
            case '\t': fprintf(f, "\\t");  break;
           
            default: 
            {
                if (c < 0x20) fprintf(f, "\\u%.4x", c);
                else          fputc(c, f);     
                break;
            }
        }
    }
    
    fputc('"', f);
}

// note: does not give a copy
String read_line() {
    
    String s = context.input_buffer;

    fgets((char*) s.data, s.count, stdin);
    s.count = strlen((const char*) s.data);
    
    if (s.count == 0) return (String) {0};
    if (s.data[s.count - 1] == '\n') s.count -= 1;
    
    return s;
}





/* ==== File IO ==== */

// todo: do we really need to switch allocator?
String load_file(char* path) {

    FILE* f = fopen(path, "rb");
    if (!f) return (String) {0}; // todo: this is not enough, what if we have a file of size 0? 

    fseek(f, 0, SEEK_END);
    u64 count = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8* data = context.alloc(count);
    if (!data) {
        fclose(f);
        return (String) {0}; 
    }
    
    fread(data, 1, count, f);
    fclose(f);

    return (String) {data, count};
}

u8 save_file(String in, char* path) {

    FILE* f = fopen(path, "wb");
    if (!f) return 0; 

    fwrite(in.data, sizeof(u8), in.count, f);
    fflush(f);
    fclose(f);

    return 1;
}


