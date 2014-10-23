/* This file is included into "Fuzzy.xs". The reason for having it as
   a separate file from "Fuzzy.xs" is so that this file can easily be
   edited in C mode without the C mode causing problems when editing
   "Fuzzy.xs". */

/* Get memory via Perl. */

#define get_memory(value, number, what) {                       \
        Newxz (value, number, what);                            \
        if (! value) {                                          \
            croak ("%s:%d: "                                    \
                   "Could not allocate memory for %d %s",       \
                   __FILE__, __LINE__, number, #what);          \
        }                                                       \
        text_fuzzy->n_mallocs++;                                \
    }

/* Send a bad return value from one of the C routines in
   "text-fuzzy.c" back to the user via Perl's error handlers. The
   parameters "file_name" and "line_number" are the name of the C file
   and the line number where the error occurred in the C file. These
   are discarded by this error handler. */

int perl_error_handler (const char * file_name, int line_number,
                        const char * format, ...)
{
    va_list a;
    //    warn ("%s:%d: ", file_name, line_number);
    va_start (a, format);
    vcroak (format, & a);
    va_end (a);
    return 0;
}

#define SMALL 0x1000

/* Decide how many ints to allocate for "text_fuzzy->b.unicode". It
   has to be bigger than "minimum", the actual length of the
   string. Also, we don't want to keep reallocating it, so make it
   large enough for most of the cases (SMALL). */

static void fake_length (text_fuzzy_t * text_fuzzy, int minimum)
{
    int r = SMALL;
 again:
    if (minimum < r) {
	text_fuzzy->b_unicode_length = r;
	return;
    }
    r *= 2;
    if (r > STRING_MAX_CHARS) {

	/* Stupid value. */

	croak ("String length %d longer than maximum allowed for, %d.\n",
	       minimum, STRING_MAX_CHARS);
    }
    goto again;
}

/* Allocate the memory for b. */

static void allocate_b_unicode (text_fuzzy_t * text_fuzzy, int b_length)
{

    if (! text_fuzzy->b.unicode) {

	/* We have not allocated any memory yet. */

	fake_length (text_fuzzy, b_length);
	get_memory (text_fuzzy->b.unicode,
		    text_fuzzy->b_unicode_length, int);
    }
    else if (b_length > text_fuzzy->b_unicode_length) {

	/* "b" is bigger than what we allowed for. */

	fake_length (text_fuzzy, b_length);

	/* "Renew" is "realloc" for Perl. See "perldoc perlapi". */

	Renew (text_fuzzy->b.unicode, text_fuzzy->b_unicode_length, int);
    }
}

/* Given a Perl string in "text" which is marked as being Unicode
   characters, use Perl's Unicode handlers to turn it into a string of
   integers. */

static void sv_to_int_ptr (SV * text, text_fuzzy_string_t * tfs)
{
    int i;
    U8 * utf;
    STRLEN curlen;
    STRLEN length;
    unsigned char * stuff;

    stuff = (unsigned char *) SvPV (text, length);

    utf = stuff;
    curlen = length;
    for (i = 0; i < tfs->ulength; i++) {
        STRLEN len;

	/* The documentation for "utf8n_to_uvuni" can be found in
	   "perldoc perlapi". There is an online version here:
	   "http://perldoc.perl.org/perlapi.html#Unicode-Support". */

        tfs->unicode[i] = utf8n_to_uvuni (utf, curlen, & len, 0);
        curlen -= len;
        utf += len;
    }
}

/* Convert a Perl SV into the text_fuzzy_t structure. */

static void
sv_to_text_fuzzy (SV * text, text_fuzzy_t ** text_fuzzy_ptr)
{
    STRLEN length;
    unsigned char * stuff;
    text_fuzzy_t * text_fuzzy;
    int i;
    int is_utf8;

    /* Allocate memory for "text_fuzzy". */
    get_memory (text_fuzzy, 1, text_fuzzy_t);
    text_fuzzy->max_distance = NO_MAX_DISTANCE;

    /* Copy the string in "text" into "text_fuzzy". */
    stuff = (unsigned char *) SvPV (text, length);
    text_fuzzy->text.length = length;
    get_memory (text_fuzzy->text.text, length + 1, char);
    for (i = 0; i < (int) length; i++) {
        text_fuzzy->text.text[i] = stuff[i];
    }
    text_fuzzy->text.text[text_fuzzy->text.length] = '\0';
    is_utf8 = SvUTF8 (text);
    if (is_utf8) {

	/* Put the Unicode version of the string into
	   "text_fuzzy->text". */

        text_fuzzy->unicode = 1;
	text_fuzzy->text.ulength = sv_len_utf8 (text);

	get_memory (text_fuzzy->text.unicode, text_fuzzy->text.ulength, int);

	sv_to_int_ptr (text, & text_fuzzy->text);

	/* Generate the Unicode alphabet. */

	TEXT_FUZZY (generate_ualphabet (text_fuzzy));
    }
    else {
	TEXT_FUZZY (generate_alphabet (text_fuzzy));
    }
    * text_fuzzy_ptr = text_fuzzy;
}

