/*
 * internal.c: internal data structures and helpers
 *
 * Copyright (C) 2007-2011 David Lutterkort
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

#include <config.h>

#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <locale.h>

#include "internal.h"
#include "memory.h"
#include "fa.h"

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

/* Cap file reads somwhat arbitrarily at 32 MB */
#define MAX_READ_LEN (32*1024*1024)

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
            if (REALLOC_N(*path, len) == -1) {
                FREE(*path);
                return -1;
            }
            if (strlen(*path) == 0 || (*path)[strlen(*path)-1] != SEP)
                strcat(*path, "/");
            if (seg[0] == SEP)
                seg += 1;
            strcat(*path, seg);
        } else {
            if ((*path = malloc(len)) == NULL)
                return -1;
            strcpy(*path, seg);
        }
    }
    va_end(ap);
    return 0;
}

/* Like gnulib's fread_file, but read no more than the specified maximum
   number of bytes.  If the length of the input is <= max_len, and
   upon error while reading that data, it works just like fread_file.

   Taken verbatim from libvirt's util.c
*/

static char *
fread_file_lim (FILE *stream, size_t max_len, size_t *length)
{
    char *buf = NULL;
    size_t alloc = 0;
    size_t size = 0;
    int save_errno;

    for (;;) {
        size_t count;
        size_t requested;

        if (size + BUFSIZ + 1 > alloc) {
            char *new_buf;

            alloc += alloc / 2;
            if (alloc < size + BUFSIZ + 1)
                alloc = size + BUFSIZ + 1;

            new_buf = realloc (buf, alloc);
            if (!new_buf) {
                save_errno = errno;
                break;
            }

            buf = new_buf;
        }

        /* Ensure that (size + requested <= max_len); */
        requested = MIN (size < max_len ? max_len - size : 0,
                         alloc - size - 1);
        count = fread (buf + size, 1, requested, stream);
        size += count;

        if (count != requested || requested == 0) {
            save_errno = errno;
            if (ferror (stream))
                break;
            buf[size] = '\0';
            *length = size;
            return buf;
        }
    }

    free (buf);
    errno = save_errno;
    return NULL;
}

char* xread_file(const char *path) {
    FILE *fp = fopen(path, "r");
    char *result;
    size_t len;

    if (!fp)
        return NULL;

    result = fread_file_lim(fp, MAX_READ_LEN, &len);
    fclose (fp);

    if (result != NULL
        && len <= MAX_READ_LEN
        && (int) len == len)
        return result;

    free(result);
    return NULL;
}

/*
 * Escape/unescape of string literals
 */
static const char *const escape_chars = "\a\b\t\n\v\f\r";
static const char *const escape_names = "abtnvfr";

char *unescape(const char *s, int len, const char *extra) {
    size_t size;
    const char *n;
    char *result, *t;
    int i;

    if (len < 0 || len > strlen(s))
        len = strlen(s);

    size = 0;
    for (i=0; i < len; i++, size++) {
        if (s[i] == '\\' && strchr(escape_names, s[i+1])) {
            i += 1;
        } else if (s[i] == '\\' && extra && strchr(extra, s[i+1])) {
            i += 1;
        }
    }

    if (ALLOC_N(result, size + 1) < 0)
        return NULL;

    for (i = 0, t = result; i < len; i++, size++) {
        if (s[i] == '\\' && (n = strchr(escape_names, s[i+1])) != NULL) {
            *t++ = escape_chars[n - escape_names];
            i += 1;
        } else if (s[i] == '\\' && extra && strchr(extra, s[i+1]) != NULL) {
            *t++ = s[i+1];
            i += 1;
        } else {
            *t++ = s[i];
        }
    }
    return result;
}

