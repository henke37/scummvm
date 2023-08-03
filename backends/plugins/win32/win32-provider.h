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

#ifndef BACKENDS_PLUGINS_WIN32_H
#define BACKENDS_PLUGINS_WIN32_H

#include "base/plugins.h"

#if defined(DYNAMIC_MODULES) && defined(_WIN32)

#include "backends/plugins/dynamic-plugin.h"

class Win32PluginProvider final : public FilePluginProvider {
protected:
	Plugin* createPlugin(const Common::FSNode &node) const override;

	bool isPluginFilename(const Common::FSNode &node) const override;
};

class Win32Plugin final : public DynamicPlugin {
protected:
	void *_dlHandle;

	VoidFunc findSymbol(const char *symbol) override;

public:
	Win32Plugin(const Common::String &filename)
		: DynamicPlugin(filename), _dlHandle(0) {}

	bool loadPlugin() override;

	void unloadPlugin() override;

	Win32::Win32ResourceArchive *_arch;
};

#endif // defined(DYNAMIC_MODULES) && defined(_WIN32)

#endif
