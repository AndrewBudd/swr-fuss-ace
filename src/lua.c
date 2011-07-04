/* CT2 lua interface module */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <dlfcn.h>

#include "mud.h"
#include "mssp.h"

LUA_DATA *first_lua = NULL;
LUA_DATA *last_lua = NULL;
//static void stackDump (lua_State *L);

static const char Key = 'k';

int luaopen_mud (lua_State *L);

static int lua_mud_send_to_char(lua_State *L);
static int lua_mud_get_first_char(lua_State *L);
static int lua_mud_get_last_char(lua_State *L);
static int lua_mud_find_location(lua_State *L);
static int lua_set_test_callback(lua_State *L);

static const struct luaL_reg mudlib [] = {
	{"send_to_char", lua_mud_send_to_char},
	{"getFirstChar", lua_mud_get_first_char},
	{"getLastChar", lua_mud_get_last_char},
	{"findLocation",lua_mud_find_location},
	{"setTestCallback", lua_set_test_callback},
	{NULL, NULL}
};


int luaopen_character (lua_State *L);

static int lua_push_ch(lua_State *L,CHAR_DATA *ch);
static CHAR_DATA *checkcharacter (lua_State *L, int index);

//methods
static int lua_ch_get_name(lua_State *L);
static int lua_ch_get_top_level(lua_State *L);
static int lua_ch_get_in_room(lua_State *L);
static int lua_ch_get_was_in_room(lua_State *L);
static int lua_ch_get_leader(lua_State *L);
static int lua_ch_get_next(lua_State *L);
static int lua_ch_get_prev(lua_State *L);
static int lua_ch_get_next_in_room(lua_State *L);
static int lua_ch_get_prev_in_room(lua_State *L);
static int lua_ch_to_room(lua_State *L);

static const struct luaL_reg characterlib_m [] = {
	{"getName", lua_ch_get_name},
	{"getTopLevel", lua_ch_get_top_level},
	{"getInRoom",lua_ch_get_in_room},
	{"getWasInRoom",lua_ch_get_was_in_room},
	{"getLeader",lua_ch_get_leader},
	{"getNext",lua_ch_get_next},
	{"getPrev",lua_ch_get_prev},
	{"getNextInRoom",lua_ch_get_next_in_room},
	{"getPrevInRoom",lua_ch_get_prev_in_room},
	{"toRoom",lua_ch_to_room},
	{NULL, NULL}
};

static const struct luaL_reg characterlib_f [] = {
      {NULL, NULL}
};

/** PLANET declarations **/
int luaopen_planet(lua_State *L);

//push and check utility functions
static int lua_pushplanet(lua_State *L, PLANET_DATA *pPlanet);
static PLANET_DATA *checkplanet (lua_State *L, int index);

//methods
static int lua_planet_get_name_m(lua_State *L);
static int lua_planet_get_governed_by_m(lua_State *L);
static int lua_planet_get_population_m(lua_State *L);
static int lua_planet_get_base_value_m(lua_State *L);
static int lua_planet_set_governed_by_m(lua_State *L);
static int lua_planet_set_population_m(lua_State *L);
static int lua_planet_set_base_value_m(lua_State *L);

//functions
static int lua_planet_get_by_name_f(lua_State *L);
static int lua_planet_get_all_planets_f(lua_State *L);
static int lua_planet_create_planet_f(lua_State *L);

// non-static methods
static const struct luaL_reg planetlib_m [] = {
	//getters 
	{"getName", lua_planet_get_name_m},
	{"getGovernedBy", lua_planet_get_governed_by_m},
	{"getPopulation", lua_planet_get_population_m},
	{"getBaseValue", lua_planet_get_base_value_m},
	//setters
	{"setGovernedBy", lua_planet_set_governed_by_m},
	{"setPopulation", lua_planet_set_population_m},
	{"setBaseValue", lua_planet_set_base_value_m},
	{NULL, NULL}
};

