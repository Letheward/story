/* ==== Utils ==== */

void hard_error(char* s, ...) {
    va_list va;
    va_start(va, s);
    printf("Error: ");
    vprintf(s, va);
    exit(1);
}




/* ==== Debug ==== */

void debug_print_scene(Scene* scene) {
    
    printf("text:\n");
    for (u64 i = 0; i < max_language_count; i++) {
        print(string("@\n"), scene->text[i]);
    }
    
    for (u64 i = 0; i < scene->option_count; i++) {
        print(string("link [@]\n"), scene->options[i].link);
        for (u64 j = 0; j < max_language_count; j++) {
            print(string("@\n"), scene->options[i].text[j]);
        }
    }
}

void print_table(HashTable table) {
   
    printf("[%llu / %llu] | Max Load Factor: %.2f\n", table.entry_count, table.size, table.load_factor);
    printf("\n");
    
    char* format = "";
    if      (table.size > 99999999) format = "%16llu ";
    else if (table.size > 9999)     format = "%8llu ";
    else                            format = "%4llu ";
    
    for (u64 i = 0; i < table.size; i++) {
        HashTableEntry* it = &table.entries[i];
        printf(format, i);
        if (it->occupied) {
            printf("[%.8x] ", it->hash);
            print(string("[@]\n"), it->key);
            debug_print_scene(&it->value);
        }
        printf("\n");
    }
   
    printf("\n");
}




/* ==== Language Table ==== */

// just a simple max count dynamic array

typedef struct {
    String data[max_language_count];
    u64    count;
} LanguageTable;