/* The following palaver is related to the macros "FAIL" and
   "FAIL_MSG" in "text-fuzzy.c.in". */

#undef FAIL_STATUS
#define FAIL_STATUS -1

static void
sv_to_text_fuzzy_string (SV * word, text_fuzzy_t * tf)
{
    STRLEN length;
    tf->b.text = SvPV (word, length);
    tf->b.length = length;
    if (SvUTF8 (word) || tf->unicode) {

	/* Make a Unicode version of b. */

	tf->b.ulength = sv_len_utf8 (word);
	allocate_b_unicode (tf, tf->b.ulength);
	sv_to_int_ptr (word, & tf->b);
	if (! tf->unicode) {

	    /* Make a non-Unicode version of b. */

	    int i;

	    tf->b.length = tf->b.ulength;
	    for (i = 0; i < tf->b.ulength; i++) {
		int c;

		c = tf->b.unicode[i];
		if (c <= 0x80) {
		    tf->b.text[i] = c;
		}
		else {
		    /* Put a non-matching character in there. */

		    tf->b.text[i] = tf->invalid_char;
		}
	    }
	}
    }
}

static int
text_fuzzy_sv_distance (text_fuzzy_t * tf, SV * word)
{
    sv_to_text_fuzzy_string (word, tf);
    TEXT_FUZZY (compare_single (tf));
    if (tf->found) {
        return tf->distance;
    }
    else {
        return tf->max_distance + 1;
    }
}

typedef struct candidate candidate_t;

struct candidate {
    int distance;
    int offset;
    candidate_t * next;
};

static int
text_fuzzy_av_distance (text_fuzzy_t * text_fuzzy, AV * words, AV * wantarray)
{
    int i;
    int n_words;
    int nearest;
    candidate_t first = {0};
    candidate_t * last;

    TEXT_FUZZY (begin_scanning (text_fuzzy));

    if (wantarray) {
	last = & first;
    }

    nearest = -1;

    n_words = av_len (words) + 1;

    /* Check for empty array. */

    if (n_words == 0) {
        return -1;
    }

    for (i = 0; i < n_words; i++) {
        SV * word;
        word = * av_fetch (words, i, 0);
        sv_to_text_fuzzy_string (word, text_fuzzy);
        TEXT_FUZZY (compare_single (text_fuzzy));
        if (text_fuzzy->found) {
            nearest = i;
	    if (wantarray) {
		candidate_t * c;
		get_memory (c, 1, candidate_t);
		c->distance = text_fuzzy->distance;
		c->offset = i;
		c->next = 0;
		last->next = c;
		last = c;
	    }
	    else {
		if (text_fuzzy->distance == 0) {
		    /* Stop the search if there is an exact
		       match. Note that "no_exact" is checked in
		       "compare_single", so we don't need to check it
		       here. */
		    break;
		}
	    }
	}
    }
    text_fuzzy->distance = text_fuzzy->max_distance;

    /* Set the maximum distance back to the user's value. */

    TEXT_FUZZY (end_scanning (text_fuzzy));

    /* If the user wants an array of values, we go through the linked
       list and collect them into "wantarray". Because we went through
       the list of words from the top to the bottom, gathering
       whatever was the minimum value at that point in the progress,
       our list may contain false hits which must be discarded. */

    if (wantarray) {
	candidate_t * c;
	last = first.next;
	while (last) {
	    c = last;

	    /* Set "last" to the next one here so that we do not
	       access freed memory. */
	    last = last->next;

	    /* Some of the entries might be things which had a lower
	       distance initially, but then were beaten by later
	       entries, so here we check that the entry actually does
	       have the lowest distance, and only if so do we keep
	       it. */

	    if (c->distance == text_fuzzy->distance) {
		SV * offset;

		offset = newSViv (c->offset);
		av_push (wantarray, offset);
	    }
	    Safefree (c);
	    text_fuzzy->n_mallocs--;
	}
    }
    return nearest;
}


/* Free the memory allocated to "text_fuzzy" and check that there has
   not been a memory leak. */

static int text_fuzzy_free (text_fuzzy_t * text_fuzzy)
{
    if (text_fuzzy->b.unicode) {
	Safefree (text_fuzzy->b.unicode);
	text_fuzzy->n_mallocs--;
    }

    /* See the comments in "text-fuzzy.c.in" about why this is
       necessary. */

    TEXT_FUZZY (free_memory (text_fuzzy));

    if (text_fuzzy->unicode) {
        Safefree (text_fuzzy->text.unicode);
        text_fuzzy->n_mallocs--;
    }

    Safefree (text_fuzzy->text.text);
    text_fuzzy->n_mallocs--;

    if (text_fuzzy->n_mallocs != 1) {
        warn ("memory leak: n_mallocs %d != 1", text_fuzzy->n_mallocs);
    }
    Safefree (text_fuzzy);

    return 0;
}