// static methods
static const struct luaL_reg planetlib_f [] = {
	{"getByName", lua_planet_get_by_name_f},
	{"getAllPlanets", lua_planet_get_all_planets_f},
	{"create",lua_planet_create_planet_f},
	{NULL, NULL}
};

/** CLAN declarations **/
int luaopen_clan(lua_State *L);

//push and check utility functions
static int lua_pushclan(lua_State *L, CLAN_DATA *pClan);
static CLAN_DATA *checkclan (lua_State *L, int index);

//methods
static int lua_clan_get_name_m(lua_State *L);

//functions
static int lua_clan_get_by_name_f(lua_State *L);
static int lua_clan_get_all_clans_f(lua_State *L);

// non-static methods
static const struct luaL_reg clanlib_m [] = {
	{"getName", lua_clan_get_name_m},
	{NULL, NULL}
};

// static methods
static const struct luaL_reg clanlib_f [] = {
	{"getByName", lua_clan_get_by_name_f},
	{"getAllClans",lua_clan_get_all_clans_f},
	{NULL, NULL}
};

/** CLAN_PLANET_DATA declarations **/
int luaopen_CPD(lua_State *L);

//push and check utility functions
static int lua_pushCPD(lua_State *L, CLAN_PLANET_DATA *pCPD);
static CLAN_PLANET_DATA *checkCPD (lua_State *L, int index);

//methods
static int lua_CPD_get_clan_m(lua_State *L);
static int lua_CPD_get_planet_m(lua_State *L);
static int lua_CPD_get_alignment_m(lua_State *L);
static int lua_CPD_set_alignment_m(lua_State *L);

//functions
static int lua_CPD_get_by_planet_and_clan_f(lua_State *L);
static int lua_CPD_create_f(lua_State *L);

// non-static methods
static const struct luaL_reg CPDlib_m [] = {
	{"getClan", lua_CPD_get_clan_m},
	{"getPlanet", lua_CPD_get_planet_m},
	{"getAlignment", lua_CPD_get_alignment_m},
	{"setAlignment", lua_CPD_set_alignment_m},
	{NULL, NULL}
};

// static methods
static const struct luaL_reg CPDlib_f [] = {
	{"getByPlanetAndClan", lua_CPD_get_by_planet_and_clan_f},
	{"create", lua_CPD_create_f},
	{NULL, NULL}
};

/** ROOM DECLARATIONS **/

int luaopen_room (lua_State *L);
static ROOM_INDEX_DATA *checkroom (lua_State *L, int index);
static int lua_pushroom(lua_State *L, ROOM_INDEX_DATA *room);
static int lua_room_get_vnum(lua_State *L);
static int lua_room_get_first_person(lua_State *L);
static int lua_room_get_last_person(lua_State *L);
static int lua_room_get_name(lua_State *L);
static int lua_room_get_description(lua_State *L);

static const struct luaL_reg roomlib_m [] = {
	{"getVnum", lua_room_get_vnum},
	{"getFirstPerson", lua_room_get_first_person},
	{"getLastPerson", lua_room_get_last_person},
	{"getName", lua_room_get_name},
	{"getDescription", lua_room_get_description},
	{NULL, NULL}
};

static const struct luaL_reg roomlib_f [] = {
      {NULL, NULL}
};


//Creates a standard lua environment 
LUA_DATA *load_lua_file(const char *filename)
{
	char buff[256];
	int error;
	LUA_DATA *pLua;

	//modify filename to include lua directory
	sprintf( buff, "%s%s", LUA_DIR, filename );

	//internal datastructuer for us
	CREATE(pLua,LUA_DATA,1);
	
	//initialize state
	pLua->filename = STRALLOC(filename);
	pLua->state = lua_open();

	//standard base libraries (io excluded intentionally)
	luaopen_base(pLua->state);
	luaopen_table(pLua->state);
	luaopen_math(pLua->state);
	luaopen_string(pLua->state);

	//our libraries
	luaopen_mud(pLua->state);
	luaopen_character(pLua->state);
	luaopen_room(pLua->state);
	luaopen_planet(pLua->state);
	luaopen_clan(pLua->state);
	luaopen_CPD(pLua->state);

	//add us to the instance list
	LINK(pLua,first_lua,last_lua,next,prev);

	error = luaL_loadfile(pLua->state,buff) || lua_pcall(pLua->state, 0, 0, 0);
	if (error) {
		sprintf(log_buf, "%s", lua_tostring(pLua->state, -1));
		lua_pop(pLua->state, 1);  
		log_string( log_buf );
	}

	log_string( " lua file loaded " );
	//fclose(fp);
	return pLua;
}

