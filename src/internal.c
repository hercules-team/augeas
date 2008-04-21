/*
 * internal.c: internal data structures and helpers
 *
 * Copyright (C) 2007, 2008 Red Hat Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: David Lutterkort <dlutter@redhat.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>

#include "internal.h"

int pathjoin(char **path, int nseg, ...) {
    va_list ap;

    va_start(ap, nseg);
    for (int i=0; i < nseg; i++) {
        const char *seg = va_arg(ap, const char *);
        if (seg == NULL)
            seg = "()";
        int len = strlen(seg) + 1;

        if (*path != NULL) {
            len += strlen(*path) + 1;
            REALLOC(*path, len);
            if (*path == NULL)
                return -1;
            if (strlen(*path) == 0 || (*path)[strlen(*path)-1] != SEP)
                strcat(*path, "/");
            if (seg[0] == SEP)
                seg += 1;
            strcat(*path, seg);
        } else {
            *path = malloc(len);
            strcpy(*path, seg);
        }
    }
    va_end(ap);
    return 0;
}

const char* aug_read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    struct stat st;
    char *result;

    if (!fp)
        return NULL;

    if (fstat(fileno(fp), &st) < 0) {
        fclose(fp);
        return NULL;
    }

    CALLOC(result, st.st_size + 1);
    if (result == NULL) {
        fclose(fp);
        return NULL;
    }

    if (st.st_size) {
        if (fread(result, st.st_size, 1, fp) != 1) {
            fclose(fp);
            free(result);
            return NULL;
        }
    }
    result[st.st_size] = '\0';

    fclose(fp);
    return result;
}

/*
 * Escape/unescape of string literals
 */
static const char *const escape_chars    = "\a\b\t\n\v\f\r\\";
static const char *const escape_names = "abtnvfr\\";

char *unescape(const char *s, int len) {
    size_t size;
    const char *n;
    char *result, *t;
    int i;

    if (len > strlen(s))
        len = strlen(s);

    size = 0;
    for (i=0; i < len; i++, size++)
        if (s[i] == '\\' && strchr(escape_names, s[i+1]) != NULL) {
            i += 1;
        }

    CALLOC(result, size + 1);
    for (i = 0, t = result; i < len; i++, size++) {
        if (s[i] == '\\' && (n = strchr(escape_names, s[i+1])) != NULL) {
            *t++ = escape_chars[n - escape_names];
            i += 1;
        } else {
            *t++ = s[i];
        }
    }
    return result;
}

char *escape(const char *text, int cnt) {

    int len = 0;
    char *esc = NULL, *e;

    if (cnt < 0 || cnt > strlen(text))
        cnt = strlen(text);

    for (int i=0; i < cnt; i++) {
        if (strchr(escape_chars, text[i]) != NULL)
            len += 2;  /* Escaped as '\x' */
        else if (! isprint(text[i]))
            len += 4;  /* Escaped as '\ooo' */
        else
            len += 1;
    }
    CALLOC(esc, len+1);
    e = esc;
    for (int i=0; i < cnt; i++) {
        char *p;
        if ((p = strchr(escape_chars, text[i])) != NULL) {
            *e++ = '\\';
            *e++ = escape_names[p - escape_chars];
        } else if (! isprint(text[i])) {
            sprintf(e, "\\%03o", text[i]);
        } else {
            *e++ = text[i];
        }
    }
    return esc;
}

int print_chars(FILE *out, const char *text, int cnt) {
    int total = 0;
    char *esc;

    if (text == NULL) {
        fprintf(out, "nil");
        return 3;
    }
    if (cnt < 0)
        cnt = strlen(text);

    esc = escape(text, cnt);
    total = strlen(esc);
    if (out != NULL)
        fprintf(out, esc);
    free(esc);

    return total;
}

char *format_pos(const char *text, int pos) {
    char *buf;
    size_t size;
    FILE *stream = open_memstream(&buf, &size);
    print_pos(stream, text, pos);
    fclose(stream);
    return buf;
}

void print_pos(FILE *out, const char *text, int pos) {
    static const int window = 28;
    int before = pos;
    int total;
    if (before > window)
        before = window;
    total = print_chars(NULL, text + pos - before, before);
    if (total < window)
        fprintf(out, "%*s", window - total, "<");
    print_chars(out, text + pos - before, before);
    fprintf(out, "|=|");
    total = print_chars(out, text + pos, window);
    fprintf(out, "%*s\n", window - total, ">");
}



/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
