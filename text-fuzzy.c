#include <string.h>
#include <errno.h>
#include "edit-distance-char.h"
#include "edit-distance-int.h"

#define ERROR_HANDLER text_fuzzy_error_handler;
#include "text-fuzzy.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
typedef int (* error_handler_t) (const char * source_file,
                                 int source_line_number,
                                 const char * message, ...)
    __attribute__((format (printf, 3, 4)));
#endif /* ndef ERROR_HANDLER_H */

extern error_handler_t text_fuzzy_error_handler;


/* This is the default error handler for this namespace. */

static int
text_fuzzy_default_error_handler (const char * source_file,
                                        int source_line_number,
                                        const char * message, ...)
{
    fprintf (stderr, "%s:%d: ", source_file, source_line_number);
    va_list args;
    va_start (args, message);
    vfprintf (stderr, message, args);
    exit (EXIT_FAILURE);
}

/* This global variable is the error handler for this namespace. */

error_handler_t text_fuzzy_error_handler =
    text_fuzzy_default_error_handler;


/* Print an error message for a failed condition "condition" at the
   appropriate line. */

#define LINE_ERROR(condition, status)                                   \
    if (text_fuzzy_error_handler) {                               \
        (* text_fuzzy_error_handler)                              \
            (__FILE__, __LINE__,                                        \
             "Failed test '%s', returning status '%s': %s",             \
             #condition, #status,                                       \
             text_fuzzy_statuses                                  \
             [text_fuzzy_status_ ## status]);                     \
    }                                                               

/* Fail a test, without message. */

#define FAIL(condition, status)                                         \
    if (condition) {                                                    \
        LINE_ERROR (condition, status);                                 \
        return text_fuzzy_status_ ## status;                      \
    }

/* Fail a test, with message. */

#define FAIL_MSG(condition, status, msg, args...)                       \
    if (condition) {                                                    \
        LINE_ERROR (condition, status);                                 \
        if (text_fuzzy_error_handler) {                           \
            (* text_fuzzy_error_handler)                          \
                (__FILE__, __LINE__,                                    \
                 msg, ## args);                                         \
        }                                                               \
        return text_fuzzy_status_ ## status;                      \
    }

#define OK return text_fuzzy_status_ok;

/* Call a function and print an error message and return if the
   function returns an error value. */

#define CALL(x) {                                                       \
	text_fuzzy_status_t _status = text_fuzzy_ ## x;     \
	if (_status != text_fuzzy_status_ok) {                    \
            if (text_fuzzy_error_handler) {                       \
                (* text_fuzzy_error_handler)                      \
                    (__FILE__, __LINE__,                                \
                     "Call 'text_fuzzy_%s' "                      \
                     "failed with status '%d': %s",                     \
                     #x, _status,                                       \
                     text_fuzzy_statuses[_status]);       \
            }                                                           \
            return _status;                                             \
        }                                                               \
    }

/*
Local variables:
mode: c
End:
*/

const char * text_fuzzy_statuses[] = {
    "normal operation",
    "out of memory",
    "open error",
    "close error",
    "read error",
};

#define STATIC static
#define FUNC(name) text_fuzzy_status_t text_fuzzy_ ## name

#ifdef VERBOSE
#define MESSAGE(format, args...) {              \
        printf ("%s:%d: ", __FILE__, __LINE__); \
        printf (format, ## args);               \
}
#else /* VERBOSE */
#define MESSAGE(format, args...)
#endif /* VERBOSE */

/* Local variables:
mode: c
End:
*/

#line 1 "/usr/home/ben/projects/Text-Fuzzy/text-fuzzy.c.in"





#ifdef HEADER

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

#endif /* HEADER */

FUNC (compare_single) (text_fuzzy_t * text_fuzzy,
                       text_fuzzy_string_t * b)
{
    /* The edit distance between "text_fuzzy->search_term" and the
       truncated version of "text_fuzzy->buf". */
    int d;

    text_fuzzy->found = 0;

    if (text_fuzzy->unicode) {
        int d;
        int allocated;

        allocated = 0;
        if (! b->unicode) {
            int i;
            b->unicode = calloc (b->length, sizeof (int));
            FAIL (! b->unicode, memory_error);
            allocated = 1;
            for (i = 0; i < b->length; i++) {
                unsigned char c;
                c = b->text[i];
                if (c < 0x80) {
                    b->unicode[i] = c;
                }
                else {
                    /* Cannot be equivalent to any Unicode character &
                       do not want to match it to 0x80 - 0x100
                       unicodes, so put a "nothing" value in here. */
                    b->unicode[i] = -1;
                }
            }
            b->ulength = b->length;
        }
        d = distance_int (b->unicode, b->ulength,
                          text_fuzzy->text.unicode,
                          text_fuzzy->text.ulength,
                          text_fuzzy->max_distance);
        if (d < text_fuzzy->max_distance) {
            text_fuzzy->found = 1;
            text_fuzzy->distance = d;
        }
        if (allocated) {
            free (b->unicode);
            b->unicode = 0;
        }
    }
    else {

        /* Alphabet filter: eliminate terms which cannot match. */

        if (text_fuzzy->use_alphabet) {
            int alphabet_misses;
            int l;

            alphabet_misses = 0;
            for (l = 0; l < b->length; l++) {
                int a = (unsigned char) b->text[l];
                if (! text_fuzzy->alphabet[a]) {
                    alphabet_misses++;
                    if (alphabet_misses > text_fuzzy->max_distance) {
                        OK;
                    }
                }
            }
        }

        /* Calculate the edit distance. */

        d = distance_char (b->text, b->length,
                           text_fuzzy->text.text, text_fuzzy->text.length,
                           text_fuzzy->max_distance);
        if (d < text_fuzzy->max_distance) {
            text_fuzzy->found = 1;
            text_fuzzy->distance = d;
        }
    }
    OK;
}

static int max_unique_characters = 45;

FUNC (set_search_term) (text_fuzzy_t * text_fuzzy)
{
    int unique_characters;
    int i;
    if (text_fuzzy->use_alphabet) {
        for (i = 0; i < 0x100; i++) {
            text_fuzzy->alphabet[i] = 0;
        }
        unique_characters = 0;
        for (i = 0; i < text_fuzzy->text.length; i++) {
            int c;
            c = (unsigned char) text_fuzzy->text.text[i];
            if (! text_fuzzy->alphabet[c]) {
                unique_characters++;
                text_fuzzy->alphabet[c] = 1;
            }
        }
        if (unique_characters > max_unique_characters) {
            text_fuzzy->no_alphabet_filter = 1;
        }
        else {
            text_fuzzy->no_alphabet_filter = 0;
        }
    }
    OK;
}

#define SIZE 0x100

FUNC (scan_file) (text_fuzzy_t * text_fuzzy, char * file_name,
                  char ** nearest_ptr)
{
    FILE * fh;
    char nearest[SIZE];
    int found;
    int max_distance_holder;

    max_distance_holder = text_fuzzy->max_distance;
    
    fh = fopen (file_name, "r");
    FAIL_MSG (! fh, open_error, "failed to open %s: %s", file_name,
              strerror (errno));
    found = 0;
    while (1) {
        text_fuzzy_string_t b = {0};
        char s[SIZE];
        char * r;
        int l;
        r = fgets (s, SIZE - 1, fh);
        if (! r) {
            if (feof (fh)) {
                break;
            }
            FAIL_MSG (! r, read_error, "Error reading %s: %s",
                      file_name, strerror (errno));
        }
        l = strlen (s);
        s[l - 1] = '\0';
        b.text = s;
        b.length = l - 1;
        CALL (compare_single (text_fuzzy, & b));
        if (text_fuzzy->found) {
            found = 1;
            if (text_fuzzy->distance < text_fuzzy->max_distance) {
                text_fuzzy->max_distance = text_fuzzy->distance;
                strncpy (nearest, s, SIZE);
            }
        }
    }

    FAIL (fclose (fh), close_error);
    text_fuzzy->max_distance = max_distance_holder;
    if (found) {
        * nearest_ptr = strdup (nearest);
    }
    else {
        * nearest_ptr = 0;
    }
    OK;
}

/* statuses:

status: open_error

status: close_error

status: read_error

*/