LUA_DATA *load_lua_buffer(const char *name, const char *buff, size_t size) {

  // LUALIB_API int luaL_loadbuffer (lua_State *L, const char *buff, size_t size,
  //                           const char *name)

 	int error;
	LUA_DATA *pLua;

	CREATE(pLua, LUA_DATA, 1);

	pLua->filename = STRALLOC(name);
	pLua->state = lua_open();

	//standard base libraries (io excluded intentionally)
	luaopen_base(pLua->state);
	luaopen_table(pLua->state);
	luaopen_math(pLua->state);
	luaopen_string(pLua->state);

	//our libraries
	luaopen_mud(pLua->state);
	luaopen_character(pLua->state);
	luaopen_room(pLua->state);
	luaopen_planet(pLua->state);
	luaopen_clan(pLua->state);
	luaopen_CPD(pLua->state);
  
	LINK(pLua, first_lua, last_lua, next, prev);

	error = luaL_loadbuffer(pLua->state,buff,size,name);
	if (error) {
		sprintf(log_buf, "%s", lua_tostring(pLua->state, -1));
		lua_pop(pLua->state, 1);  
		log_string( log_buf );
	}

	//log_string( " lua file loaded " );
	//fclose(fp);
	return pLua;
}

void unload_lua(LUA_DATA *pLua)
{
	if(pLua == NULL)
		return;

	//unlink functions, listeners, etc.
	
	UNLINK(pLua,first_lua,last_lua,next,prev);
	lua_close(pLua->state);
	STRFREE(pLua->filename);
	DISPOSE(pLua);
}

//open a buffer to function like an immediate window
void do_luaimmediate( CHAR_DATA *ch, const char *arg) {

  const char *buffer;
  int error;
  LUA_DATA *pLua;

  if( IS_NPC( ch ) )
  {
     send_to_char( "Monsters are too dumb to do that!\r\n", ch );
     return;
  }



  if(arg != '\0' && strlen(arg) != 0) {
    buffer = arg;
  } else {

  // alright, we're going to open an editor, then when the buffer closes (if it doesnt abort)
  // we will load it, set the ch global, and then run it

  // copied from desc editing

   if( !ch->desc )
   {
      bug( "do_description: no descriptor", 0 );
      return;
   }

   switch ( ch->substate )
   {
      default:
         bug( "do_description: illegal substate", 0 );
         return;

      case SUB_RESTRICTED:
         send_to_char( "You cannot use this command from within another command.\r\n", ch );
         return;

      case SUB_NONE:
	buffer = STRALLOC("");
         ch->substate = SUB_LUA_IMMEDIATE;
         ch->dest_buf = ch;
         start_editing( ch, buffer );
         return;

      case SUB_LUA_IMMEDIATE:
         STRFREE( buffer );
         buffer = copy_buffer( ch );
         stop_editing( ch );
   }

  }
 //now the fun begins


	 pLua = load_lua_buffer("Immediate",buffer,strlen(buffer));
	 
	 lua_push_ch(pLua->state,ch);
	 lua_setglobal(pLua->state,"ch");
	 if((error = lua_pcall(pLua->state,0,0,0))!= 0)
	 {
		 if (error) {
			sprintf(log_buf, "%s", lua_tostring(pLua->state, -1));
			lua_pop(pLua->state, 1);  
			log_string( log_buf );
		}
	 }
	 unload_lua(pLua);

	return;




}

