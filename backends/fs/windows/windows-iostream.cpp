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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/windows/windows-iostream.h"

WindowsIoStream* WindowsIoStream::makeFromPath(const Common::String& path, bool writeMode) {
	DWORD access = writeMode ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ;
	DWORD share = writeMode ? FILE_SHARE_WRITE : FILE_SHARE_READ;
	DWORD create = writeMode ? CREATE_ALWAYS : OPEN_EXISTING;
	HANDLE fileObjHandle=CreateFile(toUnicode(path.c_str()), access, share, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL);

	if(fileObjHandle==INVALID_HANDLE_VALUE) return nullptr;

	return new WindowsIoStream(fileObjHandle, writeMode);
}

WindowsIoStream::WindowsIoStream(HANDLE handle, bool writeMode) : fileObjHandle(handle), error(false), eof(false) {
}

WindowsIoStream::~WindowsIoStream() {
	if (fileObjHandle != INVALID_HANDLE_VALUE) close();
}

void WindowsIoStream::close() {
	CloseHandle(fileObjHandle);
	fileObjHandle = INVALID_HANDLE_VALUE;
}

bool WindowsIoStream::err() const { return error; }

void WindowsIoStream::clearErr() { error = false; eof = false; }

int32 WindowsIoStream::size() const {
	DWORD sizeHighBuf;
	DWORD sizeLow=GetFileSize(fileObjHandle, &sizeHighBuf);

	if (sizeLow == INVALID_FILE_SIZE) {
		return -1;
	}

	return sizeLow;
}

bool WindowsIoStream::eos() const {
	return eof;
}

uint32 WindowsIoStream::read(void* dataPtr, uint32 dataSize) {
	DWORD numRead;
	BOOL success = ReadFile(fileObjHandle, dataPtr, dataSize, &numRead, NULL);
	if (!success) {
		error = true;
	} else if (numRead == 0) {
		eof = true;
	}
	return numRead;
}

int32 WindowsIoStream::pos() const {
	LARGE_INTEGER distance;
	distance.QuadPart = 0;
	BOOL success = SetFilePointerEx(fileObjHandle, distance, &distance, FILE_CURRENT);
	if (!success) {
		return -1;
	}

	return distance.QuadPart;
}

bool WindowsIoStream::seek(int32 offset, int whence) {
	LARGE_INTEGER distance;
	DWORD moveMethod;

	switch (whence) {
	case SEEK_SET: moveMethod = FILE_BEGIN; break;
	case SEEK_CUR: moveMethod = FILE_CURRENT; break;
	case SEEK_END: moveMethod = FILE_END; break;
	default: return false;
	}

	distance.QuadPart = offset;

	BOOL success=SetFilePointerEx(fileObjHandle, distance, NULL, moveMethod);

	if (!success) {
		error = true;
		return false;
	}

	eof = false;

	return true;
}

uint32 WindowsIoStream::write(const void* dataPtr, uint32 dataSize) {
	DWORD numWritten;
	BOOL success=WriteFile(fileObjHandle, dataPtr, dataSize, &numWritten, NULL);
	error = !success;
	return numWritten;
}

bool WindowsIoStream::flush() {
	BOOL success=FlushFileBuffers(fileObjHandle);
	return success;
}

void WindowsIoStream::finalize() {
	flush();
	close();
}
