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

        context.input_buffer      = (String) { calloc(8192, sizeof(u8)), 8192 }; 
        context.command_line_args = (Array(String)) { malloc(sizeof(String) * arg_count), arg_count };

        for (int i = 0; i < arg_count; i++) {
            context.command_line_args.data[i] = (String) {(u8*) args[i], strlen(args[i])};
        }
        
        setvbuf(stdout, NULL, _IONBF, 0); // force some shell to print immediately (so stdout before input will not be hidden) 
    }

    if (arg_count < 3) {
        hard_error(
            "Unknown Command!\n"
            "Example Usage:\n"
            "story run foo.story\n"
            "story export foo.story foo.c\n"
        );
    }

    if (strcmp(args[1], "run") == 0) {
        
        String file = load_file(args[2]);
        if (!file.count) hard_error("Please open a valid file!\n");

        String lang_table_buffer[16];
        Story story = {
            .scene_table = table_init(256, 0.7, get_hash_fnv1a),
            .lang_table = { 
                .data      = lang_table_buffer, 
                .allocated = count_of(lang_table_buffer) 
            },
        };
        
        parse_file_to_story(file, &story);
        run_story(&story);
    }

    if (strcmp(args[1], "export") == 0) {
        
        if (arg_count < 4) hard_error("Invalid export command! Maybe you miss the output filename?\n");

        String file = load_file(args[2]);
        if (!file.count) hard_error("Please open a valid file!\n");

        String lang_table_buffer[16];
        Story story = {
            .scene_table = table_init(256, 0.7, get_hash_fnv1a),
            .lang_table = { 
                .data      = lang_table_buffer, 
                .allocated = count_of(lang_table_buffer) 
            },
        };
        
        parse_file_to_story(file, &story);
        u8 ok = export_story_as_c_code(&story, args[3]);
        if (!ok) hard_error("Cannot export %s to %s\n", args[2], args[3]);
    }
}
