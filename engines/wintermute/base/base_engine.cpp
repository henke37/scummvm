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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/system/sys_class_registry.h"
#include "common/system.h"

namespace Wintermute {

BaseEngine::BaseEngine() {
	_fileManager = nullptr;
	_gameRef = nullptr;
	_classReg = nullptr;
	_rnd = nullptr;
	_gameId = "";
	_language = Common::UNK_LANG;
	_targetExecutable = LATEST_VERSION;
	_flags = 0;
}

BaseEngine::~BaseEngine() {
	delete _fileManager;
	delete _rnd;
	delete _classReg;
}

void BaseEngine::createInstance(const Common::String &targetName, const Common::String &gameId, Common::Language lang, WMETargetExecutable targetExecutable, uint32 flags) {
	_targetName = targetName;
	_gameId = gameId;
	_language = lang;
	_targetExecutable = targetExecutable;
	_flags = flags;

	_fileManager = new BaseFileManager(_language, false, flags);
	// Don't forget to register your random source
	_rnd = new Common::RandomSource("Wintermute");
	_classReg = new SystemClassRegistry();
	_classReg->registerClasses();
}

void BaseEngine::LOG(bool res, const char *fmt, ...) {
	uint32 secs = g_system->getMillis() / 1000;
	uint32 hours = secs / 3600;
	secs = secs % 3600;
	uint32 mins = secs / 60;
	secs = secs % 60;

	char buff[512];
	va_list va;

	va_start(va, fmt);
	Common::vsprintf_s(buff, fmt, va);
	va_end(va);

	if (WinterBaseEngine->_gameRef) {
		WinterBaseEngine->_gameRef->LOG(res, "%s", buff);
	} else {
		debugCN(kWintermuteDebugLog, "%02d:%02d:%02d: %s\n", hours, mins, secs, buff);
	}
}

uint32 BaseEngine::randInt(int from, int to) {
	return _rnd->getRandomNumberRng(from, to);
}

BaseSoundMgr *BaseEngine::getSoundMgr() {
	if (WinterBaseEngine->_gameRef) {
		return _gameRef->_soundMgr;
	} else {
		return nullptr;
	}
}

BaseRenderer *BaseEngine::getRenderer() {
	if (WinterBaseEngine->_gameRef) {
		return WinterBaseEngine->_gameRef->_renderer;
	} else {
		return nullptr;
	}
}

const Timer *BaseEngine::getTimer() {
	if (WinterBaseEngine->_gameRef) {
		return WinterBaseEngine->_gameRef->getTimer();
	} else {
		return nullptr;
	}
}

const Timer *BaseEngine::getLiveTimer() {
	if (WinterBaseEngine->_gameRef) {
		return WinterBaseEngine->_gameRef->getLiveTimer();
	} else {
		return nullptr;
	}
}

} // End of namespace Wintermute
