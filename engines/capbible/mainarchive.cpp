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

#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"

#include "engines/util.h"

#include "capbible/mainarchive.h"

namespace CapBible {
MainArchive::MainArchive(Common::String fileName) {
	if (!_archiveFile.open(fileName)) {
		error("Failed to open %s", fileName.c_str());
	}

	readTOC();
}
MainArchive::~MainArchive() {
}

void MainArchive::readTOC() {
	uint16 fileC = _archiveFile.readUint16LE();
	for (uint16 fileIndex = 0; fileIndex < fileC; ++fileIndex) {
		MainArchiveMember *entry = new MainArchiveMember(this);

		char baseNameBuf[9] = {'\0'};
		char extBuf[4] = {'\0'};

		_archiveFile.read(&baseNameBuf, 8);
		entry->_baseName = Common::String(baseNameBuf);
		entry->_compressionType = _archiveFile.readByte();
		_archiveFile.read(&extBuf, 3);
		entry->_extension = Common::String(extBuf);
		entry->_offset = _archiveFile.readUint32LE();
		entry->_decompressedSize = _archiveFile.readUint32LE();
		entry->_compressedSize = _archiveFile.readUint32LE();

		_fileEntries.setVal(entry->getName(), Common::ArchiveMemberPtr(entry));
	}
}

bool MainArchive::hasFile(const Common::Path &path) const {
	return _fileEntries.contains(path.rawString());
}
int MainArchive::listMembers(Common::ArchiveMemberList &list) const {
	int addC = 0;
	for (EntryMap::iterator itr = _fileEntries.begin(); itr != _fileEntries.end(); ++itr) {
		list.push_back(itr->_value);
	}
	return addC;
}
const Common::ArchiveMemberPtr MainArchive::getMember(const Common::Path &path) const {
	return _fileEntries.getValOrDefault(path.rawString());
}
Common::SeekableReadStream *MainArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::ArchiveMemberPtr entry = getMember(path);
	if (!entry)
		return nullptr;

	return entry->createReadStream();
}
Common::String MainArchiveMember::getName() const {
	if (_extension.empty())
		return _baseName;
	return _baseName + "." + _extension;
}

Common::SeekableReadStream *MainArchiveMember::createReadStream() const {
	return nullptr;
}
} // End of namespace CapBible
