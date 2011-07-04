/* CT2 project instances system */

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


extern bool fBootDb;
INSTANCE_PROTOTYPE_DATA *first_instance_prototype;
INSTANCE_PROTOTYPE_DATA *last_instance_prototype;
INSTANCE_DATA *first_instance;
INSTANCE_DATA *last_instance;

INSTANCE_DATA *invoke_instance( INSTANCE_PROTOTYPE_DATA *pPrototype );


INSTANCE_PROTOTYPE_DATA *instance_prototype_from_name(char *name)
{
	INSTANCE_PROTOTYPE_DATA *pPrototype;
	for( pPrototype = first_instance_prototype; pPrototype; pPrototype = pPrototype->next ) {
		if(!str_cmp( name, pPrototype->name ))
		{
			return pPrototype;
		}
	}
	return NULL;
}

INSTANCE_PROTOTYPE_DATA *instance_prototype_from_pmv(int pmv)
{
	INSTANCE_PROTOTYPE_DATA *pPrototype;
	int size;
	for( pPrototype = first_instance_prototype; pPrototype; pPrototype = pPrototype->next ) {
		size = pPrototype->high_proto_vnum - pPrototype->low_proto_vnum;
		if(pPrototype->phantom_mapping_vnum >= pmv && pPrototype->phantom_mapping_vnum + size <= pmv)
		{
			return pPrototype;
		}
	}
	return NULL;
}

int next_instance_vnum()
{
	INSTANCE_PROTOTYPE_DATA *pPrototype;
	INSTANCE_DATA *pInstance;
	int i = 131072;
	int size;	
	int lastvnum;
	   for( pInstance = first_instance; pInstance; pInstance = pInstance->next ){
		pPrototype = pInstance->prototype;
		size = pPrototype->high_proto_vnum - pPrototype->low_proto_vnum;
		lastvnum = pInstance->first_vnum + size - 1;
		if( lastvnum > i )
			i = lastvnum +1;
		}
	return i;
	
}

void fread_instance_prototype( INSTANCE_PROTOTYPE_DATA * instance, FILE * fp )
{
   const char *word;
   bool fMatch;


   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;

      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case 'F':
            KEY( "Flags", instance->flags, fread_number( fp ) );
            break;
         case 'G':
            KEY( "GateRoom", instance->gate_room, fread_number( fp ) );
            break;
         case 'H':
            KEY( "HighProtoVnum", instance->high_proto_vnum, fread_number( fp ) );
            break;
         case 'L':
            KEY( "LowProtoVnum", instance->low_proto_vnum, fread_number( fp ) );
            break;
        case 'N':
            KEY( "Name", instance->name, fread_string( fp ) );
	    break;
         case 'P':
            KEY( "PhantomMappingVnum", instance->phantom_mapping_vnum, fread_number( fp ) );
            break;
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               return;
            }
            break;
      }

      if( !fMatch )
      {
         bug( "fread_instance_prototype: no match: %s", word );
      }
   }
}


//loads an instance file into memory (there should only be one)

void load_instance_file( char *filename )
{
   FILE *fp;
   const char *word;
   INSTANCE_PROTOTYPE_DATA *instance;

   if( !( fp = fopen( filename, "r" ) ) )
   {
      perror( filename );
      bug( "%s: error loading file (can't open) %s", __FUNCTION__, filename );
      return;
   }

   if( fread_letter( fp ) != '#' )
   {
      if( fBootDb )
      {
         bug( "%s: No # found at start of instance file.", __FUNCTION__ );
         exit( 1 );
      }
      else
      {
         bug( "%s: No # found at start of instance file.", __FUNCTION__ );
         fclose( fp );
         return;
      }
   }

   word = fread_word( fp );

   for( ;; )
   {
      if( fread_letter( fp ) != '#' )
      {
         bug( "%s: # not found", __FUNCTION__ );
         exit( 1 );
      }

      word = ( feof( fp ) ? "END_INSTANCE_DATABASE" : fread_word( fp ) );


	if( !str_cmp( word, "INSTANCE_PROTOTYPE" ) )
	{
		CREATE(instance,INSTANCE_PROTOTYPE_DATA,1);
		fread_instance_prototype(instance,fp);
		LINK(instance,first_instance_prototype,last_instance_prototype,next,prev);
	}
	else if( !str_cmp( word, "END_INSTANCE_DATABASE" ) )
	{
		break;
	}
      else
      {
         bug( "%s: bad section name: %s", __FUNCTION__, word );
         if( fBootDb )
            exit( 1 );
         else
         {
            fclose( fp );
            return;
         }
      }
   }
   fclose( fp );
}
/*
 * Write an instance prototype to a file
 */
