/*
 * Copyright 2011-2019 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ARX_IO_INIWRITER_H
#define ARX_IO_INIWRITER_H

#include <ostream>
#include <string_view>

/*!
 * Simple writer for ini-formatted data.
 */
class IniWriter {
	
	std::ostream & output;
	
public:
	
	IniWriter(const IniWriter &) = delete;
	IniWriter & operator=(const IniWriter &) = delete;
	
	/*!
	 * Initialize this ini writer.
	 * \param _output Reference to an ostream that mus remain valid while the writer is used.
	 */
	explicit IniWriter(std::ostream & _output) : output(_output) { }
	
	/*!
	 * Flush the output stream.
	 * \return true if there were no errors during writing.
	 */
	bool flush() {
		return !output.flush().bad();
	}
	
	/*!
	 * Write a section header to the output stream.
	 * \param section The section to start.
	 */
	void beginSection(std::string_view section);
	
	void writeKey(std::string_view key, std::string_view value);
	void writeKey(std::string_view key, int value);
	void writeKey(std::string_view key, float value);
	void writeKey(std::string_view key, bool value);
	
};

#endif // ARX_IO_INIWRITER_H
