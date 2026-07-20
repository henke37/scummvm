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

#include "base/plugins.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"
#include <common/compression/unzip.h>
#include "common/path.h"

#include "engines/util.h"

#include "capbible/capbible.h"
#include "capbible/debugger.h"
#include "capbible/mainarchive.h"
#include "capbible/music.h"

namespace CapBible {

CapBibleEngine::CapBibleEngine(OSystem *syst, const ADGameDescription *gameDescription)
	: Engine(syst), _mainArchive(nullptr), _gameDescription(gameDescription), _scene(NULL),
	randomizer("capbible") {
}

CapBibleEngine::~CapBibleEngine() {
	delete _music;
    delete _scene;
}

bool CapBible::CapBibleEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error CapBibleEngine::run() {
	initGraphics(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

	setDebugger(_debugger=new Debugger(this));

	_mainArchive = new MainArchive(isDemo() ? "cbse.dat" : "dd1.dat");
	SearchMan.add("Main archive", _mainArchive);

	if (isDemo()) {
		Common::Archive *soundArch = Common::makeZipArchive("CBSEDRV.EXE");
		SearchMan.add("Sound archive", soundArch);
	}

	// Setup mixer
	syncSoundSettings();
	_music = new Music();

    Common::Error err = newScene("LOGO.BIN");
    if (_scene != NULL) {
        _scene->step(); // Might need to do this every cycle
    }

	while (!shouldQuit()) {
		Common::Event evt;
		g_system->getEventManager()->pollEvent(evt);
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

void CapBibleEngine::pauseEngineIntern(bool pause) {
	if (pause) {
		_music->pause();
	} else {
		_music->resume();
	}
}

Common::Error CapBibleEngine::newScene(const char *path) {
    Common::SeekableReadStream *stream = _mainArchive->createReadStreamForMember(path);
    if (stream == NULL) {
        error("Failed to open stream %s", path);
        return Common::kPathDoesNotExist;
    }

    int64 streamSize = stream->size();
    debug("streamSize = %llu", streamSize);

    Common::Array<byte> bytes((uint32)streamSize);
    uint32 bytesRead = stream->read(bytes.data(), bytes.size());
    if (bytesRead != bytes.size() || stream->err()) {
        error("Failed to read stream %s", path);
        delete stream;
        return Common::kReadingFailed;
    }

    delete stream;

    debug("Finished reading script");

    delete _scene;
    _scene = new Scene(bytes);

    return Common::kNoError;
}

Scene::Scene(Common::Array<byte> scr) : script(scr) {
    // Create initial thread
    _threads.push_back(Thread(this));
}

Scene::~Scene() {
}

void Scene::step() {
    for (Thread &t : _threads) {
        if (t.state == tsActive) {
            t.step();
        }
    }
}

Thread::Thread(Scene *scene) : state(tsActive), _scene(scene), _pc(0) {
}

Thread::~Thread() {
    // Don't delete scene, we don't own it
}

void Thread::step() {
    debug("runScript: size = %d", _scene->script.size());
    if (_pc >= _scene->script.size()) {
        error("Invalid pc %u", _pc);
        return;
    }

    for (;;) {
        byte opcode = _scene->script[_pc];
        _pc++;
        // debug("pc = 0x%x, opcode = 0x%02x", _pc - 1, opcode);

        switch (opcode) {
        case 0x01: {
            // load_art: load the named ART member into the next art slot.
            Common::String name = readString();
            debug("load_art '%s'", name.c_str());
            break;
        }

        case 0x06: {
            // begin_animation_sequence: declare an animation with the supplied step interval
            uint16 interval = readUint16LE();
            debug("begin_animation_sequence %d", interval);
            break;
        }

        case 0x4C: {
            // fill_screen: fill all 320×200 pixels with the palette index
            if (_pc >= _scene->script.size()) {
                error("Invalid pc %u", _pc);
                return;
            }
            byte palette_index = _scene->script[_pc];
            _pc++;

            debug("fill_screen 0x%02x", palette_index);
            break;
        }

        case 0x4D: {
            // load_palette: load named PAL member
            Common::String name = readString();
            debug("load_palette '%s'", name.c_str());
            break;
        }
        default:
            error("Unknown opcode 0x%02x at address 0x%x", opcode, _pc - 1);
        }
    }
}

Common::String Thread::readString() {
    Common::Array<byte> s;
    while (_pc < _scene->script.size()) {
        char c = _scene->script[_pc];
        _pc++;
        if (c == 0)
            break;
        else
            s.push_back(c);
    }
    return Common::String((const char *)s.data(), s.size());
}

byte Thread::readByte() {
    if (_pc < _scene->script.size()) {
        byte b = _scene->script[_pc];
        _pc++;
        return b;
    }
    else {
        return 0;
    }
}

uint16 Thread::readUint16LE() {
    uint16_t lo = readByte();
    uint16_t hi = readByte();
    return (hi << 8) | lo;
}

} // End of namespace CapBible
