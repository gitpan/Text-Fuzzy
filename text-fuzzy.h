#ifndef TEXT_FUZZY_H
#define TEXT_FUZZY_H
extern const char * text_fuzzy_statuses[];
#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
typedef int (* error_handler_t) (const char * source_file,
                                 int source_line_number,
                                 const char * message, ...)
    __attribute__((format (printf, 3, 4)));
#endif /* ndef ERROR_HANDLER_H */

extern error_handler_t text_fuzzy_error_handler;

#ifndef FAIL_STATUS
#define FAIL_STATUS -1
#endif /* FAIL_STATUS */
#ifndef ERROR_HANDLER
#include <stdio.h>
#include <stdarg.h>
static void default_error_handler (const char * file, int line,
                                   const char * format, ...)
{
    va_list a;
    va_start (a, format);
    fprintf (stderr, "%s:%d ", file, line);
    vfprintf (stderr, format, a);
    fprintf (stderr, "\n");
    va_end (a);
}
#define ERROR_HANDLER default_error_handler
#endif /* ERROR_HANDLER */
#define TEXT_FUZZY(x) {                                                 \
    text_fuzzy_status_t status;                                   \
    status = text_fuzzy_ ## x;                                    \
    if (status != text_fuzzy_status_ok) {                         \
    /* Print error and return. */                                       \
    ERROR_HANDLER (__FILE__, __LINE__,                                  \
                   "Call to %s failed: %s",                             \
                   #x, text_fuzzy_statuses[status]);              \
    return FAIL_STATUS;                                                 \
    }                                                                   \
    }

/*
  Local variables:
  mode: c
  End: 
*/

typedef enum {
    text_fuzzy_status_ok,
    text_fuzzy_status_memory_error,
    text_fuzzy_status_open_error,
    text_fuzzy_status_close_error,
    text_fuzzy_status_read_error,
    text_fuzzy_status_line_too_long,
}
text_fuzzy_status_t;


typedef struct text_fuzzy_string {
    char * text;
    int length;
    int * unicode;
    int ulength;
}
text_fuzzy_string_t;

typedef struct text_fuzzy {
    /* The string we are to match. */
    text_fuzzy_string_t text;
    /* The maximum edit distance we allow for. */
    int max_distance;
    /* The number of mallocs we are guilty of. */
    int n_mallocs;
    /* Alphabet */
    int alphabet[0x100];
    int distance;
    /* Use alphabet filter? */
    int no_alphabet_filter : 1;

    int use_alphabet : 1;
    /* Do we account for transpositions? */
    int transpositions_ok : 1;

    int found : 1;

    int unicode : 1;

}
text_fuzzy_t;
#line 47 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_compare_single (text_fuzzy_t * tf, text_fuzzy_string_t * b);
#line 158 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_set_search_term (text_fuzzy_t * text_fuzzy);
#line 240 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_scan_file (text_fuzzy_t * text_fuzzy, char * file_name, char ** nearest_ptr);
#endif /* TEXT_FUZZY_H */
