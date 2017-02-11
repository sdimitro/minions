#include <stdlib.h>
#include <string.h>
#include <stdio.h>
 
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "libsdb.h"

typedef struct lsdb_userdata {
	lsdb_obj_t *lobj;
	char *name;
} lsdb_userdata_t;

static int
libsdb_new(lua_State *L)
{
	lsdb_userdata_t *lu;
	const char *name;

	/* Check if argument is valid. */
	name  = luaL_checkstring(L, 1);
	if (name == NULL)
		luaL_error(L, "name cannot be empty");

	/*
	 * Create the user data pushing it onto the stack.
	 * We also pre-initialize the member of the userdata
	 * in case initialization fails in some way. If that
	 * happens we want the userdata to be in a consistent
	 * state for __gc.
	 */
	lu = (lsdb_userdata_t *)lua_newuserdata(L, sizeof(*lu));
	lu->lobj = NULL;
	lu->name = NULL;

	/* Add the metatable to the stack. */
	luaL_getmetatable(L, "LSdb");
	/* Set the metatable on the userdata. */
	lua_setmetatable(L, -2); /* XXX: Double-check if it should be -2 */

	/* Create the data that comprises the userdata (the lobj). */
	lu->lobj = lsdb_obj_alloc();
	lu->name = strdup(name);

	return 1;
}

static int
libsdb_grab_proc(lua_State *L)
{
	lsdb_userdata_t *lu;
	long pid;
	int flags;

	lu = (lsdb_userdata_t *) luaL_checkudata(L, 1, "LSdb");
	pid = luaL_checkinteger(L, 2);
	flags = luaL_checkinteger(L, 3);

	if (lsdb_attach2pid(lu->lobj, pid, flags) != 0)
		luaL_error(L, lu->lobj->err_msg);

	return 0;
}

static int
libsdb_grab_core(lua_State *L)
{
	lsdb_userdata_t *lu;
	const char *core, *aout;
	int flags;

	lu = (lsdb_userdata_t *) luaL_checkudata(L, 1, "LSdb");
	core = luaL_checkstring(L, 2);
	if (core == NULL)
		luaL_error(L, "core path cannot be empty");
	aout  = luaL_checkstring(L, 3);
	/* XXX: pass it a string representation of NULL */
	flags = luaL_checkinteger(L, 4);

	if (lsdb_grab_core(lu->lobj, core, aout, flags) != 0)
		luaL_error(L, lu->lobj->err_msg);

	return 0;
}

static int
libsdb_grab_file(lua_State *L)
{
	lsdb_userdata_t *lu;
	const char *fname;

	lu = (lsdb_userdata_t *) luaL_checkudata(L, 1, "LSdb");
	fname = luaL_checkstring(L, 2);
	if (fname == NULL)
		luaL_error(L, "filename cannot be empty");

	if (lsdb_grab_file(lu->lobj, fname) != 0)
		luaL_error(L, lu->lobj->err_msg);

	return 0;
}

static int
libsdb_crelease(lua_State *L)
{
	lsdb_userdata_t *lu;
	int flags;

	lu = (lsdb_userdata_t *) luaL_checkudata(L, 1, "LSdb");
	flags = luaL_checkinteger(L, 2);

	lsdb_release(lu->lobj, flags);
	lsdb_obj_free(lu->lobj);
	lu->lobj = NULL;
	free(lu->name);
	lu->name = NULL;
	return 0;
}

static int
libsdb_executable(lua_State *L)
{
	lsdb_userdata_t *lu;
	char path[MAX_EXECNAME];

	lu = (lsdb_userdata_t *) luaL_checkudata(L, 1, "LSdb");
	if (lsdb_execname(lu->lobj, path, MAX_EXECNAME) != 0) {
		lua_pushstring(L, lu->lobj->err_msg);
	} else {
		lua_pushstring(L, path);
	}
	return 1;
}

static int
libsdb_getcontents(lua_State *L)
{
	lsdb_userdata_t *lu;
	core_content_t cct;

	lu = (lsdb_userdata_t *) luaL_checkudata(L, 1, "LSdb");
	cct = lsdb_getcontents(lu->lobj);
	if (lu->lobj->err != 0) {
		lua_pushstring(L, lu->lobj->err_msg);
	} else {
		lua_pushfstring(L, "%d", cct);
	}

	return 1;
}

static int
libsdb_getreg(lua_State *L)
{
	lsdb_userdata_t *lu;
	int reg, regval;

	lu = (lsdb_userdata_t *) luaL_checkudata(L, 1, "LSdb");
	reg = luaL_checkinteger(L, 2);
	regval = lsdb_getareg(lu->lobj, reg);
	if (lu->lobj->err != 0) {
		lua_pushstring(L, lu->lobj->err_msg);
	} else {
		lua_pushfstring(L, "%d", regval);
	}

	return 1;
}

static int
libsdb_tostring(lua_State *L)
{
	lsdb_userdata_t *lu;
	lu = (lsdb_userdata_t *) luaL_checkudata(L, 1, "LSdb");
	lua_pushfstring(L, "%s", lu->name);
	return 1;
}

static const struct luaL_Reg libsdb_methods[] = {
	{ "load_core",   libsdb_grab_core   },
	{ "load_file",   libsdb_grab_file   },
	{ "attach_proc", libsdb_grab_proc   },
	{ "contents",    libsdb_getcontents },
	{ "execfile",    libsdb_executable  },
	{ "reg",         libsdb_getreg      },
	{ "release",     libsdb_crelease    },
	{ "__gc",        libsdb_crelease    },
	{ "__tostring",  libsdb_tostring    },
	{ NULL,          NULL            },
};

static const struct luaL_Reg libsdb_functions[] = {
	{ "new", libsdb_new },
	{ NULL,  NULL     }
};

int
luaopen_libsdb(lua_State *L)
{
	/* Create the metatable and put it on the stack. */
	luaL_newmetatable(L, "LSdb");

	/* Duplicate the metatable on the stack (We now have 2). */
	lua_pushvalue(L, -1);

	/*
	 * Pop the first metatable off the stack and
	 * assign it to __index of the second one. We
	 * set the metatable for the table to itself.
	 * This is equivalent to the following in lua:
	 * metatable = {}
	 * metatable.__index = metatable
	 */
	lua_setfield(L, -2, "__index");

	/*
	 * Set the methods to the metatable that should
	 * be accessed via object:func
	 */
	luaL_setfuncs(L, libsdb_methods, 0);

	/*
	 * Register the object.func functions into the
	 * table that is at the top of the stack.
	 */
	luaL_newlib(L, libsdb_functions);

	return 1;
}
