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

#ifndef BACKENDS_FS_WINDOWS_WINDOWSIOSTREAM_H
#define BACKENDS_FS_WINDOWS_WINDOWSIOSTREAM_H

#include "backends/fs/windows/windows-fs.h"

#include "common/scummsys.h"
#include "common/noncopyable.h"
#include "common/stream.h"
#include "common/str.h"

#include <windows.h>

class WindowsIoStream : public Common::SeekableReadStream, public Common::SeekableWriteStream, public Common::NonCopyable {
public:
	static WindowsIoStream* makeFromPath(const Common::String& path, bool writeMode);
	WindowsIoStream(HANDLE handle, bool writeMode);
	~WindowsIoStream();

	void close();

	int32 size() const override;

	// Inherited via SeekableReadStream
	virtual bool eos() const override;
	virtual uint32 read(void* dataPtr, uint32 dataSize) override;
	virtual int32 pos() const override;
	virtual bool seek(int32 offset, int whence = SEEK_SET) override;

	// Inherited via SeekableWriteStream
	virtual uint32 write(const void* dataPtr, uint32 dataSize) override;
	virtual bool flush() override;
	virtual void finalize() override;

private:
	HANDLE fileObjHandle;

};


#endif