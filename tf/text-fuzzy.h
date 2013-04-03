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
    text_fuzzy_status_string_too_long,
    text_fuzzy_status_max_distance_misuse,
}
text_fuzzy_status_t;
#ifndef __GNUC__
static int ok = text_fuzzy_status_ok;
static int memory_error = text_fuzzy_status_memory_error;
static int open_error = text_fuzzy_status_open_error;
static int close_error = text_fuzzy_status_close_error;
static int read_error = text_fuzzy_status_read_error;
static int line_too_long = text_fuzzy_status_line_too_long;
static int ualphabet_on_non_unicode = text_fuzzy_status_ualphabet_on_non_unicode;
static int max_min_miscalculation = text_fuzzy_status_max_min_miscalculation;
static int string_too_long = text_fuzzy_status_string_too_long;
static int max_distance_misuse = text_fuzzy_status_max_distance_misuse;
#endif /* __GNUC__ */

/* Alphabet over unicode characters. */

typedef struct ualphabet {

    /* The smallest character in our alphabet. */
    int min;

    /* The largest character in our alphabet. */
    int max;

    /* Number of chars allocated in the following array. */
    int size;

    /* Array containing Unicode alphabet, as a bitmap. */
    unsigned char * alphabet;

    /* The number of characters which were rejected using the Unicode
       alphabet. */
    int rejections;
}
ualphabet_t;

/* This structure contains one string of whatever type. */

typedef struct text_fuzzy_string {

    /* The text of the string. */
    char * text;

    /* The length of "text". */
    int length;

    /* The characters of "text" expanded out into unicode
       characters. */
    int * unicode;

    /* The length of "unicode". */
    int ulength;
}
text_fuzzy_string_t;

/* The following structure contains one string plus additional
   paraphenalia used in searching for the string, for example the
   alphabet of the string. */

typedef struct text_fuzzy {

    /* The string we are to match. */
    text_fuzzy_string_t text;

    /* The matching string. */

    text_fuzzy_string_t b;

    /* The maximum edit distance we allow for. */
    int max_distance;

    /* The maximum edit distance the user will allow. We are going to
       cheat and ignore the user's value. */
    int max_distance_holder;

    /* The number of mallocs we are guilty of. */
    int n_mallocs;

    /* ASCII alphabet */
    int alphabet[0x100];

    /* The number of characters which were rejected using the ASCII
       alphabet. */
    int alphabet_rejections;

    /* Unicode alphabet. */
    ualphabet_t ualphabet;

    /* The minimum distance we got in our most recent effort. */
    int distance;

    /* The number of units allocated for "b.unicode". This is not the
       string length. This is used when deciding whether there is
       sufficient space to store a test string. */
    int b_unicode_length;

    /* The number of items which have been rejected because the length
       difference is bigger than the maximum edit distance. */
    int length_rejections;

    /* A character which is not in use. */
    unsigned char invalid_char;

    /* Does the user want to use an alphabet filter? Default is yes,
       so this must be set to a non-zero value to switch off use. */
    unsigned int user_no_alphabet : 1;

    /* Are we actually going to use it? (This may be false even if the
       user wants to use it, for silly cases, but is not true if the
       user does not want to use it.) */
    unsigned int use_alphabet : 1;
    unsigned int use_ualphabet : 1;

    /* Variable edit costs? (currently unused) */
    unsigned int variable_edit_costs : 1;

    /* Do we account for transpositions? */
    unsigned int transpositions_ok : 1;

    /* Did we find it? */
    unsigned int found : 1;

    /* Is this Unicode? */
    unsigned int unicode : 1;

    /* Do we want to skip exact matches? */
    unsigned int no_exact : 1;

    /* Are we scanning a list of entries? */
    unsigned int scanning : 1;
}
text_fuzzy_t;
#line 156 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_generate_ualphabet (text_fuzzy_t * tf);
#line 341 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_compare_single (text_fuzzy_t * tf);
#line 513 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_generate_alphabet (text_fuzzy_t * text_fuzzy);
#line 551 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_begin_scanning (text_fuzzy_t * text_fuzzy);
#line 576 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_end_scanning (text_fuzzy_t * text_fuzzy);
#line 670 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_scan_file (text_fuzzy_t * text_fuzzy, char * file_name, char ** nearest_ptr);
#line 716 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_alphabet_rejections (text_fuzzy_t * text_fuzzy, int * r);
#line 729 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"
text_fuzzy_status_t text_fuzzy_free_memory (text_fuzzy_t * text_fuzzy);
#endif /* TEXT_FUZZY_H */