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

#ifndef STARK_SERVICES_SERVICES_H
#define STARK_SERVICES_SERVICES_H

#include "engines/stark/stark.h"

namespace Common {
class RandomSource;
}

namespace Stark {

namespace Gfx {
class Driver;
}

class ArchiveLoader;
class DialogPlayer;
class Diary;
class FontProvider;
class GameInterface;
class Global;
class ResourceProvider;
class StaticProvider;
class Scene;
class UserInterface;
class Settings;
class StateProvider;
class GameChapter;
class GameMessage;

/** Shortcuts for accessing the services. */
#define StarkArchiveLoader      ((StarkEngine *)g_engine)->archiveLoader
#define StarkDialogPlayer       ((StarkEngine *)g_engine)->dialogPlayer
#define StarkDiary              ((StarkEngine *)g_engine)->diary
#define StarkGfx                ((StarkEngine *)g_engine)->gfx
#define StarkGlobal             ((StarkEngine *)g_engine)->global
#define StarkResourceProvider   ((StarkEngine *)g_engine)->resourceProvider
#define StarkRandomSource       ((StarkEngine *)g_engine)->randomSource
#define StarkScene              ((StarkEngine *)g_engine)->scene
#define StarkStaticProvider     ((StarkEngine *)g_engine)->staticProvider
#define StarkGameInterface      ((StarkEngine *)g_engine)->gameInterface
#define StarkUserInterface      ((StarkEngine *)g_engine)->userInterface
#define StarkFontProvider       ((StarkEngine *)g_engine)->fontProvider
#define StarkSettings           ((StarkEngine *)g_engine)->settings
#define StarkGameChapter        ((StarkEngine *)g_engine)->gameChapter
#define StarkGameMessage        ((StarkEngine *)g_engine)->gameMessage
#define StarkStateProvider      ((StarkEngine *)g_engine)->stateProvider

} // End of namespace Stark

#endif // STARK_SERVICES_SERVICES_H