void do_luarunfile( CHAR_DATA *ch, const char *arg )
{
	LUA_DATA *pLua;
	pLua = load_lua_file(arg);

	//file runs when it is loaded

	unload_lua(pLua);
	return;
}

void do_luatest( CHAR_DATA *ch, const char *arg )
{
	int error;
	LUA_DATA *pLua;
	pLua = load_lua_file("callback_test.lua");
//	lua_getglobal(pLua->state,"helloworld");
	lua_pushlightuserdata(pLua->state, (void *)&Key);  /* push address */
	lua_gettable(pLua->state, LUA_REGISTRYINDEX);  /* retrieve value */
	
	lua_push_ch(pLua->state,ch);

	lua_pushstring(pLua->state,arg);
     	if ((error = lua_pcall(pLua->state, 2, 0, 0)) != 0)
	{
		 if (error) {
			sprintf(log_buf, "%s", lua_tostring(pLua->state, -1));
			lua_pop(pLua->state, 1);  
			log_string( log_buf );
		}
	}
	unload_lua(pLua);
	return;
}

/* Library Functions */

int luaopen_mud (lua_State *L) {
      luaL_openlib(L, "Mud", mudlib, 0);
      return 1;
}

// lua connector for classic send_to_char function
static int lua_mud_send_to_char(lua_State *L)
{
	CHAR_DATA *ch;
	const char *buf;
	buf = luaL_checkstring(L,1);
	ch = checkcharacter(L,2);

	send_to_char(buf,ch);
	return 1;
}

//returns the first_char global pointer
static int lua_mud_get_first_char(lua_State *L)
{
	lua_push_ch(L,first_char);
	return 1;
}

//returns the last_char global pointer
static int lua_mud_get_last_char(lua_State *L)
{
	lua_push_ch(L,last_char);
	return 1;
}


static int lua_mud_find_location(lua_State *L)
{
	CHAR_DATA *ch;
	const char *location;	
	ROOM_INDEX_DATA *room;

	ch = checkcharacter(L,1);
	location = luaL_checkstring(L,2);
	
	room = find_location(ch,location);

	if(room == NULL)
	{
		lua_pushnil(L);
	}
	else
	{
		lua_pushroom(L,room);
	}	
	return 1;
}

/* character object methods */

// shoves a ch onto the lua stack and sets its type so that we can
// use it like an object.  NICE?
static int lua_push_ch(lua_State *L, CHAR_DATA *ch)
{
	CHAR_DATA **pCh;
	if(ch == NULL )
	{
		lua_pushnil(L);
	}
	else
	{
		//lua_pushlightuserdata(L,ch);
		pCh = (CHAR_DATA **)lua_newuserdata(L,sizeof(CHAR_DATA *));
		*pCh = ch;

		luaL_getmetatable(L, "MudBook.Character");
		lua_setmetatable(L, -2);
	}	
	return 1;
}

// loads the library, quote unquote, for object based character interaction
int luaopen_character (lua_State *L) 
{
	luaL_newmetatable(L, "MudBook.Character");
    
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
    
	luaL_openlib(L, NULL, characterlib_m, 0);

	luaL_openlib(L, "Character", characterlib_f, 0);
    
	return 1;
}

// verifies that the ch is the right kind of ch
static CHAR_DATA *checkcharacter (lua_State *L, int index) {
	//stackDump(L);
//	CHAR_DATA *target;
	CHAR_DATA *ch = *(CHAR_DATA **)(luaL_checkudata(L, index, "MudBook.Character"));
	
	//better safe than sorry -- probably not necessary in the long run, huge performance impact
	/*for(target = first_char;target;target = target->next)
		if(target == ch)
			break;*/

	luaL_argcheck(L, ch != NULL, index, "`Character` null or invalid");
	return (CHAR_DATA *)ch;
}

/* Methods */

static int lua_ch_to_room(lua_State *L)
{
	CHAR_DATA *ch;
	ROOM_INDEX_DATA *room;
	ch = checkcharacter(L,1);
	room = checkroom(L,2);

	char_from_room(ch);
	char_to_room(ch,room);
	do_look( ch, "auto" );
	return 1;
}

