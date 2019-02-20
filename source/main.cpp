
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

std::string config_file;
std::string crossepg_lib;
std::string crossepg_data;
std::string demux_path;
std::string title_format;
std::string desc_format;
std::string service_reference;
std::string webif_user;
std::string webif_pass;
uint16_t webif_port = 0;
uint32_t max_retries = ~0;
bool crossepg_clear = false;

typedef struct buf_s
{
	uint8_t *buf;
	uint32_t len;
	
	buf_s(uint8_t *b, uint32_t l)
	{
		buf = b;
		len = l;
	}
	
	~buf_s()
	{
		if(buf != NULL)
			delete [] buf;
	}
} buf_t;

typedef struct summary_s
{
	epgdb_title_t *title;
	struct summary_s *next;
	
	summary_s(epgdb_title_t *t, struct summary_s *n)
	{
		title = t;
		next = n;
	}
	
	~summary_s()
	{
		if(next != NULL)
			delete next;
	}
} summary_t;

std::map<uint32_t, buf_t *> bufs;
std::vector<epgdb_channel_t *> channels;
std::map<uint32_t, epgdb_title_t *> titles;
std::vector<bool> summaries;
std::vector<summary_t *> summaries_titles;
uint32_t summaries_count = 0;

void bufs_clear()
{
	for(std::map<uint32_t, buf_t *>::iterator it = bufs.begin(); it != bufs.end(); it++)
		delete it->second;
	
	bufs.clear();
}

void summaries_clear()
{
	for(std::vector<summary_t *>::iterator it = summaries_titles.begin(); it != summaries_titles.end(); it++)
		delete (*it);
	
	summaries.clear();
	summaries_titles.clear();
	summaries_count = 0;
}

void mhw2_channels(CFilter *filter)
{
	printf("LOGTEXT DESCARGANDO LISTA DE CANALES\n");
	fflush(stdout);
	
	filter->SetFilter(0x231, 0xC8, 0xFF);
	
	uint32_t timeout = tickcount() + 15000;
	
	while(true)
	{
		uint8_t *buf;
		uint32_t len = filter->Read(&buf);
		
		if(len != 0)
		{
			CReader reader(buf, len);
			
			uint8_t type;
			
			try
			{
				reader.Read(type);
			}
			catch(const exception_t &exception)
			{
				delete [] buf;
				throw;
			}
			
			if(type == 0 || type == 2)
			{
				if(bufs.find(type) == bufs.end())
				{
					bufs[type] = new buf_t(buf, len);
					
					if(bufs.size() == 2)
						break;
					
					timeout = tickcount() + 15000;
					continue;
				}
			}
			
			delete [] buf;
		}
		
		if(timeout < tickcount())
			throw exception_t(false, true, "Tiempo de espera agotado");
	}
	
	CReader sd_reader(bufs[0]->buf, bufs[0]->len);
	CReader hd_reader(bufs[2]->buf, bufs[2]->len);
	
	uint8_t sd_num, hd_num;
	
	sd_reader.Seek(117);
	sd_reader.Read(sd_num);
	
	hd_reader.Seek(117);
	hd_reader.Read(hd_num);
	
	if(sd_num != hd_num)
		throw exception_t(false, true, "Número de canales no válido");
	
	channels.resize(sd_num);
	
	for(uint8_t i = 0; i < sd_num; i++)
	{
		uint16_t sd_nid, sd_tsid, sd_sid;
		uint16_t hd_nid, hd_tsid, hd_sid;
		
		sd_reader.Read(sd_nid);
		sd_reader.Read(sd_tsid);
		sd_reader.Read(sd_sid);
		sd_reader.Seek(2);
		
		hd_reader.Read(hd_nid);
		hd_reader.Read(hd_tsid);
		hd_reader.Read(hd_sid);
		hd_reader.Seek(2);
		
		epgdb_channel_t *channel = epgdb_channels_add(sd_nid, sd_tsid, sd_sid);
		
		if(sd_nid != hd_nid || sd_tsid != hd_tsid || sd_sid != hd_sid)
			 epgdb_aliases_add(channel, hd_nid, hd_tsid, hd_sid);
		
		channels[i] = channel;
	}
	
	bufs_clear();
}

