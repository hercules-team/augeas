/*
 * auglua.c: lua integration for augtool
 *
 * Copyright (C) 2015 Raphaël Pinson
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
 * Author: Raphaël Pinson <raphael.pinson@camptocamp.com>
 */

#include "internal.h"
#include "augeas.h"
#include "auglua.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdbool.h>

typedef struct LuaAug {
  augeas *aug;
} LuaAug;

static const char Key = 'k';

static augeas *checkaug(lua_State *L) {
  lua_pushlightuserdata(L, (void *)&Key);
  lua_gettable(L, LUA_REGISTRYINDEX);
  augeas *b = (augeas *)lua_touserdata(L, -1); // Convert value
  return b;
}

static void lua_checkargs(lua_State *L, const char *name, int arity) {
  int n = lua_gettop(L);
  char msg[1024];
  if (n != arity) {
      snprintf(msg, sizeof(msg), "Wrong number of arguments for '%s'", name);
      lua_pushstring(L, msg);
      lua_error(L);
  }
}

static int lua_pusherror(lua_State *L) {
  augeas *aug = checkaug(L);
  lua_pushstring(L, aug_error_message(aug));
  lua_error(L);
  return 1;
}

static int lua_aug_get(lua_State *L) {
  int r;
  const char *path, *value;

  /* get number of arguments */
  lua_checkargs(L, "aug_get", 1);

  path = luaL_checkstring(L, 1);

  augeas *aug = checkaug(L);
  r = aug_get(aug, path, &value);
  if (r < 0)
      return lua_pusherror(L);
  lua_pushstring(L, value);

  /* return the number of results */
  return 1;
}

static int lua_aug_label(lua_State *L) {
  int r;
  const char *path, *value;

  lua_checkargs(L, "aug_label", 1);

  path = luaL_checkstring(L, 1);
  // TODO: check string really

  augeas *aug = checkaug(L);
  r = aug_label(aug, path, &value);
  if (r < 0)
      return lua_pusherror(L);
  lua_pushstring(L, value);

  /* return the number of results */
  return 1;
}

