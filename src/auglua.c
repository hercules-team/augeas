/*
 * auglua.c: lua integration for augtool
 *
 * Copyright (C) 2015 Raphaël Pinson
 * Copyright (C) 2016 Kaarle Ritvanen
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
#include "luamod.h"
#include "auglua.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdbool.h>


#define REG_KEY_MODULE "augeas-module"
#define REG_KEY_INSTANCE "augeas-instance"

static void push_reg_value(lua_State *L, const char *key) {
    lua_pushstring(L, key);
    lua_gettable(L, LUA_REGISTRYINDEX);
}

static int call_function(lua_State *L, const char *name) {
    push_reg_value(L, REG_KEY_MODULE);
    lua_pushstring(L, name);
    lua_gettable(L, -2);
    lua_insert(L, 1);

    push_reg_value(L, REG_KEY_INSTANCE);
    lua_insert(L, 2);

    lua_call(L, lua_gettop(L) - 1, LUA_MULTRET);
    return lua_gettop(L);
}

static int call_bound_function(lua_State *L) {
    int vals = call_function(L, lua_tostring(L, lua_upvalueindex(1)));
    if (lua_isnil(L, 1) && lua_isstring(L, 2)) {
        lua_pushvalue(L, 2);
        lua_error(L);
    }
    return vals;
}

static int bind_function(lua_State *L) {
    lua_pushcclosure(L, call_bound_function, 1);
    return 1;
}

static int lua_aug_save(lua_State *L) {
    int n;

    call_function(L, "save");

    if (lua_isnil(L, 1)) {
        lua_pushstring(L, "saving failed (run 'errors' for details)");
        lua_error(L);
    } else {
        lua_settop(L, 0);
        lua_pushliteral(L, "/augeas/events/saved");
        call_function(L, "match");

        if (!lua_isnil(L, 1)) {
            n = (int) lua_tointeger(L, 2);
            if (n > 0)
                printf("Saved %d file(s)\n", n);
        }
    }

    return 0;
}

struct lua_State *setup_lua(augeas *a) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
  
    lua_pushliteral(L, REG_KEY_MODULE);
    luaopen_augeas(L);
    lua_settable(L, LUA_REGISTRYINDEX);

    lua_pushliteral(L, REG_KEY_INSTANCE);
    luamod_push_augeas(L, a);
    lua_settable(L, LUA_REGISTRYINDEX);

    static const luaL_Reg augfuncs[] = {
        //{ "span", lua_aug_span },
        { "save", lua_aug_save },
        //{ "escape_name", lua_aug_escape_name },
        //{ "to_xml", lua_aug_to_xml },
        //{ "errors", lua_aug_errors },
        { NULL, NULL }
    };

    luaL_newlib(L, augfuncs);

    static const luaL_Reg meta[] = {{"__index", bind_function}, {NULL, NULL}};
    luaL_newlib(L, meta);
    lua_setmetatable(L, -2);

    lua_setglobal(L, "aug");

    return L;
}

int aug_lua(lua_State *L, const char *text) {
    int code = luaL_loadbuffer(L, text, strlen(text), "line") || lua_pcall(L, 0, 0, 0);
    return code;
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

