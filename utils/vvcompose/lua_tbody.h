#include "core.h"

int luaopen_tbody (lua_State *L);

/* push userdata with (TBody*) and set metatable "LibVVD.TBody" */
void  pushTBody(lua_State *L, shared_ptr<TBody>& body);
/* get userdata (TBody*) and check metatable is "LibVVD.TBody" */
shared_ptr<TBody> checkTBody(lua_State *L, int idx);

int luavvd_load_body(lua_State *L);
