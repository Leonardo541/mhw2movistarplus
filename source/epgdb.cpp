
/*
 * Copyright (C) 2018 Leonardo Almeida <leonardo.devcom@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include "main.h"

#define LIB_GETFUNC(func) 															\
																					\
	func = (func##_t)dlsym(lib, #func); 											\
																					\
	if(func == NULL) 																\
		throw exception_t(true, true, "Error al obtener la función " #func)

#define CLEAR_DATA(name)															\
																					\
	snprintf(filename, 256, "%s/%s", crossepg_data, name);							\
	remove(filename)
		
epgdb_open_t						epgdb_open							= NULL;
epgdb_save_t						epgdb_save							= NULL;
epgdb_get_revision_t				epgdb_get_revision					= NULL;

epgdb_aliases_add_t					epgdb_aliases_add					= NULL;
epgdb_aliases_clear_t				epgdb_aliases_clear					= NULL;

epgdb_channels_add_t				epgdb_channels_add					= NULL;
epgdb_channels_reset_t				epgdb_channels_reset				= NULL;

epgdb_title_alloc_t					epgdb_title_alloc					= NULL;
epgdb_titles_set_description_t		epgdb_titles_set_description		= NULL;
epgdb_titles_set_long_description_t	epgdb_titles_set_long_description	= NULL;
epgdb_titles_add_t					epgdb_titles_add					= NULL;

epgdb_index_init_t					epgdb_index_init					= NULL;

void epgdb_init(const char *crossepg_lib, const char *crossepg_data, bool crossepg_clear)
{
	void *lib = dlopen(crossepg_lib, RTLD_LAZY);
	
	if(lib != NULL)
	{
		LIB_GETFUNC(epgdb_open);
		LIB_GETFUNC(epgdb_save);
		LIB_GETFUNC(epgdb_get_revision);
		
		LIB_GETFUNC(epgdb_channels_add);
		LIB_GETFUNC(epgdb_channels_reset);
		
		LIB_GETFUNC(epgdb_aliases_add);
		LIB_GETFUNC(epgdb_aliases_clear);
		
		LIB_GETFUNC(epgdb_title_alloc);
		LIB_GETFUNC(epgdb_titles_set_description);
		LIB_GETFUNC(epgdb_titles_set_long_description);
		LIB_GETFUNC(epgdb_titles_add);
		
		LIB_GETFUNC(epgdb_index_init);
		
		if(epgdb_get_revision() != DB_REVISION)
			throw exception_t(true, true, "Error en la revisión de crossepg");
		
		char filename[256];
		
		if(crossepg_clear)
		{
			CLEAR_DATA("crossepg.headers.db");
			CLEAR_DATA("crossepg.descriptors.db");
			CLEAR_DATA("crossepg.indexes.db");
			CLEAR_DATA("crossepg.aliases.db");
		}
		
		if(!epgdb_open(crossepg_data))
			throw exception_t(true, true, "Error al abrir el directorio de datos de crossepg");
		
		epgdb_channels_reset();
		epgdb_index_init();
	}
	else
	{
		throw exception_t(true, true, "Error al abrir el archivo de librería de crossepg");
	}
}
