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
	DWORD create = writeMode ? OPEN_ALWAYS : OPEN_EXISTING;
	HANDLE fileObjHandle=CreateFile(toUnicode(path.c_str()), access, share, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL);

	if(fileObjHandle==INVALID_HANDLE_VALUE) return nullptr;

	return new WindowsIoStream(fileObjHandle, writeMode);
}

WindowsIoStream::WindowsIoStream(HANDLE handle, bool writeMode) : fileObjHandle(handle) {
}

WindowsIoStream::~WindowsIoStream() {
	if (fileObjHandle != INVALID_HANDLE_VALUE) close();
}

void WindowsIoStream::close() {
	CloseHandle(fileObjHandle);
	fileObjHandle = INVALID_HANDLE_VALUE;
}

int32 WindowsIoStream::size() const {
	DWORD sizeHighBuf;
	DWORD sizeLow=GetFileSize(fileObjHandle, &sizeHighBuf);

	if (sizeLow == INVALID_FILE_SIZE) return -1;

	return sizeLow;
}

bool WindowsIoStream::eos() const {
	return pos()>=size();
}

uint32 WindowsIoStream::read(void* dataPtr, uint32 dataSize) {
	DWORD numRead;
	BOOL success = ReadFile(fileObjHandle, dataPtr, dataSize, &numRead, NULL);
	return numRead;
}

int32 WindowsIoStream::pos() const {
	LONG distanceHigh = 0;
	DWORD curPosHigh;
	DWORD curPosLow = SetFilePointer(fileObjHandle, 0, &distanceHigh, FILE_CURRENT);
	if (curPosLow == INVALID_SET_FILE_POINTER) return -1;

	return curPosLow;
}

bool WindowsIoStream::seek(int32 offset, int whence) {
	LONG distanceHigh = 0;
	DWORD moveMethod;

	switch (whence) {
	case SEEK_SET: moveMethod = FILE_BEGIN; break;
	case SEEK_CUR: moveMethod = FILE_CURRENT; break;
	case SEEK_END: moveMethod = FILE_END; break;
	}

	DWORD newPosLow=SetFilePointer(fileObjHandle, offset, &distanceHigh, moveMethod);
	if(newPosLow == INVALID_SET_FILE_POINTER) return false;

	return true;
}

uint32 WindowsIoStream::write(const void* dataPtr, uint32 dataSize) {
	DWORD numWritten;
	BOOL success=WriteFile(fileObjHandle, dataPtr, dataSize, &numWritten, NULL);
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