/* Accessors */

// name accessor
static int lua_ch_get_name(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_pushstring(L,ch->name);
	return 1;
}

// topLevel accessor
static int lua_ch_get_top_level(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_pushinteger(L,ch->top_level);
	return 1;
}

// leader accessor
static int lua_ch_get_leader(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_push_ch(L,ch->leader);
	return 1;
}

// room accessor
static int lua_ch_get_in_room(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_pushroom(L,ch->in_room);
	return 1;
}

// room accessor
static int lua_ch_get_was_in_room(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_pushroom(L,ch->was_in_room);
	return 1;
}

// room accessor
static int lua_ch_get_next(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_push_ch(L,ch->next);
	return 1;
}

static int lua_ch_get_prev(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_push_ch(L,ch->prev);
	return 1;
}

static int lua_ch_get_next_in_room(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_push_ch(L,ch->next_in_room);
	return 1;
}
static int lua_ch_get_prev_in_room(lua_State *L)
{
	CHAR_DATA *ch;
	ch = checkcharacter(L,1);

	lua_push_ch(L,ch->prev_in_room);
	return 1;
}

/*
static void stackDump (lua_State *L) {
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++) { 
	int t = lua_type(L, i);
	switch (t) {
	
	  case LUA_TSTRING:
	    bug("`%s'", lua_tostring(L, i));
	    break;

	  case LUA_TBOOLEAN:
	    bug(lua_toboolean(L, i) ? "true" : "false");
	    break;
	
	  case LUA_TNUMBER: 
	    bug("%g", lua_tonumber(L, i));
	    break;
	
	  default: 
	    bug("%s", lua_typename(L, t));
	    break;
	
	}
	}
}
*/
/** Room Object **/
static int lua_pushroom(lua_State *L, ROOM_INDEX_DATA *room)
{
	ROOM_INDEX_DATA **pRoom;
	if(room == NULL)
	{
		lua_pushnil(L);
	}
	else
	{
		//lua_pushlightuserdata(L,room);
		pRoom = (ROOM_INDEX_DATA **)lua_newuserdata(L,sizeof(ROOM_INDEX_DATA *));
		*pRoom = room;
		luaL_getmetatable(L, "MudBook.Room");

		lua_setmetatable(L, -2);

	}	
	return 1;
}


// loads the library, quote unquote, for object based character interaction
int luaopen_room (lua_State *L) 
{
	luaL_newmetatable(L, "MudBook.Room");
    
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
    
	luaL_openlib(L, NULL, roomlib_m, 0);

	luaL_openlib(L, "room", roomlib_f, 0);
    
	return 1;
}

// verifies that the room is valid
static ROOM_INDEX_DATA *checkroom (lua_State *L, int index) {
	ROOM_INDEX_DATA *room = *(ROOM_INDEX_DATA **)luaL_checkudata(L, index, "MudBook.Room");

	luaL_argcheck(L, room != NULL && get_room_index(room->vnum), index, "`Room` expected");
	return (ROOM_INDEX_DATA *)room;
}

// room vnum accessor
static int lua_room_get_vnum(lua_State *L)
{
	ROOM_INDEX_DATA *room;
	room = checkroom(L,1);

	lua_pushinteger(L,room->vnum);
	return 1;
}

// room name accessor
static int lua_room_get_name(lua_State *L)
{
	ROOM_INDEX_DATA *room;
	room = checkroom(L,1);

	lua_pushstring(L,room->name);
	return 1;
}

// room name accessor
static int lua_room_get_description(lua_State *L)
{
	ROOM_INDEX_DATA *room;
	room = checkroom(L,1);

	lua_pushstring(L,room->description);
	return 1;
}


static int lua_room_get_first_person(lua_State *L)
{
	ROOM_INDEX_DATA *room;
	room = checkroom(L,1);

	lua_push_ch(L,room->first_person);
	return 1;
}

