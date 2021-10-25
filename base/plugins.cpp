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

#include "base/plugins.h"
#include "base/pluginCollection.h"

#include "common/func.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/config-manager.h"

#ifdef DYNAMIC_MODULES
#include "common/fs.h"
#endif

// Plugin versioning

int pluginTypeVersions[PLUGIN_TYPE_MAX] = {
	PLUGIN_TYPE_ENGINE_DETECTION_VERSION,
	PLUGIN_TYPE_ENGINE_VERSION,
	PLUGIN_TYPE_MUSIC_VERSION,
	PLUGIN_TYPE_DETECTION_VERSION,
	PLUGIN_TYPE_SCALER_VERSION,
};


// Abstract plugins

void Plugin::assertLoaded() const {
	assert(isLoaded());
	if (!isLoaded()) {
		warning("Plugin used without being loaded first");
		bool success = PluginMan.tryLoadPlugin(const_cast<Plugin *>(this));
		if (!success)
			error("Panic loading plugin failed");
	}
}

PluginType Plugin::getType() const {
	assertLoaded();
	return _type;
}

const char *Plugin::getName() const {
	assertLoaded();
	return _pluginObject->getName();
}

const char *Plugin::getEngineId() const {
	assertLoaded();
	if (_type == PLUGIN_TYPE_ENGINE_DETECTION) {
		return _pluginObject->getEngineId();
	}

	return nullptr;
}

StaticPlugin::StaticPlugin(PluginObject *pluginobject, PluginType type) {
	assert(pluginobject);
	assert(type < PLUGIN_TYPE_MAX);
	_pluginObject = pluginobject;
	_type = type;
}

StaticPlugin::~StaticPlugin() {
	delete _pluginObject;
}

bool StaticPlugin::loadPlugin()		{ return true; }
void StaticPlugin::unloadPlugin()	{}

class StaticPluginProvider : public PluginProvider {
public:
	StaticPluginProvider() {
	}

	~StaticPluginProvider() {
	}

