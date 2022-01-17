/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CAPBIBLE_MAINARCHIVE_H
#define CAPBIBLE_MAINARCHIVE_H

#include "common/archive.h"
#include "common/hashmap.h"

namespace Common {
class SeekableReadStream;
}

namespace CapBible {

class MainArchive : public Common::Archive {
public:
	MainArchive(Common::String fileName);
	~MainArchive();
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	void readTOC();

	Common::File _archiveFile;

	typedef Common::HashMap<Common::String, Common::ArchiveMemberPtr> EntryMap;
	EntryMap _fileEntries;

	friend class MainArchiveMember;
};

class MainArchiveMember : public Common::ArchiveMember {
public:
	Common::SeekableReadStream *createReadStream() const override;
	Common::String getName() const override;

private:
	MainArchiveMember(MainArchive *archive) : _archive(archive) {}

	MainArchive *_archive;
	Common::String _baseName;
	Common::String _extension;
	uint32 _offset;
	uint32 _compressedSize;
	uint32 _decompressedSize;
	byte _compressionType;

	friend class MainArchive;
};
} // End of namespace CapBible

#endif
