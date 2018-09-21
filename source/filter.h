
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

class CFilter
{
	private:
		int			m_fd;
		uint8_t		m_filter;
		bool		m_error;
	
	public:
		CFilter(const char *demux);
		~CFilter();
		
		void SetFilter(uint16_t pid, uint8_t filter, uint8_t mask);
		uint32_t Read(uint8_t **buf);
};