static int lua_room_get_last_person(lua_State *L)
{
	ROOM_INDEX_DATA *room;
	room = checkroom(L,1);

	lua_push_ch(L,room->last_person);
	return 1;
}

/** planet object **/

int luaopen_planet(lua_State *L)
{
  luaL_newmetatable(L, "MudBook.Planet");
  
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2);  /* pushes the metatable */
  lua_settable(L, -3);  /* metatable.__index = metatable */
    
  luaL_openlib(L, NULL, planetlib_m, 0);

  luaL_openlib(L, "Planet", planetlib_f, 0);
    
  return 1;
}

static int lua_pushplanet(lua_State *L, PLANET_DATA *pPlanet)
{
	PLANET_DATA **ppPlanet;
	if(pPlanet == NULL)
	{
		lua_pushnil(L);
	}
	else
	{
		//lua_pushlightuserdata(L,room);
		ppPlanet = (PLANET_DATA **)lua_newuserdata(L,sizeof(PLANET_DATA *));
		*ppPlanet = pPlanet;
		luaL_getmetatable(L, "MudBook.Planet");

		lua_setmetatable(L, -2);

	}	
	return 1;
}


static int lua_planet_get_by_name_f(lua_State *L) {
	//PLANET_DATA *get_planet( const char *name )
	
	PLANET_DATA *pPlanet;
	const char *planetName;
	
	
	planetName = luaL_checkstring(L,1);
	
	pPlanet = get_planet(planetName);
	
	if(pPlanet == NULL)
	{
		lua_pushnil(L);
	}
	else
	{
		lua_pushplanet(L,pPlanet);
	}	
	return 1;
	
}

static int lua_planet_create_planet_f(lua_State *L) {
	PLANET_DATA *pPlanet;
	
	const char *planetName;
	
	planetName = luaL_checkstring(L,1);
	pPlanet = get_planet(planetName);
	
	if(pPlanet != NULL) {
		lua_pushnil(L);//planet already exists
	} else {
		pPlanet = create_planet(planetName);
		lua_pushplanet(L,pPlanet);
	}
	
	return 1;
}

//returns all the planets as a table of planet 'objects'
static int lua_planet_get_all_planets_f(lua_State *L) {

	PLANET_DATA *pPlanet;
	int i;
	
	if(first_planet == NULL) {
		lua_pushnil(L);
	} else {
		
		lua_newtable(L);
		
		i = 1;
		for(pPlanet = first_planet; pPlanet != NULL; pPlanet = pPlanet->next) {
			lua_pushinteger(L,i++);
			lua_pushplanet(L,pPlanet);
			lua_rawset(L,-3);
		}
	}
	return 1;
}

static int lua_planet_get_governed_by_m(lua_State *L) {
	PLANET_DATA *pPlanet;
	pPlanet = checkplanet(L,1);
	
	lua_pushclan(L,pPlanet->governed_by);
	return 1;
}

static int lua_planet_get_base_value_m(lua_State *L) {
	PLANET_DATA *pPlanet;
	pPlanet = checkplanet(L,1);
	
	lua_pushinteger(L,pPlanet->base_value);
	return 1;
}

static int lua_planet_get_name_m(lua_State *L) {
	PLANET_DATA *pPlanet;
	pPlanet = checkplanet(L,1);
	
	lua_pushstring(L,pPlanet->name);
	return 1;
}

static int lua_planet_get_population_m(lua_State *L) {
	PLANET_DATA *pPlanet;
	pPlanet = checkplanet(L,1);
	
	lua_pushinteger(L,pPlanet->population);
	return 1;
}

static int lua_planet_set_governed_by_m(lua_State *L) {
	PLANET_DATA *pPlanet;
	CLAN_DATA *pClan;
	
	pPlanet = checkplanet(L,1);
	pClan = checkclan(L,2);
	
	pPlanet->governed_by = pClan;
	return 1;
}

static int lua_planet_set_population_m(lua_State *L) {
	PLANET_DATA *pPlanet;
	int population;
	
	pPlanet = checkplanet(L,1);
	population = luaL_checkinteger(L,2);
	
	pPlanet->population = population;
	return 1;
}

