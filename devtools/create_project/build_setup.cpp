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

#define ENABLE_XCODE

#if (defined(_WIN32) || defined(WIN32)) && !defined(__GNUC__)
#define USE_WIN32_API
#endif

#if (defined(_WIN32) || defined(WIN32))
#define _WIN32_WINNT 0x0502
#include <windows.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sstream>
#include <sys/param.h>
#include <sys/stat.h>
#endif

#include "create_project.h"
#include "config.h"

#include "cmake.h"
#include "codeblocks.h"
#include "msbuild.h"
#include "msvc.h"
#include "xcode.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stack>
#include <utility>

#include <cstdlib>
#include <cstring>
#include <ctime>

namespace {
/**
 * Parse the configure.engine file of a given engine directory and return a
 * list of all defined engines.
 *
 * @param engineDir The directory of the engine.
 * @return The list of all defined engines.
 */
EngineDescList parseEngineConfigure(const std::string &engineDir);


#ifdef FIRST_ENGINE
/**
 * Compares two FSNode entries in a strict-weak fashion based on engine name
 * order.
 *
 * @param left  The first operand.
 * @param right The second operand.
 * @return "true" when the name of the left operand is strictly smaller than
 *         the name of the second operand. "false" otherwise.
 */
bool compareEngineNames(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right);
#endif
} // End of anonymous namespace

EngineDescList parseEngines(const std::string &srcDir) {
	using CreateProjectTool::FileList;
	using CreateProjectTool::listDirectory;

	EngineDescList engineList;

	FileList engineFiles = listDirectory(srcDir + "/engines/");

#ifdef FIRST_ENGINE
	// In case we want to sort an engine to the front of the list we will
	// use some manual sorting predicate which assures that.
	engineFiles.sort(&compareEngineNames);
#else
	// Otherwise, we simply sort the file list alphabetically this allows
	// for a nicer order in --list-engines output, for example.
	engineFiles.sort(&compareFSNode);
#endif

	for (FileList::const_iterator i = engineFiles.begin(), end = engineFiles.end(); i != end; ++i) {
		// Each engine requires its own sub directory thus we will skip all
		// non directory file nodes here.
		if (!i->isDirectory) {
			continue;
		}

		// Retrieve all engines defined in this sub directory and add them to
		// the list of all engines.
		EngineDescList list = parseEngineConfigure(srcDir + "/engines/" + i->name);
		engineList.splice(engineList.end(), list);
	}

	return engineList;
}

bool isSubEngine(const std::string &name, const EngineDescList &engines) {
	for (EngineDescList::const_iterator i = engines.begin(); i != engines.end(); ++i) {
		if (std::find(i->subEngines.begin(), i->subEngines.end(), name) != i->subEngines.end())
			return true;
	}

	return false;
}

bool setEngineBuildState(const std::string &name, EngineDescList &engines, bool enable) {
	if (enable && isSubEngine(name, engines)) {
		// When we enable a sub engine, we need to assure that the parent is also enabled,
		// thus we enable both sub engine and parent over here.
		EngineDescList::iterator engine = std::find(engines.begin(), engines.end(), name);
		if (engine != engines.end()) {
			engine->enable = enable;

			for (engine = engines.begin(); engine != engines.end(); ++engine) {
				if (std::find(engine->subEngines.begin(), engine->subEngines.end(), name) != engine->subEngines.end()) {
					engine->enable = true;
					break;
				}
			}

			return true;
		}
	} else {
		EngineDescList::iterator engine = std::find(engines.begin(), engines.end(), name);
		if (engine != engines.end()) {
			engine->enable = enable;

			// When we disable an engine, we also need to disable all the sub engines.
			if (!enable && !engine->subEngines.empty()) {
				for (StringList::const_iterator j = engine->subEngines.begin(); j != engine->subEngines.end(); ++j) {
					EngineDescList::iterator subEngine = std::find(engines.begin(), engines.end(), *j);
					if (subEngine != engines.end())
						subEngine->enable = false;
				}
			}

			return true;
		}
	}

	return false;
}

StringList getEngineDefines(const EngineDescList &engines) {
	StringList result;

	for (EngineDescList::const_iterator i = engines.begin(); i != engines.end(); ++i) {
		if (i->enable)
			result.push_back("ENABLE_" + CreateProjectTool::toUpper(i->name));
	}

	return result;
}