void mhw2_titles(CFilter *filter)
{
	printf("LOGTEXT DESCARGANDO LISTA DE TÍTULOS\n");
	fflush(stdout);
	
	filter->SetFilter(0x234, 0xE6, 0xFF);
	
	uint32_t timeout = tickcount() + 5000;
	
	while(true)
	{
		uint8_t *buf;
		uint32_t len = filter->Read(&buf);
		
		if(len != 0)
		{
			if(len > 15)
			{
				CReader reader(buf, len);
				
				uint32_t title_id;
				
				try
				{
					reader.Seek(22);
					reader.Read(title_id);
				}
				catch(const exception_t &exception)
				{
					delete [] buf;
					throw;
				}
				
				if(bufs.find(title_id) == bufs.end())
				{
					bufs[title_id] = new buf_t(buf, len);
					
					timeout = tickcount() + 5000;
					continue;
				}
			}
			
			delete [] buf;
		}
		
		if(timeout < tickcount())
			break;
	}
	
	for(std::map<uint32_t, buf_t *>::iterator it = bufs.begin(); it != bufs.end(); it++)
	{
		CReader reader(it->second->buf, it->second->len);
		
		reader.Seek(15);
		
		while(reader.GetOffset() < reader.GetLength())
		{
			uint8_t channel_id;
			uint32_t title_id;
			uint16_t mjd, duration;
			uint8_t hours, minutes, seconds;
			uint8_t title_length;
			uint16_t summary_id;
			
			reader.Read(channel_id);
			reader.Seek(6);
			reader.Read(title_id);
			reader.Read(mjd);
			reader.Read(hours);
			reader.Read(minutes);
			reader.Read(seconds);
			reader.Read(duration);
			reader.Read(title_length);
			reader.Seek(title_length & 0x3F);
			reader.Seek(1);
			reader.Read(summary_id);
			
			if(titles.find(title_id) == titles.end())
			{
				epgdb_title_t *title = epgdb_title_alloc();
				title->event_id = titles.size();
				title->mjd = mjd;
				title->start_time = mjdhms_to_timestamp(mjd, hours, minutes, seconds);
				title->length = (duration >> 4) * 60;
				title->iso_639_1 = 's';
				title->iso_639_2 = 'p';
				title->iso_639_3 = 'a';
				title = epgdb_titles_add(channels[channel_id], title);
				titles[title_id] = title;
				
				if(summary_id != 0xFFFF)
				{
					if(!summaries[summary_id])
					{
						summaries[summary_id] = true;
						summaries_count++;
					}
					
					summaries_titles[summary_id] = new summary_t(title, summaries_titles[summary_id]);
				}
			}
		}
	}
	
	bufs_clear();
}