static int lua_planet_set_base_value_m(lua_State *L) {
	PLANET_DATA *pPlanet;
	long base_value;
	
	pPlanet = checkplanet(L,1);
	base_value = luaL_checkinteger(L,2);
	
	pPlanet->base_value = base_value;
	return 1;
}

static PLANET_DATA *checkplanet (lua_State *L, int index) {
	PLANET_DATA *pPlanet = *(PLANET_DATA **)luaL_checkudata(L, index, "MudBook.Planet");

	//all we can do here is make sure the pointer isn't null, at least for the moment.
	luaL_argcheck(L, pPlanet != NULL, index, "`Planet` expected");
	return (PLANET_DATA *)pPlanet;
}

/** clan object **/

int luaopen_clan(lua_State *L)
{
  luaL_newmetatable(L, "MudBook.Clan");
  
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2);  /* pushes the metatable */
  lua_settable(L, -3);  /* metatable.__index = metatable */
    
  luaL_openlib(L, NULL, clanlib_m, 0);

  luaL_openlib(L, "Clan", clanlib_f, 0);
    
  return 1;
}

static int lua_pushclan(lua_State *L, CLAN_DATA *pClan)
{
	CLAN_DATA **ppClan;
	if(pClan == NULL)
	{
		lua_pushnil(L);
	}
	else
	{
		//lua_pushlightuserdata(L,room);
		ppClan = (CLAN_DATA **)lua_newuserdata(L,sizeof(CLAN_DATA *));
		*ppClan = pClan;
		luaL_getmetatable(L, "MudBook.Clan");

		lua_setmetatable(L, -2);

	}	
	return 1;
}


static int lua_clan_get_by_name_f(lua_State *L) {
	//PLANET_DATA *get_planet( const char *name )
	
	CLAN_DATA *pClan;
	const char *clanName;
	
	
	clanName = luaL_checkstring(L,1);
	
	pClan = get_clan(clanName);
	
	if(pClan == NULL)
	{
		lua_pushnil(L);
	}
	else
	{
		lua_pushclan(L,pClan);
	}	
	return 1;
	
}

//returns all the clans as a table of 'clan' objects
static int lua_clan_get_all_clans_f(lua_State *L) {

	CLAN_DATA *pClan;
	int i;
	
	if(first_clan == NULL) {
		lua_pushnil(L);
	} else {
		
		lua_newtable(L);
		
		i = 1;
		for(pClan = first_clan; pClan != NULL; pClan = pClan->next) {
			lua_pushinteger(L,i++);
			lua_pushclan(L,pClan);
			lua_rawset(L,-3);
		}
	}
	return 1;
}

static int lua_clan_get_name_m(lua_State *L) {
	CLAN_DATA *pClan;
	pClan = checkclan(L,1);
	
	lua_pushstring(L,pClan->name);
	return 1;
}

static CLAN_DATA *checkclan (lua_State *L, int index) {
	CLAN_DATA *pClan = *(CLAN_DATA **)luaL_checkudata(L, index, "MudBook.Clan");

	//all we can do here is make sure the pointer isn't null, at least for the moment.
	luaL_argcheck(L, pClan != NULL, index, "`Clan` expected");
	return (CLAN_DATA *)pClan;
}


/** CLAN_PLANET_DATA declarations **/
int luaopen_CPD(lua_State *L) {
	  luaL_newmetatable(L, "MudBook.ClanPlanetData");

	  lua_pushstring(L, "__index");
	  lua_pushvalue(L, -2);  /* pushes the metatable */
	  lua_settable(L, -3);  /* metatable.__index = metatable */

	  luaL_openlib(L, NULL, CPDlib_m, 0);

	  luaL_openlib(L, "ClanPlanetData", CPDlib_f, 0);

	  return 1;
}

