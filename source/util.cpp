
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

uint32_t tickcount()
{
	uint32_t ticks, usec, sec;
	struct timeval tempo;
	gettimeofday(&tempo, NULL);
	usec = tempo.tv_usec;
	sec = tempo.tv_sec;
	ticks = (1000 * sec) + (usec / 1000);
	return ticks;
}

uint32_t from_bcd(uint32_t bcd)
{
	if((bcd & 0xF0) >= 0xA0)
		return 0;
	
	if((bcd & 0xF) >= 0xA)
		return 0;
	
	return ((bcd & 0xF0) >> 4) * 10 + (bcd & 0xF);
}

uint32_t mjdhms_to_timestamp(uint32_t mjd, uint32_t hours, uint32_t minutes, uint32_t seconds)
{
	return ((mjd - 40587) * 86400) + (from_bcd(hours) * 3600) + (from_bcd(minutes) * 60) + from_bcd(seconds);
}

void load_args(int argc, char *argv[], std::string *config_file, std::string *crossepg_lib, std::string *crossepg_data)
{
	bool help = false;
	
	config_file->clear();
	crossepg_lib->clear();
	crossepg_data->clear();
	
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config-file") == 0)
		{
			if(++i < argc)
				config_file->assign(argv[i]);
			
			continue;
		}
		
		if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--crossepg-lib") == 0)
		{
			if(++i < argc)
				crossepg_lib->assign(argv[i]);
			
			continue;
		}
		
		if(strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--crossepg-data") == 0)
		{
			if(++i < argc)
				crossepg_data->assign(argv[i]);
			
			continue;
		}
		
		if(strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--work-dir") == 0)
		{
			if(++i < argc)
				chdir(argv[i]);
			
			continue;
		}
		
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			help = true;
			continue;
		}
	}
	
	if(help || config_file->empty() || crossepg_lib->empty() || crossepg_data->empty())
	{
		printf("Uso: %s [argumentos]\n", argv[0]);
		printf("\n");
		printf("Los argumentos obligatorios son las siguientes:\n");
		printf("\n");
		printf("  -c,  --config-file               Archivo de configuración\n");
		printf("  -l,  --crossepg-lib              Archivo de librería de crossepg\n");
		printf("  -d,  --crossepg-data             Directorio de datos de crossepg\n");
		printf("\n");
		printf("Los argumentos opcionales son los siguientes:\n");
		printf("\n");
		printf("  -w,  --work-dir                  Directorio de trabajo\n");
		printf("  -h,  --help                      Muestra esta ayuda\n");
		printf("\n");
		
		throw exception_t(true, false, "");
	}
}

void load_config(const char *path, std::string *demux_path, std::string *title_format, std::string *desc_format, std::string *service_reference, std::string *webif_user, std::string *webif_pass, uint16_t *webif_port, uint32_t *max_retries, bool *crossepg_clear)
{
	demux_path->clear();
	title_format->clear();
	desc_format->clear();
	service_reference->clear();
	webif_user->clear();
	webif_pass->clear();
	*webif_port = 0;
	*max_retries = ~0;
	
	FILE *file = fopen(path, "r");
	
	if(file != NULL)
	{
		char buf[1024];
		char key[24], val[1024];
		
		while(!feof(file))
		{
			if(fgets(buf, 1024, file) == NULL)
				break;
			
			if(buf[0] == '#' || buf[0] == '\r' || buf[0] == '\n' || buf[0] == '\0')
				continue;
			
			if(sscanf(buf, "%24[^=]=%1024[^\r\n]", key, val) != 2)
				continue;
			
			if(strcmp(key, "demux-path") == 0)
			{
				demux_path->assign(val);
				continue;
			}
			
			if(strcmp(key, "title-format") == 0)
			{
				title_format->assign(val);
				continue;
			}
			
			if(strcmp(key, "desc-format") == 0)
			{
				desc_format->assign(val);
				continue;
			}
			
			if(strcmp(key, "service-reference") == 0)
			{
				service_reference->assign(val);
				continue;
			}
			
			if(strcmp(key, "webif-user") == 0)
			{
				webif_user->assign(val);
				continue;
			}
			
			if(strcmp(key, "webif-pass") == 0)
			{
				webif_pass->assign(val);
				continue;
			}
			
			if(strcmp(key, "webif-port") == 0)
			{
				*webif_port = atoi(val) & 0xFFFF;
				continue;
			}
			
			if(strcmp(key, "max-retries") == 0)
			{
				*max_retries = atoi(val);
				continue;
			}
			
			if(strcmp(key, "crossepg-clear") == 0)
			{
				*crossepg_clear = atoi(val) != 0;
				continue;
			}
		}
		
		fclose(file);
		
		if(demux_path->empty())
			throw exception_t(true, true, "Error al leer el campo 'demux-path' desde el archivo de configuración");
		
		if(title_format->empty())
			throw exception_t(true, true, "Error al leer el campo 'title-format' desde el archivo de configuración");
		
		if(desc_format->empty())
			throw exception_t(true, true, "Error al leer el campo 'desc-format' desde el archivo de configuración");
		
		if(service_reference->empty())
			throw exception_t(true, true, "Error al leer el campo 'service-reference' desde el archivo de configuración");
		
		//if(webif_user->empty())
		//	throw exception_t(true, true, "Error al leer el campo 'webif-user' desde el archivo de configuración");
		
		//if(webif_pass->empty())
		//	throw exception_t(true, true, "Error al leer el campo 'webif-pass' desde el archivo de configuración");
		
		if(*webif_port == 0)
			throw exception_t(true, true, "Error al leer el campo 'webif-port' desde el archivo de configuración");
		
		if(*max_retries == ~0)
			throw exception_t(true, true, "Error al leer el campo 'max-retries' desde el archivo de configuración");
	}
	else
	{
		throw exception_t(true, true, "Error al abrir el archivo de configuración");
	}
}

