
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

#pragma once

#include "main.h"

#define DB_REVISION	0x07

typedef struct epgdb_title_s
{
	uint16_t event_id;
	uint16_t mjd;
	uint32_t start_time;
	uint16_t length;
	uint8_t genre_id;
	uint8_t flags;
	uint32_t description_crc;
	uint32_t description_seek;
	uint32_t long_description_crc;
	uint32_t long_description_seek;
	uint16_t description_length;
	uint16_t long_description_length;
	uint8_t iso_639_1;
	uint8_t iso_639_2;
	uint8_t iso_639_3;
	uint8_t revision;
	
	bool changed;
	struct epgdb_title_s *prev;
	struct epgdb_title_s *next;
} epgdb_title_t;

typedef struct epgdb_index_s
{
	uint32_t crc;
	uint32_t seek;
	uint16_t length;
	
	uint8_t used;
	struct epgdb_index_s *prev;
	struct epgdb_index_s *next;
} epgdb_index_t;

typedef struct epgdb_alias_s
{
	uint16_t nid;
	uint16_t tsid;
	uint16_t sid;
} epgdb_alias_t;

typedef struct epgdb_channel_s
{
	uint16_t nid;
	uint16_t tsid;
	uint16_t sid;
	
	struct epgdb_channel_s *prev;
	struct epgdb_channel_s *next;
	struct epgdb_title_s *title_first;
	struct epgdb_title_s *title_last;
	
	epgdb_alias_t *aliases;
	uint8_t aliases_count;
} epgdb_channel_t;

typedef bool				(*epgdb_open_t)							(const char *db_root);
typedef bool				(*epgdb_save_t)							(void(*progress_callback)(int, int));
typedef int					(*epgdb_get_revision_t)					();

typedef epgdb_channel_t *	(*epgdb_channels_add_t)					(uint16_t nid, uint16_t tsid, uint16_t sid);
typedef void				(*epgdb_channels_reset_t)				();

typedef epgdb_channel_t *	(*epgdb_aliases_add_t)					(epgdb_channel_t *channel, uint16_t nid, uint16_t tsid, uint16_t sid);
typedef void				(*epgdb_aliases_clear_t)				();

typedef epgdb_title_t *		(*epgdb_title_alloc_t)					();
typedef epgdb_title_t *		(*epgdb_titles_set_description_t)		(epgdb_title_t *title, const char *description);
typedef epgdb_title_t *		(*epgdb_titles_set_long_description_t)	(epgdb_title_t *title, const char *description);
typedef epgdb_title_t *		(*epgdb_titles_add_t)					(epgdb_channel_t *channel, epgdb_title_t *title);

typedef void				(*epgdb_index_init_t)					();

extern epgdb_open_t							epgdb_open;
extern epgdb_save_t							epgdb_save;
extern epgdb_get_revision_t					epgdb_get_revision;

extern epgdb_aliases_add_t					epgdb_aliases_add;
extern epgdb_aliases_clear_t				epgdb_aliases_clear;

extern epgdb_channels_add_t					epgdb_channels_add;
extern epgdb_channels_reset_t				epgdb_channels_reset;

extern epgdb_title_alloc_t					epgdb_title_alloc;
extern epgdb_titles_set_description_t		epgdb_titles_set_description;
extern epgdb_titles_set_long_description_t	epgdb_titles_set_long_description;
extern epgdb_titles_add_t					epgdb_titles_add;

extern epgdb_index_init_t					epgdb_index_init;

void epgdb_init(const char *crossepg_lib, const char *crossepg_data, bool crossepg_clear);
