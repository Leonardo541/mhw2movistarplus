
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

CFilter::CFilter(const char *demux)
{
	m_fd = open(demux, O_RDWR);
	m_filter = 0;
	
	if(m_fd == -1)
		throw exception_t(true, true, "Error en CFilter::CFilter() -> open()");
	
	if(ioctl(m_fd, DMX_SET_BUFFER_SIZE, 4096 * 256) == -1)
		throw exception_t(true, true, "Error en CFilter::CFilter() -> ioctl() -> DMX_SET_BUFFER_SIZE");
}

CFilter::~CFilter()
{
	if(m_fd != -1)
		close(m_fd);
}

void CFilter::SetFilter(uint16_t pid, uint8_t filter, uint8_t mask)
{
	if(m_filter != 0)
	{
		if(ioctl(m_fd, DMX_STOP) == -1)
			throw exception_t(true, true, "Error en CFilter::SetFilter() -> ioctl() -> DMX_STOP");
	}
	
	if(filter != 0)
	{
		struct dmx_sct_filter_params sct_filter_params;
		memset(&sct_filter_params, 0, sizeof(struct dmx_sct_filter_params));
		
		sct_filter_params.pid = pid;
		sct_filter_params.timeout = 5000;
		sct_filter_params.flags = DMX_IMMEDIATE_START;
		sct_filter_params.filter.filter[0] = filter;
		sct_filter_params.filter.mask[0] = mask;
		
		if(ioctl(m_fd, DMX_SET_FILTER, &sct_filter_params) == -1)
			throw exception_t(true, true, "Error en CFilter::SetFilter() -> ioctl() -> DMX_SET_FILTER");
	}
	
	m_filter = filter;
}

uint32_t CFilter::Read(uint8_t **buf)
{
	uint8_t tmp[3];
	int n = read(m_fd, tmp, 3);
	
	if(n == -1)
	{
		if(errno == EOVERFLOW)
			return 0;
		
		throw exception_t(true, true, "Error en CFilter::Read() -> read() > n == -1");
	}
	
	if(n != 3)
		throw exception_t(true, true, "Error en CFilter::Read() -> read() -> n != 3");
	
	int len = ((tmp[1] & 0x0F) << 8) | (tmp[2] & 0xFF);
	
	if(len != 0)
	{
		*buf = new uint8_t[len];
		
		n = read(m_fd, *buf, len);
		
		if(n == -1)
		{
			delete [] *buf;
			
			if(errno != EOVERFLOW)
				return 0;
			
			throw exception_t(true, true, "Error en CFilter::Read() -> read() -> n == -1");
		}
		
		if(n != len)
		{
			delete [] *buf;
			
			throw exception_t(true, true, "Error en CFilter::Read() -> read() -> n != len");
		}
	}
	
	return len;
}
