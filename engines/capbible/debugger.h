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

#ifndef CAPBIBLE_DEBUGGER_H
#define CAPBIBLE_DEBUGGER_H

#include "capbible/capbible.h"
#include "gui/debugger.h"

namespace Common {
class SeekableReadStream;
}

namespace CapBible {

class Debugger : public GUI::Debugger {
public:
	Debugger(CapBibleEngine *eng);

private:
	CapBibleEngine *_engine;

	bool cmdDumpMainArch(int argc, const char **argv);
	bool cmdGiveItem(int argc, const char **argv);
};
} // End of namespace CapBible

#endif