void fwrite_instance_prototype( INSTANCE_PROTOTYPE_DATA *instance, FILE *fp )
{
	if(!instance)
	{
		bug("fwrite_instance_prototype: null instance");
		return;
	}

	if(!fp)
	{
		bug("fwrite_instance_prototype: null file pointer");
		return;
	}

	fprintf( fp, "#INSTANCE_PROTOTYPE\n" );
	fprintf( fp, "LowProtoVnum	%d\n",	instance->low_proto_vnum );
	fprintf( fp, "HighProtoVnum	%d\n",	instance->high_proto_vnum );
	fprintf( fp, "Name		%s~\n", instance->name );
	fprintf( fp, "Flags		%d\n", instance->flags );
	fprintf( fp, "PhantomMappingVnum %d\n", instance->phantom_mapping_vnum );
	fprintf( fp, "GateRoom		%d\n", instance->gate_room );
	fprintf( fp, "End\n");
	return;
}

void write_instance_file( char *fname )
{
   char buf[256];
   FILE *fpout;
   INSTANCE_PROTOTYPE_DATA *prototype;

   snprintf( buf, 256, "%s.bak", fname );
   rename( fname, buf );
   if( !( fpout = fopen( fname, "w" ) ) )
   {
      bug( "%s: fopen", __FUNCTION__ );
      perror( fname );
      return;
   }

   fprintf( fpout, "%s", "#INSTANCE_DATABASE\n" );

   for( prototype = first_instance_prototype; prototype; prototype = prototype->next )
   {
      fwrite_instance_prototype( prototype , fpout );
   }

   fprintf( fpout, "%s", "#END_INSTANCE_DATABASE\n" );
   fclose( fpout );
   fpout = NULL;
   return;
}