void detecting_demux(std::string *demux_path)
{
	for(uint32_t i = 0; i < 4; i++)
	{
		for(uint32_t j = 0; j < 4; j++)
		{
			char path[48];
			
			sprintf(path, "/dev/dvb/adapter%u/demux%u", i, j);
			
			struct stat stbuf;
			
			if(stat(path, &stbuf) == -1)
				continue;
			
			try
			{
				CFilter filter(path);
				filter.SetFilter(0x231, 0xC8, 0xFF);
				
				uint8_t *buf;
				int len = filter.Read(&buf);
				
				if(len != 0)
				{
					delete [] buf;
					
					demux_path->assign(path);
					return;
				}
			}
			catch(const exception_t &exception)
			{
				
			}
		}
	}
	
	throw exception_t(true, true, "Error al detectar el adaptador demux");
}

void webif_get(const char *user, const char *pass, uint16_t port, std::string *ref)
{
	ref->clear();
	
	char buf[256];
	sprintf(buf, "wget -O - -q http://%s:%s@127.0.0.1:%u/web/subservices", user, pass, port);
	
	FILE *file = popen(buf, "r");
	
	if(file != NULL)
	{
		while(!feof(file))
		{
			if(fgets(buf, 256, file) == NULL)
				break;
			
			char *str1 = strstr(buf, "<e2servicereference>");
			
			if(str1 == NULL)
				continue;
			
			str1 += 20;
			
			char *str2 = strstr(str1, "</e2servicereference>");
			
			if(str2 == NULL)
				continue;
			
			str2[0] = '\0';
			
			ref->assign(str1);
			
			fclose(file);
			return;
		}
		
		fclose(file);
	}
	
	throw exception_t(false, true, "Error al obtener el canal actual desde webif");
}

void webif_zap(const char *user, const char *pass, uint16_t port, const char *ref)
{
	char buf[256];
	sprintf(buf, "wget -O - -q http://%s:%s@127.0.0.1:%u/web/zap?sRef=%s", user, pass, port, ref);
	
	FILE *file = popen(buf, "r");
	
	if(file != NULL)
	{
		while(!feof(file))
		{
			if(fgets(buf, 256, file) == NULL)
				break;
			
			char *str1 = strstr(buf, "<e2state>");
			
			if(str1 == NULL)
				continue;
			
			str1 += 9;
			
			char *str2 = strstr(str1, "</e2state>");
			
			if(str2 == NULL)
				continue;
			
			str2[0] = '\0';
			
			if(strcmp(str1, "False") == 0)
				break;
			
			fclose(file);
			return;
		}
		
		fclose(file);
	}
	
	throw exception_t(false, true, "Error al cambiar de canal desde webif");
}

bool str_enclosed(const char *text, char a, char b, std::string *ret_enclosed)
{
	if(text[0] == a)
	{
		for(uint32_t i = 1; text[i] != '\0'; i++)
		{
			if(text[i] == b)
			{
				ret_enclosed->assign(text + 1, i - 1);
				return true;
			}
		}
	}
	
	return false;
}

