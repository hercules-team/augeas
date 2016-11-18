/*
--- Lua wrapper for Augeas library.
--
-- In general, the functions map straight to the C library. See the
-- descriptions below for details.
 */

/*
 * Copyright (C) 2010-2013 Natanael Copa <ncopa@alpinelinux.org>
 * Copyright (C) 2013-2016 Kaarle Ritvanen
 */

#include <assert.h>
#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "augeas.h"
#include "luamod.h"

#define LIBNAME "augeas"
#define PAUG_META "augeas"

#ifndef VERSION
#define VERSION "unknown"
#endif

#define LUA_FILEHANDLE	"FILE*"

#if LUA_VERSION_NUM < 502
#  define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
#  define luaL_setfuncs(L,l,n) (assert(n==0), luaL_register(L,NULL,l))
#else
static int luaL_typerror (lua_State *L, int narg, const char *tname)
{
	const char *msg = lua_pushfstring(L, "%s expected, got %s",
					  tname, luaL_typename(L, narg));
	return luaL_argerror(L, narg, msg);
}
#endif

struct aug_userdata {
	augeas *aug;
};

struct aug_flagmap {
	const char *name;
	int value;
};

struct aug_flagmap Taug_flagmap[] = {
	{ "none",		AUG_NONE },
	{ "save_backup",	AUG_SAVE_BACKUP },
	{ "save_newfile",	AUG_SAVE_NEWFILE },
	{ "typecheck",		AUG_TYPE_CHECK },
	{ "no_stdinc",		AUG_NO_STDINC },
	{ "save_noop",		AUG_SAVE_NOOP },
	{ "no_load",		AUG_NO_LOAD },
	{ "no_modl_autoload",	AUG_NO_MODL_AUTOLOAD },
	{ NULL, 0 }
};

static const char *get_opt_string_field(lua_State *L, int index,
				        const char *key, const char *def)
{
	const char *value;
	lua_getfield(L, index, key);
	value = luaL_optstring(L, -1, def);
	lua_pop(L, 1);
	return value;
}

static int get_boolean_field(lua_State *L, int index, const char *key)
{
	int value;
	lua_getfield(L, index, key);
	value = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return value;
}

static int pusherror(lua_State *L, augeas *aug, const char *info)
{
	lua_pushnil(L);
	if (info==NULL)
		lua_pushstring(L, aug_error_message(aug));
	else
		lua_pushfstring(L, "%s: %s", info, aug_error_message(aug));
	lua_pushinteger(L, aug_error(aug));
	return 3;
}

static int pushresult(lua_State *L, int i, augeas *aug, const char *info)
{
	if (i < 0)
		return pusherror(L, aug, info);
	lua_pushinteger(L, i);
	return 1;
}

static augeas *Paug_checkarg(lua_State *L, int index)
{
	struct aug_userdata *ud;
	luaL_checktype(L, index, LUA_TUSERDATA);
	ud = (struct aug_userdata *) luaL_checkudata(L, index, PAUG_META);
	if (ud == NULL)
		luaL_typerror(L, index, PAUG_META);
	return ud->aug;
}

static int Paug_init(lua_State *L)
{
	struct aug_userdata *ud;
	struct aug_flagmap *f;
	const char *root = NULL, *loadpath = NULL;
	int flags = 0;

	if (lua_istable(L, 1)) {
		root = get_opt_string_field(L, 1, "root", NULL);
		loadpath = get_opt_string_field(L, 1, "loadpath", NULL);
		for (f = Taug_flagmap; f->name != NULL; f++)
			if (get_boolean_field(L, 1, f->name))
				flags |= f->value;
	} else {
		root = luaL_optstring(L, 1, NULL);
		loadpath = luaL_optstring(L, 2, NULL);
		flags = luaL_optinteger(L, 3, AUG_NONE);
	}

	ud = (struct aug_userdata *) lua_newuserdata(L, sizeof(struct aug_userdata));
	luaL_getmetatable(L, PAUG_META);
	lua_setmetatable(L, -2);

	ud->aug = aug_init(root, loadpath, flags);
	if (ud->aug == NULL)
		luaL_error(L, "aug_init failed");
	return 1;
}

static int Paug_defvar(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	const char *name = luaL_checkstring(L, 2);
	const char *expr = luaL_checkstring(L, 3);
	return pushresult(L, aug_defvar(a, name, expr), a, NULL);
}

static int Paug_defnode(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	const char *name = luaL_checkstring(L, 2);
	const char *expr = luaL_checkstring(L, 3);
	const char *value = luaL_checkstring(L, 4);
	return pushresult(L, aug_defnode(a, name, expr, value, NULL), a, NULL);
}

static int Paug_close(lua_State *L)
{
	struct aug_userdata *ud;
	luaL_checktype(L, 1, LUA_TUSERDATA);
	ud = (struct aug_userdata *) luaL_checkudata(L, 1, PAUG_META);

	if (ud && ud->aug) {
		aug_close(ud->aug);
		ud->aug = NULL;
	}
	return 0;
}

static int Paug_get(lua_State *L)
{
	augeas *a;
	const char *path;
	const char *value = NULL;
	int r;

	a = Paug_checkarg(L, 1);
	path = luaL_checkstring(L, 2);
	r = aug_get(a, path, &value);
	if (r < 0)
		return pusherror(L, a, path);
	lua_pushstring(L, value);
	return 1;
}

static int Paug_set(lua_State *L)
{
	augeas *a;
	const char *path, *value;

	a = Paug_checkarg(L, 1);
	path = luaL_checkstring(L, 2);
	value = lua_isnil(L, 3) ? NULL : luaL_checkstring(L, 3);
	return pushresult(L, aug_set(a, path, value), a, path);
}