namespace {
/**
 * Try to parse a given line and create an engine definition
 * out of the result.
 *
 * This may take *any* input line, when the line is not used
 * to define an engine the result of the function will be "false".
 *
 * Note that the contents of "engine" are undefined, when this
 * function returns "false".
 *
 * @param line Text input line.
 * @param engine Reference to an object, where the engine information
 *               is to be stored in.
 * @return "true", when parsing succeeded, "false" otherwise.
 */
bool parseEngine(const std::string &line, EngineDesc &engine) {
	// Format:
	// add_engine engine_name "Readable Description" enable_default ["SubEngineList"] ["base games"] ["dependencies"]
	TokenList tokens = tokenize(line);

	if (tokens.size() < 4)
		return false;

	TokenList::const_iterator token = tokens.begin();

	if (*token != "add_engine")
		return false;
	++token;

	engine.name = *token;
	++token;
	engine.desc = *token;
	++token;
	engine.enable = (*token == "yes");
	++token;
	if (token != tokens.end()) {
		engine.subEngines = tokenize(*token);
		++token;
		if (token != tokens.end())
			++token;
		if (token != tokens.end())
			engine.requiredFeatures = tokenize(*token);
	}

	return true;
}

EngineDescList parseEngineConfigure(const std::string &engineDir) {
	std::string configureFile = engineDir + "/configure.engine";

	std::ifstream configure(configureFile.c_str());
	if (!configure)
		return EngineDescList();

	std::string line;
	EngineDescList engines;

	for (;;) {
		std::getline(configure, line);
		if (configure.eof())
			break;

		if (configure.fail())
			error("Failed while reading from " + configureFile);

		EngineDesc desc;
		if (parseEngine(line, desc))
			engines.push_back(desc);
	}

	return engines;
}

bool compareFSNode(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right) {
	return left.name < right.name;
}

#ifdef FIRST_ENGINE
bool compareEngineNames(const CreateProjectTool::FSNode &left, const CreateProjectTool::FSNode &right) {
	if (left.name == FIRST_ENGINE) {
		return right.name != FIRST_ENGINE;
	} else if (right.name == FIRST_ENGINE) {
		return false;
	} else {
		return compareFSNode(left, right);
	}
}
#endif
} // End of anonymous namespace

namespace {
	// clang-format off
	const Feature s_features[] = {
		// Libraries (must be added in generators)
		{      "zlib",        "USE_ZLIB", true, true,  "zlib (compression) support" },
		{       "mad",         "USE_MAD", true, true,  "libmad (MP3) support" },
		{   "fribidi",     "USE_FRIBIDI", true, true,  "BiDi support" },
		{       "ogg",         "USE_OGG", true, true,  "Ogg support" },
		{    "vorbis",      "USE_VORBIS", true, true,  "Vorbis support" },
		{    "tremor",      "USE_TREMOR", true, false, "Tremor support" },
		{      "flac",        "USE_FLAC", true, true,  "FLAC support" },
		{       "png",         "USE_PNG", true, true,  "libpng support" },
		{       "gif",         "USE_GIF", true, false, "libgif support" },
		{      "faad",        "USE_FAAD", true, false, "AAC support" },
		{     "mpeg2",       "USE_MPEG2", true, true,  "MPEG-2 support" },
		{ "theoradec",   "USE_THEORADEC", true, true,  "Theora decoding support" },
		{ "freetype2",   "USE_FREETYPE2", true, true,  "FreeType support" },
		{      "jpeg",        "USE_JPEG", true, true,  "libjpeg support" },
		{"fluidsynth",  "USE_FLUIDSYNTH", true, true,  "FluidSynth support" },
		{ "fluidlite",   "USE_FLUIDLITE", true, false, "FluidLite support" },
		{   "libcurl",     "USE_LIBCURL", true, true,  "libcurl support" },
		{    "sdlnet",     "USE_SDL_NET", true, true,  "SDL_net support" },
		{   "discord",     "USE_DISCORD", true, false, "Discord support" },
		{ "retrowave",   "USE_RETROWAVE", true, false, "RetroWave OPL3 support" },

		// Feature flags
		{             "bink",                      "USE_BINK", false, true,  "Bink video support" },
		{          "scalers",                   "USE_SCALERS", false, true,  "Scalers" },
		{        "hqscalers",                "USE_HQ_SCALERS", false, true,  "HQ scalers" },
		{      "edgescalers",              "USE_EDGE_SCALERS", false, true,  "Edge scalers" },
		{           "aspect",                    "USE_ASPECT", false, true,  "Aspect ratio correction" },
		{            "16bit",                 "USE_RGB_COLOR", false, true,  "16bit color support" },
		{          "highres",                   "USE_HIGHRES", false, true,  "high resolution" },
		{          "mt32emu",                   "USE_MT32EMU", false, true,  "integrated MT-32 emulator" },
		{              "lua",                       "USE_LUA", false, true,  "lua" },
		{             "nasm",                      "USE_NASM", false, true,  "IA-32 assembly support" }, // This feature is special in the regard, that it needs additional handling.
		{           "tinygl",                    "USE_TINYGL", false, true,  "TinyGL support" },
		{           "opengl",                    "USE_OPENGL", false, true,  "OpenGL support" },
		{      "opengl_game",               "USE_OPENGL_GAME", false, true,  "OpenGL support (classic) in 3d games" },
		{   "opengl_shaders",            "USE_OPENGL_SHADERS", false, true,  "OpenGL support (shaders) in 3d games" },
		{          "taskbar",                   "USE_TASKBAR", false, true,  "Taskbar integration support" },
		{            "cloud",                     "USE_CLOUD", false, true,  "Cloud integration support" },
		{      "translation",               "USE_TRANSLATION", false, true,  "Translation support" },
		{           "vkeybd",                 "ENABLE_VKEYBD", false, false, "Virtual keyboard support"},
		{    "eventrecorder",          "ENABLE_EVENTRECORDER", false, false, "Event recorder support"},
		{          "updates",                   "USE_UPDATES", false, false, "Updates support"},
		{          "dialogs",                "USE_SYSDIALOGS", false, true,  "System dialogs support"},
		{       "langdetect",                "USE_DETECTLANG", false, true,  "System language detection support" }, // This feature actually depends on "translation", there
																												   // is just no current way of properly detecting this...
		{     "text-console", "USE_TEXT_CONSOLE_FOR_DEBUGGER", false, false, "Text console debugger" }, // This feature is always applied in xcode projects
		{              "tts",                       "USE_TTS", false, true,  "Text to speech support"},
		{"builtin-resources",             "BUILTIN_RESOURCES", false, true,  "include resources (e.g. engine data, fonts) into the binary"},
		{   "detection-full",                "DETECTION_FULL", false, true,  "Include detection objects for all engines" },
		{ "detection-static", "USE_DETECTION_FEATURES_STATIC", false, true,  "Static linking of detection objects for engines."},
	};

