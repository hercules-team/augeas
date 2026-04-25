/*
 * Copyright (C) 2022 George Hansper <george@hansper.id.au>
 * -----------------------------------------------------------------------
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
 * Author: George Hansper <george@hansper.id.au>
 * -----------------------------------------------------------------------
 *
 Have:
     /head/label_a[pos1]/mid/label_b[pos2]/tail  value_a1_b1
     /head/label_a[pos3]/mid/label_b[pos4]/tail  value_a2_b1

 +-------------------------------------------------------+
 | augeas_path_value                                     |
 |   path = "/head/label_a[pos1]/mid/label_b[pos2]/tail" |
 |   value = "value_a1_b1"                               |
 |   value_qq = "'value_a1_b1'" or "\"value_a1_b1\""     |
 |   segments --.                                        |
 +---------------\---------------------------------------+
                  \
 +----------------------------------------+     +--------------------------------------------+
 | path_segment                           |     | path_segment                               |
 |   head = "/head/label_a"               |     |   head = "/head/label_a[pos1]/mid/label_b" |
 |   segment = "/head/label_a"            |     |   seqment = "/mid/label_b"                 |
 |   simplified_tail = "mid/label_b/tail" |     |   simplified_tail = "tail"                 |
 |   position = (int) pos1                |     |   position = (int) pos2                    |
 |   next ------------------------------------->|   next --> NULL                            |
 |   group --.                            |     |   group --.                                |
 +------------\---------------------------+     +------------\-------------------------------+
               \                                              \
 +-----------------------------+                +--------------------------------------------+
 | group                       |                | group                                      |
 |   head = "/head/label_a"    |                |   head = "/head/label_a[pos1]/mid/label_b" |
 |   max_position              |                |   max_position                             |
 |   chosen_tail[]             |                |   chosen_tail[]                            |   array of *tail, index is position
 |   tails_at_position[]----------------.       |   tails_at_position[]----------------.     |   array of *tail_stub lists, index is position
 |   all_tails ---.            |         \      |   all_tails ---.                      \    |   linked list, unique to group
 +-----------------\-----------+          \     +-----------------\----------------------\---+
                    \                      \                       \                      \
 +------------------------------------+     \   +------------------------------------+     \
 | tail                               |-+   |   | tail                               |-+   |
 |   simple_tail = "mid/label_b/tail" | |   |   |   simple_tail = "tail"             | |   |
 |   value = "value_a1_b1"            | |   |   |   value = "value_a1_b1"            | |   |
 |   next (next in all_tails list)    | |   |   |   next (next in all_tails list)    | |   |
 |   tail_value_found                 | |   |   |   tail_value_found                 | |   |     count of matching tail+value
 |   tail_value_found_map[]           | |   |   |   tail_value_found_map[]           | |   |     per-position count of tail+value
 |   tail_found_map[]                 | |   |   |   tail_found_map[]                 | |   |     per-position count of matching tail
 +------------------------------------+ |   |   +------------------------------------+ |   |
   +------------------------------------+   |     +------------------------------------+   |
   (linked-list)       ^                    |     (linked-list)      ^                     |
                       |                    |                        |                     |
                       |        .-----------'                        |        .------------'
                       |        |                                    |        |
                       |        |                                    |        |
                       |        v                                    |        v
 +---------------------|--------------+        +---------------------|--------------+
 | tail_stub           |              |-+      | tail_stub           |              |-+
 |   *tail    (ptr) ---'              | |      |   *tail    (ptr) ---'              | |
 |   next (in order of appearance)    | |      |   next (in order of appearance)    | |
 +------------------------------------+ |      +------------------------------------+ |
   +------------------------------------+        +------------------------------------+
   (linked-list)                                 (linked-list)

all_tails is a linked-list of (struct tail), in no particular order, where the combination of tail+value is unique in this list

all_tails list is unique to a group, and the (struct tail) records are not shared outside the group
The (struct tail) records are shared within the group, across all [123] positions

Each (struct_tail) contains three counters:
* tail_value_found
    This is the number of times this tail+value combination appears within the group
    If this counter >1, this indicates a duplicate tail+value, ie two (or more) identical entries within the group
* tail_value_found_map
    This is similar to tail_value_found, but there is an individiual counter for each position within the group
* tail_found_map
    This is the number of times this tail (regardless of value) appears for each position within the group

There is a (struct tail_stub) record for _every_ tail that we find for this group, including duplicates

The (struct group) record keeps an array tails_at_position[] which is indexed by position
Each array-element points to a linked-list of tail_stub records, which contain
a pointer to a (struct tail) record from the all_tails linked list
The tails_at_position[position] linked-list give us a complete list of all the tail+value records
for this position in the group, in their original order of appearance
*/

