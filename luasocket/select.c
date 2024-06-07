/*=========================================================================*\
* Select implementation
* LuaSocket toolkit
\*=========================================================================*/
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "socket.h"
#include "timeout.h"
#include "select.h"

/*=========================================================================*\
* Internal function prototypes.
\*=========================================================================*/
static t_socket getfd(lua_State *L);
static int dirty(lua_State *L);
static void collect_fd(lua_State *L, int tab, int itab, PollFDSet *set, short events);
static int check_dirty(lua_State *L, int tab, int dtab, PollFDSet *set);
static void return_fd(lua_State *L, PollFDSet *set,int itab, int tab, int start);
static void make_assoc(lua_State *L, int tab);
static int global_select(lua_State *L);

/* functions in library namespace */
static luaL_Reg func[] = {
    {"select", global_select},
    {NULL,     NULL}
};

/*=========================================================================*\
* Exported functions
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Initializes module
\*-------------------------------------------------------------------------*/
int select_open(lua_State *L) {
    lua_pushstring(L, "_SETSIZE");
    lua_pushnumber(L, FD_SETSIZE);
    lua_rawset(L, -3);
#if LUA_VERSION_NUM > 501 && !defined(LUA_COMPAT_MODULE)
    luaL_setfuncs(L, func, 0);
#else
    luaL_openlib(L, NULL, func, 0);
#endif
    return 0;
}

/*=========================================================================*\
* Global Lua functions
\*=========================================================================*/
/*-------------------------------------------------------------------------*\
* Waits for a set of sockets until a condition is met or timeout.
\*-------------------------------------------------------------------------*/
static int global_select(lua_State *L) {
    int rtab, wtab, itab, rret,wret, ndirty;
    t_socket max_fd = SOCKET_INVALID;
    PollFDSet rset, wset;
    int t = (int)luaL_optnumber(L, 3, -1);//毫秒//
    POLLFD_ZERO(&rset); POLLFD_ZERO(&wset);
    lua_settop(L, 3);
    lua_newtable(L); itab = lua_gettop(L);
    lua_newtable(L); rtab = lua_gettop(L);
    lua_newtable(L); wtab = lua_gettop(L);
    collect_fd(L, 1, itab, &rset,POLLIN);
    collect_fd(L, 2, itab, &wset,POLLOUT);
    ndirty = check_dirty(L, 1, rtab, &rset);
    t = ndirty > 0? 0: t;
    rret = socket_select(&rset, t);
    wret += socket_select(&wset, t);
    lua_getglobal(L,"print");
    lua_pushstring(L, "xxxxretvalue");
    lua_pushnumber(L, rret);
    lua_pushnumber(L, wret);
    lua_call(L, 3, 0);

    if (rret > 0 || wret >0 || ndirty > 0) {
        return_fd(L, &rset,itab, rtab, ndirty);
        return_fd(L, &wset, itab, wtab, 0);
        make_assoc(L, rtab);
        make_assoc(L, wtab);
        return 2;
    } else if (rret == 0 || wret==0) {
        lua_pushstring(L, "timeout");
        return 3;
    } else {
        luaL_error(L, "select failed");
        return 3;
    }
}

/*=========================================================================*\
* Internal functions
\*=========================================================================*/
static t_socket getfd(lua_State *L) {
    t_socket fd = SOCKET_INVALID;
    lua_pushstring(L, "getfd");
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_pushvalue(L, -2);
        lua_call(L, 1, 1);
        if (lua_isnumber(L, -1)) {
            double numfd = lua_tonumber(L, -1); 
            fd = (numfd >= 0.0)? (t_socket) numfd: SOCKET_INVALID;
        }
    } 
    lua_pop(L, 1);
    return fd;
}

static int dirty(lua_State *L) {
    int is = 0;
    lua_pushstring(L, "dirty");
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_pushvalue(L, -2);
        lua_call(L, 1, 1);
        is = lua_toboolean(L, -1);
    } 
    lua_pop(L, 1);
    return is;
}

static void collect_fd(lua_State *L, int tab, int itab, PollFDSet *set,short events) 
{
    int i = 1, n = 0;
    /* nil is the same as an empty table */
    if (lua_isnil(L, tab)) return;
    /* otherwise we need it to be a table */
    luaL_checktype(L, tab, LUA_TTABLE);
    for ( ;; ) {
        t_socket fd;
        lua_pushnumber(L, i);
        lua_gettable(L, tab);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            break;
        }
        /* getfd figures out if this is a socket */
        fd = getfd(L);
        if (fd != SOCKET_INVALID) {
            /* make sure we don't overflow the fd_set */

            if (n >= POLLFD_SETSIZE) 
                luaL_argerror(L, tab, "too many sockets in socket.select");

            POLLFD_SET(fd, set,events);
            n++;
            /* make sure we can map back from descriptor to the object */
            lua_pushnumber(L, (lua_Number) fd);
            lua_pushvalue(L, -2);
            lua_settable(L, itab);
        }
        lua_pop(L, 1);
        i = i + 1;
    }
}

static int check_dirty(lua_State *L, int tab, int dtab, PollFDSet *set) {
    int ndirty = 0, i = 1;
    if (lua_isnil(L, tab)) 
        return 0;
    for ( ;; ) { 
        t_socket fd;
        lua_pushnumber(L, i);
        lua_gettable(L, tab);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            break;
        }
        fd = getfd(L);
        if (fd != SOCKET_INVALID && dirty(L)) {
            lua_pushnumber(L, ++ndirty);
            lua_pushvalue(L, -2);
            lua_settable(L, dtab);
            POLLFD_CLR(fd, set);
        }
        lua_pop(L, 1);
        i = i + 1;
    }
    return ndirty;
}

static void return_fd(lua_State *L, PollFDSet *set,int itab, int tab, int start) {
    unsigned int i=0;
    for (i = 0; i < set->fd_count; i++)
    {
        if (set->pollfds[i].revents & set->pollfds[i].events)
        {
            t_socket fd = set->pollfds[i].fd;
            lua_pushnumber(L, ++start);
            lua_pushnumber(L, (lua_Number) fd);
            lua_gettable(L, itab);
            lua_settable(L, tab);
        }
    }
}

static void make_assoc(lua_State *L, int tab) {
    int i = 1, atab;
    lua_newtable(L); atab = lua_gettop(L);
    for ( ;; ) {
        lua_pushnumber(L, i);
        lua_gettable(L, tab);
        if (!lua_isnil(L, -1)) {
            lua_pushnumber(L, i);
            lua_pushvalue(L, -2);
            lua_settable(L, atab);
            lua_pushnumber(L, i);
            lua_settable(L, atab);
        } else {
            lua_pop(L, 1);
            break;
        }
        i = i+1;
    }
}

