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

#include "common/file.h"

#include "capbible/art.h"

namespace CapBible {

struct ArtFrameRecord {
	int16 xOffset;
	int16 yOffset;
	uint16 width;
	uint16 height;
	uint32 dataOffset;
};

Art::Art(const Common::Path &path) {
	Common::File file;
	file.open(path);
	assert(file.isOpen());

	Common::Array<ArtFrameRecord> frameRecords;

	// read frame records
	do {
		ArtFrameRecord frameRecord;

		frameRecord.xOffset = file.readSint16LE();
		frameRecord.yOffset = file.readSint16LE();
		frameRecord.width = file.readUint16LE();
		frameRecord.height = file.readUint16LE();

		frameRecord.dataOffset = file.readUint32LE();

		frameRecords.push_back(frameRecord);

	} while (file.pos() < frameRecords[0].dataOffset);

	_frames.reserve(frameRecords.size());

	// load the frames
	for (uint32 i = 0; i < frameRecords.size(); ++i) {
		const ArtFrameRecord &frameRecord = frameRecords[i];
		Frame frame(frameRecord.xOffset, frameRecord.yOffset);

		frame.surface.create(frameRecord.width, frameRecord.height, Graphics::PixelFormat::createFormatCLUT8());
		file.seek(frameRecord.dataOffset);
		file.read(frame.surface.getPixels(), frameRecord.width * frameRecord.height);
		_frames.push_back(std::move(frame));
	}
}

Art::Frame::Frame(int16 xOffset, int16 yOffset) : xOffset(xOffset), yOffset(yOffset) {
}

} // namespace CapBible

