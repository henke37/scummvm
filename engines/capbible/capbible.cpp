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
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "engines/util.h"

#include "capbible/capbible.h"

namespace CapBible {

CapBibleEngine::CapBibleEngine(OSystem *syst)
	: Engine(syst), 
	randomizer("capbible") {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "drivers");
}

CapBibleEngine::~CapBibleEngine() {
}

bool CapBibleEngine::canLoadGameStateCurrently() {
	return true;
}

bool CapBibleEngine::canSaveGameStateCurrently() {
	return true;
}

bool CapBible::CapBibleEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

Common::Error CapBibleEngine::run() {
	initGraphics(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

	// Setup mixer
	syncSoundSettings();

	while (!shouldQuit()) {
		Common::Event evt;
		g_system->getEventManager()->pollEvent(evt);
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

} // End of namespace CapBible
