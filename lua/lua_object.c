#include "lua_object.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <string.h>

#include "../scene.h"

#include "lua_vector.h"

#define TYPE_NAME "object"

object_ref* luaG_checkobject(lua_State* L, int index)
{
	object_ref* o;
	luaL_checktype(L, index, LUA_TUSERDATA);
	o = (object_ref*)luaL_checkudata(L, index, TYPE_NAME);
	if (o == NULL) luaL_typerror(L, index, TYPE_NAME);
	return o;
}

object_ref* luaG_pushobject(lua_State *L, object* obj)
{
	object_ref* o = (object_ref*)lua_newuserdata(L, sizeof(object_ref));
	luaL_getmetatable(L, TYPE_NAME);
	lua_setmetatable(L, -2);
	o->o = obj;
	return o;
}

static int lua_object_equals(lua_State* l)
{
	object_ref* a = luaG_checkobject(l, 1);
	object_ref* b = luaG_checkobject(l, 2);

	lua_pushboolean(l, a->o == b->o);
	return 1;
}

static int lua_object_gc(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	return 0;
}

static int lua_object_tostring(lua_State *L)
{
	object_ref* o = luaG_checkobject(L, 1);
	lua_pushstring(L, o->o->name);
	return 1;
}

static int lua_object_position(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	cpVect* v = luaG_pushvect(l);
	*v = get_object_pos(o->o);
	return 1;
}

static int lua_object_setposition(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);

	double x, y;
	if(lua_isnumber(l, 2)) {
		x = luaL_checknumber(l, 2);
		y = luaL_checknumber(l, 3);
	} else {
		cpVect* v = luaG_checkvect(l, 2);
		x = v->x;
		y = v->y;
	}

	set_object_pos(o->o, cpv(x, y));

	return 0;
}

static int lua_object_mass(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	lua_pushnumber(l, get_object_mass(o->o));
	return 1;
}

static int lua_object_moment(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	lua_pushnumber(l, get_object_moment(o->o));
	return 1;
}

static int lua_object_torque(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	lua_pushnumber(l, get_object_torque(o->o));
	return 1;
}

static int lua_object_angvel(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	lua_pushnumber(l, get_object_angular_velocity(o->o));
	return 1;
}

static int lua_object_angvel_limit(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	lua_pushnumber(l, get_object_angular_velocity_limit(o->o));
	return 1;
}

static int lua_object_vel_limit(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	lua_pushnumber(l, get_object_velocity_limit(o->o));
	return 1;
}

static int lua_object_angle(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	lua_pushnumber(l, get_object_angle(o->o));
	return 1;
}

static int lua_object_setsprite(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	size_t s = luaL_checkinteger(l, 2);

	o->o->sprite = s;
	return 0;
}

static int lua_object_set_static(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	disable_object_physics(o->o);
	return 0;
}

static int lua_object_set_physics(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	double mass = luaL_checknumber(l, 2);
	double moment = luaL_checknumber(l, 3);

	enable_object_physics(o->o, mass, moment);
	return 0;
}

static int lua_object_worldpos2local(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	cpVect* v = luaG_checkvect(l, 2);
	cpVect* res = luaG_pushvect(l);
	*res = object_convert_world2localpos(o->o, *v);
	return 1;
}

static int lua_object_localpos2world(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	cpVect* v = luaG_checkvect(l, 2);
	cpVect* res = luaG_pushvect(l);
	*res = object_convert_local2worldpos(o->o, *v);
	return 1;
}

static int lua_object_reset_forces(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	object_reset_forces(o->o);
	return 0;
}

static int lua_object_apply_force(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	cpVect* f = luaG_checkvect(l, 2);
	object_apply_force(o->o, *f);
	return 0;
}

static int lua_object_apply_impulse(lua_State* l)
{
	object_ref* o = luaG_checkobject(l, 1);
	cpVect* f = luaG_checkvect(l, 2);
	object_apply_impulse(o->o, *f);
	return 0;
}

static const luaL_reg methods[] = {
	{"pos", lua_object_position},
	{"set_pos", lua_object_setposition},
	{"angle", lua_object_angle},
	{"set_sprite", lua_object_setsprite},
	{"disable_physics", lua_object_set_static},
	{"enable_physics", lua_object_set_physics},
	{"apply_force", lua_object_apply_force},
	{"apply_impulse", lua_object_apply_impulse},
	{"reset_forces", lua_object_reset_forces},
	{"local2worldpos", lua_object_localpos2world},
	{"world2localpos", lua_object_worldpos2local},
	{"torque", lua_object_torque},
	{"mass", lua_object_mass},
	{"moment", lua_object_moment},
	{"ang_vel", lua_object_angvel},
	{"vel_limit", lua_object_vel_limit},
	{"angular_vel_limit", lua_object_angvel_limit},
	{NULL, NULL}
};

static const luaL_reg meta_methods[] = {
	{"__tostring", lua_object_tostring},
	{"__eq", lua_object_equals},
	{"__gc", lua_object_gc},
	{NULL, NULL}
};

int register_object(lua_State *L)
{
	/* Create methods table & add it to globals */
	luaL_openlib(L, TYPE_NAME, methods, 0);
	/* Create metatable for object, and add it to registry */
	luaL_newmetatable(L, TYPE_NAME);
	luaL_openlib(L, 0, meta_methods, 0); /* dill metatable */
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -3); 		/* duplicate methods table */
	lua_rawset(L, -3);		/* metatable.__index = methods */
	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, -3);		/* duplicate methods table */
	lua_rawset(L, -3);		/* hide metatable:
					   metatabme.__metatable = methods */

	lua_pop(L, 1); 			/* drop metatable */
	return 1;			/* leave methods on stack */
}


