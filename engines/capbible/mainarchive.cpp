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
#include "common/memstream.h"

#include "engines/util.h"

#include "capbible/mainarchive.h"

namespace CapBible {

	class Decompressor {
	public:
		Decompressor(Common::SeekableReadStream *src, byte *dst, size_t compressedSize, size_t decompressedSize) : _src(src), _dst(dst), _compressedSize(compressedSize), _decompressedSize(decompressedSize) {
			_prefixes = new int16[prefixCount];
			_suffixes = new byte[suffixCount];
			_suffixChain = new int16[maxChainLength];
		}

		~Decompressor() {
			delete[] _prefixes;
			delete[] _suffixes;
			delete[] _suffixChain;
		}

		void decompress();

		void mainLoop();

	private:

		constexpr static int prefixCount = 0x1001;
		constexpr static int suffixCount = 0x1001;
		constexpr static int maxCode = 0x100;
		constexpr static int maxChainLength = 0x1000;
		
		Common::SeekableReadStream *_src;
		byte *_dst;
		size_t _outputPos;
		size_t _compressedSize;
		size_t _decompressedSize;
		int16 *_prefixes;
		byte *_suffixes;

		int _nextCode;

		int16 *_suffixChain;

		int getPlaneCount();
		void writeToken(int16 token);
	};

MainArchive::MainArchive(const Common::Path &fileName) {
	if (!_archiveFile.open(fileName)) {
		error("Failed to open %s", fileName.toString().c_str());
	}

	readTOC();
}
MainArchive::~MainArchive() {
}

void MainArchive::readTOC() {
	uint16 fileC = _archiveFile.readUint16LE();
	for (uint16 fileIndex = 0; fileIndex < fileC; ++fileIndex) {
		MainArchiveMember *entry = new MainArchiveMember(this);
		
		entry->_baseName = _archiveFile.readString('\0', 8);
		entry->_compressionType = _archiveFile.readByte();
		entry->_extension = _archiveFile.readString('\0', 3);
		entry->_offset = _archiveFile.readUint32LE();
		entry->_decompressedSize = _archiveFile.readUint32LE();
		entry->_compressedSize = _archiveFile.readUint32LE();

		_fileEntries.setVal(entry->getName(), Common::ArchiveMemberPtr(entry));
	}
}

bool MainArchive::hasFile(const Common::Path &path) const {
	return _fileEntries.contains(path.baseName());
}
int MainArchive::listMembers(Common::ArchiveMemberList &list) const {
	int addC = 0;
	for (EntryMap::iterator itr = _fileEntries.begin(); itr != _fileEntries.end(); ++itr) {
		list.push_back(itr->_value);
	}
	return addC;
}
const Common::ArchiveMemberPtr MainArchive::getMember(const Common::Path &path) const {
	return _fileEntries.getValOrDefault(path.baseName());
}
Common::SeekableReadStream *MainArchive::createReadStreamForMember(const Common::Path &path) const {
	Common::ArchiveMemberPtr entry = getMember(path);
	if (!entry)
		return nullptr;

	return entry->createReadStream();
}
Common::String MainArchiveMember::getFileName() const {
	if (_extension.empty())
		return _baseName;
	return _baseName + "." + _extension;
}
Common::String MainArchiveMember::getName() const { return getFileName(); }

Common::Path MainArchiveMember::getPathInArchive() const {
	return Common::Path(Common::String(_archive->_archiveFile.getName()) + "/" + getFileName());
}

Common::SeekableReadStream *MainArchiveMember::createReadStream() const {
	_archive->_archiveFile.seek(_offset, SEEK_SET);

	auto sig = _archive->_archiveFile.readUint16LE();
	assert(sig == 0x4347);

	if (this->_compressionType == 1) {
		byte *decompressedBuff = (byte*)malloc(_decompressedSize);
		Decompressor dec(&_archive->_archiveFile, decompressedBuff, _compressedSize, _decompressedSize);
		dec.decompress();
		return new Common::MemoryReadStream(decompressedBuff, _decompressedSize, DisposeAfterUse::YES);
	} else {
		byte *buff = (byte *)malloc(_compressedSize);
		_archive->_archiveFile.read(buff, _compressedSize);
		return new Common::MemoryReadStream(buff, _decompressedSize, DisposeAfterUse::YES);
	}

}
Common::SeekableReadStream *MainArchiveMember::createReadStreamForAltStream(Common::AltStreamType altStreamType) const {
	return nullptr;
}

void Decompressor::decompress() {
	for (int i = 0; i < 0x1001; i++)
		_prefixes[i] = -1;

	for (int i = 0; i < 0x100; i++)
		_suffixes[i] = (uint8_t)i;

	_outputPos = 0;

	while (_outputPos < _decompressedSize) {
		mainLoop();
	}
}

void Decompressor::mainLoop() {
	uint8_t first_code = _src->readByte();
	_prefixes[0x100] = first_code;
	_dst[_outputPos++] = first_code;

	uint8_t planeBytes[8];
	int planeCount = 0;
	int planeBit = 8;

	for (_nextCode = 0x101; _nextCode < 0x1001 && _outputPos < _decompressedSize; ++_nextCode) {

		if (planeBit == 8) {
			planeCount = getPlaneCount();

			for (int i = 0; i < planeCount; i++) {
				planeBytes[i] = _src->readByte();
				assert(!_src->err());
			}
			planeBit = 0;
		}

		uint16_t code = _src->readByte();

		for (int bit = 0; bit < planeCount; bit++) {
			code |= (planeBytes[bit] & 1) << (8 + bit);
			planeBytes[bit] >>= 1;
		}

		planeBit++;
		assert(code < _nextCode);
		_prefixes[_nextCode] = (int16_t)code;

		writeToken(code);
	}
}

int Decompressor::getPlaneCount() {
	int code_limit = maxCode;
	int planeCount = 0;

	while (_nextCode > code_limit) {
		planeCount++;
		code_limit <<= 1;
	}
	return planeCount;
}

void Decompressor::writeToken(int16 code) {
	uint16_t cursor = code;
	int chainLength = 0;

	while (_prefixes[cursor] != -1) {
		_suffixChain[chainLength++] = cursor;
		cursor = (uint16_t)_prefixes[cursor];
	}

	byte first_character = _suffixes[cursor];
	_suffixes[_nextCode - 1] = first_character;

	_dst[_outputPos++] = first_character;

	for (int i = chainLength - 1; i >= 0; i--) {
		_dst[_outputPos++] = _suffixes[_suffixChain[i]];
	}
}
} // End of namespace CapBible
