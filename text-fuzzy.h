#ifndef TEXT_FUZZY_H
#define TEXT_FUZZY_H
extern const char * text_fuzzy_statuses[];
#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
typedef int (* error_handler_t) (const char * source_file,
                                 int source_line_number,
                                 const char * message, ...)
#ifdef __GNUC__
    __attribute__((format (printf, 3, 4)))
#endif /* __GNUC__ */
;
#endif /* ndef ERROR_HANDLER_H */

extern error_handler_t text_fuzzy_error_handler;

#ifndef FAIL_STATUS
#define FAIL_STATUS -1
#endif /* FAIL_STATUS */
#ifndef ERROR_HANDLER
#include <stdio.h>
#include <stdarg.h>

#ifdef __GNUC__

/* The following tells GCC not to warn that "default_error_handler" is
   unused. */

static void
default_error_handler (const char *,
		       int,
		       const char *, ...) 
    __attribute__ ((unused));

#endif /* __GNUC__ */

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
    text_fuzzy_status_ualphabet_on_non_unicode,
    text_fuzzy_status_max_min_miscalculation,
}
text_fuzzy_status_t;


/* Alphabet over unicode characters. */

typedef struct ualphabet {
    int min;
    int max;
    /* Number of chars allocated in the following array. */
    int size;
    /* Array containing Unicode alphabet. */
    unsigned char * alphabet;

    int rejected;
}
ualphabet_t;

/* This structure contains one string of whatever type. */

typedef struct text_fuzzy_string {
    char * text;
    int length;
    int * unicode;
    int ulength;
}
text_fuzzy_string_t;

/* This string contains one string plus additional paraphenalia used
   in searching for the string, for example the alphabet of the
   string. */

typedef struct text_fuzzy {
    /* The string we are to match. */
    text_fuzzy_string_t text;
    /* The maximum edit distance we allow for. */
    int max_distance;
    /* The number of mallocs we are guilty of. */
    int n_mallocs;
    /* Alphabet */
    int alphabet[0x100];
    /* Unicode alphabet. */
    ualphabet_t ualphabet;
    int distance;
    /* Does the user want to use alphabet filter? Default is yes, so
       this must be set to a non-zero value to switch off use. */
    int user_no_alphabet : 1;
    /* Are we actually going to use it? (This may be false even if the
       user wants to use it, for silly cases, but is not true if the
       user does not want to use it.) */
    int use_alphabet : 1;
    int use_ualphabet : 1;
    /* Variable edit costs? */
    int variable_edit_costs : 1;
    /* Do we account for transpositions? */
    int transpositions_ok : 1;
    /* Did we find it? */
    int found : 1;
    /* Is this Unicode? */
    int unicode : 1;
}
text_fuzzy_t;
#line 92 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_generate_ualphabet (text_fuzzy_t * tf);
#line 253 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_compare_single (text_fuzzy_t * tf, text_fuzzy_string_t * b);
#line 402 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_generate_alphabet (text_fuzzy_t * text_fuzzy);
#line 514 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_scan_file (text_fuzzy_t * text_fuzzy, char * file_name, char ** nearest_ptr);
#endif /* TEXT_FUZZY_H */
