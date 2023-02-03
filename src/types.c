// make these dynamic?

#define max_language_count 8

typedef struct {
    String link;
    String text[max_language_count];
} Option;

typedef struct {
    String text[max_language_count];
    Option options[8];
    u64    option_count;
} Scene;