	const Tool s_tools[] = {
		{ "create_cryo",         true},
		{ "create_drascula",     true},
		{ "create_hugo",         true},
		{ "create_kyradat",      true},
		{ "create_lure",         true},
		{ "create_neverhood",    true},
		{ "create_teenagent",    true},
		{ "create_titanic",      true},
		{ "create_tony",         true},
		{ "create_toon",         true},
		{ "create_translations", true},
		{ "qtable",              true}
	};

}

FeatureList getAllFeatures() {
	const size_t featureCount = sizeof(s_features) / sizeof(s_features[0]);

	FeatureList features;
	for (size_t i = 0; i < featureCount; ++i)
		features.push_back(s_features[i]);

	return features;
}

StringList getFeatureDefines(const FeatureList &features) {
	StringList defines;

	for (FeatureList::const_iterator i = features.begin(); i != features.end(); ++i) {
		if (i->enable && i->define && i->define[0])
			defines.push_back(i->define);
	}

	return defines;
}

bool setFeatureBuildState(const std::string &name, FeatureList &features, bool enable) {
	FeatureList::iterator i = std::find(features.begin(), features.end(), name);
	if (i != features.end()) {
		i->enable = enable;
		return true;
	} else {
		return false;
	}
}

bool getFeatureBuildState(const std::string &name, const FeatureList &features) {
	FeatureList::const_iterator i = std::find(features.begin(), features.end(), name);
	if (i != features.end()) {
		return i->enable;
	} else {
		return false;
	}
}

BuildSetup removeFeatureFromSetup(BuildSetup setup, const std::string &feature) {
	// TODO: disable feature instead of removing from setup
	for (FeatureList::iterator i = setup.features.begin(); i != setup.features.end(); ++i) {
		if (i->enable && feature == i->name) {
			if (i->define && i->define[0]) {
				setup.defines.remove(i->define);
			}
			setup.features.erase(i);
			break;
		}
	}
	return setup;
}

ToolList getAllTools() {
	const size_t toolCount = sizeof(s_tools) / sizeof(s_tools[0]);

	ToolList tools;
	for (size_t i = 0; i < toolCount; ++i)
		tools.push_back(s_tools[i]);

	return tools;
}

bool BuildSetup::featureEnabled(std::string feature) const {
	return getFeature(feature).enable;
}

Feature BuildSetup::getFeature(std::string feature) const {
	for (FeatureList::const_iterator itr = features.begin(); itr != features.end(); ++itr) {
		if (itr->name != feature)
			continue;
		return *itr;
	}
	error("invalid feature request: " + feature);
}
