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

#ifndef CAPBIBLE_ART_H
#define CAPBIBLE_ART_H

#include "common/noncopyable.h"
#include "common/array.h"
#include "common/path.h"
#include "graphics/managed_surface.h"

namespace CapBible {
class Art {
public:
	Art(const Common::Path &path);
	~Art() = default;

	class Frame {
	private:
		Frame(int16 xOffset, int16 yOffset);
		friend class Art;
	public:
		Frame(const Frame &frame) = delete;
		Frame(Frame &&frame) = default;
		~Frame() = default;

		Frame &operator=(const Frame & frame) = delete;
		Frame &operator=(Frame && frame) = default;

		int16 xOffset;
		int16 yOffset;
		Graphics::ManagedSurface surface;
	};

private:
	Common::Array<Frame> _frames;
};

} // namespace CapBible

#endif // CAPBIBLE_ART_H
