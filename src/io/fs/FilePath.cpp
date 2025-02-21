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

#include "io/fs/FilePath.h"

#include <algorithm>

#include <boost/algorithm/string/predicate.hpp>

#include "platform/Platform.h"

namespace fs {

namespace {

inline bool is_path_up(std::string_view str, size_t pos) {
	return (str.length() == pos + 2 && str[pos] == '.' && str[pos + 1] == '.')
	       || (str.length() >= pos + 3 && str[pos] == '.' && str[pos + 1] == '.'
	           && str[pos + 2] == path::dir_sep);
}

} // anonymous namespace

path path::create(std::string src) {
	path result;
	result.pathstr = std::move(src);
	return result;
}

path path::resolve(const path & base, const path & branch) {
	
	size_t bpos = 0;
	size_t apos = base.pathstr.length();
	while(true) {
		
		size_t dirpos = base.pathstr.find_last_of(dir_sep, apos - 1);
		
		if(is_path_up(base.pathstr, (dirpos == std::string::npos) ? 0 : dirpos + 1)) {
			return create(base.pathstr.substr(0, apos) + dir_sep + branch.pathstr.substr(bpos));
		}
		
		if(dirpos == std::string::npos) {
			if(bpos + 3 >= branch.pathstr.length()) {
				return create(".");
			} else {
				return branch.pathstr.substr(bpos + 3);
			}
		}
		
		if(dirpos == 0 || (dirpos == 1 && base.pathstr[0] == dir_sep)) {
			if(dirpos != apos - 1) {
				bpos += 3;
			}
			if(bpos >= branch.pathstr.length()) {
				return create(base.pathstr.substr(0, dirpos) + dir_sep);
			} else {
				return create(base.pathstr.substr(0, dirpos) + dir_sep + branch.pathstr.substr(bpos));
			}
		}
		
		apos = dirpos, bpos += 3;
		
		if(!is_path_up(branch.pathstr, bpos)) {
			if(bpos >= branch.pathstr.length()) {
				return create(base.pathstr.substr(0, apos));
			} else {
				return create(base.pathstr.substr(0, apos) + dir_sep + branch.pathstr.substr(bpos));
			}
		}
	}
	
}

path path::operator/(const path & other) const {
	if(other.is_absolute() || empty() || (is_dot() && !other.empty())) {
		return other;
	} else if(other.empty() || other.is_dot()) {
		return *this;
	} else if(other.is_up()) {
		return resolve(*this, other);
	} else {
		path result = *this;
		if(result.pathstr[result.pathstr.length() - 1] != dir_sep) {
			result.pathstr += dir_sep;
		}
		result.pathstr += other.pathstr;
		return result;
	}
}

path & path::operator/=(const path & other) {
	if(other.is_absolute() || empty() || (is_dot() && !other.empty())) {
		return *this = other;
	} else if(other.empty() || other.is_dot()) {
		return *this;
	} else if(other.is_up()) {
		return *this = resolve(*this, other);
	} else {
		if(pathstr[pathstr.length() - 1] != dir_sep) {
			pathstr += dir_sep;
		}
		pathstr += other.pathstr;
		return *this;
	}
}

std::string_view path::basename() const {
	if(!has_info()) {
		return empty() ? std::string_view() : std::string_view("..");
	}
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos) {
		return pathstr;
	} else if(pathstr[extpos] != ext_sep) {
		return std::string_view(pathstr).substr(extpos + 1);
	} else if(extpos == 0) {
		return std::string_view();
	}
	size_t dirpos = pathstr.find_last_of(dir_sep, extpos - 1);
	if(dirpos == std::string::npos) {
		return std::string_view(pathstr).substr(0, extpos);
	} else {
		return std::string_view(pathstr).substr(dirpos + 1, extpos - dirpos - 1);
	}
}

std::string_view path::ext() const {
	if(!has_info()) {
		return std::string_view();
	}
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos || pathstr[extpos] != ext_sep) {
		return std::string_view();
	} else {
		return std::string_view(pathstr).substr(extpos);
	}
}