bool str_endswith(const char *text, uint32_t len, ...)
{
	va_list args;
	va_start(args, len);
	
	std::string str;
	
	while(true)
	{
		const char *ends = va_arg(args, const char *);
		
		if(ends == NULL)
			break;
		
		str.append(ends);
	}
	
    va_end(args);
    
	return str.length() <= len && strncmp(text + len - str.length(), str.c_str(), str.length()) == 0;
}

const char *str_quote(const char *str)
{
	const char *ret = NULL;
	
	while(str[0] != '\0')
	{
		if(str[0] == '"')
		{
			if(ret != NULL)
			{
				ret = NULL;
				break;
			}
			
			ret = str;
		}
		
		str++;
	}
	
	return ret;
}

const char *str_trim(const char *text)
{
	while(isspace(text[0]))
		text++;
	
	return text;
}

bool str_isnum(const char *text)
{
	while(*text != '\0')
	{
		if(!isdigit(*text))
			return false;
		
		text++;
	}
	
	return true;
}

void set_title(const char *title_full, const char *desc_full, std::string *title, std::string *desc, std::string *ret_season, std::string *episode, std::string *ret_episode_num)
{
	if(desc_full[0] == '"')
	{
		uint32_t idx = ~0;
		uint32_t len = strlen(title_full);
		
		const char *str = desc_full + 1;
		
		if(len >= 3 && title_full[len - 3] == '.' && title_full[len - 2] == '.' && title_full[len - 1] == '.')
		{
			len -= 3;
			
			if(strncmp(title_full, str, len) == 0)
			{
				const char *end = str_quote(str + len);
				
				if(end != NULL)
				{
					title->assign(str, (uint32_t)(end - str));
					desc->assign(str_trim(end + 1));
					
					return;
				}
			}
			else
			{
				if(len >= 2 && title_full[len - 2] == ':' || title_full[len - 1] == ' ')
				{
					idx  = 0;
					len -= 2;
				}
				else if(len >= 1 && title_full[len - 1] == ':')
				{
					idx  = 0;
					len -= 2;
				}
				else
				{
					for(uint32_t i = 2; i < len; i++)
					{
						if(title_full[i - 2] == ':' && title_full[i - 1] == ' ' && strncmp(title_full + i, str, len - i) == 0)
						{
							idx = len - i;
							len = i - 2;
							
							break;
						}
					}
				}
				
				if(idx != ~0)
				{
					const char *end = str_quote(str + idx);
					
					if(end != NULL)
					{
						idx = (uint32_t)(end - str);
					}
					else
					{
						idx = ~0;
					}
				}
			}
		}
		else
		{
			for(uint32_t i = 2; i < len; i++)
			{
				if(title_full[i - 2] == ':' && title_full[i - 1] == ' ' && strncmp(title_full + i, str, len - i) == 0 && str[len - i] == '"')
				{
					idx = len - i;
					len = i - 2;
					
					break;
				}
			}
		}
		
		if(idx != ~0)
		{
			episode->assign(str, idx);
			
			std::string ret;
			
			if(str_enclosed(str + idx + 1, ' ', ' ', &ret) && str_endswith(title_full, len, " (", ret.c_str(), ")", NULL))
			{
				len -= ret.length() + 3;
				idx += ret.length() + 2;
				
				ret_season->assign(ret);
				
				if(strncmp(str + idx + 1, "Ep.", 3) == 0 && str_enclosed(str + idx + 3, '.', ' ', &ret) && str_isnum(ret.c_str()))
				{
					idx += ret.length() + 4;
					
					ret_episode_num->assign(ret);
				}
			}
			
			title->assign(title_full, len);
			desc->assign(str_trim(str + idx + 1));
			
			return;
		}
	}
	
	title->assign(title_full);
	desc->assign(desc_full);
}

const char *fmt_close(const char *fmt, const char *code)
{
	size_t length = strlen(code);

	const char *str = fmt;
	while(str[0] != '\0')
	{
		if(str[0] == '{' && str[1] == '/')
		{
			const char *str1 = str + 2;
			const char *str2 = str1;

			while(str2[0] != '\0' && str2[0] != '}')
				str2++;

			if(str2[0] == '}' && str1 != str2)
			{
				if((str2 - str1) == length && strncmp(code, str1, length) == 0)
					return str;
			}
		}
		else if(str[0] == '{')
		{
			const char *str1 = str + 1;
			const char *str2 = str1;
			const char *tag3 = str1;

			while(tag3[0] != '\0' && tag3[0] != '}')
			{
				if(str2[0] != ':')
					str2++;

				tag3++;
			}

			if(tag3[0] == '}' && str1 != str2 && str1 != tag3)
			{
				if((str2 - str1) == length && strncmp(code, str1, length) == 0)
				{
					const char *tag4 = fmt_close(tag3, code);

					if(tag4 != NULL)
					{
						str = tag4 + length + 3;
						continue;
					}
				}
			}
		}

		str++;
	}
	
	return NULL;
}


