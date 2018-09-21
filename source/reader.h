
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

class CReader
{
	private:
		uint8_t *		m_buffer;
		uint32_t		m_length;
		uint32_t		m_offset;
	
	public:
		CReader(uint8_t *buf, uint32_t len);
		~CReader();
		
		void Seek(uint32_t size);
		void Read(void *data, uint32_t size);
		
		template <class T> inline void Read(T &value)
		{
			uint8_t *buf = (uint8_t *)&value;
			uint32_t len = sizeof(value);
			
			while(len > 0)
			{
				Read(&buf[--len], sizeof(uint8_t));
			}
		}
		
		inline uint8_t *GetBuffer() { return m_buffer; }
		inline uint32_t GetLength() { return m_length; }
		inline uint32_t GetOffset() { return m_offset; }
};
