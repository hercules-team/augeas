#include <config.h>
#include "augeas.h"

#include <stdlib.h>
#include <stdio.h>

const char *abs_top_srcdir;
const char *abs_top_builddir;
char *root = NULL, *src_root = NULL, *lensdir = NULL;
struct augeas *aug = NULL;


#define die(msg)                                                    \
    do {                                                            \
        fprintf(stderr, "%s:%d: Fatal error: %s\n", __FILE__, __LINE__, msg); \
        exit(EXIT_FAILURE);                                         \
    } while(0)


int main (void)
{
  abs_top_srcdir = getenv("abs_top_srcdir");
  if (abs_top_srcdir == NULL)
    die("env var abs_top_srcdir must be set");

  abs_top_builddir = getenv("abs_top_builddir");
  if (abs_top_builddir == NULL)
    die("env var abs_top_builddir must be set");

  if (asprintf(&src_root, "%s/tests/root", abs_top_srcdir) < 0) {
    die("failed to set src_root");
  }

  if (asprintf(&lensdir, "%s/lenses", abs_top_srcdir) < 0)
    die("asprintf lensdir failed");


  aug = aug_init (src_root, lensdir, AUG_NO_STDINC);
  if (!aug) { perror ("aug_init"); exit (1); }
  aug_close (aug);

  free(root);
  free(src_root);
  free(lensdir);
  return 0;
}