const char *fmt_open(const char *fmt, const char *code, bool close, std::string *ret_params, std::string *ret_options, std::string *ret_prevfmt, std::string *ret_nextfmt)
{
	size_t length = strlen(code);

	const char *str = fmt;
	while(str[0] != '\0')
	{
		if(str[0] == '{')
		{
			const char *str1 = str + 1;
			const char *str2 = str1;
			const char *str3 = str1;

			while(str3[0] != '\0' && str3[0] != '}')
			{
				if(str2[0] != ':')
					str2++;

				str3++;
			}
			
			if(str3[0] == '}' && str1 != str2 && str1 != str3)
			{
				if((str2 - str1) == length && strncmp(code, str1, length) == 0)
				{
					if(close)
					{
						const char *str4 = fmt_close(str3 + 1, code);

						if(str4 != NULL)
						{
							if(str2[0] == ':')
								str2++;
							
							ret_params->assign(str2, (uint32_t)(str3 - str2));
							ret_options->assign(str3 + 1, (uint32_t)(str4 - str3 - 1));
							ret_prevfmt->assign(fmt, (uint32_t)(str - fmt));
							ret_nextfmt->assign(str4 + length + 3);
							
							return str;
						}
					}
					else
					{
						if(str2[0] == ':')
							str2++;
						
						ret_params->assign(str2, (uint32_t)(str3 - str2));
						ret_options->assign("");
						ret_prevfmt->assign(fmt, (uint32_t)(str - fmt));
						ret_nextfmt->assign(str3 + 1);
						
						return str;
					}
				}
			}
		}

		str++;
	}

	return NULL;
}

bool fmt_process(const char *fmt, const char *code, bool close, std::string *ret_params, std::string *ret_options, std::string *ret_prevfmt, std::string *ret_nextfmt)
{
	return fmt_open(fmt, code, close, ret_params, ret_options, ret_prevfmt, ret_nextfmt) != NULL;
}

void fmt_upper(const char *text, std::string *ret_formatted)
{
	while(text[0] != '\0')
	{
		char c = text[0];
		
		if(c == 'á') c = 'Á';
		else if(c == 'à') c = 'À';
		else if(c == 'ä') c = 'Ä';
		else if(c == 'â') c = 'Â';
		else if(c == 'é') c = 'É';
		else if(c == 'è') c = 'È';
		else if(c == 'ë') c = 'Ë';
		else if(c == 'ê') c = 'Ê';
		else if(c == 'í') c = 'Í';
		else if(c == 'ì') c = 'Ì';
		else if(c == 'ï') c = 'Ï';
		else if(c == 'î') c = 'Î';
		else if(c == 'ó') c = 'Ó';
		else if(c == 'ò') c = 'Ò';
		else if(c == 'ö') c = 'Ö';
		else if(c == 'ô') c = 'Ô';
		else if(c == 'ú') c = 'Ú';
		else if(c == 'ù') c = 'Ù';
		else if(c == 'ü') c = 'Ü';
		else if(c == 'û') c = 'Û';
		else if(c == 'ý') c = 'Ý';
		else if(c == 'ñ') c = 'Ñ';
		else if(c == 'ç') c = 'Ç';
		else if(c == 'ø') c = 'Ø';
		else if(c == 'æ') c = 'Æ';
		else c = toupper(c);
		
		ret_formatted->append(1, c);
		text++;
	}
}

void fmt_lower(const char *text, std::string *ret_formatted)
{
	while(text[0] != '\0')
	{
		char c = text[0];
		
		switch(c)
		{
			if(c == 'Á') c = 'á';
			else if(c == 'À') c = 'à';
			else if(c == 'Ä') c = 'ä';
			else if(c == 'Â') c = 'â';
			else if(c == 'É') c = 'é';
			else if(c == 'È') c = 'è';
			else if(c == 'Ë') c = 'ë';
			else if(c == 'Ê') c = 'ê';
			else if(c == 'Í') c = 'í';
			else if(c == 'Ì') c = 'ì';
			else if(c == 'Ï') c = 'ï';
			else if(c == 'Î') c = 'î';
			else if(c == 'Ó') c = 'ó';
			else if(c == 'Ò') c = 'ò';
			else if(c == 'Ö') c = 'ö';
			else if(c == 'Ô') c = 'ô';
			else if(c == 'Ú') c = 'ú';
			else if(c == 'Ù') c = 'ù';
			else if(c == 'Ü') c = 'ü';
			else if(c == 'Û') c = 'û';
			else if(c == 'Ý') c = 'ý';
			else if(c == 'Ñ') c = 'ñ';
			else if(c == 'Ç') c = 'ç';
			else if(c == 'Ø') c = 'ø';
			else if(c == 'Æ') c = 'æ';
			else c = tolower(c);
		}
		
		ret_formatted->append(1, c);
		text++;
	}
}

