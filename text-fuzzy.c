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
    "line too long",
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

FUNC (compare_single) (text_fuzzy_t * tf,
                       text_fuzzy_string_t * b)
{
    /* The edit distance between "tf->search_term" and the
       truncated version of "tf->buf". */
    int d;

    tf->found = 0;

    if (tf->unicode) {
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
        if (tf->max_distance >= 0) {
        /* If the distance in the length of the strings is greater
           than the max distance, give up. */
        if (abs (tf->text.ulength - b->ulength) > tf->max_distance) {
            OK;
        }
        }
        d = distance_int (b->unicode, b->ulength,
                          tf->text.unicode,
                          tf->text.ulength,
                          tf->max_distance);
        if (allocated) {
            free (b->unicode);
            b->unicode = 0;
        }
    }
    else {

        if (tf->max_distance >= 0) {
        /* If the distance in the length of the strings is greater
           than the max distance, give up. */
        if (abs (tf->text.length - b->length) > tf->max_distance) {
            OK;
        }

        /* Alphabet filter: eliminate terms which cannot match. */

        if (tf->use_alphabet) {
            int alphabet_misses;
            int l;

            alphabet_misses = 0;
            for (l = 0; l < b->length; l++) {
                int a = (unsigned char) b->text[l];
                if (! tf->alphabet[a]) {
                    alphabet_misses++;
                    if (alphabet_misses > tf->max_distance) {
                        OK;
                    }
                }
            }
        }
        }
        /* Calculate the edit distance. */

        d = distance_char (b->text, b->length,
                           tf->text.text, tf->text.length,
                           tf->max_distance);
    }
    if (d < tf->max_distance) {
        tf->found = 1;
        tf->distance = d;
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

#define BUF_SIZE 0x1000

typedef struct fuzzy_file {
    const char * file_name;
    FILE * fh;
    char buf[BUF_SIZE];
    char * line;
    int length;
    text_fuzzy_string_t b;
    int remaining;
    int offset;
    int eof : 1;
}
fuzzy_file_t;

#define SIZE 0x1000

STATIC FUNC (more_bytes) (fuzzy_file_t * ff)
{
    int bytes;

    bytes = fread (ff->buf, sizeof (char), SIZE, ff->fh);
    if (bytes != SIZE) {
        if (feof (ff->fh)) {
            ff->eof = 1;
        }
        else {
            FAIL (bytes != SIZE, read_error);
        }
    }
    ff->remaining = bytes;
    ff->offset = 0;
    OK;
}

STATIC FUNC (get_line) (fuzzy_file_t * ff)
{
    int i;
    static char s[SIZE];

    i = 0;
    while (1) {
        char c;
        if (! ff->remaining) {
            CALL (more_bytes (ff));
        }
        c = ff->buf[ff->offset];
        ff->offset++;
        ff->remaining--;
        if (c == '\n' || (ff->remaining == 0 && ff->eof)) {
            s[i] = '\0';
            break;
        }
        else {
            s[i] = c;
        }
        i++;
        FAIL (i >= SIZE, line_too_long);
    }

    ff->b.text = s;
    ff->b.length = i;

    OK;
}

STATIC FUNC (open) (fuzzy_file_t * ff, const char * file_name)
{
    ff->file_name = file_name;
    ff->fh = fopen (ff->file_name, "r");
    FAIL_MSG (! ff->fh, open_error, "failed to open %s: %s", ff->file_name,
              strerror (errno));
    OK;
}

STATIC FUNC (close) (fuzzy_file_t * ff)
{
    FAIL (fclose (ff->fh), close_error);
    OK;
}

FUNC (scan_file) (text_fuzzy_t * text_fuzzy, char * file_name,
                  char ** nearest_ptr)
{
    fuzzy_file_t ff = {0};
    FILE * fh;
    char * nearest;
    int found;
    int max_distance_holder;

    CALL (open (& ff, file_name));

    max_distance_holder = text_fuzzy->max_distance;
    
    found = 0;
    nearest = 0;
    while (1) {
        CALL (get_line (& ff));
        CALL (compare_single (text_fuzzy, & ff.b));
        if (text_fuzzy->found) {
            found = 1;
            if (text_fuzzy->distance < text_fuzzy->max_distance) {
                text_fuzzy->max_distance = text_fuzzy->distance;
                if (! nearest) {
                    nearest = malloc (ff.b.length + 1);
                }
                else {
                    nearest = realloc (nearest, ff.b.length + 1);
                }
                FAIL (! nearest, memory_error);
                strncpy (nearest, ff.b.text, ff.b.length);
                nearest[ff.b.length] = '\0';
            }
        }
        if (ff.eof && ff.remaining == 0) {
            break;
        }
    }

    CALL (close (& ff));

    text_fuzzy->max_distance = max_distance_holder;
    if (found) {
        * nearest_ptr = nearest;
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

status: line_too_long

*/

