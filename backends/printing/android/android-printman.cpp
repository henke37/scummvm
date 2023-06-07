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
#define FORBIDDEN_SYMBOL_ALLOW_ALL
 
#include "backends/printing/printman.h"

#if defined(__ANDROID__)
#ifdef USE_PRINTING

#include "common/system.h"

#include "android-printman.h"
#include "common/ustr.h"


#include <android/bitmap.h>

#include "backends/platform/android/jni-android.h"

#include "common/error.h"
#include "graphics/managed_surface.h"

class AndroidPrintJob;
class AndroidPrintSettings;

class AndroidPrintingManager : public PrintingManager {
public:
	AndroidPrintingManager();
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
	jobject jobObj;

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
	
	jobject toManaged() const;
	
private:
	int duplexMode=0;
	int colorMode=0;
};

jobject surf2Bitmap(const Graphics::ManagedSurface &surf);

void initJNI();

AndroidPrintingManager::AndroidPrintingManager() {
	initJNI();
}

AndroidPrintingManager::~AndroidPrintingManager() {}

jmethodID MID_bitmap_createBitmap;
jmethodID MID_printAttsBuilder_ctor;
jmethodID MID_printAttsBuilder_build;
jmethodID MID_printAttsBuilder_setDuplexMode;
jmethodID MID_printAttsBuilder_setColorMode;

PrintJob *AndroidPrintingManager::createJob(const Common::String &jobName, PrintSettings *settings) {
	return new AndroidPrintJob(jobName, (AndroidPrintSettings*)settings);
}


AndroidPrintJob::AndroidPrintJob(const Common::String &jobName, AndroidPrintSettings *settings) : settings(settings) {
	JNIEnv *env = JNI::getEnv();
	
	jobject printSettingsObj = settings->toManaged();
	
	jobObj = JNI::startPrintJob(jobName, printSettingsObj);
	
	env->DeleteLocalRef(printSettingsObj);
}

AndroidPrintJob::~AndroidPrintJob() {
	JNIEnv *env = JNI::getEnv();

	delete settings;
	
	env->DeleteLocalRef(jobObj);
}

void AndroidPrintJob::drawBitmap(const Graphics::ManagedSurface &surf, Common::Point pos) {
	JNIEnv *env = JNI::getEnv();
	
	jobject bitmapObj = surf2Bitmap(surf);
	
	env->DeleteLocalRef(bitmapObj);
}

