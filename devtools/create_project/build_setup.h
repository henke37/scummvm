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

#ifndef TOOLS_BUILD_SETUP_H
#define TOOLS_BUILD_SETUP_H

#include <list>
#include <map>
#include <map>
#include <string>
#include <vector>

#include <cassert>
#include "create_project.h"

/**
 * Structure to describe a game engine to be built into ScummVM.
 *
 * We do get the game engines available by parsing the "configure"
 * script of our source distribution. See "parseConfigure" for more
 * information on that.
 * @see parseConfigure
 */
struct EngineDesc {
	/**
	 * The name of the engine. We use this to determine the directory
	 * the engine is in and to create the define, which needs to be
	 * set to enable the engine.
	 */
	std::string name;

	/**
	 * A human readable description of the engine. We will use this
	 * to display a description of the engine to the user in the list
	 * of which engines are built and which are disabled.
	 */
	std::string desc;

	/**
	 * Whether the engine should be included in the build or not.
	 */
	bool enable;

	/**
	 * Features required for this engine.
	 */
	StringList requiredFeatures;

	/**
	 * A list of all available sub engine names. Sub engines are engines
	 * which are built on top of an existing engines and can be only
	 * enabled when the parten engine is enabled.
	 */
	StringList subEngines;

	bool operator==(const std::string &n) const {
		return (name == n);
	}
};

typedef std::list<EngineDesc> EngineDescList;

/**
 * This function parses the project directory and creates a list of
 * available engines.
 *
 * It will also automatically setup the default build state (enabled
 * or disabled) to the state specified in the individual configure.engine
 * files.
 *
 * @param srcDir Path to the root of the project source.
 * @return List of available engines.
 */
EngineDescList parseEngines(const std::string &srcDir);

/**
 * Checks whether the specified engine is a sub engine. To determine this
 * there is a fully setup engine list needed.
 *
 * @param name Name of the engine to check.
 * @param engines List of engines.
 * @return "true", when the engine is a sub engine, "false" otherwise.
 */
bool isSubEngine(const std::string &name, const EngineDescList &engines);

/**
 * Enables or disables the specified engine in the engines list.
 *
 * This function also disables all sub engines of an engine, when it is
 * to be disabled.
 * Also this function does enable the parent of a sub engine, when a
 * sub engine is to be enabled.
 *
 * @param name Name of the engine to be enabled or disabled.
 * @param engines The list of engines, which should be operated on.
 * @param enable Whether the engine should be enabled or disabled.
 * @return "true", when it succeeded, "false" otherwise.
 */
bool setEngineBuildState(const std::string &name, EngineDescList &engines, bool enable);

/**
 * Returns a list of all defines, according to the engine list passed.
 *
 * @param features The list of engines, which should be operated on. (this may contain engines, which are *not* enabled!)
 */
StringList getEngineDefines(const EngineDescList &engines);

/**
 * Structure to define a given feature, usually an external library,
 * used to build ScummVM.
 */
struct Feature {
	const char *name;   ///< Name of the feature
	const char *define; ///< Define of the feature
	bool library; ///< Whether this feature needs to be linked to a library

	bool enable; ///< Whether the feature is enabled or not

	const char *description; ///< Human readable description of the feature

	bool operator==(const std::string &n) const {
		return (name == n);
	}
};
typedef std::list<Feature> FeatureList;

struct Tool {
	const char *name; ///< Name of the tools
	bool enable;      ///< Whether the tools is enabled or not
};
typedef std::list<Tool> ToolList;

/**
 * Creates a list of all features available for MSVC.
 *
 * @return A list including all features available.
 */
FeatureList getAllFeatures();

/**
 * Returns a list of all defines, according to the feature set
 * passed.
 *
 * @param features List of features for the build (this may contain features, which are *not* enabled!)
 */
StringList getFeatureDefines(const FeatureList &features);

/**
 * Sets the state of a given feature. This can be used to
 * either include or exclude an feature.
 *
 * @param name Name of the feature.
 * @param features List of features to operate on.
 * @param enable Whether the feature should be enabled or disabled.
 * @return "true", when it succeeded, "false" otherwise.
 */
bool setFeatureBuildState(const std::string &name, FeatureList &features, bool enable);

/**
 * Gets the state of a given feature.
 *
 * @param name Name of the feature.
 * @param features List of features to operate on.
 * @return "true", when the feature is enabled, "false" otherwise.
 */
bool getFeatureBuildState(const std::string &name, const FeatureList &features);

ToolList getAllTools();

/**
 * Structure to describe a build setup.
 *
 * This includes various information about which engines to
 * enable, which features should be built into the main executable.
 * It also contains the path to the project source root.
 */
struct BuildSetup {
	std::string projectName;        ///< Project name
	std::string projectDescription; ///< Project description

	std::string srcDir;     ///< Path to the sources.
	std::string filePrefix; ///< Prefix for the relative path arguments in the project files.
	std::string outputDir;  ///< Path where to put the MSVC project files.

	StringList includeDirs; ///< List of additional include paths
	StringList libraryDirs; ///< List of additional library paths

	EngineDescList engines; ///< Engine list for the build (this may contain engines, which are *not* enabled!).
	FeatureList features;   ///< Feature list for the build (this may contain features, which are *not* enabled!).

	StringList defines;   ///< List of all defines for the build.
	StringList testDirs;  ///< List of all folders containing tests

	bool devTools;             ///< Generate project files for the tools
	bool tests;                ///< Generate project files for the tests
	bool runBuildEvents;       ///< Run build events as part of the build (generate revision number and copy engine/theme data & needed files to the build folder
	bool createInstaller;      ///< Create installer after the build
	bool useSDL2;              ///< Whether to use SDL2 or not.
	bool useCanonicalLibNames; ///< Whether to use canonical libraries names or default ones
	bool useStaticDetection;   ///< Whether to link detection features inside the executable or not.
	bool useWindowsUnicode;    ///< Whether to use Windows Unicode APIs or ANSI APIs.

	BuildSetup() {
		devTools = false;
		tests = false;
		runBuildEvents = false;
		createInstaller = false;
		useSDL2 = true;
		useCanonicalLibNames = false;
		useStaticDetection = true;
		useWindowsUnicode = true;
	}

	bool featureEnabled(std::string feature) const;
	Feature getFeature(std::string feature) const;
};

/**
 * Removes given feature from setup.
 *
 * @param setup The setup to be processed.
 * @param feature The feature to be removed
 * @return A copy of setup less feature.
 */
BuildSetup removeFeatureFromSetup(BuildSetup setup, const std::string &feature);


#endif // TOOLS_BUILD_SETUP_H