	virtual PluginList getPlugins() {
		PluginList pl;

		#define LINK_PLUGIN(ID) \
			extern PluginType g_##ID##_type; \
			extern PluginObject *g_##ID##_getObject(); \
			pl.push_back(new StaticPlugin(g_##ID##_getObject(), g_##ID##_type));

		// "Loader" for the static plugins.
		// Iterate over all registered (static) plugins and load them.

		// Engine plugins
		#include "engines/plugins_table.h"

		#ifdef DETECTION_STATIC
		// Engine-detection plugins are included if we don't use uncached plugins.
		#include "engines/detection_table.h"
		#endif

		// Music plugins
		// TODO: Use defines to disable or enable each MIDI driver as a
		// static/dynamic plugin, like it's done for the engines
		LINK_PLUGIN(AUTO)
		LINK_PLUGIN(NULL)
		#if defined(WIN32) && !defined(__SYMBIAN32__)
		LINK_PLUGIN(WINDOWS)
		#endif
		#if defined(USE_ALSA)
		LINK_PLUGIN(ALSA)
		#endif
		#if defined(USE_SEQ_MIDI)
		LINK_PLUGIN(SEQ)
		#endif
		#if defined(USE_SNDIO)
		LINK_PLUGIN(SNDIO)
		#endif
		#if defined(__MINT__)
		LINK_PLUGIN(STMIDI)
		#endif
		#if defined(IRIX)
		LINK_PLUGIN(DMEDIA)
		#endif
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		LINK_PLUGIN(CAMD)
		#endif
		#if defined(MACOSX)
		LINK_PLUGIN(COREAUDIO)
		LINK_PLUGIN(COREMIDI)
		#endif
		#ifdef USE_FLUIDSYNTH
		LINK_PLUGIN(FLUIDSYNTH)
		#endif
		#ifdef USE_MT32EMU
		LINK_PLUGIN(MT32)
		#endif
		#if defined(__ANDROID__)
		LINK_PLUGIN(EAS)
		#endif
		LINK_PLUGIN(ADLIB)
		LINK_PLUGIN(PCSPK)
		LINK_PLUGIN(PCJR)
		LINK_PLUGIN(CMS)
		#ifndef DISABLE_SID
		LINK_PLUGIN(C64)
		#endif
		LINK_PLUGIN(AMIGA)
		LINK_PLUGIN(APPLEIIGS)
		LINK_PLUGIN(TOWNS)
		LINK_PLUGIN(PC98)
		LINK_PLUGIN(SEGACD)
		#if defined(USE_TIMIDITY)
		LINK_PLUGIN(TIMIDITY)
		#endif

		// Scaler plugins
		LINK_PLUGIN(NORMAL)
#ifdef USE_SCALERS
#ifdef USE_HQ_SCALERS
		LINK_PLUGIN(HQ)
#endif
#ifdef USE_EDGE_SCALERS
		LINK_PLUGIN(EDGE)
#endif
		LINK_PLUGIN(ADVMAME)
		LINK_PLUGIN(SAI)
		LINK_PLUGIN(SUPERSAI)
		LINK_PLUGIN(SUPEREAGLE)
		LINK_PLUGIN(PM)
		LINK_PLUGIN(DOTMATRIX)
		LINK_PLUGIN(TV)
#endif

		return pl;
	}
};

#ifdef DYNAMIC_MODULES

PluginList FilePluginProvider::getPlugins() {
	PluginList pl;

	// Prepare the list of directories to search
	Common::FSList pluginDirs;

	// Add the default directories
	#ifndef WIN32
	pluginDirs.push_back(Common::FSNode("."));
	#endif
	pluginDirs.push_back(Common::FSNode("plugins"));

	// Add the provider's custom directories
	addCustomDirectories(pluginDirs);

	// Add the user specified directory
	Common::String pluginsPath(ConfMan.get("pluginspath"));
	if (!pluginsPath.empty())
		pluginDirs.push_back(Common::FSNode(pluginsPath));

	Common::FSList::const_iterator dir;
	for (dir = pluginDirs.begin(); dir != pluginDirs.end(); ++dir) {
		// Load all plugins.
		// Scan for all plugins in this directory
		Common::FSList files;
		if (!dir->getChildren(files, Common::FSNode::kListFilesOnly)) {
			debug(1, "Couldn't open plugin directory '%s'", dir->getPath().c_str());
			continue;
		} else {
			debug(1, "Reading plugins from plugin directory '%s'", dir->getPath().c_str());
		}

		for (Common::FSList::const_iterator i = files.begin(); i != files.end(); ++i) {
			if (isPluginFilename(*i)) {
				pl.push_back(createPlugin(*i));
			}
		}
	}

	return pl;
}

bool FilePluginProvider::isPluginFilename(const Common::FSNode &node) const {
	Common::String filename = node.getName();

#ifdef PLUGIN_PREFIX
	// Check the plugin prefix
	if (!filename.hasPrefix(PLUGIN_PREFIX))
		return false;
#endif

#ifdef PLUGIN_SUFFIX
	// Check the plugin suffix
	if (!filename.hasSuffix(PLUGIN_SUFFIX))
		return false;
#endif

	return true;
}

void FilePluginProvider::addCustomDirectories(Common::FSList &dirs) const {
#ifdef PLUGIN_DIRECTORY
	dirs.push_back(Common::FSNode(PLUGIN_DIRECTORY));
#endif
}

#endif // DYNAMIC_MODULES

#pragma mark -

PluginManager *PluginManager::_instance = NULL;

PluginManager &PluginManager::instance() {
	if (_instance)
		return *_instance;

#if defined(UNCACHED_PLUGINS) && defined(DYNAMIC_MODULES)
		_instance = new PluginManagerUncached();
#else
		_instance = new PluginManagerCached();
#endif
	return *_instance;
}

PluginManager::PluginManager() {
	// Always add the static plugin provider.
	addPluginProvider(new StaticPluginProvider());
}

PluginManager::~PluginManager() {
	// Explicitly unload all loaded plugins
	unloadAllPlugins();

	// Delete the plugin providers
	for (ProviderList::iterator pp = _providers.begin();
	                            pp != _providers.end();
	                            ++pp) {
		delete *pp;
	}
}

void PluginManager::addPluginProvider(PluginProvider *pp) {
	_providers.push_back(pp);

	PluginList pl=pp->getPlugins();
	for (PluginList::const_iterator it = pl.begin(); it != pl.end();++it) {
		Plugin *p = *it;
		if (!p->isLoaded())
			continue;
		_loadedPluginsByType[p->getType()].push_back(p);
	}
}

void PluginManager::removePluginProvider(PluginProvider *pp) {
	PluginList pl = pp->getPlugins();
	for (PluginList::const_iterator it = pl.begin(); it != pl.end(); ++it) {
		Plugin *p = *it;

		if (!p->isLoaded())
			continue;

		removePluginFromInMemList(p);
	}

	for (ProviderList::iterator it = _providers.begin(); it != _providers.end();) {
		if (*it == pp) {
			it = _providers.erase(it);
			break;
		} else {
			++it;
		}
	}
}

/**
 * This should only be called once by main()
 **/
void PluginManagerUncached::init() {
	ConfMan.setBool("always_run_fallback_detection_extern", false);

	unloadPluginsExcept(PLUGIN_TYPE_ENGINE, NULL, false); // empty the engine plugins

#ifndef DETECTION_STATIC
	Common::String detectPluginName = "detection";
#ifdef PLUGIN_SUFFIX
	detectPluginName += PLUGIN_SUFFIX;
#endif

	_detectionPlugin = getPluginByFileName(detectPluginName);
#endif
}

#ifndef DETECTION_STATIC
void PluginManagerUncached::loadDetectionPlugin() {
	tryLoadPlugin(_detectionPlugin);
}

void PluginManagerUncached::unloadDetectionPlugin() {
	unloadPlugin(_detectionPlugin);
}
#endif

void PluginManager::unloadAllPlugins() {
	for (int i = 0; i < PLUGIN_TYPE_MAX; i++)
		unloadPluginsExcept((PluginType)i, NULL);
}

Plugin *PluginManager::getPluginByFileName(Common::String fileName) const {

	for (PluginManager::PluginIterator p; !p.atEnd(); ++p) {
		Common::String filename = (*p)->getFileName();
		if (filename.hasSuffixIgnoreCase(fileName)) {
			return *p;
		}
	}
	return NULL;
}

void PluginManager::unloadPluginsExcept(PluginType type, const Plugin *plugin, bool deletePlugin /*=true*/) {
	Plugin *found = NULL;

	if (plugin) {
		assert(plugin->getType() == type);
	}

	// copy the list since unloadPlugin modifies the main one
	PluginList pl = _loadedPluginsByType[type];
	for (PluginList::iterator p = pl.begin(); p != pl.end(); ++p) {
		if (*p == plugin) {
			found = *p;
		} else {
			unloadPlugin(*p);
			if (deletePlugin) {
				delete *p;
			}
		}
	}
}

/*
 * Used only by the cached plugin manager since it deletes the plugin.
 */
bool PluginManager::tryLoadPlugin(Plugin *plugin) {
	assert(plugin);

	if (plugin->isLoaded())
		return true;

	// Try to load the plugin
	if (plugin->loadPlugin()) {
		addToPluginsInMemList(plugin);
		return true;
	} else {
		// Failed to load the plugin
		delete plugin;
		return false;
	}
}

void PluginManager::unloadPlugin(Plugin *plugin) {
	assert(plugin);

	if (!plugin->isLoaded())
		return;

	removePluginFromInMemList(plugin);

	plugin->unloadPlugin();
}

/**
 * Add to the list of plugins loaded in memory.
 */
void PluginManager::addToPluginsInMemList(Plugin *plugin) {
	bool found = false;
	// The plugin is valid, see if it provides the same module as an
	// already loaded one and should replace it.

	PluginType type = plugin->getType();

	PluginList::iterator pl = _loadedPluginsByType[type].begin();
	while (!found && pl != _loadedPluginsByType[type].end()) {
		if (!strcmp(plugin->getName(), (*pl)->getName())) {
			// Found a duplicated module. Replace the old one.
			found = true;
			unloadPlugin(*pl);
			delete *pl;
			*pl = plugin;
			debug(1, "Replaced the duplicated plugin: '%s'", plugin->getName());
		}
		pl++;
	}

	if (!found) {
		// If it provides a new module, just add it to the list of known plugins in memory.
		_loadedPluginsByType[type].push_back(plugin);
	}

	if (type == PLUGIN_TYPE_COLLECTION) {
		PluginCollection *col = &plugin->get<PluginCollection>();
		addPluginProvider(col);
	}
}

void PluginManager::removePluginFromInMemList(Plugin *plugin) {
	PluginType type = plugin->getType();

	if (type == PLUGIN_TYPE_COLLECTION) {
		PluginCollection *col = &plugin->get<PluginCollection>();
		removePluginProvider(col);
	}

	PluginList &pl = _loadedPluginsByType[type];
	for (PluginList::iterator itr = pl.begin(); itr != pl.end();) {
		if (*itr == plugin)
			itr = pl.erase(itr);
		else
			++itr;
	}
}

void PluginManagerCached::init() {
	loadAllPlugins();
}
/**
 * Used by only the cached plugin manager. The uncached manager can only have
 * one plugin in memory at a time.
 **/
void PluginManagerCached::loadAllPlugins() {
	for (ProviderList::iterator pp = _providers.begin();
		 pp != _providers.end();
		 ++pp) {
		PluginList pl((*pp)->getPlugins());
		Common::for_each(pl.begin(), pl.end(), Common::bind1st(Common::mem_fun(&PluginManager::tryLoadPlugin), this));
	}
}

bool PluginManager::PluginIterator::next(bool acceptCurrent) {
	if (_currentProvider == PluginMan._providers.end())
		return false;
	for (;;) {

		for (;;) {
			if (_currentPlugin == _list.end())
				break;

			if (acceptCurrent && shouldStopAtCurrent())
				return true;

			acceptCurrent = true;
			++_currentPlugin;
		}

		++_currentProvider;
		if (_currentProvider == PluginMan._providers.end())
			break;
		_list = (*_currentProvider)->getPlugins();
		_currentPlugin = _list.begin();
	}
	return false;
}

bool PluginManager::PluginIterator::shouldStopAtCurrent() const {
	if (_allPlugins)
		return true;
	if (!(*_currentPlugin)->isLoaded())
		return false;
	return (*_currentPlugin)->getType() == _type;
}

PluginManager::PluginIterator::PluginIterator(PluginType type) : _type(type), _allPlugins(false) {
	_currentProvider = PluginMan._providers.begin();
	_list = (*_currentProvider)->getPlugins();
	_currentPlugin = _list.begin();
	next(true);
}

PluginManager::PluginIterator::PluginIterator() : _allPlugins(true) {
	_currentProvider = PluginMan._providers.begin();
	_list = (*_currentProvider)->getPlugins();
	_currentPlugin = _list.begin();
	next(true);
}

bool PluginManager::PluginIterator::operator++() {
	return next(false);
}

Plugin *PluginManager::PluginIterator::operator*() {
	assert(_currentPlugin != _list.end());
	return *_currentPlugin;
}

bool PluginManager::PluginIterator::atEnd() {
	return _currentPlugin == _list.end();
}

// Music plugins

#include "audio/musicplugin.h"

namespace Common {
DECLARE_SINGLETON(MusicManager);
}
// Scaler plugins

#include "graphics/scalerplugin.h"

namespace Common {
DECLARE_SINGLETON(ScalerManager);
}

const PluginList &ScalerManager::getLoadedPlugins() const {
	return PluginMan.getLoadedPluginsOfType(PLUGIN_TYPE_SCALER);
}

uint ScalerManager::getMaxExtraPixels() const {
	uint maxPixels = 0;
	PluginList plugins = getLoadedPlugins();
	PluginList::iterator i = plugins.begin();
	for (; i != plugins.end(); ++i) {
		uint n = (*i)->get<ScalerPluginObject>().extraPixels();
		if (n > maxPixels) {
			maxPixels = n;
		}
	}
	return maxPixels;
}

Plugin *ScalerManager::findScalerPlugin(const char *name) const {
	const PluginList &plugins = getLoadedPlugins();
	for (PluginList::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if (!strcmp((*i)->get<ScalerPluginObject>().getName(), name)) {
			return *i;
		}
	}

	return 0;
}

uint ScalerManager::findScalerPluginIndex(const char *name) const {
	const PluginList &plugins = getLoadedPlugins();
	uint index = 0;

	for (PluginList::const_iterator i = plugins.begin(); i != plugins.end(); ++i) {
		if (!strcmp((*i)->get<ScalerPluginObject>().getName(), name)) {
			return index;
		}
		index++;
	}

	return 0;
}

struct LegacyGraphicsMode {
	const char *oldName;
	const char *newName;
	uint factor;
};

// Table for using old names for scalers in the configuration
// to keep compatibiblity with old config files.
static const LegacyGraphicsMode s_legacyGraphicsModes[] = {
	{ "1x", "normal", 1 },
	{ "2x", "normal", 2 },
	{ "3x", "normal", 3 },
	{ "normal1x", "normal", 1 },
	{ "normal2x", "normal", 2 },
	{ "normal3x", "normal", 3 },
	{ "normal4x", "normal", 4 },
	{ "hq2x", "hq", 2 },
	{ "hq3x", "hq", 3 },
	{ "edge2x", "edge", 2 },
	{ "edge3x", "edge", 3 },
	{ "advmame2x", "advmame", 2 },
	{ "advmame3x", "advmame", 3 },
	{ "advmame4x", "advmame", 4 },
	{ "2xsai", "sai", 2 },
	{ "sai2x", "sai", 2 },
	{ "super2xsai", "supersai", 2 },
	{ "supersai2x", "supersai", 2 },
	{ "supereagle", "supereagle", 2 },
	{ "supereagle2x", "supereagle", 2 },
	{ "pm2x", "pm", 2 },
	{ "dotmatrix", "dotmatrix", 2 },
	{ "dotmatrix2x", "dotmatrix", 2 },
	{ "tv2x", "tv", 2 }
};

void ScalerManager::updateOldSettings() {
	// Search for legacy gfx_mode and replace it
	if (ConfMan.hasKey("gfx_mode")) {
		Common::String gfxMode(ConfMan.get("gfx_mode"));
		for (uint i = 0; i < ARRAYSIZE(s_legacyGraphicsModes); ++i) {
			if (gfxMode == s_legacyGraphicsModes[i].oldName) {
				ConfMan.set("scaler", s_legacyGraphicsModes[i].newName);
				ConfMan.setInt("scale_factor", s_legacyGraphicsModes[i].factor);
				break;
			}
		}
	}

	// Look in all game domains as well
	for (Common::ConfigManager::DomainMap::iterator domain = ConfMan.beginGameDomains(); domain != ConfMan.endGameDomains(); ++domain) {
		if (domain->_value.contains("gfx_mode")) {
			Common::String gfxMode(domain->_value.getVal("gfx_mode"));
			for (uint i = 0; i < ARRAYSIZE(s_legacyGraphicsModes); ++i) {
				if (gfxMode == s_legacyGraphicsModes[i].oldName) {
					warning("%s: %s -> %s@%dx", domain->_value.getDomainComment().c_str(), s_legacyGraphicsModes[i].oldName, s_legacyGraphicsModes[i].newName, s_legacyGraphicsModes[i].factor);
					domain->_value.setVal("scaler", s_legacyGraphicsModes[i].newName);
					domain->_value.setVal("scale_factor", Common::String::format("%i", s_legacyGraphicsModes[i].factor));
					domain->_value.erase("gfx_mode");
					break;
				}
			}
		}
	}
}
