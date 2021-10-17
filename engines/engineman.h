/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ENGINES_ENGINEMAN_H
#define ENGINES_ENGINEMAN_H

#include "common/singleton.h"
#include "base/plugins.h"
#include "engines/metaengine.h"

/**
 * Singleton class that manages all engine plugins.
 */
class EngineManager : public Common::Singleton<EngineManager> {
public:
	/**
	 * Given a list of FSNodes in a given directory, detect a set of games contained within.
	 *
	 * Returns an empty list if none are found.
	 */
	DetectionResults detectGames(const Common::FSList &fslist) const;

	/** Find a plugin by its engine ID. */
	const Plugin *findPlugin(const Common::String &engineId) const;

	const Plugin *findEnginePlugin(const Common::String &engineId);

	/**
	 * A method which takes in a plugin of type ENGINE,
	 * and returns the appropriate & matching METAENGINE.
	 * It uses the Engine plugin's getName method, which is an identifier,
	 * and then tries to matches it with each plugin present in memory.
	 *
	 * @param plugin A plugin of type ENGINE.
	 *
	 * @return A plugin of type METAENGINE.
	 */
	const Plugin *getMetaEngineFromEngine(const Plugin *plugin);

	/**
	 * A method which takes in a plugin of type METAENGINE,
	 * and returns the appropriate & matching ENGINE.
	 * It uses the MetaEngine's getEngineID to reconstruct the name
	 * of engine plugin, and then tries to matches it with each plugin in memory.
	 *
	 * @param A plugin of type METAENGINE.
	 *
	 * @return A plugin of type ENGINE.
	 */
	const Plugin *getEngineFromMetaEngine(const Plugin *plugin);

	/** Find a target. */
	QualifiedGameDescriptor findTarget(const Common::String &target, const Plugin **plugin = NULL) const;

	/**
	 * List games matching the specified criteria.
	 *
	 * If the engine ID is not specified, this scans all the plugins,
	 * loading them from the disk if necessary. This is a slow operation on
	 * some platforms and should not be used for the happy path.
	 */
	QualifiedGameList findGamesMatching(const Common::String &engineId, const Common::String &gameId) const;

	
	virtual bool loadPluginFromEngineId(const Common::String &engineId);

	/**
	 * Create a target from the supplied game descriptor.
	 *
	 * @return The created target name.
	 */
	Common::String createTargetForGame(const DetectedGame &game);

	/** Upgrade a target to the current configuration format. */
	void upgradeTargetIfNecessary(const Common::String &target) const;

private:
	/** Find a game across all loaded plugins. */
	QualifiedGameList findGameInLoadedPlugins(const Common::String &gameId) const;

	/** Use heuristics to complete a target lacking an engine ID. */
	void upgradeTargetForEngineId(const Common::String &target) const;
};

/** Convenience shortcut for accessing the engine manager. */
#define EngineMan EngineManager::instance()
/** @} */

#endif
