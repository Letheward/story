#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>


#include "base.c"
#include "string.c"
#include "types.c"
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

    char* example_string = 
        "Example Usages:\n"
        "story run          foo.story\n"
        "story export       foo.story foo.c\n"
        "story export-graph foo.story foo.dot\n"
        "story export-twee  foo.story foo.twee en_us\n"
    ;

    if (arg_count < 2) hard_error("You need to specify a command!\n%s", example_string);

    char* command = args[1];

    if (strcmp(command, "help") == 0) {
        
        printf("%s", example_string);

    } else if (strcmp(command, "run") == 0) {
        
        if (arg_count < 3) hard_error("You need to provide a file to run!\n");
        
        Story story = {0};
        parse_file_to_story(args[2], &story);
        
        run_story(&story);
    
    } else if (strcmp(command, "export") == 0 || strcmp(command, "export-c") == 0) {
        
        if (arg_count < 3) hard_error("Missing input filename.\n");
        if (arg_count < 4) hard_error("Missing output filename for \"%s\".\n", args[2]);
        
        char* input    = args[2];
        char* output   = args[3];
        
        Story story = {0};
        parse_file_to_story(input, &story);
        
        u8 ok = export_story_as_c_code(&story, output);
        if (!ok) hard_error("Cannot export \"%s\" to \"%s\".\n", input, output);
    
    } else if (strcmp(command, "export-twee") == 0) {
    
        if (arg_count < 3) hard_error("Missing input filename.\n");
        if (arg_count < 4) hard_error("Missing output filename for \"%s\".\n", args[2]);
        if (arg_count < 5) hard_error("Missing export language for \"%s\".\n", args[2]);
        
        char* input    = args[2];
        char* output   = args[3];
        char* language = args[4];

        Story story = {0};
        parse_file_to_story(input, &story);
        
        u64 language_index = 0;
        if (!language_table_get_index(&story.lang_table, c_string_to_string(language), &language_index)) {
            hard_error("The file \"%s\" does not contain language \"%s\".", input, language); 
        }
       
        u8 ok = export_story_as_twee(&story, language_index, output);
        if (!ok) hard_error("Cannot export \"%s\" to \"%s\".\n", input, output);
    
    } else if (strcmp(command, "export-graph") == 0) {
        
        // todo
        
    } else {
    
        hard_error("Unknown command \"%s\".\n%s", command, example_string);
    }
    
    // we let the OS clean up all the memory
}