path & path::set_ext(std::string_view ext) {
	arx_assert_msg(ext.empty()
	               || (ext[0] != dir_sep
	                   && ext.find_first_of(dir_or_ext_sep, 1) == std::string_view::npos),
	               "bad file ext: \"%s\"", std::string(ext).c_str());
	if(!has_info() && !empty()) {
		return *this;
	}
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos || pathstr[extpos] != ext_sep) {
		return (((ext.empty() || ext[0] != ext_sep) ? (pathstr += ext_sep)
		                                            : pathstr).append(ext), *this);
	} else {
		if(ext.empty() || ext[0] != ext_sep) {
			pathstr.resize(extpos + 1 + ext.size());
			std::copy(ext.begin(), ext.end(), pathstr.begin() + extpos + 1);
		} else {
			pathstr.resize(extpos + ext.size());
			std::copy(ext.begin() +  1, ext.end(), pathstr.begin() + extpos + 1);
		}
		return *this;
	}
}

path & path::remove_ext() {
	if(!has_info()) {
		return *this;
	}
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos != std::string::npos && pathstr[extpos] == ext_sep) {
		pathstr.resize(extpos);
	}
	return *this;
}

path & path::set_filename(std::string_view filename) {
	arx_assert_msg(!filename.empty() && filename != "." && filename != ".."
	               && filename.find_first_of(any_dir_sep) == std::string_view::npos,
	               "bad filename: \"%s\"", std::string(filename).c_str());
	if(!has_info()) {
		return ((empty() ? pathstr = filename : (pathstr += dir_sep).append(filename)), *this);
	}
	size_t dirpos = pathstr.find_last_of(dir_sep);
	if(dirpos == std::string::npos) {
		return (*this = filename);
	} else {
		pathstr.resize(dirpos + 1 + filename.size());
		std::copy(filename.begin(), filename.end(), pathstr.begin() + dirpos + 1);
		return *this;
	}
}

path & path::set_basename(std::string_view basename) {
	
	arx_assert_msg(!basename.empty() && basename != "." && basename != ".."
	               && basename.find_first_of(any_dir_sep) == std::string_view::npos,
	               "bad basename: \"%s\"", std::string(basename).c_str());
	
	if(!has_info()) {
		return ((empty() ? pathstr = basename : (pathstr += dir_sep).append(basename)), *this);
	}
	
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	
	// handle paths without an extension.
	if(extpos == std::string::npos) {
		return (*this = basename);
	} else if(pathstr[extpos] != ext_sep) {
		pathstr.resize(extpos + 1 + basename.size());
		std::copy(basename.begin(), basename.end(), pathstr.begin() + extpos + 1);
		return *this;
	}
	
	size_t dirpos = (extpos == 0) ? std::string::npos : pathstr.find_last_of(dir_sep, extpos - 1);
	
	if(dirpos == std::string::npos) { // no parent path
		pathstr = (std::string(basename) += std::string_view(pathstr).substr(extpos));
	} else {
		pathstr = (pathstr.substr(0, dirpos + 1) += basename) + pathstr.substr(extpos);
	}
	
	return *this;
}

path & path::append_basename(std::string_view basename_part) {
	
	arx_assert_msg(basename_part != "." && basename_part != ".." &&
	               basename_part.find_first_of(any_dir_sep) == std::string_view::npos,
	               "bad basename: \"%s\"", std::string(basename_part).c_str());
	
	if(!has_info()) {
		return ((empty() ? pathstr = basename_part : (pathstr += dir_sep).append(basename_part)), *this);
	}
	
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos || pathstr[extpos] != ext_sep) {
		return (pathstr += basename_part, *this);
	}
	size_t len = pathstr.length();
	pathstr.resize(pathstr.length() + basename_part.length());
	std::copy_backward(pathstr.begin() + extpos, pathstr.begin() + len, pathstr.end());
	std::copy(basename_part.begin(), basename_part.end(), pathstr.begin() + extpos);
	return *this;
}