/* all_tails record */
struct tail {
  char         *simple_tail;
  char         *value;
  char         *value_qq;               /* The value, quoted and escaped as-needed */
  char         *value_re;               /* The value expressed as a regular-expression, long enough to uniquely identify the value */
  struct tail  *next;                   /* next all_tails record */
  unsigned int  tail_value_found;       /* number of times we have seen this tail+value within this group, (used by 1st preference) */
  unsigned int *tail_value_found_map;   /* Array, indexed by position, number of times we have seen this tail+value within this group (used by 3rd preference) */
  unsigned int *tail_found_map;         /* Array, indexed by position, number of times we have seen this tail (regardless of value) within this group (used by 2nd preference) */
};

/* Linked list of pointers into the all_tails list
 * One such linked-list exists for each position within the group
 * Each list begins at group->tails_at_position[position]
 */
struct tail_stub {
  struct tail      *tail;
  struct tail_stub *next;
};

/* subgroup exists only to analyse the 3rd preference
 * it maps a subset of positions within a group
 */
struct subgroup {
  struct tail      *first_tail;
  unsigned int     *matching_positions;   /* zero-terminated array of positions with the same first_tail */
  struct subgroup  *next;
};

typedef enum {
    NOT_DONE=0,
    FIRST_TAIL=1,                         /* 1st preference */
    CHOSEN_TAIL_START=4,                  /* 2nd preference - unique tail found for this position */
    CHOSEN_TAIL_WIP=5,                    /* 2nd preference */
    CHOSEN_TAIL_DONE=6,                   /* 2nd preference */
    CHOSEN_TAIL_PLUS_FIRST_TAIL_START=8,  /* 3rd preference - unique tail found in a subgroup with a common first_tail */
    CHOSEN_TAIL_PLUS_FIRST_TAIL_WIP=9,    /* 3rd preference */
    CHOSEN_TAIL_PLUS_FIRST_TAIL_DONE=10,  /* 3rd preference */
    FIRST_TAIL_PLUS_POSITION=12,          /* Fallback - use first_tail subgroup and append a position */
    NO_CHILD_NODES=16,                    /* /head/123 with no child nodes */
} chosen_tail_state_t;

struct group {
  char                   *head;
  struct tail            *all_tails;             /* Linked list */
  struct tail_stub      **tails_at_position;     /* array of linked-lists, index is position */
  struct tail           **chosen_tail;           /* array of (struct tail)      pointers, index is position */
  struct tail_stub      **first_tail;            /* array of (struct tail_stub) pointers, index is position */
  unsigned int            max_position;          /* highest position seen for this group */
  unsigned int            position_array_size;   /* array size for arrays indexed by position, >= max_position+1, used for malloc() */
  chosen_tail_state_t    *chosen_tail_state;     /* array, index is position */
  struct subgroup        *subgroups;             /* Linked list, subgroups based on common first-tail - used only for 3rd preference and fallback */
  unsigned int           *subgroup_position;     /* array, position within subgroup for this position - used only for fallback */
  /* For --pretty */
  unsigned int           *pretty_width_ct;      /* array, index is position, value width to use for --pretty */
  /* For --regexp */
  unsigned int           *re_width_ct;           /* array, index is position, matching width to use for --regexp */
  unsigned int           *re_width_ft;           /* array, index is position, matching width to use for --regexp */
};

struct path_segment {
  char                *head;
  char                *segment;
  char                *simplified_tail;
  unsigned int         position;
  struct group        *group;
  struct path_segment *next;
};

/* Results of aug_match() and aug_get() - one record per path returned by aug_match() */
struct augeas_path_value {
  char *path;
  char *value;
  char *value_qq;            /* value in quotes - used in path-expressions, and as the value being assigned */
  /* result of split_path() */
  struct path_segment *segments;
};
