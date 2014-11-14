#define _DEFAULT_SOURCE
#include "lua_scene.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define TYPE_NAME "scene"
#define REGISTRY_KEY "geng.scene"

#include "../scene.h"
#include "lua_object.h"

#include <string.h>
#include <libgen.h>

object* create_prefab(lua_State* l, scene* s, const char* name)
{
	char* key = (char*)calloc(256, sizeof(char));
	strcat(key, "geng.prefabs.");
	strcat(key, name);

	lua_pushstring(l, key);
	free(key);
	lua_rawget(l, LUA_REGISTRYINDEX);
	if(lua_isnil(l, -1)) luaL_error(l, "Unknown prefab");
	int run_result = lua_pcall(l, 0, 1, 0);
	switch(run_result) {
		case 0: {
			// OK
			object_ref* res = luaG_checkobject(l, -1);
			if(res == NULL) return NULL;
			return res->o;
			}
		case LUA_ERRERR:
		case LUA_ERRMEM:
		case LUA_ERRRUN:
			{
			char* balle = lua_tolstring(l, -1, NULL);
			lua_error(l);
			}
	}
	return NULL;
}

scene* luaG_checkscene(lua_State* L, int index)
{
	luaL_checktype(L, index, LUA_TUSERDATA);
	scene* s = NULL;
	s = (scene*)luaL_checkudata(L, index, TYPE_NAME);
	if (s == NULL) luaL_typerror(L, index, TYPE_NAME);
	return s;
}

scene* get_scene_registry(lua_State* l)
{
	lua_pushstring(l, REGISTRY_KEY);
	lua_rawget(l, LUA_REGISTRYINDEX);
	scene* ret = (scene*)lua_touserdata(l, -1);
	if(ret == NULL) luaL_error(l, "No current scene");
	return ret;
}

void set_scene_registry(lua_State* l, scene* s)
{
	lua_pushstring(l, REGISTRY_KEY);
	lua_pushlightuserdata(l, s);
	/* registry['REGISTRY_KEY'] = s */
	lua_rawset(l, LUA_REGISTRYINDEX);
}

static int lua_load_prefab(lua_State* l)
{
	const char* filename = luaL_checklstring(l, 1, NULL);
	if(filename == NULL) luaL_error(l, "Valid name to prefab file required.");
	char* tmp = strdup(filename);
	if(tmp == NULL) luaL_error(l, "Memory allocation error");
	char* prefabname = basename(tmp);
	char* end = strchr(prefabname, '.');
	if(end != NULL)
		*end = '\0';

	char* key = (char*)calloc(256, sizeof(char));
	strcat(key, "geng.prefabs.");
	strcat(key, prefabname);
	free(tmp);

	lua_pushstring(l, key);
	free(key);
	switch(luaL_loadfile(l, filename)) {
		case 0:
			// OK
			lua_rawset(l, LUA_REGISTRYINDEX);
			break;
		case LUA_ERRSYNTAX:
			luaL_error(l, "Syntax error in prefab.");
		case LUA_ERRMEM:
			luaL_error(l, "Memory allocation error on load of prefab.");
		case  LUA_ERRFILE:
			luaL_error(l, "Cannot open prefab file");
	}
	return 0;
}

static int lua_load_behaviour(lua_State* l)
{
	const char* filename = luaL_checklstring(l, 1, NULL);
	if(filename == NULL) luaL_error(l, "Valid name to behaviour file required.");
	char* tmp = strdup(filename);
	if(tmp == NULL) luaL_error(l, "Memory allocation error");
	char* behavname = basename(tmp);
	char* end = strchr(behavname, '.');
	if(end != NULL)
		*end = '\0';

	char* key = (char*)calloc(256, sizeof(char));
	strcat(key, "geng.behaviours.");
	strcat(key, behavname);
	free(tmp);

	lua_pushstring(l, key);
	free(key);
	switch(luaL_loadfile(l, filename)) {
		case 0:
			// OK
			lua_rawset(l, LUA_REGISTRYINDEX);
			break;
		case LUA_ERRSYNTAX:
			luaL_error(l, "Syntax error in behaviour.");
		case LUA_ERRMEM:
			luaL_error(l, "Memory allocation error on load of behaviour.");
		case  LUA_ERRFILE:
			luaL_error(l, "Cannot open behaviour file");
	}
	return 0;
}

static int lua_new_object(lua_State* l)
{
	scene* s = get_scene_registry(l);
	object* o = create_object(s);
	luaG_pushobject(l, o);
	return 1;
}

static int lua_spawn_prefab(lua_State* l)
{
	scene* s = get_scene_registry(l);
	const char* name = luaL_checklstring(l, 1, NULL);

	if(name == NULL) luaL_error(l, "Name required");

	lua_State* newt = lua_newthread(l);
	lua_pop(l, -1);
	object* o = create_prefab(newt, s, name);
	if(o == NULL) luaL_error(l, "Object NULL error");
	luaG_pushobject(l, o);
	return 1;
}

static const luaL_Reg methods[] = {
	{"newobject", lua_new_object},
	{"spawn_prefab", lua_spawn_prefab},
	{"load_prefab_file", lua_load_prefab},
	{"load_behaviour_file", lua_load_behaviour},
	{NULL, NULL}
};

static const luaL_Reg meta_methods[] = {
	{NULL, NULL}
};

int register_scene(lua_State *L, scene* s)
{
	set_scene_registry(L, s);

	luaL_register(L, TYPE_NAME, methods);
	lua_pop(L, 1);
	return 0;
}
