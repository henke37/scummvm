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

#include "base/plugins.h"

#include "engines/advancedDetector.h"

#include "common/gui_options.h"
#include "common/file.h"
#include "common/translation.h"

#include "capbible/detection.h"

static const PlainGameDescriptor capbibleGames[] = {
	{"domeofdarkness", "Captain Bible in Dome of Darkness"},
	{nullptr, nullptr}
};

namespace CapBible {

static const CapBibleGameDescription gameDescriptions[] = {
	// English
	{
		{
			"domeofdarkness",
			"",
			AD_ENTRY2s("cb.exe", "64e43d07e24e103d126c6b7c012fcc10", 64299, "dd1.dat", "ada87cd9a3b0d792fc50339e8e6c3459", 1866068),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIPCSPK, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM)
		},
	},
	{
		{
			"domeofdarkness",
			"Special Edition",
			AD_ENTRY2s("cbse.exe", "3c37e1c44f318385c81cffeda24fac53", 64251, "cbse.dat", "ecfebe47b7a901d3b557cf3a575cfd57", 738241),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO5(GUIO_NOSPEECH, GUIO_MIDIPCSPK, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM)
		},
	},
	{ AD_TABLE_END_MARKER }
};

} // End of namespace CapBible

class CapBibleMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	CapBibleMetaEngineDetection() : AdvancedMetaEngineDetection(CapBible::gameDescriptions, sizeof(CapBible::CapBibleGameDescription), capbibleGames) {
	}

	const char *getEngineId() const override {
		return "capbible";
	}

	const char *getName() const override {
		return "Captain Bible";
	}

	const char *getOriginalCopyright() const override {
		return "Captain Bible in Dome of Darkness (C) Bridgestone Multimedia Group";
	}

};

REGISTER_PLUGIN_STATIC(CAPBIBLE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, CapBibleMetaEngineDetection);