char *escape(const char *text, int cnt, const char *extra) {

    int len = 0;
    char *esc = NULL, *e;

    if (cnt < 0 || cnt > strlen(text))
        cnt = strlen(text);

    for (int i=0; i < cnt; i++) {
        if (text[i] && (strchr(escape_chars, text[i]) != NULL))
            len += 2;  /* Escaped as '\x' */
        else if (text[i] && extra && (strchr(extra, text[i]) != NULL))
            len += 2;  /* Escaped as '\x' */
        else if (! isprint(text[i]))
            len += 4;  /* Escaped as '\ooo' */
        else
            len += 1;
    }
    if (ALLOC_N(esc, len+1) < 0)
        return NULL;
    e = esc;
    for (int i=0; i < cnt; i++) {
        char *p;
        if (text[i] && ((p = strchr(escape_chars, text[i])) != NULL)) {
            *e++ = '\\';
            *e++ = escape_names[p - escape_chars];
        } else if (text[i] && extra && (strchr(extra, text[i]) != NULL)) {
            *e++ = '\\';
            *e++ = text[i];
        } else if (! isprint(text[i])) {
            sprintf(e, "\\%03o", (unsigned char) text[i]);
            e += 4;
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

    esc = escape(text, cnt, NULL);
    total = strlen(esc);
    if (out != NULL)
        fprintf(out, "%s", esc);
    free(esc);

    return total;
}

char *format_pos(const char *text, int pos) {
    static const int window = 28;
    char *buf = NULL, *left = NULL, *right = NULL;
    int before = pos;
    int llen, rlen;
    int r;

    if (before > window)
        before = window;
    left = escape(text + pos - before, before, NULL);
    if (left == NULL)
        goto done;
    right = escape(text + pos, window, NULL);
    if (right == NULL)
        goto done;

    llen = strlen(left);
    rlen = strlen(right);
    if (llen < window && rlen < window) {
        r = asprintf(&buf, "%*s%s|=|%s%-*s\n", window - llen, "<", left,
                     right, window - rlen, ">");
    } else if (strlen(left) < window) {
        r = asprintf(&buf, "%*s%s|=|%s>\n", window - llen, "<", left, right);
    } else if (strlen(right) < window) {
        r = asprintf(&buf, "<%s|=|%s%-*s\n", left, right, window - rlen, ">");
    } else {
        r = asprintf(&buf, "<%s|=|%s>\n", left, right);
    }
    if (r < 0) {
        buf = NULL;
    }

 done:
    free(left);
    free(right);
    return buf;
}

void print_pos(FILE *out, const char *text, int pos) {
    char *format = format_pos(text, pos);

    if (format != NULL) {
        fputs(format, out);
        FREE(format);
    }
}

int __aug_init_memstream(struct memstream *ms) {
    MEMZERO(ms, 1);
#if HAVE_OPEN_MEMSTREAM
    ms->stream = open_memstream(&(ms->buf), &(ms->size));
    return ms->stream == NULL ? -1 : 0;
#else
    ms->stream = tmpfile();
    if (ms->stream == NULL) {
        return -1;
    }
    return 0;
#endif
}

int __aug_close_memstream(struct memstream *ms) {
#if !HAVE_OPEN_MEMSTREAM
    rewind(ms->stream);
    ms->buf = fread_file_lim(ms->stream, MAX_READ_LEN, &(ms->size));
#endif
    if (fclose(ms->stream) == EOF) {
        FREE(ms->buf);
        ms->size = 0;
        return -1;
    }
    return 0;
}

char *path_expand(struct tree *tree, const char *ppath) {
    struct tree *siblings = tree->parent->children;

    char *path;
    const char *label;
    int cnt = 0, ind = 0, r;

    list_for_each(t, siblings) {
        if (streqv(t->label, tree->label)) {
            cnt += 1;
            if (t == tree)
                ind = cnt;
        }
    }

    if (ppath == NULL)
        ppath = "";

    if (tree == NULL)
        label = "(no_tree)";
    else if (tree->label == NULL)
        label = "(none)";
    else
        label = tree->label;

    if (cnt > 1) {
        r = asprintf(&path, "%s/%s[%d]", ppath, label, ind);
    } else {
        r = asprintf(&path, "%s/%s", ppath, label);
    }
    if (r == -1)
        return NULL;
    return path;
}

char *path_of_tree(struct tree *tree) {
    int depth, i;
    struct tree *t, **anc;
    char *path = NULL;

    for (t = tree, depth = 1; ! ROOT_P(t); depth++, t = t->parent);
    if (ALLOC_N(anc, depth) < 0)
        return NULL;

    for (t = tree, i = depth - 1; i >= 0; i--, t = t->parent)
        anc[i] = t;

    for (i = 0; i < depth; i++) {
        char *p = path_expand(anc[i], path);
        free(path);
        path = p;
    }
    FREE(anc);
    return path;
}

/* User-facing path cleaning */
static char *cleanstr(char *path, const char sep) {
    if (path == NULL || strlen(path) == 0)
        return path;
    char *e = path + strlen(path) - 1;
    while (e >= path && (*e == sep || isspace(*e)))
        *e-- = '\0';
    return path;
}

char *cleanpath(char *path) {
    if (path == NULL || strlen(path) == 0)
        return path;
    if (STREQ(path, "/"))
        return path;
    return cleanstr(path, SEP);
}

const char *xstrerror(int errnum, char *buf, size_t len) {
#ifdef HAVE_STRERROR_R
# ifdef __USE_GNU
    /* Annoying linux specific API contract */
    return strerror_r(errnum, buf, len);
# else
    strerror_r(errnum, buf, len);
    return buf;
# endif
#else
    int n = snprintf(buf, len, "errno=%d", errnum);
    return (0 < n && n < len
            ? buf : "internal error: buffer too small in xstrerror");
#endif
}

int xasprintf(char **strp, const char *format, ...) {
  va_list args;
  int result;

  va_start (args, format);
  result = vasprintf (strp, format, args);
  va_end (args);
  if (result < 0)
      *strp = NULL;
  return result;
}

/* From libvirt's src/xen/block_stats.c */
int xstrtoint64(char const *s, int base, int64_t *result) {
    long long int lli;
    char *p;

    errno = 0;
    lli = strtoll(s, &p, base);
    if (errno || !(*p == 0 || *p == '\n') || p == s || (int64_t) lli != lli)
        return -1;
    *result = lli;
    return 0;
}

void calc_line_ofs(const char *text, size_t pos, size_t *line, size_t *ofs) {
    *line = 1;
    *ofs = 0;
    for (const char *t = text; t < text + pos; t++) {
        *ofs += 1;
        if (*t == '\n') {
            *ofs = 0;
            *line += 1;
        }
    }
}

#if HAVE_USELOCALE
int regexp_c_locale(ATTRIBUTE_UNUSED char **u, ATTRIBUTE_UNUSED size_t *len) {
    /* On systems with uselocale, we are ok, since we make sure that we
     * switch to the "C" locale any time we enter through the public API
     */
    return 0;
}
#else
int regexp_c_locale(char **u, size_t *len) {
    /* Without uselocale, we need to expand character ranges */
    int r;
    char *s = *u;
    size_t s_len, u_len;
    if (len == NULL) {
        len = &u_len;
        s_len = strlen(s);
    } else {
        s_len = *len;
    }
    r = fa_expand_char_ranges(s, s_len, u, len);
    if (r != 0) {
        *u = s;
        *len = s_len;
    }
    if (r < 0)
        return -1;
    /* Syntax errors will be caught when the result is compiled */
    if (r > 0)
        return 0;
    free(s);
    return 1;
}
#endif

#if ENABLE_DEBUG
bool debugging(const char *category) {
    const char *debug = getenv("AUGEAS_DEBUG");
    const char *s;

    if (debug == NULL)
        return false;

    for (s = debug; s != NULL; ) {
        if (STREQLEN(s, category, strlen(category)))
            return true;
        s = strchr(s, ':');
        if (s != NULL)
            s+=1;
    }
    return false;
}

FILE *debug_fopen(const char *format, ...) {
    va_list ap;
    FILE *result = NULL;
    const char *dir;
    char *name = NULL, *path = NULL;
    int r;

    dir = getenv("AUGEAS_DEBUG_DIR");
    if (dir == NULL)
        goto error;

    va_start(ap, format);
    r = vasprintf(&name, format, ap);
    va_end(ap);
    if (r < 0)
        goto error;

    r = xasprintf(&path, "%s/%s", dir, name);
    if (r < 0)
        goto error;

    result = fopen(path, "w");

 error:
    free(name);
    free(path);
    return result;
}
#endif
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