//push and check utility functions
static int lua_pushCPD(lua_State *L, CLAN_PLANET_DATA *pCPD){
	CLAN_PLANET_DATA **ppCPD;
	if(pCPD == NULL)
	{
		lua_pushnil(L);
	}
	else
	{
		//lua_pushlightuserdata(L,room);
		ppCPD = (CLAN_PLANET_DATA **)lua_newuserdata(L,sizeof(CLAN_PLANET_DATA *));
		*ppCPD = pCPD;
		luaL_getmetatable(L, "MudBook.ClanPlanetData");

		lua_setmetatable(L, -2);

	}	
	return 1;
}

static CLAN_PLANET_DATA *checkCPD (lua_State *L, int index){
	CLAN_PLANET_DATA *pCPD = *(CLAN_PLANET_DATA **)luaL_checkudata(L, index, "MudBook.ClanPlanetData");

	//all we can do here is make sure the pointer isn't null, at least for the moment.
	luaL_argcheck(L, pCPD != NULL, index, "`ClanPlanetData` expected");
	return (CLAN_PLANET_DATA *)pCPD;
}

//methods
static int lua_CPD_get_clan_m(lua_State *L){
	CLAN_PLANET_DATA *pCPD;
	pCPD = checkCPD(L,1);
	
	lua_pushclan(L,pCPD->clan);
	return 1;
}

static int lua_CPD_get_planet_m(lua_State *L){
	CLAN_PLANET_DATA *pCPD;
	pCPD = checkCPD(L,1);
	
	lua_pushplanet(L,pCPD->planet);
	return 1;
}

static int lua_CPD_get_alignment_m(lua_State *L){
	CLAN_PLANET_DATA *pCPD;
	pCPD = checkCPD(L,1);
	
	lua_pushnumber(L,pCPD->alignment);
	return 1;
}

static int lua_CPD_set_alignment_m(lua_State *L){
	CLAN_PLANET_DATA *pCPD;
	float alignment;
	
	pCPD = checkCPD(L,1);
	alignment = luaL_checknumber(L,2);
	
	pCPD->alignment = alignment;
	return 1;
}

//functions
static int lua_CPD_get_by_planet_and_clan_f(lua_State *L){
	CLAN_PLANET_DATA *pCPD;
	CLAN_DATA *pClan;
	PLANET_DATA *pPlanet;
	
	pPlanet = checkplanet(L,1);
	pClan = checkclan(L,2);
	
	pCPD = get_clan_planet_data(pClan,pPlanet);
	
	if(pCPD == NULL) {
		lua_pushnil(L);
	} else {
		lua_pushCPD(L,pCPD);
	}
	return 1;
}

static int lua_CPD_create_f(lua_State *L){
	CLAN_PLANET_DATA *pCPD;
	CLAN_DATA *pClan;
	PLANET_DATA *pPlanet;
	
	pPlanet = checkplanet(L,1);
	pClan = checkclan(L,2);
	
	pCPD = create_clan_planet_data(pClan,pPlanet);
	
	if(pCPD == NULL) {
		lua_pushnil(L);
	} else {
		lua_pushCPD(L,pCPD);
	}
	return 1;
}
/*
//returns all the CPDs as a table of 'CPD' objects
static int lua_CPD_get_all_CPDs_f(lua_State *L) {

	CLAN_PLANET_DATA *pCPD;
	int i;
	
	if(first_clan_planet_data == NULL) {
		lua_pushnil(L);
	} else {
		
		lua_newtable(L);
		
		i = 1;
		for(pCPD = first_clan_planet_data; pCPD != NULL; pCPD = pCPD->next) {
			lua_pushinteger(L,i++);
			lua_pushCPD(L,pCPD);
			lua_rawset(L,-3);
		}
	}
	return 1;
}*/

//experimental callback function, stores a callback in the registry metatable 
static int lua_set_test_callback(lua_State *L)  {
	
	if(!lua_isfunction(L,-1)) {
		luaL_error(L,"function expected"); //we never return
	}
	
	lua_pushlightuserdata(L, (void *)&Key);  /* push address */
	lua_insert(L,-2); //swap the function pointer 
	lua_settable(L, LUA_REGISTRYINDEX);
	return 1;
}

