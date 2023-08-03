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
 
// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#include "backends/platform/sdl/win32/win32_wrapper.h"
#include "backends/plugins/win32/win32-provider.h"
#include "backends/plugins/dynamic-plugin.h"
#include "common/debug.h"
#include "common/fs.h"

DynamicPlugin::VoidFunc Win32Plugin::findSymbol(const char *symbol) {
	FARPROC func = GetProcAddress(_dlHandle, symbol);
	if (!func)
		debug("Failed loading symbol '%s' from plugin '%s'", symbol, _filename.c_str());

	return (DynamicPlugin::VoidFunc)func;
}

bool Win32Plugin::loadPlugin() {
	assert(!_dlHandle);
	TCHAR *tFilename = Win32::stringToTchar(_filename);
	_dlHandle = LoadLibrary(tFilename);
	free(tFilename);

	if (!_dlHandle) {
		debug("Failed loading plugin '%s' (error code %d)", _filename.c_str(), (int32) GetLastError());
		return false;
	} else {
		debug(1, "Success loading plugin '%s', handle %p", _filename.c_str(), _dlHandle);
	}

	_arch = new Win32::Win32ResourceArchive(_dlHandle);

	SearchMan.add(_filename, _arch, -1, false);

	return DynamicPlugin::loadPlugin();
}


void Win32Plugin::unloadPlugin() {
	SearchMan.remove(_filename);

	delete _arch;
	_arch = nullptr;

	DynamicPlugin::unloadPlugin();
	if (_dlHandle) {
		if (!FreeLibrary((HMODULE)_dlHandle))
			warning("Failed unloading plugin '%s'", _filename.c_str());
		else
			debug(1, "Success unloading plugin '%s'", _filename.c_str());
		_dlHandle = 0;
	}
}


Plugin* Win32PluginProvider::createPlugin(const Common::FSNode &node) const {
	return new Win32Plugin(node.getPath());
}

bool Win32PluginProvider::isPluginFilename(const Common::FSNode &node) const {
	// Check the plugin suffix
	Common::String filename = node.getName();
	return filename.hasSuffix(".dll");
}


#endif // defined(DYNAMIC_MODULES) && defined(_WIN32)
