
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

typedef struct exception_s
{
	bool abort;
	bool print;
	std::string msg;
	
	exception_s(bool a, bool p, const char *m) : msg(m)
	{
		abort = a;
		print = p;
	}
} exception_t;

uint32_t tickcount();
uint32_t from_bcd(uint32_t bcd);
uint32_t mjdhms_to_timestamp(uint32_t mjd, uint32_t hours, uint32_t minutes, uint32_t seconds);
void load_args(int argc, char *argv[], std::string *config_file, std::string *crossepg_lib, std::string *crossepg_data);
void load_config(const char *path, std::string *demux_path, std::string *title_format, std::string *desc_format, std::string *service_reference, std::string *webif_user, std::string *webif_pass, uint16_t *webif_port, uint32_t *max_retries, bool *crossepg_clear);
void detecting_demux(std::string *demux_path);
void webif_get(const char *user, const char *pass, uint16_t port, std::string *ref);
void webif_zap(const char *user, const char *pass, uint16_t port, const char *ref);
bool str_enclosed(const char *text, char a, char b, std::string *ret_enclosed);
bool str_endswith(const char *text, uint32_t len, ...);
const char *str_quote(const char *str);
const char *str_trim(const char *text);
bool str_isnum(const char *text);
void set_title(const char *title_full, const char *desc_full, std::string *ret_title, std::string *ret_desc, std::string *ret_season, std::string *ret_episode, std::string *ret_episode_num);
const char *fmt_close(const char *fmt, const char *code);
const char *fmt_open(const char *fmt, const char *code, bool close, std::string *ret_params, std::string *ret_options, std::string *ret_prevfmt, std::string *ret_nextfmt);
bool fmt_process(const char *fmt, const char *code, bool close, std::string *ret_params, std::string *ret_options, std::string *ret_prevfmt, std::string *ret_nextfmt);
void fmt_upper(const char *text, std::string *ret_formatted);
void fmt_lower(const char *text, std::string *ret_formatted);
void fmt_title(const char *fmt, const char *title_full, const char *desc_full, const char *title, const char *desc, const char *season, const char *episode, const char *episode_num, std::string *ret_formatted);
