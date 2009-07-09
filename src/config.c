#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#include "config.h"
#include "net.h"
#include "utils.h"


void stackDump(lua_State* L);

void config_database(char* filename) {

	//TODO: read all variables and make syntox error handler

	lua_State* L;

	char *hostname, *database, *user, *password;
	char *newhostname, *newdatabase, *newuser, *newpassword;
	int port;
	
	L = lua_open();
	lua_baselibopen(L);
	lua_dofile(L, filename);

	lua_getglobal(L, "database");
		lua_pushstring(L,"auth");
		lua_gettable(L,-2);
			lua_pushstring(L,"username");
			lua_gettable(L,-2);
 			if (!lua_isstring(L, -1))
        			error(L, "`user' should be a string\n");
      			user = (char *)lua_tostring(L, -1);
			//copy user into another memory space because the mysql lib 
			//chokes on the strings from the lua lib
			newuser = malloc(strlen(user)+1);
			strcpy(newuser,user);

			lua_pushstring(L,"password");
			lua_gettable(L,-3);
 			if (!lua_isstring(L, -1))
      			  	error(L, "`password' should be a string\n");
      			password = (char *)lua_tostring(L, -1);
			//mysql - lua choke fix (see above)
			newpassword = malloc(strlen(password)+1);
			strcpy(newpassword,password);

		lua_pushstring(L,"server");
		lua_gettable(L,-5);
			lua_pushstring(L,"hostname");
			lua_gettable(L,-2);
 			if (!lua_isstring(L, -1))
        			error(L, "`hostname' should be a string\n");
      			hostname = (char *)lua_tostring(L, -1);
			//mysql - lua choke fix (see above)
			newhostname = malloc(strlen(hostname)+1);
			strcpy(newhostname,hostname);

			lua_pushstring(L,"port");
			lua_gettable(L,-3);
 			if (!lua_isnumber(L, -1))
        			error(L, "`port' should be a number\n");
      			port = (int)lua_tonumber(L, -1);
		lua_pushstring(L,"database");
		lua_gettable(L,-8);
 		if (!lua_isstring(L, -1))
        		error(L, "`database' should be a string\n");
      		database = (char *)lua_tostring(L, -1);
		//mysql - lua choke fix (see above)
		newdatabase = malloc(strlen(database)+1);
		strcpy(newdatabase,database);

	lua_close(L);
	
	//connect to database
	connect_database(newhostname,port,newuser,newpassword,newdatabase);

}

void config_listeners(char* filename) {

	lua_State* L;
	char *hostname,*type;
	int port;
	pthread_t* threads;
	
	num_sockets = 0;
	sockets = malloc(0);
	threads = malloc(0);

	L = lua_open();	
	lua_baselibopen(L);
	lua_dofile(L,filename);

	lua_getglobal(L, "interfaces");
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
       		/* uses 'key' (at index -2) and 'value' (at index -1) */
		lua_pushstring(L,"type");
		lua_gettable(L,-2);
		if(!lua_isstring(L,-1))
			error(L,"type should be a string");
		type = (char*)lua_tostring(L,-1);
       		lua_pop(L, 1);

		lua_pushstring(L,"hostname");	
		lua_gettable(L,-2);
		if(!lua_isstring(L,-1))
			error(L,"hostname should be a string");
		hostname = (char*)lua_tostring(L,-1);
       		lua_pop(L, 1);

		lua_pushstring(L,"port");
		lua_gettable(L,-2);
		if(!lua_isnumber(L,-1))
			error(L,"port should be a number");
		port = (int)lua_tonumber(L,-1);
		lua_pop(L,1);
       		/* removes 'value'; keeps 'key' for next iteration */
       		lua_pop(L, 1);

		num_sockets++;
		realloc(sockets,sizeof(int)*num_sockets);
		realloc(threads,sizeof(pthread_t)*num_sockets);

		if(strcmp(type,"IPv4")==0) {

			sockets[num_sockets-1] = bind4(hostname,port);
			if( sockets[num_sockets-1] < 0 ) {

				syslog(LOG_ERR, "could not create sockets on %s:%u",hostname,port);
			}
			else {
				pthread_create( &threads[num_sockets-1], NULL, (void*) send_receive_loop4,(void*) sockets[num_sockets-1]);	
				syslog(LOG_INFO, "listening for data on  %s:%u",hostname,port);
			}
		}
		else {
		
			sockets[num_sockets-1] = bind6(hostname,port);
			if( sockets[num_sockets-1] < 0 ) {

				syslog(LOG_ERR, "could not create sockets on [%s]:%u",hostname,port);
			}
			else {
				pthread_create( &threads[num_sockets-1], NULL, (void*) send_receive_loop6,(void*) sockets[num_sockets-1]);	
				syslog(LOG_INFO, "listening for data on  [%s]:%u",hostname,port);
			}
		}
	}

	lua_close(L);

	//wait for the threads to end
	int i;
	for(i=0;i<num_sockets;i++) {
	
        	pthread_join(threads[i], NULL);
	} 
}

void config_other(char *filename) {

	lua_State* L;

	L = lua_open();	
	lua_baselibopen(L);
	lua_dofile(L,filename);
	lua_getglobal(L, "others");
	lua_pushstring(L,"user");	
	lua_gettable(L,-2);
	if(!lua_isstring(L,-1))
		error(L,"user should be a string\n");
	user = (char*) lua_tostring(L,-1);
	lua_pop(L,1);

	lua_pushstring(L,"pidfile");	
	lua_gettable(L,-2);
	if(!lua_isstring(L,-1))
		error(L,"pidfile should be a string\n");
	pidfile = (char*) lua_tostring(L,-1);
	lua_pop(L,1);

	lua_pushstring(L,"mtu");	
	lua_gettable(L,-2);
	if(!lua_isnumber(L,-1))
		error(L,"mtu should be a number\n");
	mtu = (int) lua_tonumber(L,-1);
	lua_pop(L,1);

	lua_pushstring(L,"interval");	
	lua_gettable(L,-2);
	if(!lua_isnumber(L,-1))
		error(L,"interval should be a number\n");
	interval = (int) lua_tonumber(L,-1);

	lua_close(L);
}

void stackDump (lua_State *L) {
      int i;
      int top = lua_gettop(L);
      for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {
    
          case LUA_TSTRING:  /* strings */
            printf("`%s'", lua_tostring(L, i));
            break;
    
          case LUA_TBOOLEAN:  /* booleans */
            printf(lua_toboolean(L, i) ? "true" : "false");
            break;
    
          case LUA_TNUMBER:  /* numbers */
            printf("%g", lua_tonumber(L, i));
            break;
    
          default:  /* other values */
            printf("%s", lua_typename(L, t));
            break;
    
        }
        printf("  ");  /* put a separator */
      }
      printf("\n");  /* end the listing */
    }

