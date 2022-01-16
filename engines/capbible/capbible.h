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

#include "engines/engine.h"
#include "common/random.h"

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

enum {
	GAME_SCREEN_WIDTH = 320,
	GAME_SCREEN_HEIGHT = 240
};


class CapBibleEngine : public Engine {
public:

	CapBibleEngine(OSystem *syst);
	~CapBibleEngine() override;

	Common::RandomSource randomizer;

	bool canLoadGameStateCurrently() override;
	bool canSaveGameStateCurrently() override;
	int getAutosaveSlot() const override { return 99; }

protected:

	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
};

} // End of namespace CapBible

#endif