void fmt_title(const char *fmt, const char *title_full, const char *desc_full, const char *title, const char *desc, const char *season, const char *episode, const char *episode_num, std::string *ret_formatted)
{
	std::string params;
	std::string options;
	std::string prevfmt;
	std::string nextfmt;
	
	if(fmt_process(fmt, "if", true, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		
		if(strcmp(params.c_str(), "title-full") == 0)
		{
			if(title_full[0] != '\0')
				fmt_title(options.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		}
		else if(strcmp(params.c_str(), "desc-full") == 0)
		{
			if(desc_full[0] != '\0')
				fmt_title(options.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		}
		else if(strcmp(params.c_str(), "title") == 0)
		{
			if(title[0] != '\0')
				fmt_title(options.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		}
		else if(strcmp(params.c_str(), "desc") == 0)
		{
			if(desc[0] != '\0')
				fmt_title(options.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		}
		else if(strcmp(params.c_str(), "season") == 0)
		{
			if(season[0] != '\0')
				fmt_title(options.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		}
		else if(strcmp(params.c_str(), "episode") == 0)
		{
			if(episode[0] != '\0')
				fmt_title(options.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		}
		else if(strcmp(params.c_str(), "episode-num") == 0)
		{
			if(episode_num[0] != '\0')
				fmt_title(options.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		}
		
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "upper", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		
		if(strcmp(params.c_str(), "title-full") == 0)
		{
			fmt_upper(title_full, ret_formatted);
		}
		else if(strcmp(params.c_str(), "desc-full") == 0)
		{
			fmt_upper(desc_full, ret_formatted);
		}
		else if(strcmp(params.c_str(), "title") == 0)
		{
			fmt_upper(title, ret_formatted);
		}
		else if(strcmp(params.c_str(), "desc") == 0)
		{
			fmt_upper(desc, ret_formatted);
		}
		else if(strcmp(params.c_str(), "season") == 0)
		{
			fmt_upper(season, ret_formatted);
		}
		else if(strcmp(params.c_str(), "episode") == 0)
		{
			fmt_upper(episode, ret_formatted);
		}
		else if(strcmp(params.c_str(), "episode-num") == 0)
		{
			fmt_upper(episode_num, ret_formatted);
		}
		
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "lower", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		
		if(strcmp(params.c_str(), "title-full") == 0)
		{
			fmt_lower(title_full, ret_formatted);
		}
		else if(strcmp(params.c_str(), "desc-full") == 0)
		{
			fmt_lower(desc_full, ret_formatted);
		}
		else if(strcmp(params.c_str(), "title") == 0)
		{
			fmt_lower(title, ret_formatted);
		}
		else if(strcmp(params.c_str(), "desc") == 0)
		{
			fmt_lower(desc, ret_formatted);
		}
		else if(strcmp(params.c_str(), "season") == 0)
		{
			fmt_lower(season, ret_formatted);
		}
		else if(strcmp(params.c_str(), "episode") == 0)
		{
			fmt_lower(episode, ret_formatted);
		}
		else if(strcmp(params.c_str(), "episode-num") == 0)
		{
			fmt_lower(episode_num, ret_formatted);
		}
		
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "title-full", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		ret_formatted->append(title_full);
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "desc-full", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		ret_formatted->append(desc_full);
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "title", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		ret_formatted->append(title);
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "desc", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		ret_formatted->append(desc);
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "season", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		ret_formatted->append(season);
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "episode", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		ret_formatted->append(episode);
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else if(fmt_process(fmt, "episode-num", false, &params, &options, &prevfmt, &nextfmt))
	{
		fmt_title(prevfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
		ret_formatted->append(episode_num);
		fmt_title(nextfmt.c_str(), title_full, desc_full, title, desc, season, episode, episode_num, ret_formatted);
	}
	else
	{
		ret_formatted->append(fmt);
	}
}