static int Paug_setm(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	const char *base = luaL_checkstring(L, 2);
	const char *sub= luaL_checkstring(L, 3);
	const char *value = lua_isnil(L, 4) ? NULL : luaL_checkstring(L, 4);
	return pushresult(L, aug_setm(a, base, sub, value), a, NULL);
}

static int Paug_insert(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	const char *path = luaL_checkstring(L, 2);
	const char *label = luaL_checkstring(L, 3);
	int before = lua_toboolean(L, 4);
	return pushresult(L, aug_insert(a, path, label, before), a, path);
}

static int Paug_rm(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	const char *path = luaL_checkstring(L, 2);
	return pushresult(L, aug_rm(a, path), a, NULL);
}

static int Paug_mv(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	const char *src = luaL_checkstring(L, 2);
	const char *dst = luaL_checkstring(L, 3);
	return pushresult(L, aug_mv(a, src, dst), a, NULL);
}

static int Paug_matches(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	const char *path = luaL_checkstring(L, 2);
	return pushresult(L, aug_match(a, path, NULL), a, path);
}

static int Paug_match(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	const char *path = luaL_checkstring(L, 2);
	char **match = NULL;
	int i, n;
	n = aug_match(a, path, &match);
	if (n < 0)
		return pusherror(L, a, path);

	lua_newtable(L);
	for (i = 0; i < n; i++) {
		lua_pushnumber(L, i+1);
		lua_pushstring(L, match[i]);
		lua_settable(L, -3);
		free(match[i]);
	}
	free(match);
	lua_pushinteger(L, n);
	return 2;
}

static int Paug_save(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	return pushresult(L, aug_save(a), a, NULL);
}

static int Paug_load(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	return pushresult(L, aug_load(a), a, NULL);
}

static int Paug_print(lua_State *L)
{
	augeas *a;
	const char *path;
	FILE *f = stdout;
	a = Paug_checkarg(L, 1);
	path = luaL_checkstring(L, 2);
	if (lua_isuserdata(L, 3))
		f = *(FILE**) luaL_checkudata(L, 3, LUA_FILEHANDLE);
	return pushresult(L, aug_print(a, f, path), a, path);
}

static int Paug_error(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	lua_pushinteger(L, aug_error(a));
	return 1;
}

static int Paug_error_message(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	lua_pushstring(L, aug_error_message(a));
	return 1;
}

static int Paug_error_minor_message(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	lua_pushstring(L, aug_error_minor_message(a));
	return 1;
}

static int Paug_error_details(lua_State *L)
{
	augeas *a = Paug_checkarg(L, 1);
	lua_pushstring(L, aug_error_details(a));
	return 1;
}

static const luaL_Reg Paug_methods[] = {
/*
--- Initializes the library.
--
-- * `params` Table of initialization parameters; all optional. `root` path to
--   Augeas, `loadpath` to Augeas lenses, and string/boolean pairs for the
--   flags in the Taug_flagmap array.
-- * `[return]` augeas object, used as first parameter in subsequent function
--   calls
function init(params)
 */
	{"init",	Paug_init},
	{"defvar",	Paug_defvar},
	{"defnode",	Paug_defnode},
/*
--- Closes the library. Optional; automatically called by garbage collector.
--
-- * `augobj` Augeas object from init()
function close(augobj)
 */
	{"close",	Paug_close},
/*
--- Gets the value for an Augeas path.
--
-- * `augobj` Augeas object from init()
-- * `path` Augeas path
-- * `[return]` Value for path
function get(augobj, path)
 */
	{"get",		Paug_get},
/*
--- Sets the value for an Augeas path.
--
-- * `augobj` Augeas object from init()
-- * `path` Augeas path
-- * `value` Value for path
function set(augobj, path, value)
 */
	{"set",		Paug_set},
	{"setm",	Paug_setm},
	{"insert",	Paug_insert},
	{"rm",		Paug_rm},
	{"mv",		Paug_mv},
	{"matches",	Paug_matches},
/*
--- Collects paths in the Augeas tree.
--
-- * `augobj` Augeas object from init()
-- * `path` Source path to be matched
-- * `[return]` Array of matching paths
function match(augobj, path)
 */
	{"match",	Paug_match},
	{"save",	Paug_save},
/*
--- Loads the values for the Augeas tree.
--
-- * `augobj` Augeas object from init()
function load(augobj)
 */
	{"load",	Paug_load},
/*
--- Prints the value(s) for an Augeas path.
--
-- * `augobj` Augeas object from init()
-- * `path` Augeas path
-- * `file_handle` *optional* open file for output; otherwise uses stdout
function print(augobj, path, file_handle)
 */
	{"print",	Paug_print},
	{"error",	Paug_error},
	{"error_message",	Paug_error_message},
	{"error_minor_message",	Paug_error_minor_message},
	{"error_details",	Paug_error_details},
	{NULL,		NULL}
};

static const luaL_Reg Laug_meta_methods[] = {
	{"__gc",	Paug_close},
	{NULL,		NULL}
};


LUALIB_API int luaopen_augeas(lua_State *L)
{
	struct aug_flagmap *f = Taug_flagmap;
	luaL_newlib(L, Paug_methods);
	lua_pushliteral(L, "version");
	lua_pushliteral(L, VERSION);
	lua_settable(L, -3);

	while (f->name != NULL) {
		lua_pushstring(L, f->name);
		lua_pushinteger(L, f->value);
		lua_settable(L, -3);
		f++;
	}

	luaL_newmetatable(L, PAUG_META);
	luaL_setfuncs(L, Laug_meta_methods, 0);
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 1);

	return 1;
}