void do_instances( CHAR_DATA * ch, const char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   char arg4[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *room;
   int vnum, tpmv, size, pmv, pcount;
   INSTANCE_PROTOTYPE_DATA *pPrototype;
   INSTANCE_PROTOTYPE_DATA *tPrototype;
   INSTANCE_DATA *pInstance;
   CHAR_DATA *pChar;

   argument = smash_tilde_static( argument );
   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   argument = one_argument( argument, arg4 );

   if( arg[0] == '\0' || !str_cmp( arg, "?" ) )
   {

      send_to_char( "Syntax: instances <command> <field> <value>\r\n", ch );
      send_to_char( "\r\n", ch );
      send_to_char( "Command being one of:\r\n", ch );
      send_to_char( "  listprototypes  setprototype  newprototype\r\n", ch );
      send_to_char( "  list   invoke   save\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "listprototypes" ) )
   {
	   set_pager_color( AT_PLAIN, ch );
	   send_to_pager( "\r\n    Name    | Gate Room | Low Proto Vnum | High Proto Vnum | Phantom Mapping Vnum\r\n", ch );
	   send_to_pager( "------------+-----------+----------------+-----------------+---------------------\r\n", ch );
	
	   for( pPrototype = first_instance_prototype; pPrototype; pPrototype = pPrototype->next )
	      pager_printf( ch, "%-11s | %-9d | %-14d | %-15d | %-20d \r\n",
	                    pPrototype->name,pPrototype->gate_room,pPrototype->low_proto_vnum,pPrototype->high_proto_vnum,
			    pPrototype->phantom_mapping_vnum);
	   return;
   }

   if( !str_cmp( arg, "list" ) )
   {
	   set_pager_color( AT_PLAIN, ch );
	   send_to_pager( "\r\n  Prototype  | First Vnum | Players\r\n", ch );
	   send_to_pager( "-------------+------------+---------\r\n", ch );
	
	   for( pInstance = first_instance; pInstance; pInstance = pInstance->next ){
		   pcount=0;
		   for( pChar = pInstance->first_char; pChar; pChar = pChar->next_in_instance )
			pcount++;
		pager_printf( ch, "%-12s | %-10d | %-12d \r\n",
	                    pInstance->prototype->name,pInstance->first_vnum,pcount);
		}
	      
	   return;
   }


   if( !str_cmp( arg, "save" ) )
   {
	   sprintf( buf, "%sinstances.dat", SYSTEM_DIR );
           send_to_char( "Writing instances file.\r\n", ch );
	   write_instance_file(buf);
           send_to_char( "OK.\r\n", ch );
	   return;
   }

   if( !str_cmp( arg, "invoke" ) )
   {

	if( arg2[0] == '\0' || !str_cmp( arg2, "?" ) )
	{	
	      send_to_char( "Syntax: instances invoke <prototype name>\r\n", ch );
	      return;
	}
	if( ( pPrototype = instance_prototype_from_name( arg2 ) ) == NULL )
	{
		sprintf( buf, "%s is not a valid instance prototype.\r\n", arg2 );
		send_to_char( buf, ch );
		return;
	}

	pInstance = invoke_instance( pPrototype );
	sprintf( buf, "Instance invoked.  First vnum: %d\r\n", pInstance->first_vnum );
	send_to_char( buf, ch );
	return;
    }

   if( !str_cmp( arg, "newprototype" ) )
   {

	if( arg2[0] == '\0' || !str_cmp( arg2, "?" ) )
	{	
	      send_to_char( "Syntax: instances newprototype <name>\r\n", ch );
	      return;
	}

	for( pPrototype = first_instance_prototype; pPrototype; pPrototype = pPrototype->next ) {
		if(!str_cmp( arg2, pPrototype->name ))
		{
			sprintf( buf, "The name %s is already in use!\r\n", pPrototype->name );
			send_to_char( buf, ch );
			return;
		}
	}
	CREATE(pPrototype,INSTANCE_PROTOTYPE_DATA,1);
	pPrototype->name = STRALLOC( arg2 );
	pPrototype->low_proto_vnum = 0;
	pPrototype->high_proto_vnum = 0;
	pPrototype->flags = 0;
	pPrototype->phantom_mapping_vnum = 0;
	pPrototype->gate_room = 0;
	pPrototype->first_instance = NULL;
	pPrototype->last_instance = NULL;
	pPrototype->prev = NULL;
	pPrototype->next = NULL;
	LINK(pPrototype,first_instance_prototype,last_instance_prototype,next,prev);
	send_to_char( "OK.", ch );
	return;
   }

   if( !str_cmp( arg, "setprototype" ) )
   {

	if( arg2[0] == '\0' || !str_cmp( arg2, "?" ) || arg3[0] == '\0' || arg3[0] == '\0' )
	{	
	      send_to_char( "Syntax: instances setprototype <prototype name> <variable> <value>\r\n", ch );
	      send_to_char( "Variable being one of:\r\n", ch );
	      send_to_char( "  lowprotovnum  highprotovnum  gateroom\r\n", ch );
	      send_to_char( "  pmv\r\n", ch );

	      return;
	}
	if( ( pPrototype = instance_prototype_from_name( arg2 ) ) == NULL )
	{
		sprintf( buf, "%s is not a valid instance prototype.\r\n", arg2 );
		send_to_char( buf, ch );
		return;
	}
	if( !str_cmp( arg3, "lowprotovnum" ) )
   	{
		vnum = atoi( arg4 );
		if (vnum > 65536 || !is_valid_vnum( vnum, VCHECK_ROOM ))
		{
			send_to_char( "Invalid vnum.\r\n", ch );
	      		return;
		}

		if( pPrototype->high_proto_vnum != 0 && vnum >= pPrototype->high_proto_vnum )
		{
			send_to_char( "highprotovnum larger than lowprotovnum and/or highprotovnum has to be changed!\r\n", ch );
	      		return;
		}

		if ( pPrototype->high_proto_vnum != 0 && pPrototype->high_proto_vnum - vnum < 10)
		{
			send_to_char( "Instance prototypes cannot have fewer than 10 rooms!\r\n", ch );
	      		return;
		}

		pPrototype->low_proto_vnum = vnum;
		send_to_char( "OK.\r\n", ch );
     		return;			
	
	}
	if( !str_cmp( arg3, "highprotovnum" ) )
   	{
		vnum = atoi( arg4 );
		if (vnum > 65536 || !is_valid_vnum( vnum, VCHECK_ROOM ))
		{
			send_to_char( "Invalid vnum.\r\n", ch );
	      		return;
		}

		if (vnum <= pPrototype->low_proto_vnum || pPrototype->low_proto_vnum == 0)
		{
			send_to_char( "highprotovnum larger than lowprotovnum and/or lowprotovnum is not set!\r\n", ch );
	      		return;
		}
	
		if (vnum - pPrototype->low_proto_vnum < 10)
		{
			send_to_char( "Instance prototypes cannot have fewer than 10 rooms!\r\n", ch );
	      		return;
		}
	
		pPrototype->high_proto_vnum = vnum;
		send_to_char( "OK.\r\n", ch );
      		return;			
	
	}
	if( !str_cmp( arg3, "gateroom" ) )
   	{
		vnum = atoi( arg4 );
		if ((room = get_room_index( vnum ))==NULL)
		{
			send_to_char( "That room does not exist yet!\r\n", ch );
	      		return;
		}
		else
		{
			pPrototype->gate_room = vnum;
			send_to_char( "OK.\r\n", ch );
	      		return;			
		}
	}

	if( !str_cmp( arg3, "pmv" ) )
   	{
		pmv = atoi( arg4 );
		if ( pmv < 65536 || pmv > 131072 )
		{
			send_to_char( "Phantom Mapping Vnum must be between 65536 and 131072!\r\n", ch );
	      		return;
		}

		if ( pPrototype->low_proto_vnum == 0 || pPrototype->high_proto_vnum == 0 )
		{
			send_to_char( "Proto vnums must be set first!\r\n", ch );
	      		return;
		}

		size = pPrototype->high_proto_vnum - pPrototype->low_proto_vnum;
		for(tpmv = pmv; tpmv < pmv + size; tpmv++)
		{
			if((tPrototype = instance_prototype_from_pmv(tpmv)) != NULL)
			{
				sprintf( buf, "Overlaps with PMV allocation from instance %s\r\n", tPrototype->name );
				send_to_char( buf, ch );
				return;				
			}
		}

		pPrototype->phantom_mapping_vnum = pmv;
		send_to_char( "OK.\r\n", ch );
      		return;			
	}
   }

   send_to_char( "unknown command\r\n", ch );
   return;
}

INSTANCE_DATA *invoke_instance( INSTANCE_PROTOTYPE_DATA *pPrototype )
{
	INSTANCE_DATA *pInstance;
	ROOM_INDEX_DATA *tRoom;
	ROOM_INDEX_DATA *vRoom;
	EXIT_DATA *pExit,*tExit;;
	RESET_DATA *pReset;
	EXTRA_DESCR_DATA *pED, *tED;

	int i;

	//create!
	CREATE(pInstance,INSTANCE_DATA,1);
	//Get first room
	pInstance->first_vnum = next_instance_vnum();
	pInstance->prototype = pPrototype;
	pInstance->prev = NULL;
	pInstance->next = NULL;
	pInstance->first_char = NULL;
	pInstance->last_char = NULL;
	LINK(pInstance,first_instance,last_instance,next,prev);
	LINK(pInstance,pPrototype->first_instance,pPrototype->last_instance,next_instance,prev_instance);

	i = 0;
	//copy rooms
	for( i = pPrototype->low_proto_vnum; i <= pPrototype->high_proto_vnum; i++)
		if ((tRoom = get_room_index( i ))!=NULL)
		{

			vRoom = make_room(pInstance->first_vnum + (i-pPrototype->low_proto_vnum) , tRoom->area);
                  	STRFREE( vRoom->name );
			vRoom->name = STRALLOC( tRoom->name );
                  	STRFREE( vRoom->description );
			vRoom->description = STRALLOC( tRoom->description );
			vRoom->instance = pInstance;
			vRoom->room_flags = tRoom->room_flags;
			vRoom->progtypes = tRoom->progtypes;
			vRoom->light = tRoom->light;
			vRoom->sector_type = tRoom->sector_type;
			vRoom->tele_vnum = tRoom->tele_vnum;
			if(vRoom->tele_vnum >= pPrototype->low_proto_vnum && vRoom->tele_vnum <= pPrototype->high_proto_vnum)
				vRoom->tele_vnum = (pInstance->first_vnum) + ( vRoom->tele_vnum - pPrototype->low_proto_vnum );
			vRoom->tele_delay = tRoom->tele_delay;
			vRoom->tunnel = tRoom->tunnel;

			//copy reset data
			for( pReset = tRoom->first_reset; pReset; pReset = pReset->next )
				add_reset( vRoom, pReset->command, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
			//we will use the resets to setup the room

			for ( pED = tRoom->first_extradesc; pED; pED = pED->next )
			{
				CREATE(tED,EXTRA_DESCR_DATA,1);
				tED->keyword = STRALLOC(pED->keyword);
				tED->description = STRALLOC(pED->description);
				LINK(tED,vRoom->first_extradesc,vRoom->last_extradesc,next,prev);
			}

		}
	for( i = pPrototype->low_proto_vnum; i <= pPrototype->high_proto_vnum; i++)
		if ((tRoom = get_room_index( i ))!=NULL)
		{ //do the exits in a second pass
			vRoom = get_room_index(pInstance->first_vnum + (i-pPrototype->low_proto_vnum));
			//exits
			for(pExit = tRoom->first_exit; pExit; pExit = pExit->next) {
				if(pExit->to_room == NULL)
					continue; //valid exits only i guess
				else if( pExit->to_room->vnum >= pPrototype->low_proto_vnum 
				   && pExit->to_room->vnum <= pPrototype->high_proto_vnum)
				tExit = make_exit(vRoom,
						get_room_index(pInstance->first_vnum + (pExit->to_room->vnum - pPrototype->low_proto_vnum)),
						pExit->vdir);
				else
					tExit = make_exit(vRoom,pExit->to_room,pExit->vdir);
				tExit->description = STRALLOC(pExit->description);
               			tExit->keyword = STRALLOC(pExit->keyword);
				tExit->exit_info = pExit->exit_info;
		                tExit->key = pExit->key;
/*				sprintf( log_buf, "Exit linked from %d to vnum %d", vRoom->vnum, tExit->to_room->vnum );
				to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 100 ); 

				reset_room(vRoom); //sneaky sneaky
				sprintf( log_buf, "Added room to instance %s, vnum %d", pPrototype->name, vRoom->vnum );
				to_channel( log_buf, CHANNEL_MONITOR, "Monitor", 100 ); */
			}
		}
	return pInstance;
}
