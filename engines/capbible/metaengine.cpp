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

#include "engines/advancedDetector.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "capbible/capbible.h"
#include "capbible/detection.h"

namespace CapBible {
static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_NOMATURE,
		{
			_s("No Mature Content"),
			_s("Disables mature content"),
			 "no_mature",
			 false,
			 0,
			 0
		 }
	},
	{
		GAMEOPTION_NOCOMBAT,
		{
			_s("No Combat"),
			_s("Disables combat and other forms of faith loss"),
			 "no_combat",
			 false,
			 0,
			 0
		 }
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};
} // End of namespace CapBible

class CapBibleMetaEngine : public AdvancedMetaEngine <ADGameDescription> {
public:
	const char *getName() const override {
		return "capbible";
	}

	bool hasFeature(MetaEngineFeature f) const override;
	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return CapBible::optionsList;
	}
};

bool CapBibleMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSimpleSavesNames) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSavesUseExtendedFormat );
}

Common::Error CapBibleMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new CapBible::CapBibleEngine(syst, desc);
	return Common::kNoError;
}

#if PLUGIN_ENABLED_DYNAMIC(CAPBIBLE)
	REGISTER_PLUGIN_DYNAMIC(CAPBIBLE, PLUGIN_TYPE_ENGINE, CapBibleMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(CAPBIBLE, PLUGIN_TYPE_ENGINE, CapBibleMetaEngine);
#endif

