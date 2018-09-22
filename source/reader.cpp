
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

CReader::CReader(uint8_t *buf, uint32_t len)
{
	m_buffer = buf;
	m_length = len;
	m_offset = 0;
}

CReader::~CReader()
{
	
}

void CReader::Seek(uint32_t size)
{
	if(size != 0)
	{
		uint32_t needed = m_offset + size;
		
		if(needed > m_length)
			throw exception_t(false, true, "CReader::Seek()");
		
		m_offset = needed;
	}
}

void CReader::Read(void *data, uint32_t size)
{
	if(size != 0)
	{
		uint32_t needed = m_offset + size;
		
		if(needed > m_length)
			throw exception_t(false, true, "CReader::Read()");
		
		memcpy(data, m_buffer + m_offset, size);
		m_offset = needed;
	}
}