bool mhw2_summaries(CFilter *filter)
{
	printf("LOGTEXT DESCARGANDO LISTA DE RESUMENES\n");
	fflush(stdout);
	
	filter->SetFilter(0x236, 0x96, 0xFF);
	
	uint32_t timeout = tickcount() + 15000;
	
	while(true)
	{
		uint8_t *buf;
		uint32_t len = filter->Read(&buf);
		
		if(len != 0)
		{
			CReader reader(buf, len);
			
			uint16_t summary_id;
			
			try
			{
				reader.Read(summary_id);
			}
			catch(const exception_t &exception)
			{
				delete [] buf;
				throw;
			}
			
			if(summaries[summary_id])
			{
				bufs[summary_id] = new buf_t(buf, len);
				
				summaries[summary_id] = false;
				summaries_count--;
				
				if(summaries_count == 0)
					break;
				
				timeout = tickcount() + 15000;
				continue;
			}
			
			delete [] buf;
		}
		
		if(timeout < tickcount())
			throw exception_t(false, true, "Tiempo de espera agotado");
	}
	
	bool title_formatting = true;
	bool desc_formatting = true;
	
	if(strcmp(title_format.c_str(), "{title-full}") == 0)
		title_formatting = false;
	
	if(strcmp(desc_format.c_str(), "{desc-full}") == 0)
		desc_formatting = false;
	
	bool formatting = title_formatting || desc_formatting;
	
	std::string title;
	std::string desc;
	std::string season;
	std::string episode;
	std::string episode_num;
	std::string title_formatted;
	std::string desc_formatted;
	
	for(std::map<uint32_t, buf_t *>::iterator it = bufs.begin(); it != bufs.end(); it++)
	{
		CReader reader(it->second->buf, it->second->len);
		
		uint16_t summary_id;
		uint8_t desc_length, desc_count;
		char title_full[256];
		char desc_full[4096 + 16];
		
		reader.Read(summary_id);
		reader.Seek(9);
		reader.Read(desc_length);
		reader.Read(title_full, desc_length);
		reader.Read(desc_count);
		
		title_full[desc_length] = '\0';
		
		uint32_t idx = 0;
		
		desc_count &= 0x0F;
		
		while(desc_count != 0)
		{
			if(idx != 0)
				desc_full[idx++] = ' ';
			
			reader.Read(desc_length);
			reader.Read(desc_full + idx, desc_length);
			
			idx += desc_length;
			desc_count--;
		}
		
		desc_full[idx] = '\0';
		
		summary_t *summary = summaries_titles[summary_id];
		
		if(summary != NULL)
		{
			if(formatting)
			{
				title.clear();
				desc.clear();
				season.clear();
				episode.clear();
				episode_num.clear();
				set_title(title_full, desc_full, &title, &desc, &season, &episode, &episode_num);
			}
			
			const char *title_text;
			
			if(title_formatting)
			{
				title_formatted.clear();
				fmt_title(title_format.c_str(), title_full, desc_full, title.c_str(), desc.c_str(), season.c_str(), episode.c_str(), episode_num.c_str(), &title_formatted);
				title_text = title_formatted.c_str();
			}
			else
			{
				title_text = title_full;
			}
			
			const char *desc_text;
			
			if(desc_formatting)
			{
				desc_formatted.clear();
				fmt_title(desc_format.c_str(), title_full, desc_full, title.c_str(), desc.c_str(), season.c_str(), episode.c_str(), episode_num.c_str(), &desc_formatted);
				desc_text = desc_formatted.c_str();
			}
			else
			{
				desc_text = desc_full;
			}
			
			while(summary != NULL)
			{
				epgdb_titles_set_description(summary->title, title_text);
				epgdb_titles_set_long_description(summary->title, desc_text);
				summary = summary->next;
			}
			
			delete summaries_titles[summary_id];
			summaries_titles[summary_id] = NULL;
		}
	}
	
	bufs_clear();
}

int main(int argc, char *argv[])
{
	printf("TYPE RUNNING CSCRIPT mhw2movistarplus\n");
	fflush(stdout);
	
	std::string current_sref;
	
	uint32_t retries = 0;
	
	do
	{
		summaries.resize(65535, false);
		summaries_titles.resize(65535, NULL);
		
		try
		{
			load_args(argc, argv, &config_file, &crossepg_lib, &crossepg_data);
			load_config(config_file.c_str(), &demux_path, &title_format, &desc_format, &service_reference, &webif_user, &webif_pass, &webif_port, &max_retries, &crossepg_clear);
			
			if(current_sref.empty())
				webif_get(webif_user.c_str(), webif_pass.c_str(), webif_port, &current_sref);
			
			webif_zap(webif_user.c_str(), webif_pass.c_str(), webif_port, service_reference.c_str());
			
			if(strcmp(demux_path.c_str(), "auto-detect") == 0)
			{
				printf("LOGTEXT DETECTANDO ADAPTADOR DEMUX\n");
				fflush(stdout);
				
				detecting_demux(&demux_path);
			}
			
			CFilter filter(demux_path.c_str());
			
			epgdb_init(crossepg_lib.c_str(), crossepg_data.c_str(), crossepg_clear);
			
			mhw2_channels(&filter);
			mhw2_titles(&filter);
			mhw2_summaries(&filter);
			
			epgdb_save(NULL);
			
			webif_zap(webif_user.c_str(), webif_pass.c_str(), webif_port, current_sref.c_str());
			
			return EXIT_SUCCESS;
		}
		catch(const exception_t &exception)
		{
			if(exception.print)
				fprintf(stderr, "%s\n", exception.msg.c_str());
			
			if(exception.abort)
				break;
			
			usleep(1000 * 1000);
		}
		
		bufs_clear();
		channels.clear();
		titles.clear();
		summaries_clear();
	}
	while(retries++ < max_retries);
	
	if(!current_sref.empty())
		webif_zap(webif_user.c_str(), webif_pass.c_str(), webif_port, current_sref.c_str());
	
	return EXIT_FAILURE;
}
