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

#ifndef CAPBIBLE_CAPBIBLE_H
#define CAPBIBLE_CAPBIBLE_H

#include "capbible/detection.h"
#include "common/random.h"
#include "engines/engine.h"

namespace Common {
class SeekableReadStream;
}

/**
 * This is the namespace of the CapBible engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Captain Bible in Dome of Darkness
 */
namespace CapBible {

class MainArchive;
class Debugger;

enum {
	GAME_SCREEN_WIDTH = 320,
	GAME_SCREEN_HEIGHT = 240
};

class CapBibleEngine : public Engine {
public:
	CapBibleEngine(OSystem *syst, const ADGameDescription *gameDescription);
	~CapBibleEngine() override;

	Common::RandomSource randomizer;

	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	int getAutosaveSlot() const override { return 99; }

	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

protected:
	const ADGameDescription *_gameDescription;

	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

	bool isDemo() const { return _gameDescription->flags & ADGF_DEMO; }

private:
	MainArchive *_mainArchive;
	Debugger *_debugger;

	friend class Debugger;
};

} // End of namespace CapBible

#endif
