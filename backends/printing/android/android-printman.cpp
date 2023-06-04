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
 
#include "backends/printing/printman.h"

#if defined(__ANDROID__)
#ifdef USE_PRINTING

#include "common/system.h"

#include "android-printman.h"
#include "common/ustr.h"

class AndroidPrintJob;
class AndroidPrintSettings;

class AndroidPrintingManager : public PrintingManager {
public:
	virtual ~AndroidPrintingManager();
	
	PrintJob *createJob(const Common::String &jobName, PrintSettings *settings);

	PrintSettings *getDefaultPrintSettings() const;
};

class AndroidPrintJob : public PrintJob {
public:
	friend class AndroidPrintingManager;

	AndroidPrintJob(const Common::String &jobName, AndroidPrintSettings *settings);
	~AndroidPrintJob();

	void drawBitmap(const Graphics::ManagedSurface &surf, Common::Point pos);
	void drawBitmap(const Graphics::ManagedSurface &surf, Common::Rect posAndSize);
	void drawText(const Common::String &text, Common::Point pos);

	void setTextColor(int r, int g, int b);
	Common::Rect getTextBounds(const Common::String &text) const;
	TextMetrics getTextMetrics();

	Common::Rational getPixelAspectRatio() const;
	Common::Rect getPrintableArea() const;
	Common::Point getPrintableAreaOffset() const;
	Common::Rect getPaperDimensions() const;

	const PrintSettings *getPrintSettings() const {
		return (const PrintSettings *)(settings);
	}

	void beginPage();
	void endPage();
	void endDoc();
	void abortJob();

private:
	AndroidPrintSettings *settings;

	friend class AndroidPrintSettings;
};

class AndroidPrintSettings : public PrintSettings {
public:
	AndroidPrintSettings() {};
	~AndroidPrintSettings() {
	}

	
	DuplexMode getDuplexMode() const;
	void setDuplexMode(DuplexMode mode);
	bool getLandscapeOrientation() const;
	void setLandscapeOrientation(bool);
	bool getColorPrinting() const;
	void setColorPrinting(bool);

private:
};

AndroidPrintingManager::~AndroidPrintingManager() {}

PrintJob *AndroidPrintingManager::createJob(const Common::String &jobName, PrintSettings *settings) {
	return new AndroidPrintJob(jobName, (AndroidPrintSettings*)settings);
}


AndroidPrintJob::AndroidPrintJob(const Common::String &jobName, AndroidPrintSettings *settings) : settings(settings) {
}

AndroidPrintJob::~AndroidPrintJob() {
	delete settings;
}

void AndroidPrintJob::drawBitmap(const Graphics::ManagedSurface &surf, Common::Point pos) {
}

void AndroidPrintJob::drawBitmap(const Graphics::ManagedSurface &surf, Common::Rect posAndSize) {
}

void AndroidPrintJob::drawText(const Common::String &text, Common::Point pos) {
}

void AndroidPrintJob::setTextColor(int r, int g, int b) {
}

Common::Rect AndroidPrintJob::getTextBounds(const Common::String &text) const {
	return Common::Rect(0, 0);
}

TextMetrics AndroidPrintJob::getTextMetrics() {
	TextMetrics metrics;

	return metrics;
}

Common::Rational AndroidPrintJob::getPixelAspectRatio() const {
	return Common::Rational(
		0,
		0
	);
}

Common::Rect AndroidPrintJob::getPrintableArea() const {
	return Common::Rect(
		0,
		0
	);
}

Common::Point AndroidPrintJob::getPrintableAreaOffset() const {
	return Common::Point(
		0,
		0
	);
}

Common::Rect AndroidPrintJob::getPaperDimensions() const {
	return Common::Rect(
		0,
		0
	);
}

void AndroidPrintJob::beginPage() {
}

void AndroidPrintJob::endPage() {
}

void AndroidPrintJob::endDoc() {
}

void AndroidPrintJob::abortJob() {
}

PrintSettings *AndroidPrintingManager::getDefaultPrintSettings() const {
	return new AndroidPrintSettings();
}

PrintingManager *createAndroidPrintingManager() {
	return new AndroidPrintingManager();
}

PrintSettings::DuplexMode AndroidPrintSettings::getDuplexMode() const {}

void AndroidPrintSettings::setDuplexMode(PrintSettings::DuplexMode mode) {
}

bool AndroidPrintSettings::getLandscapeOrientation() const {
	return false;
}

void AndroidPrintSettings::setLandscapeOrientation(bool landscapeOrientation) {
}

bool AndroidPrintSettings::getColorPrinting() const {
	return true;
}

void AndroidPrintSettings::setColorPrinting(bool colorPrinting) {
}


#endif // USE_PRINTING
#endif // ANDROID
