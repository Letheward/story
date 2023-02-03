#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>


#include "base.c"
#include "string.c"
#include "types.c"
#include "language_table.c"
#include "hash_table.c"
#include "backend.c"




int main(int arg_count, char** args) {
    

    // setup context
    {
        const u64 size = 1024 * 256;
        context.temp_buffer.data = calloc(size, sizeof(u8));
        context.temp_buffer.size = size;
        context.alloc = malloc;

        context.input_buffer = (String) { calloc(8192, sizeof(u8)), 8192 }; 
        
        setvbuf(stdout, NULL, _IONBF, 0); // force some shell to print immediately (so stdout before input will not be hidden) 
    }

    char* unknown_error_string = 
        "Unknown Command!\n"
        "Example Usages:\n"
        "story run    foo.story\n"
        "story export foo.story foo.c\n"
    ;

    if (arg_count < 3) hard_error(unknown_error_string);

    u8 is_run    = strcmp(args[1], "run")    == 0;
    u8 is_export = strcmp(args[1], "export") == 0;

    if (!is_run   && !is_export)    hard_error(unknown_error_string);
    if (is_export && arg_count < 4) hard_error("Missing output filename for \"%s\"\n", args[2]);

    String file = load_file(args[2]);
    if (!file.count) hard_error("Cannot open file \"%s\"\n", args[2]);

    String lang_table_buffer[max_language_count];
    Story story = {
        .scene_table = table_init(256, 0.7, get_hash_fnv1a),
        .lang_table = { 
            .data      = lang_table_buffer, 
            .allocated = count_of(lang_table_buffer) 
        },
    };
    
    parse_file_to_story(file, &story);

    if (is_run) {
        run_story(&story);
    } else if (is_export) {
        u8 ok = export_story_as_c_code(&story, args[3]);
        if (!ok) hard_error("Cannot export \"%s\" to \"%s\"\n", args[2], args[3]);
    }
    
    // we let the OS clean up all the memory
}
