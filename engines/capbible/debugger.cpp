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
#include "common/archive.h"

#include "engines/util.h"

#include "capbible/capbible.h"
#include "capbible/mainarchive.h"
#include "capbible/music.h"
#include "debugger.h"

namespace CapBible {
Debugger::Debugger(CapBibleEngine *eng) : _engine(eng) {
	registerCmd("dumpMainArchive", WRAP_METHOD(Debugger,cmdDumpMainArch));
	registerCmd("giveItem", WRAP_METHOD(Debugger,cmdGiveItem));
	registerCmd("playMusic", WRAP_METHOD(Debugger,cmdPlayMusic));
}
bool Debugger::cmdDumpMainArch(int argc, const char **argv) {
	Common::Archive *arch = _engine->_mainArchive;
	Common::ArchiveMemberList fileList;
	arch->listMembers(fileList);

	for (Common::ArchiveMemberList::const_iterator itr = fileList.begin(); itr != fileList.end(); ++itr) {
		Common::DumpFile dump;
		Common::ArchiveMemberPtr entry=*itr;
		dump.open("mainarch/" + entry->getName(), true);
		Common::SeekableReadStream *stream = entry->createReadStream();
		dump.writeStream(stream);
		delete stream;
	}

	return true;
}
bool Debugger::cmdGiveItem(int argc, const char **argv) {

	bool sword = false;
	bool shield = false;
	bool light = false;
	bool flight = false;
	bool traps = false;

	for(int argi = 0; argi < argc; ++argi) {
		Common::String arg = argv[argi];
		if (arg == "sword")
			sword = true;
		else if (arg == "shield")
			shield = true;
		else if (arg == "light")
			light = true;
		else if (arg == "flight")
			flight = true;
		else if (arg == "traps")
			traps = true;
		else {
			this->debugPrintf("Invalid item %s", argv[argi]);
			return true;
		}
	}

	if (argc == 0) {
		sword = true;
		shield = true;
		traps = true;
		//light = map.needsLight
		//flight = map.usesFlight
	}

	return false;
}

bool Debugger::cmdPlayMusic(int argc, const char **argv) {
	if (argc < 2) {
		this->debugPrintf("Filename required\n");
		return true;
	}
	_engine->_music->playSong(argv[1]);
	return false;
}
} // End of namespace CapBible
