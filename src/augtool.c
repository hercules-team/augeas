/*
 * augtool.c: 
 *
 * Copyright (C) 2007 Red Hat Inc.
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

#include "augeas.h"
#include "internal.h"

#include <readline/readline.h>
#include <readline/history.h>

struct command {
    const char *name;
    int nargs;
    void(*handler) (char *args[]);
};

static struct command commands[];

static void ls(char *args[]) {
    int cnt;
    const char *path = args[0];
    const char **paths;

    cnt = aug_ls(path, NULL, 0);
    paths = alloca(cnt * sizeof(char *));

    aug_ls(path, paths, cnt);
    for (int i=0; i < cnt; i++) {
        const char *val = aug_lookup(paths[i]);
        if (val == NULL)
            val = "(none)";
        printf("%s = %s\n", paths[i] + strlen(path), val);
    }
}

static void rm(char *args[]) {
    int cnt;
    const char *path = args[0];
    printf("rm : %s", path);
    cnt = aug_rm(path);
    printf(" %d\n", cnt);
}

static void set(char *args[]) {
    const char *path = args[0];
    const char *val = args[1];
    int r;

    r = aug_set(path, val);
    if (r == -1)
        printf ("Failed\n");
}

static void get(char *args[]) {
    const char *path = args[0];
    const char *val;

    printf("%s", path);
    val = aug_lookup(path);
    if (val == NULL)
        val = "(none)";
    printf(" = %s\n", val);
}

static void dump(ATTRIBUTE_UNUSED char *args[]) {
    aug_dump(stdout);
}

static int chk_args(const char *cmd, int n, char *args[]) {
    for (int i=0; i<n; i++) {
        if (strlen(args[i]) == 0) {
            fprintf(stderr, "Not enough arguments for %s\n", cmd);
            return -1;
        }
    }
    return 0;
}

static char *nexttoken(char **line) {
    char *r, *s;

    s = *line;

    while (*s && isblank(*s)) s+= 1;
    r = s;
    while (*s && !isblank(*s)) s+= 1;
    if (*s)
        *s++ = '\0';
    *line = s;
    return r;
}

static char *parseline(char *line, char *args[], int argc) {
    char *cmd;

    line = strdup(line);

    cmd = nexttoken(&line);
    
    for (int i=0; i < argc; i++) {
        args[i] = nexttoken(&line);
    }

    return cmd;
}

static struct command commands[] = {
    { "ls",  1, ls },
    { "rm",  1, rm },
    { "set", 2, set },
    { "get", 1, get },
    { "dump", 0, dump },
    { NULL, -1, NULL }
};

int main(void) {
    char *line;
    char *cmd, *args[3];
    struct command *c;
    int r;

    aug_init();

    while (1) {
        line = readline("augtool> ");
        if (line == NULL)
            return 0;
        r = 0;
        cmd = parseline(line, args, 3);
        
        if (STREQ("exit", cmd) || STREQ("quit", cmd)) {
            return 0;
        }
        for (c = commands; c->name; c++) {
            if (STREQ(cmd, c->name))
                break;
        }
        if (c->name) {
            r = chk_args(cmd, c->nargs, args);
            if (r == 0) {
                (*c->handler)(args);
                add_history(line);
            }
        } else {
            fprintf(stderr, "Unknown command '%s'\n", cmd);
        }
    }
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
