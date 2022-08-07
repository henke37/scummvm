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
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/random.h"

#include "engines/advancedDetector.h"

#include "sludge/cursors.h"
#include "sludge/detection.h"
#include "sludge/event.h"
#include "sludge/fileset.h"
#include "sludge/fonttext.h"
#include "sludge/floor.h"
#include "sludge/graphics.h"
#include "sludge/language.h"
#include "sludge/main_loop.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sound.h"
#include "sludge/speech.h"
#include "sludge/statusba.h"
#include "sludge/timing.h"

namespace Sludge {

SludgeEngine *g_sludge;

Graphics::PixelFormat *SludgeEngine::getScreenPixelFormat() const { return _pixelFormat; }
Graphics::PixelFormat *SludgeEngine::getOrigPixelFormat() const { return _origFormat; }

SludgeEngine::SludgeEngine(OSystem *syst, const SludgeGameDescription *gameDesc) :
		Engine(syst), _gameDescription(gameDesc) {

	// register your random source
	_rnd = new Common::RandomSource("sludge");

	//DebugMan.enableDebugChannel("loading");
	//DebugMan.enableDebugChannel("builtin");

	_dumpScripts = ConfMan.getBool("dump_scripts");

	// init graphics
	_origFormat = new Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	_pixelFormat = new Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);

	// Init Strings
	loadNow = "";
	gamePath = "";

	// Init managers
	_timer = new Timer();
	_fatalMan = new FatalMsgManager();
	_peopleMan = new PeopleManager(this);
	_resMan = new ResourceManager();
	_languageMan = new LanguageManager();
	_objMan = new ObjectManager(this);
	_gfxMan = new GraphicsManager(this);
	_evtMan = new EventManager(this);
	_soundMan = new SoundManager();
	_txtMan = new TextManager();
	_cursorMan = new CursorManager(this);
	_speechMan = new SpeechManager(this);
	_regionMan = new RegionManager(this);
	_floorMan = new FloorManager(this);
	_statusBar = new StatusBarManager(this);
}

SludgeEngine::~SludgeEngine() {

	// Dispose resources
	delete _rnd;
	_rnd = nullptr;

	// Dispose pixel formats
	delete _origFormat;
	_origFormat = nullptr;
	delete _pixelFormat;
	_pixelFormat = nullptr;

	// Dispose managers
	delete _cursorMan;
	_cursorMan = nullptr;
	delete _txtMan;
	_txtMan = nullptr;
	delete _soundMan;
	_soundMan = nullptr;
	delete _evtMan;
	_evtMan = nullptr;
	delete _gfxMan;
	_gfxMan = nullptr;
	delete _objMan;
	_objMan = nullptr;
	delete _languageMan;
	_languageMan = nullptr;
	delete _resMan;
	_resMan = nullptr;
	delete _speechMan;
	_speechMan = nullptr;
	delete _regionMan;
	_regionMan = nullptr;
	delete _peopleMan;
	_peopleMan = nullptr;
	delete _floorMan;
	_floorMan = nullptr;
	delete _fatalMan;
	_fatalMan = nullptr;
	delete _statusBar;
	delete _timer;
}

Common::Error SludgeEngine::run() {
	// set global variable
	g_sludge = this;

	// debug log
	main_loop(getGameFile());

	return Common::kNoError;
}

bool SludgeEngine::hasFeature(EngineFeature f) const {
	if (f == kSupportsReturnToLauncher)
		return true;
	return false;
}

Common::String SludgeEngine::dataFileToGameId(Common::String dataFile) const {
	MetaEngineDetection &detect = getMetaEngineDetection();

	Common::FSNode currentPathNode(ConfMan.get("path"));
	Common::FSList fsList;
	currentPathNode.getChildren(fsList, Common::FSNode::kListFilesOnly);

	DetectedGames games = detect.detectGames(fsList, 0, true);

	for (auto gamesItr = games.begin(); gamesItr != games.end(); ++gamesItr) {
		FilePropertiesMap &files = gamesItr->matchedFiles;
		for (auto fileItr = files.begin(); fileItr != files.end(); ++fileItr) {
			if (fileItr->_key == dataFile) {
				return gamesItr->gameId;
			}
		}
	}
	return Common::String();
}

Common::String SludgeEngine::gameIdToTarget(Common::String gameId) const {
	Common::String currentPath = ConfMan.get("path");
	
	Common::ConfigManager::DomainMap::iterator iter = ConfMan.beginGameDomains();
	for (; iter != ConfMan.endGameDomains(); ++iter) {
		Common::ConfigManager::Domain &dom = iter->_value;

		if (dom.getVal("gameid") != gameId)
			continue;
		if (dom.getVal("path") != currentPath)
			continue;
		if (dom.getVal("engineid") != "sludge")
			continue;

		return iter->_key;
	}

	return Common::String();
}

Common::String SludgeEngine::getGameExecutable() const {
	Common::String gameFile(getGameFile());
	const ADGameDescription &desc = _gameDescription->desc;

	//if the executable is listed, good, use that!
	for (int fileIndex = 0; fileIndex < 14; ++fileIndex) {
		const char *fileName = desc.filesDescriptions[fileIndex].fileName;
		if (strstr(fileName, ".exe") != NULL) {
			return Common::String(fileName);
		}
	}

	//no listed executable? :(
	//try guessing

	//if there is a dat file, then it will have to be the executable
	size_t datPos = gameFile.findLastOf(".dat");
	if (datPos != Common::String::npos) {
		return gameFile.substr(0, datPos) + ".exe";
	}

	//or a slg file
	size_t slgPos = gameFile.findLastOf(".slg");
	if (slgPos != Common::String::npos) {
		return gameFile.substr(0, slgPos) + ".exe";
	}

	//dang, no dat file!
	//just hope the executable is named the same as the datafile
	return gameFile + ".exe";
}

bool SludgeEngine::launchNextGame(Common::String datafile) {
	Common::String gameId = dataFileToGameId(datafile);

	if (gameId.empty())
		return false;

	Common::String target = gameIdToTarget(gameId);
	if (target.empty())
		return false;

	ChainedGamesMan.push(target);

	// Force a return to the launcher.
	// This will start the chained game.
	Common::EventManager *eventMan = g_system->getEventManager();
	Common::Event event;
	event.type = Common::EVENT_RETURN_TO_LAUNCHER;
	eventMan->pushEvent(event);

	return true;
}

} // End of namespace Sludge