void AndroidPrintJob::drawBitmap(const Graphics::ManagedSurface &surf, Common::Rect posAndSize) {
	JNIEnv *env = JNI::getEnv();
	
	jobject bitmapObj = surf2Bitmap(surf);
	
	env->DeleteLocalRef(bitmapObj);
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

PrintSettings::DuplexMode AndroidPrintSettings::getDuplexMode() const {
	switch(duplexMode) {
		case 1:
			return PrintSettings::DuplexMode::Simplex;
		case 2:
			return PrintSettings::DuplexMode::Vertical;
		case 4:
			return PrintSettings::DuplexMode::Horizontal;
		default:
			return PrintSettings::DuplexMode::Unknown;
	}
}

void AndroidPrintSettings::setDuplexMode(PrintSettings::DuplexMode mode) {
	switch(mode) {
		case PrintSettings::DuplexMode::Simplex:
			duplexMode = 1;
		break;		
		case PrintSettings::DuplexMode::Vertical:
			duplexMode = 2;
		break;
		case PrintSettings::DuplexMode::Horizontal:
			duplexMode = 4;
		break;
		default:
			duplexMode = 0;
		break;
	}
}

bool AndroidPrintSettings::getLandscapeOrientation() const {
	return false;
}

void AndroidPrintSettings::setLandscapeOrientation(bool landscapeOrientation) {
}

bool AndroidPrintSettings::getColorPrinting() const {
	return colorMode != 1;
}

void AndroidPrintSettings::setColorPrinting(bool colorPrinting) {
	colorMode = colorPrinting ? 2 : 1;
}

jobject AndroidPrintSettings::toManaged() const {
	JNIEnv *env = JNI::getEnv();
	
	jclass printAttsBuilderClazz = env->FindClass("android/print/PrintAttributes$Builder");
	if(!printAttsBuilderClazz) {
		error("Failed to FindClass(PrintAttributes$Builder)");
	}
	
	jobject builderObj=env->NewObject(printAttsBuilderClazz, MID_printAttsBuilder_ctor);
	
	jobject junk = env->CallObjectMethod(builderObj, MID_printAttsBuilder_setColorMode, colorMode);
	env->DeleteLocalRef(junk);
	junk = env->CallObjectMethod(builderObj, MID_printAttsBuilder_setDuplexMode, duplexMode);
	env->DeleteLocalRef(junk);
	
	jobject attsObj = env->CallObjectMethod(builderObj, MID_printAttsBuilder_build);
	
	env->DeleteLocalRef(printAttsBuilderClazz);
	
	return attsObj;
}

jobject surf2Bitmap(const Graphics::ManagedSurface &srcSurf) {
	JNIEnv *env = JNI::getEnv();
	
	jclass bitmapClazz = env->FindClass("android/graphics/Bitmap");
	if(!bitmapClazz) {
		error("Failed to FindClass(Bitmap)");
		return nullptr;
	}
	
	jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID FID_ARGB888 = env->GetStaticFieldID(bitmapConfigClass, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");  
	if(!FID_ARGB888) {
		error("Failed to GetStaticFieldID(ARGB_8888)");
		return nullptr;
	}
	jobject argb8888Config = env->GetStaticObjectField(bitmapConfigClass, FID_ARGB888);
	if(!argb8888Config) {
		error("Failed to GetStaticObjectField(ARGB_8888)");
		return nullptr;
	}
	env->DeleteLocalRef(bitmapConfigClass);
	
	jobject bitmapObj = env->CallStaticObjectMethod(
		bitmapClazz,
		MID_bitmap_createBitmap,
		srcSurf.w,
		srcSurf.h,
		argb8888Config
	);
	
	env->DeleteLocalRef(argb8888Config);
	env->DeleteLocalRef(bitmapClazz);
	
	if(!bitmapObj) {
		error("Failed to create Bitmap object");
		return nullptr;
	}

	AndroidBitmapInfo bitmap_info;
	if (AndroidBitmap_getInfo(env, bitmapObj, &bitmap_info) != ANDROID_BITMAP_RESULT_SUCCESS) {
		error("Error reading bitmap info");
		env->DeleteLocalRef(bitmapObj);
		return nullptr;
	}
	
	Graphics::PixelFormat dstFmt;
	switch(bitmap_info.format) {
		case ANDROID_BITMAP_FORMAT_RGBA_8888:
#ifdef SCUMM_BIG_ENDIAN
			dstFmt = Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0);
#else
			dstFmt = Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24);
#endif
			break;
		case ANDROID_BITMAP_FORMAT_RGBA_4444:
			dstFmt = Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0);
			break;
		case ANDROID_BITMAP_FORMAT_RGB_565:
			dstFmt = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
			break;
		default:
			error("Bitmap has unsupported format");
			env->DeleteLocalRef(bitmapObj);
			return nullptr;
	}
	
	{
		Graphics::ManagedSurface dstSurf = Graphics::ManagedSurface();

		void *dst_pixels = nullptr;
		if (AndroidBitmap_lockPixels(env, bitmapObj, &dst_pixels) != ANDROID_BITMAP_RESULT_SUCCESS) {
			error("Error locking bitmap pixels");
			env->DeleteLocalRef(bitmapObj);
			return nullptr;
		}
		dstSurf.w=bitmap_info.width;
		dstSurf.h=bitmap_info.height;
		dstSurf.pitch=bitmap_info.stride;
		dstSurf.format=dstFmt;
		dstSurf.setPixels(dst_pixels);
		
		dstSurf.blitFrom(srcSurf);

		AndroidBitmap_unlockPixels(env, bitmapObj);
	}
	
	return bitmapObj;
}

void initJNI() {
	JNIEnv *env = JNI::getEnv();
	
	jclass bitmapClazz = env->FindClass("android/graphics/Bitmap");
	if(!bitmapClazz) {
		error("Failed to FindClass(Bitmap)");
	}
	
	MID_bitmap_createBitmap = env->GetStaticMethodID(bitmapClazz,
		"createBitmap",
		"(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;"
	);	
	if(!MID_bitmap_createBitmap) {
		error("Failed to GetStaticMethodId(createBitmap)");
	}
	
	env->DeleteLocalRef(bitmapClazz);
	
	jclass printAttsClazz = env->FindClass("android/print/PrintAttributes");
	if(!printAttsClazz) {
		error("Failed to FindClass(PrintAttributes)");
	}
	
	env->DeleteLocalRef(printAttsClazz);
	
	jclass printAttsBuilderClazz = env->FindClass("android/print/PrintAttributes$Builder");
	if(!printAttsBuilderClazz) {
		error("Failed to FindClass(PrintAttributes$Builder)");
	}
	
	MID_printAttsBuilder_ctor = env->GetMethodID(
		printAttsBuilderClazz, "<init>", 
		"()V"
	);	
	if(!MID_printAttsBuilder_ctor) {
		error("Failed to GetMethodId(ctor)");
	}
	
	MID_printAttsBuilder_build = env->GetMethodID(
		printAttsBuilderClazz, "build", 
		"()Landroid/print/PrintAttributes;"
	);	
	if(!MID_printAttsBuilder_build) {
		error("Failed to GetMethodId(build)");
	}
	
	MID_printAttsBuilder_setColorMode = env->GetMethodID(
		printAttsBuilderClazz, "setColorMode", 
		"(I)Landroid/print/PrintAttributes$Builder;"
	);	
	if(!MID_printAttsBuilder_setColorMode) {
		error("Failed to GetMethodId(setColorMode)");
	}
	
	MID_printAttsBuilder_setDuplexMode = env->GetMethodID(
		printAttsBuilderClazz, "setDuplexMode", 
		"(I)Landroid/print/PrintAttributes$Builder;"
	);	
	if(!MID_printAttsBuilder_setDuplexMode) {
		error("Failed to GetMethodId(setDuplexMode)");
	}
	
	env->DeleteLocalRef(printAttsBuilderClazz);
}


#endif // USE_PRINTING
#endif // ANDROID
