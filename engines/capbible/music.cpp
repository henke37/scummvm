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

#include "common/config-manager.h"
#include "common/file.h"

#include "audio/mididrv.h"
#include "audio/miles.h"

#include "engines/util.h"

#include "capbible/music.h"

namespace CapBible {
Music::Music() : _trackData(nullptr) {

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PCSPK | MDT_PREFER_MT32);
	MusicType musType = MidiDriver::getMusicType(dev);
	bool nativeMT32 = (musType == MT_MT32) || ConfMan.getBool("native_mt32");
	bool isDemo = CapBibleEngine::instance()->isDemo();

	if (musType == MT_ADLIB) {
		_driver = Audio::MidiDriver_Miles_AdLib_create("", Common::String(isDemo ? "cbsedrv/" : "drivers/")+"FAT.OPL");
	} else if (musType == MT_PCSPK) {
	} else if (musType == MT_GM || musType == MT_MT32) {
		_driver = Audio::MidiDriver_Miles_MIDI_create(musType, "");
	} else {
		_driver = nullptr;
	}
	_driver->open();

	_parser = MidiParser::createParser_XMIDI();

	_parser->setMidiDriver(_driver);
	_parser->setTimerRate(_driver->getBaseTempo());
	_driver->setTimerCallback(_parser, MidiParser::timerCallback);
}

Music::~Music() {
	_driver->setTimerCallback(nullptr, nullptr);

	_parser->unloadMusic();
	delete _parser;

	_driver->close();
	delete _driver;
	delete _trackData;
}

void Music::playSong(Common::String fileName) {
	Common::File trackFile;

	delete _trackData;

	trackFile.open(fileName);
	uint32 buffSize = trackFile.size();
	_trackData = (byte *)malloc(buffSize);
	trackFile.read(_trackData, buffSize);

	_parser->loadMusic(_trackData, buffSize);
}
} // End of namespace CapBible
