/* Copyright (C) 2016 Kaarle Ritvanen */

#include <lua.h>
#include <lualib.h>

#include "augeas.h"

LUALIB_API int luaopen_augeas(lua_State *L);
void luamod_push_augeas(lua_State *L, augeas *a);
