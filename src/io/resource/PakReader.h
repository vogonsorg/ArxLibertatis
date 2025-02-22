/*
 * Copyright 2011-2020 Arx Libertatis Team (see the AUTHORS file)
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

#ifndef ARX_IO_RESOURCE_PAKREADER_H
#define ARX_IO_RESOURCE_PAKREADER_H

#include <istream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "io/resource/PakEntry.h"
#include "io/resource/ResourcePath.h"
#include "util/Flags.h"
#include "util/MD5.h"

namespace fs { class path; }

enum Whence {
	SeekSet,
	SeekCur,
	SeekEnd
};

class PakFileHandle {
	
public:
	
	PakFileHandle(const PakFileHandle &) = delete;
	PakFileHandle & operator=(const PakFileHandle &) = delete;
	
	PakFileHandle() = default;
	
	virtual size_t read(void * buf, size_t size) = 0;
	
	virtual int seek(Whence whence, int offset) = 0;
	
	virtual size_t tell() = 0;
	
	virtual ~PakFileHandle() = default;
	
};

typedef std::map<util::md5::checksum, std::vector<std::string_view>> PakFilter;

class PakReader : public PakDirectory {
	
public:
	
	enum ReleaseType {
		Demo     = 1 << 0,
		FullGame = 1 << 1,
		Unknown  = 1 << 2,
		External = 1 << 3
	};
	DECLARE_FLAGS(ReleaseType, ReleaseFlags)
	
	PakReader(const PakReader &) = delete;
	PakReader & operator=(const PakReader &) = delete;
	
	PakReader() : release(0) { }
	~PakReader();
	
	void removeFile(const res::path & file);
	
	/*!
	 * Remove an empty directory.
	 * If the given directory is not empty, no action is taken.
	 * \return true if the directory was removed.
	 */
	bool removeDirectory(const res::path & name);
	
	/*!
	 * Add a file or directory from the filesystem to the virtual resource hierarchy.
	 *
	 * If path refers to a directory, add all files sll folders and files under this
	 * directory will be inserted and their name converted to lowercase.
	 * The mount point name will not be modified (case-sensitive).
	 *
	 * \param path Directory on the filesystem that will be imported in this PakDirectory
	 * \param mount Mount point in this PakDirectory (case-sensitive)
	 * \return false if there were problems (some data may have been read)
	 */
	bool addFiles(const fs::path & path, const res::path & mount = res::path());
	
	bool addArchive(const fs::path & pakfile, const PakFilter * filter = nullptr);
	void clear();
	
	[[nodiscard]] std::string read(const res::path & name);
	
	[[nodiscard]] std::unique_ptr<PakFileHandle> open(const res::path & name);
	
	ReleaseFlags getReleaseType() { return release; }
	
	const util::md5::checksum & getChecksum() { return m_checksum; }
	
private:
	
	util::md5::checksum m_checksum;
	ReleaseFlags release;
	std::vector<std::istream *> paks;
	
	bool addFiles(PakDirectory * dir, const fs::path & path);
	bool addFile(PakDirectory * dir, fs::path path, std::string name);
	
};

DECLARE_FLAGS_OPERATORS(PakReader::ReleaseFlags)

extern PakReader * g_resources;

#endif // ARX_IO_RESOURCE_PAKREADER_H
