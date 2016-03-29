/*
 * auglua.h: Lua helper lib for Augeas
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
#include <lua.h>

#ifndef AUGLUA_H_
#define AUGLUA_H_

#ifdef __cplusplus
extern "C" {
#endif

struct lua_State *setup_lua(augeas *a);

#ifdef __cplusplus
}
#endif

#endif

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */
