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

#include "common/file.h"

#include "capbible/verseBank.h"

namespace CapBible {

	VerseBank::VerseBank(const Common::Path &path) {
		Common::File file;
		file.open(path);
		assert(file.isOpen());

		for (;;) {
			Verse verse;
			verse.verseNumber=file.readByte();
			if (verse.verseNumber == 0)
				break;
			verse.quoteRecordsStartOffset = file.readUint16LE();
			//TODO deal with text encoding
			verse.heading = file.readString('|');
			verse.quote = file.readString(0);

			_verses.push_back(verse);
		}

	}

	VerseBank::~VerseBank() {
	}

	const VerseBank::Verse &VerseBank::getVerse(byte verseNumber) const {
		for (uint32 i = 0; i < _verses.size(); ++i) {
			if (_verses[i].verseNumber == verseNumber)
				return _verses[i];
		}
		error("VerseBank::getVerse: verse %d not found", verseNumber);
	}
	
} // namespace CapBible