void language_table_add(LanguageTable* table, String s) {
    u64 wanted = table->count + 1;
    assert(wanted < max_language_count);
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





/* ==== Story ==== */

typedef struct {
    HashTable     scene_table;
    LanguageTable lang_table;
    String        start_label;
    String        quit_label;
} Story;



/* ---- Parsing ---- */

u8 string_is_label(String s) {
    if (s.count < 2) return 0;
    return (s.data[0] == '[') && (s.data[s.count - 1] == ']');
}

// note: unsafe
String string_strip_label(String s) {
    return string_view(s, 1, s.count - 1);
}

// todo: make this return error code instead of hard exiting?
// todo: better error messages
void parse_file_to_story(char* file_name, Story* story) {
    

    /* ---- Load file and init hash table ---- */ 
    
    String file = load_file(file_name);
    if (!file.count) hard_error("Cannot open file \"%s\"\n", file_name);
    
    story->scene_table = table_init(256, 0.7, get_hash_fnv1a);


    /* ---- Init ---- */ 

    HashTable*     table      = &story->scene_table;
    LanguageTable* lang_table = &story->lang_table;
    
    String walk = file;
    u64 line_count = 0;


    /* ---- Header ---- */ 

    u8 has_language = 0;    
    u8 has_start    = 0;
    u8 has_quit     = 0;
    
    while (walk.count) {
        
        String line = string_eat_line(&walk);
        line_count++;
        
        const String start = string("start:");
        const String quit  = string("quit:");
        
        if (string_starts_with(line, string("languages:"))) {

            while (walk.count) {
                
                String line = string_eat_line(&walk);
                line_count++;
                
                if (!line.count) break;
               
                if (string_starts_with(line, string("#"))) continue;
                
                String language = string_trim_spaces(line);
                language_table_add(lang_table, language);
            }

            has_language = 1;
        }

        if (string_starts_with(line, start)) { 
            
            String label = string_trim_spaces(string_advance(line, start.count));
            if (!string_is_label(label)) {
                hard_error("Invalid start label at %llu\n", line_count);
            }

            story->start_label = string_strip_label(label);

            has_start = 1;
        }
    
        if (string_starts_with(line, quit)) {
            
            String label = string_trim_spaces(string_advance(line, quit.count));
            if (!string_is_label(label)) {
                hard_error("Invalid quit label at %llu\n", line_count);
            }

            label = string_strip_label(label);

            story->quit_label = label;
            
            table_put(table, label, (Scene) {0}); // todo: this waste a slot
            has_quit = 1;
        }
        
        if (has_language && has_start && has_quit) break;
    }
   
    if (!has_language) hard_error("File \"%s\" does not contain a language list!\n", file_name);
    if (!has_start)    hard_error("File \"%s\" does not contain a start label!\n", file_name);
    if (!has_quit)     hard_error("File \"%s\" does not contain a quit label!\n", file_name);



    /* ---- first pass to put all labels into the hash table ---- */
    
    for (String t = file; t.count;) {

        String line = string_eat_line(&t);
        if (!line.count) continue;
        
        String label = string_trim_spaces(line);
        if (!string_is_label(label)) continue;

        table_put(table, string_strip_label(label), (Scene) {0});
    }

    if (!table_get_entry(table, story->start_label)) {
        hard_error("File \"%s\" does not contain the correct start label specified in the header.\n", file_name);
    }



    /* ---- Labels ---- */

    while (walk.count) {


        String line = string_eat_line(&walk);
        line_count++;

        if (!line.count) continue;
        
        String label = string_trim_spaces(line);
        
        if (string_starts_with(line, string("#"))) continue;
        if (!string_is_label(line)) {
            hard_error("Invalid label at line %llu\n", line_count);
        }

        HashTableEntry* entry = table_get_entry(table, string_strip_label(label));
        assert(entry != NULL);

        Scene* scene = &entry->value;

        // label text
        while (walk.count) {
            
            String line = string_eat_line(&walk);
            line_count++;

            if (!line.count) break;
            if (string_starts_with(line, string("#"))) continue;
            
            String text = line;
            String lang = string_eat_by_separator(&text, string(":"));

            if (lang.count == line.count) {
                hard_error("Invalid label text at line %llu\n", line_count);
            }
            
            text = string_trim_spaces_from_start(text);
            
            // is paragraph
            if (!text.count) {
                
                const String delimiter = string("~~~");
                u64 delimiter_count = 0;
                
                u8 paragraph_has_start = 0;
                u8 paragraph_has_end   = 0;
                
                String start = {0};
                String end   = {0};

                u64 old_line_count = line_count;
                
                while (walk.count) {
                    
                    String line = string_eat_line(&walk);
                    line_count++;

                    if (!line.count) continue;
                    
                    line = string_trim_spaces(line);

                    if (string_equal(line, delimiter)) {
                        delimiter_count++;
                    }
                    
                    // todo: make these better
                    if (delimiter_count == 1 && !paragraph_has_start) {
                        start = line;
                        paragraph_has_start = 1;
                    }
                    
                    if (delimiter_count == 2) {
                        end = line;
                        paragraph_has_end = 1;
                        break;
                    }
                }
                
                if (!paragraph_has_start || !paragraph_has_end) {
                    hard_error("Invalid paragraph at line %llu\n", old_line_count);
                }
                
                text = string_trim_spaces(string_advance((String) { start.data, end.data - start.data }, delimiter.count)); // todo: make this better
            }

            u64 index;
            if (!language_table_get_index(lang_table, lang, &index)) {
                hard_error("Invalid language at line %llu\n", line_count);
            }
            
            String* slot = &scene->text[index];
            if (slot->count) {
                hard_error("Duplicate language string at line %llu\n", line_count);
            }
               
            *slot = text;
        }

        u64 option_acc = 0;

        // options
        while (walk.count) {
            
            String line = string_eat_line(&walk);
            line_count++;

            if (!line.count) break;
            if (string_starts_with(line, string("#"))) continue;

            String option = line;
            String num = string_eat_by_separator(&option, string("."));
            if (num.count == line.count) {
                hard_error("Invalid option at line %llu\n", line_count);
            }
            
            option = string_trim_spaces(option);
            if (!string_is_label(option)) {
                hard_error("Invalid option label at line %llu\n", line_count);
            }

            option = string_strip_label(option);
    
            if (!table_get_entry(table, option)) {
                hard_error("Cannot find option label at line %llu\n", line_count);
            }
            
            scene->options[option_acc].link = option;
            
            // option text
            while (walk.count) {

                String line = string_eat_line(&walk);
                line_count++;
                
                if (!line.count) break;
                if (string_starts_with(line, string("#"))) continue;
            
                String text = line;
                String lang = string_eat_by_separator(&text, string(":"));

                if (lang.count == line.count) {
                    hard_error("Invalid option text at line %llu\n", line_count);
                } 
                
                text = string_trim_spaces(text);
                
                u64 index;
                if (!language_table_get_index(lang_table, lang, &index)) {
                    hard_error("Invalid language at line %llu\n", line_count);
                }
                
                String* slot = &scene->options[option_acc].text[index];
                if (slot->count) {
                    hard_error("Duplicate language string at line %llu\n", line_count);
                }
                   
                *slot = text;
            }
            
            option_acc++;
            assert(option_acc <= count_of(scene->options));
        }

        scene->option_count = option_acc;
    }
}




/* ---- Running (terminal mode) ---- */

void print_scene(Scene* scene, u64 language) {
    
    const String missing = string("{missing string}");
    
    String text = scene->text[language];
    if (!text.count) text = missing;
    print(string("@\n"), text);
    
    for (u64 i = 0; i < scene->option_count; i++) {
        
        printf("[%llu] ", i + 1);
        
        String text = scene->options[i].text[language];
        if (!text.count) text = missing;
        print(string("@\n"), text);
    }
}

void run_story(Story* story) {
    
    HashTable*     table       = &story->scene_table;
    LanguageTable* lang_table  = &story->lang_table;
    String current_scene_label = story->start_label;
    String quit_label          = story->quit_label;
    
    u64 language = 0; 
    
    while (1) {
        
        if (string_equal(current_scene_label, quit_label)) break;

        HashTableEntry* entry = table_get_entry(table, current_scene_label);
        assert(entry != NULL);
        
        Scene* scene = &entry->value;
        print_scene(scene, language);
        
        ask_again:
        printf("> ");

        String line = string_trim_spaces(read_line());

        if (string_equal(line, string("quit"))  || string_equal(line, string("exit")))  break;
        if (string_equal(line, string("scene")) || string_equal(line, string("print"))) continue;
        
        String command = string_eat_by_spaces(&line);

        if (string_equal(command, string("help"))) {

            printf(
                "=======================\n"
                "How to use this program:\n"
                "\n"
                "    Choose an option:\n"
                "\n"
                "    Do it?\n"
                "    [1] yes\n"
                "    [2] no\n"
                "    > 1\n"
                "    \n"
                "    Change Language:\n"
                "    > lang en_us\n"
                "    \n"
                "    Print Current Scene:\n"
                "    > scene\n"
                "    \n"
                "    Quit Game:\n"
                "    > quit\n"
                "=======================\n"
            );

            goto ask_again;
        
        } else if (string_equal(command, string("language")) || string_equal(command, string("lang"))) {

            if (!line.count) {
                printf("Available languages:\n");
                for (u64 i = 0; i < lang_table->count; i++) {
                    print(string("@\n"), lang_table->data[i]);
                }
                goto ask_again;
            }
            
            u64 index;
            if (!language_table_get_index(lang_table, line, &index)) {
                
                print(string("Unknown language \"@\".\n"), line);
                printf("Available languages:\n");
                for (u64 i = 0; i < lang_table->count; i++) {
                    print(string("@\n"), lang_table->data[i]);
                }

                goto ask_again;
            }
            
            language = index;
        
        } else {
            
            u64 option_index = 0;
            if (parse_u64(command, &option_index)) {
                
                if (line.count) {
                    printf("You can only choose 1 option! Type only 1 option number to choose it.\n");
                    goto ask_again;
                }
            
                if (option_index < 1 || option_index > scene->option_count) {
                    print(string("There is no option @!\n"), command);
                    goto ask_again;
                }

                current_scene_label = scene->options[option_index - 1].link;
            
            } else {
        
                print(string("Unknown Command \"@\".\nType the option number to choose it. Type \"help\" for more information.\n"), command); // todo: hardcoded
                goto ask_again; 
            }
        }
    }
}





/* ---- Export ---- */

u8 export_story_to_graphviz_dot_file(Story* story, char* file_name) {
    
    FILE* f = fopen(file_name, "wb");
    if (!f) return 0;
    
    HashTable* table = &story->scene_table;

    fprintf(f, "digraph {\n");
    fprintf(f, "    node [fontname=\"sans-serif\", shape=\"box\"];\n");
    
    for (u64 i = 0; i < table->size; i++) {
        
        HashTableEntry* entry = &table->entries[i];
        if (!entry->occupied) continue;
        
        Scene* scene = &entry->value;
        for (u64 i = 0; i < scene->option_count; i++) {
            file_print(f, string("    \"@\" -> \"@\";\n"), entry->key, scene->options[i].link);
        }
    }
    
    fprintf(f, "}\n");
    
    fflush(f);
    fclose(f);
    
    return 1;
}

// note: hack
void file_print_string_as_twee_identifier(FILE* f, String s) {
    for (u64 i = 0; i < s.count; i++) {
        u8 c = s.data[i];
        if (c == '_') {
            fputc(' ', f);
        } else {
            fputc(c, f);
        }
    }
}

// The .twee format for Twine 
// note: currently we need to set the starting point in Twine manually
// todo: all the string_equal() with quit label may be slow
u8 export_story_to_twee(Story* story, u64 language, char* file_name) {

    HashTable* table = &story->scene_table;

    FILE* f = fopen(file_name, "wb");
    if (!f) return 0;

    for (u64 i = 0; i < table->size; i++) {
        
        HashTableEntry* entry = &table->entries[i];
        if (!entry->occupied) continue;
        if (string_equal(entry->key, story->quit_label)) continue;
        
        Scene* scene = &entry->value;

        file_print(f, string(":: "));
        file_print_string_as_twee_identifier(f, entry->key);
        file_print(f, string("\n@\n"), scene->text[language]);
        
        for (u64 j = 0; j < scene->option_count; j++) {
            
            Option* option = &scene->options[j];

            if (string_equal(option->link, story->quit_label)) continue;
            
            file_print(f, string("[[@->"), option->text[language]); 
            file_print_string_as_twee_identifier(f, option->link);
            fprintf(f, "]]\n");
        }
        
        fprintf(f, "\n");
    }

    fflush(f);
    fclose(f);

    return 1;
}

// for outputting valid C99 identifiers (because a scene label string is in utf-8)
// todo: maybe this is too long? use compressed base62 or something?
void file_print_string_as_byte_literal_identifier(FILE* f, String s) {
    fprintf(f, "identifier_");
    for (u64 i = 0; i < s.count; i++) {
        fprintf(f, "%x", s.data[i]);
    }
}

// todo: better and more robust interface, localized help command
u8 export_story_to_c_code(Story* story, char* file_name) {
    
    HashTable*     table      = &story->scene_table;
    LanguageTable* lang_table = &story->lang_table;
    
    FILE* f = fopen(file_name, "wb");
    if (!f) return 0;

    fprintf(
        f,
        "#include <stdio.h>\n"
        "#include <string.h>\n\n"
    );

    fprintf(
        f,
        "typedef struct {\n"
        "    int   link;\n"
        "    char* text[%llu];\n"
        "} Choice;\n\n",
        lang_table->count
    );

    fprintf(
        f,
        "typedef struct {\n"
        "    char*  text[%llu];\n"
        "    Choice choices[8];\n"
        "    int    choice_count;\n"
        "} Scene;\n\n",
        lang_table->count
    );
    
    fprintf(
        f,
        "void print_scene(Scene* scene, int language) {\n"
        "    printf(\"\\n%%s\\n\", scene->text[language]);\n"
        "    for (int i = 0; i < scene->choice_count; i++) {\n"
        "        printf(\"[%%d] %%s\\n\", i + 1, scene->choices[i].text[language]);\n"
        "    }\n"
        "}\n\n"
    );
    
    fprintf(f, "enum {\n");
    for (u64 i = 0; i < lang_table->count; i++) {
        file_print(f, string("    @,\n"), lang_table->data[i]); // todo: is it better to also byte literal this?
    }
    fprintf(f, "};\n\n");

    fprintf(f, "enum {\n");
    for (u64 i = 0; i < table->size; i++) {
        
        HashTableEntry* entry = &table->entries[i];
        if (!entry->occupied) continue;
        
        fprintf(f, "    ");
        file_print_string_as_byte_literal_identifier(f, entry->key);
        fprintf(f, ",\n");
    }
    fprintf(f, "};\n\n");

    u64 quit_index = 0;
    {
        u8 ok = table_get_index(table, story->quit_label, &quit_index);
        assert(ok);
    }

    fprintf(f, "Scene scenes[] = {\n");
    for (u64 i = 0; i < table->size; i++) {
        
        HashTableEntry* entry = &table->entries[i];
        if (!entry->occupied) continue;
        if (i == quit_index)  continue;
        
        fprintf(f, "    [");
        file_print_string_as_byte_literal_identifier(f, entry->key);
        fprintf(f, "] = {\n");
        
        file_print(f, string("        {\n"));
        Scene* scene = &entry->value;
        for (u64 j = 0; j < lang_table->count; j++) {
            file_print(f, string("            [@] = "), lang_table->data[j]);
            file_print_quoted_string(f, scene->text[j]);
            file_print(f, string(",\n"));
        }
        file_print(f, string("        },\n"));
        
        file_print(f, string("        {\n"));
        for (u64 j = 0; j < scene->option_count; j++) {

            Option* option = &scene->options[j];

            file_print(f, string("            {\n"));
            fprintf(f, "                ");
            file_print_string_as_byte_literal_identifier(f, option->link);
            fprintf(f, ",\n");
            
            file_print(f, string("                {\n"));
            for (u64 k = 0; k < lang_table->count; k++) {
                file_print(f, string("                    [@] = "), lang_table->data[k]);
                file_print_quoted_string(f, option->text[k]);
                file_print(f, string(",\n"));
            }
            file_print(f, string("                },\n"));

            file_print(f, string("            },\n"));
            
        }
        file_print(f, string("        },\n"));
        
        fprintf(f, "        %llu\n", scene->option_count); 
        
        file_print(f, string("    },\n"));
    }
    fprintf(f, "};\n\n");

    fprintf(
        f, 
        "int main() {\n"
        "\n"    
        "    setvbuf(stdout, NULL, _IONBF, 0);\n"
        "\n"
        "    int  language = 0;\n"
        "    int  current_scene_index = "
    );

    file_print_string_as_byte_literal_identifier(f, story->start_label);

    fprintf(
        f, 
        ";\n"
        "    char input[256];\n"
        "\n"
        "    while (1) {\n"
        "\n"        
        "        if (current_scene_index == "
    );
    
    file_print_string_as_byte_literal_identifier(f, story->quit_label);
    
    fprintf(
        f,
        ") break;\n"
        "\n"
        "        Scene* scene = &scenes[current_scene_index];\n"
        "        print_scene(scene, language);\n"
        "\n"        
        "        ask_again:\n"
        "        printf(\"> \");\n"
        "        fgets(input, sizeof(input), stdin);\n"
        "\n"
        "        if (strstr(input, \"quit\")  || strstr(input, \"exit\"))  break;\n"
        "        if (strstr(input, \"scene\") || strstr(input, \"print\")) continue;\n"
        "\n"        
        "        if (strstr(input, \"lang\")) {\n"
    );

    fprintf(f, "            const char* langs[] = {\n");
    for (u64 i = 0; i < lang_table->count; i++) {
        String lang = lang_table->data[i];
        file_print(f, string("                [@] = \"@\",\n"), lang, lang);
    }
    fprintf(f, "            };\n");

    fprintf(
        f, 
        "            for (int i = 0; i < %llu; i++) {\n"
        "                if (strstr(input, langs[i])) {\n"
        "                    language = i;\n"
        "                    goto next;\n"
        "                }\n"
        "            }\n"
        "            goto ask_again;\n"
        "        }\n\n",
        lang_table->count
    );

    fprintf(
        f, 
        "        {\n"
        "            const char* nums[] = {\"1\", \"2\", \"3\", \"4\", \"5\", \"6\", \"7\", \"8\"};\n"
        "            for (int i = 0; i < scene->choice_count; i++) {\n"
        "                if (strstr(input, nums[i])) {\n"
        "                    current_scene_index = scene->choices[i].link;\n"
        "                    goto next;\n"
        "                }\n"
        "            }\n"
        "        }\n\n"
    );

    fprintf(
        f, 
        "        printf(\"We don't know what you want to do!\\nType the option number to choose it.\\n\");\n"
        "        goto ask_again;\n" 
        "\n"        
        "        next: continue;\n"
        "    }\n"
        "}\n"
    );
    
    fflush(f);
    fclose(f);
    
    return 1;
}