static int lua_aug_set(lua_State *L) {
  int r;
  const char *path, *value;

  lua_checkargs(L, "aug_set", 2);

  path = luaL_checkstring(L, 1);
  // TODO: check string really
  value = luaL_checkstring(L, 2);

  augeas *aug = checkaug(L);
  r = aug_set(aug, path, value);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_setm(lua_State *L) {
  int r;
  const char *base, *sub, *value;

  lua_checkargs(L, "aug_setm", 3);

  base = luaL_checkstring(L, 1);
  // TODO: check string really
  sub = luaL_checkstring(L, 2);
  value = luaL_checkstring(L, 3);

  augeas *aug = checkaug(L);
  r = aug_setm(aug, base, sub, value);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_insert(lua_State *L) {
  int r;
  const char *path, *label;
  int before;

  lua_checkargs(L, "aug_insert", 3);

  path = luaL_checkstring(L, 1);
  // TODO: check string really
  label = luaL_checkstring(L, 2);
  before = lua_toboolean(L, 3);

  augeas *aug = checkaug(L);
  r = aug_insert(aug, path, label, before);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_rm(lua_State *L) {
  int r;
  const char *path;

  lua_checkargs(L, "aug_rm", 1);

  path = luaL_checkstring(L, 1);
  // TODO: check string really

  augeas *aug = checkaug(L);
  r = aug_rm(aug, path);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_mv(lua_State *L) {
  int r;
  const char *path, *new_path;

  lua_checkargs(L, "aug_mv", 2);

  path = luaL_checkstring(L, 1);
  // TODO: check string really
  new_path = luaL_checkstring(L, 2);

  augeas *aug = checkaug(L);
  r = aug_mv(aug, path, new_path);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_cp(lua_State *L) {
  int r;
  const char *path, *new_path;

  lua_checkargs(L, "aug_cp", 2);

  path = luaL_checkstring(L, 1);
  // TODO: check string really
  new_path = luaL_checkstring(L, 2);

  augeas *aug = checkaug(L);
  r = aug_cp(aug, path, new_path);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_rename(lua_State *L) {
  int r;
  const char *path, *label;

  lua_checkargs(L, "aug_rename", 2);

  path = luaL_checkstring(L, 1);
  // TODO: check string really
  label = luaL_checkstring(L, 2);

  augeas *aug = checkaug(L);
  r = aug_rename(aug, path, label);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_clear(lua_State *L) {
  int r;
  const char *path;

  lua_checkargs(L, "aug_clear", 1);

  path = luaL_checkstring(L, 1);

  augeas *aug = checkaug(L);
  r = aug_set(aug, path, NULL);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_clearm(lua_State *L) {
  int r;
  const char *base, *sub;

  lua_checkargs(L, "aug_clearm", 2);

  base = luaL_checkstring(L, 1);
  sub = luaL_checkstring(L, 2);

  augeas *aug = checkaug(L);
  r = aug_setm(aug, base, sub, NULL);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_touch(lua_State *L) {
  int r;
  const char *path;

  lua_checkargs(L, "aug_touch", 1);

  path = luaL_checkstring(L, 1);

  augeas *aug = checkaug(L);
  r = aug_match(aug, path, NULL);
  if (r == 0) {
      r = aug_set(aug, path, NULL);
      if (r < 0)
          return lua_pusherror(L);
  }

  /* return the number of results */
  return 0;
}

static int lua_aug_matches(lua_State *L) {
  int r;
  const char *path;

  lua_checkargs(L, "aug_matches", 1);

  path = luaL_checkstring(L, 1);
  // TODO: check string really

  augeas *aug = checkaug(L);
  r = aug_match(aug, path, NULL);
  if (r < 0)
      return lua_pusherror(L);

  lua_pushinteger(L, r);
  /* return the number of results */
  return 1;
}

static int lua_aug_match(lua_State *L) {
  int r, i;
  const char *path;
  char **match = NULL;

  lua_checkargs(L, "aug_match", 1);

  path = luaL_checkstring(L, 1);
  // TODO: check string really

  augeas *aug = checkaug(L);
  r = aug_match(aug, path, &match);
  if (r < 0)
      return lua_pusherror(L);

  lua_newtable(L);
  for (i = 0; i < r; i++) {
      lua_pushnumber(L, i+1);
      lua_pushstring(L, match[i]);
      lua_settable(L, -3);
      free(match[i]);
  }
  free(match);
  lua_pushinteger(L, r);

  /* return the number of results */
  return 2;
}

static int lua_aug_defvar(lua_State *L) {
  int r;
  const char *name, *expr;

  lua_checkargs(L, "aug_defvar", 2);

  name = luaL_checkstring(L, 1);
  // TODO: check string really
  expr = luaL_checkstring(L, 2);

  augeas *aug = checkaug(L);
  r = aug_defvar(aug, name, expr);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_defnode(lua_State *L) {
  int r;
  const char *name, *expr, *value = NULL;

  lua_checkargs(L, "aug_defnode", 3);

  name = luaL_checkstring(L, 1);
  // TODO: check string really
  expr = luaL_checkstring(L, 2);
  value = lua_tostring(L, 3);

  augeas *aug = checkaug(L);
  r = aug_defnode(aug, name, expr, value, NULL);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_save(lua_State *L) {
  int r;

  lua_checkargs(L, "aug_save", 0);

  augeas *aug = checkaug(L);
  r = aug_save(aug);
  if (r == -1) {
      lua_pushstring(L, "saving failed (run 'errors' for details)");
      lua_error(L);
  } else {
      r = aug_match(aug, "/augeas/events/saved", NULL);
      if (r > 0)
          printf("Saved %d file(s)\n", r);
  }

  /* return the number of results */
  return 0;
}

static int lua_aug_load(lua_State *L) {
  int r;

  lua_checkargs(L, "aug_load", 0);

  augeas *aug = checkaug(L);
  r = aug_load(aug);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_text_store(lua_State *L) {
  int r;
  const char *lens, *node, *path;

  lua_checkargs(L, "aug_text_store", 3);

  lens = luaL_checkstring(L, 1);
  node = luaL_checkstring(L, 2);
  path = luaL_checkstring(L, 3);

  augeas *aug = checkaug(L);
  r = aug_text_store(aug, lens, node, path);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_text_retrieve(lua_State *L) {
  int r;
  const char *lens, *node_in, *path, *node_out;

  lua_checkargs(L, "aug_text_retrieve", 4);

  lens = luaL_checkstring(L, 1);
  node_in = luaL_checkstring(L, 2);
  path = luaL_checkstring(L, 3);
  node_out = luaL_checkstring(L, 4);

  augeas *aug = checkaug(L);
  r = aug_text_retrieve(aug, lens, node_in, path, node_out);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

static int lua_aug_transform(lua_State *L) {
  int r;
  const char *lens, *file;
  bool excl;

  lua_checkargs(L, "aug_transform", 3);

  lens = luaL_checkstring(L, 1);
  file = luaL_checkstring(L, 2);
  excl = lua_toboolean(L, 3);

  augeas *aug = checkaug(L);
  r = aug_transform(aug, lens, file, excl);
  if (r < 0)
      return lua_pusherror(L);

  /* return the number of results */
  return 0;
}

struct lua_State *setup_lua(augeas *a) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
  
    lua_pushlightuserdata(L, (void *)&Key);
    lua_pushlightuserdata(L, (void *)a); // Push pointer
    lua_settable(L, LUA_REGISTRYINDEX);

    static const luaL_Reg augfuncs[] = {
        { "get", lua_aug_get },
        { "label", lua_aug_label },
        { "set", lua_aug_set },
        { "setm", lua_aug_setm },
        //{ "span", lua_aug_span },
        { "insert", lua_aug_insert },
        { "ins", lua_aug_insert }, // alias
        { "rm", lua_aug_rm },
        { "mv", lua_aug_mv },
        { "move", lua_aug_mv }, // alias
        { "cp", lua_aug_cp },
        { "copy", lua_aug_cp }, // alias
        { "rename", lua_aug_rename },
        { "clear", lua_aug_clear },
        { "clearm", lua_aug_clearm },
        { "touch", lua_aug_touch },
        { "matches", lua_aug_matches },
        { "match", lua_aug_match },
        { "defvar", lua_aug_defvar },
        { "defnode", lua_aug_defnode },
        { "save", lua_aug_save },
        { "load", lua_aug_load },
        { "text_store", lua_aug_text_store },
        { "store", lua_aug_text_store }, // alias
        { "text_retrieve", lua_aug_text_retrieve },
        { "retrieve", lua_aug_text_retrieve }, // alias
        //{ "escape_name", lua_aug_escape_name },
        { "transform", lua_aug_transform },
        //{ "print", lua_aug_print },
        //{ "to_xml", lua_aug_to_xml },
        //{ "errors", lua_aug_errors },
        { NULL, NULL }
    };

    luaL_newlib(L, augfuncs);
    lua_setglobal(L, "aug");

    return L;
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