path & path::append(std::string_view str) {
	
	arx_assert_msg(str != "." && str != ".."
	               && str.find_first_of(any_dir_sep) == std::string_view::npos,
	               "cannot append: \"%s\"", std::string(str).c_str());
	
	pathstr += str;
	return *this;
}

bool path::has_ext(std::string_view str) const {
	
	arx_assert_msg(str.empty()
	               || (str[0] != dir_sep
	                   && str.find_first_of(dir_or_ext_sep, 1) == std::string_view::npos),
	               "bad file ext: \"%s\"", std::string(str).c_str());
	
	if(!has_info()) {
		return false;
	}
	
	size_t extpos = pathstr.find_last_of(dir_or_ext_sep);
	if(extpos == std::string::npos || pathstr[extpos] != ext_sep) {
		return false;
	} else if(str.empty()) {
		return true;
	} else if(str[0] == ext_sep) {
		return boost::iequals(pathstr.substr(extpos), str);
	} else {
		return boost::iequals(pathstr.substr(extpos + 1), str);
	}
}

path::path(std::string_view str) {
	
	pathstr.resize(str.length());
	
	size_t istart = 0, ostart = 0;
	while(istart < str.length()) {
		
		size_t pos = str.find_first_of(any_dir_sep, istart);
		if(pos == std::string_view::npos) {
			if(istart == 0) {
				pathstr = str;
				return;
			}
			pos = str.length();
		}
		
		size_t start = istart;
		istart = pos + 1;
		
		if(pos == start) {
			if(pos == 0) {
				// Aboslute path.
				pathstr[ostart++] = dir_sep;
			}
			#if ARX_PLATFORM == ARX_PLATFORM_WIN32
			if(pos == 1) {
				// Network path
				pathstr[ostart++] = dir_sep;
			}
			#endif
			// double slash
			continue;
		}
		
		if(pos - start == 1 && str[start] == '.') {
			if(pos == 1) {
				pathstr[ostart++] = '.';
			}
			// '.'
			continue;
		}
		
		if(ostart == 1 && pathstr[0] == '.') {
			ostart = 0;
		}
		
		if(pos - start == 2 && str[start] == '.' && str[start + 1] == '.') {
			// '..'
			if(ostart == 0) {
				pathstr[ostart++] = '.', pathstr[ostart++] = '.';
			} else {
				size_t last = pathstr.find_last_of(dir_sep, ostart - 1);
				if(last == std::string::npos) {
					if(ostart == 2 && pathstr[0] == '.' && pathstr[1] == '.') {
						pathstr[ostart++] = dir_sep, pathstr[ostart++] = '.', pathstr[ostart++] = '.';
					} else {
						pathstr[0] = '.', ostart = 1;
					}
				} else if(pathstr[ostart - 1] == dir_sep) {
					pathstr[ostart++] = '.', pathstr[ostart++] = '.';
				} else if(ostart - last - 1 == 2 && pathstr[last + 1] == '.' && pathstr[last + 2] == '.') {
					pathstr[ostart++] = dir_sep, pathstr[ostart++] = '.', pathstr[ostart++] = '.';
				} else {
					if(last == 0 || (last == 1 && pathstr[0] == dir_sep)) {
						ostart = last + 1;
					} else {
						ostart = last;
					}
				}
			}
			continue;
		}
		
		if(ostart != 0 && pathstr[ostart - 1] != dir_sep) {
			pathstr[ostart++] = dir_sep;
		}
		
		for(size_t p = start; p < pos; p++) {
			pathstr[ostart++] = str[p];
		}
	}
	
#if ARX_PLATFORM == ARX_PLATFORM_WIN32
	if(ostart == 2 && pathstr[1] == ':') {
		pathstr.resize(3);
		pathstr[2] = dir_sep;
	}
	else
#endif
	{
		pathstr.resize(ostart);
	}
	
}

} // namespace fs
