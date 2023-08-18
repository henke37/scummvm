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

/*************************************
 *
 * USED IN:
 * Plates are People too!
 *
 *************************************/
/* --  PrintOMatic XObject
--  Version 1.1.4, August 8, 1995
--  ©1994-95 Electronic Ink
--
--  STANDARD METHODS
I      mNew
X      mDispose
--
--  DOCUMENT ATTRIBUTES
X      mReset
I      mNewPage
XI     mSetPage, pageNum
IIIII  mSetMargins, left, top, right, bottom
X      mSetPrintableMargins
I      mGetPageWidth
I      mGetPageHeight
--
--  GRAPHICS STATE
XIII   mSetColor, r, g, b    (scale = 0-255)
XI     mSetGray, grayLevel   (scale = 0-100)
XII    mSetPenSize, h, v
XS     mSetTextFont, fontName
XI     mSetTextSize, fontSize
XS     mSetTextStyle, styleNames
XS     mSetTextJust, [ right | left | centered ]
--
--  TEXT ELEMENTS
XIIIII mTextBox, left, top, right, bottom, linkedToPrevious
V      mSetText, textString [, autoAppend]
V      mAppendText, textString [, autoAppend]
V      mAppendTextFile, fileName [, autoAppend]
V      mAppendTextResource, id | name [, autoAppend]
ISII   mDrawText, text, startH, startV
S      mGetInsertionPoint
--
--  MASTER FLOWS
XIIII  mMasterTextBox, left, top, right, bottom
V      mAppendMasterText, textString
V      mAppendMasterTextFiles, fileName
XIIII  mMasterPictBox, left, top, right, bottom
V      mAppendMasterPict, pict | pictFile | pictFolder | pictResID
X      mStageToMasterPict
--
--  GRAPHIC ELEMENTS
XIIII  mStrokedRect, left, top, right, bottom
XIIII  mFilledRect, left, top, right, bottom
XIIIII mStrokedRoundRect, left, top, right, bottom, cornerRadius
XIIIII mFilledRoundRect, left, top, right, bottom, cornerRadius
V      mStrokedOval [, left, top, right, bottom | , centerH, centerV, radius ]
V      mFilledOval [, left, top, right, bottom | , centerH, centerV, radius ]
XIIII  mLine, startH, startV, endH, endV
V      mPicture, pict | pictFile | pictResID, left, top [ , right, bottom ]
V      mStagePicture, left, top , right, bottom [,clipLeft ,clipTop ...] 
V      m1BitStagePicture, left, top , right, bottom [,clipLeft ,clipTop ...] 
V      mEPSFile, fileName, left, top , right, bottom
--
--  PRINTING
II     mSetLandscapeMode, trueOrFalse
XS     mSetDocumentName, name
I      mDoPageSetup
I      mDoJobSetup
XS     mSetProgressMsg, text
V      mSetProgressPict, pict
XII    mSetProgressLoc, left, top
I      mPrintPreview
V      mPrintPicts [, folder]
X      mPrint
--
--  SAVING TO DISK
ISISS  mSavePageSetup, fileName, resID, fileType, fileCreator
ISI    mGetPageSetup, fileName, resID
--
--  MISCELLANEOUS
XI     mHideMessages, trueOrFalse
IS     mSetPageNumSymbol, symbol
IS     mRegister, serialNumber
*/

#include "director/director.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/printomatic.h"

#include "common/rect.h"
#include "graphics/managed_surface.h"

#include "backends/printing/printman.h"

namespace Director {

const char *PrintOMaticXObj::xlibName = "PrintOMatic";
const char *PrintOMaticXObj::fileNames[] = {
	"PMATIC",
	"PrintOMatic",
	0
};

static MethodProto xlibMethods[] = {
	{ "New",					PrintOMaticXObj::m_new,						0,	0,	400 },	// D4
	{ "Dispose",				PrintOMaticXObj::m_dispose,					0,	0,	400 },	// D4
	{ "Reset",					PrintOMaticXObj::m_reset,					0,	0,	400 },	// D4
	{ "NewPage",				PrintOMaticXObj::m_newPage,					0,	0,	400 },	// D4
	{ "SetPrintableMargins",	PrintOMaticXObj::m_setPrintableMargins,		0,	0,	400 },	// D4
	{ "GetPageWidth",			PrintOMaticXObj::m_getPageWidth,			0,	0,	400 },	// D4
	{ "GetPageHeight",			PrintOMaticXObj::m_getPageHeight,			0,	0,	400 },	// D4
	{ "Picture",				PrintOMaticXObj::m_picture,					3,	5,	400 },	// D4
	{ "StagePicture",			PrintOMaticXObj::m_stagePicture,			4,	8,	400 },	// D4
	{ "1BitStagePicture",		PrintOMaticXObj::m_1bitStagePicture,		4,	8,	400 },	// D4
	{ "SetLandscapeMode",		PrintOMaticXObj::m_setLandscapeMode,		1,	1,	400 },	// D4
	{ "DoPageSetup",			PrintOMaticXObj::m_doPageSetup,				0,	0,	400 },	// D4
	{ "DoJobSetup",				PrintOMaticXObj::m_doJobSetup,				0,	0,	400 },	// D4
	{ "SetDocumentName",		PrintOMaticXObj::m_setDocumentName,			1,	1,	400 },	// D4
	{ "SetProgressMsg",			PrintOMaticXObj::m_setProgressMsg,			1,	1,	400 },	// D4
	{ "SetProgressPict",		PrintOMaticXObj::m_setProgressPict,			1,	1,	400 },	// D4
	{ "PrintPreview",			PrintOMaticXObj::m_printPreview,			0,	0,	400 },	// D4
	{ "PrintPicts",				PrintOMaticXObj::m_printPicts,				0,	1,	400 },	// D4
	{ "Print",					PrintOMaticXObj::m_print,					0,	0,	400 },	// D4
	{ "Register",				PrintOMaticXObj::m_register,				1,	1,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void PrintOMaticXObj::open(int type) {
	if (type == kXObj) {
		PrintOMaticXObject::initMethods(xlibMethods);
		PrintOMaticXObject *xobj = new PrintOMaticXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void PrintOMaticXObj::close(int type) {
	if (type == kXObj) {
		PrintOMaticXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


PrintOMaticXObject::PrintOMaticXObject(ObjectType ObjectType) : Object<PrintOMaticXObject>("PrintOMaticXObj"), currentPage(nullptr), settings(nullptr) {
	_objType = ObjectType;
}

PrintOMaticXObject::PrintOMaticXObject(const PrintOMaticXObject &) : Object<PrintOMaticXObject>("PrintOMaticXObj") {
	error("Can't clone PrintOMatic!");
}

AbstractObject *PrintOMaticXObject::clone() {
	error("Can't clone PrintOMatic!");
	return nullptr;
}

void PrintOMaticXObj::m_new(int nargs) {
#ifdef USE_PRINTING

	PrintingManager *pm = g_system->getPrintingManager();

	if (!pm) {
		g_lingo->push(Datum());
		return;
	}

	g_lingo->push(g_lingo->_state->me);
#else
	g_lingo->push(Datum());
#endif
}

XOBJSTUBNR(PrintOMaticXObj::m_dispose)

void PrintOMaticXObj::m_register(int nargs) {
	Common::String serialNumber = g_lingo->pop().asString();
	warning("PrintOMaticXObj::m_register: Registered with serial \"%s\"", serialNumber.c_str());
}

void PrintOMaticXObj::m_reset(int nargs) {
	PrintOMaticXObject *obj = (PrintOMaticXObject *)g_lingo->_state->me.u.obj;

	obj->pages.clear();
	obj->currentPage = nullptr;
	obj->docName = "";

	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_newPage(int nargs) {
	PrintOMaticXObject *obj = (PrintOMaticXObject *)g_lingo->_state->me.u.obj;

	obj->pages.push_back(PrintOMaticXObject::Page());
	obj->currentPage = &obj->pages.back();

	g_lingo->dropStack(nargs);
	g_lingo->push(Datum((int)obj->pages.size()));
}

XOBJSTUBNR(PrintOMaticXObj::m_setPrintableMargins)
XOBJSTUB(PrintOMaticXObj::m_getPageWidth, -1)
XOBJSTUB(PrintOMaticXObj::m_getPageHeight, -1)
XOBJSTUBV(PrintOMaticXObj::m_picture)

void PrintOMaticXObj::m_stagePicture(int nargs) {
	PrintOMaticXObject *obj = (PrintOMaticXObject *)g_lingo->_state->me.u.obj;
	
	Graphics::ManagedSurface *wndsurf = g_director->getCurrentWindow()->getSurface();

	Common::Rect drawArea;
	Common::Rect clipArea;

	if (nargs == 8) {
		clipArea.bottom = g_lingo->pop().asInt();
		clipArea.right = g_lingo->pop().asInt();
		clipArea.top = g_lingo->pop().asInt();
		clipArea.left = g_lingo->pop().asInt();
	} else {
		clipArea.right = wndsurf->w;
		clipArea.bottom = wndsurf->h;
	}

	drawArea.bottom = g_lingo->pop().asInt();
	drawArea.right = g_lingo->pop().asInt();
	drawArea.top = g_lingo->pop().asInt();
	drawArea.left = g_lingo->pop().asInt();

	Graphics::ManagedSurface *snap = new Graphics::ManagedSurface(clipArea.width(), clipArea.height(), wndsurf->format);

	snap->blitFrom(*wndsurf, clipArea, Common::Rect(0, 0, snap->w, snap->h));

	obj->currentPage->drawBitmap(snap, drawArea);
}

XOBJSTUBV(PrintOMaticXObj::m_1bitStagePicture)

void PrintOMaticXObj::m_setLandscapeMode(int nargs) {
	PrintOMaticXObject *obj = (PrintOMaticXObject *)g_lingo->_state->me.u.obj;

	obj->settings->setLandscapeOrientation(g_lingo->pop().asInt());

	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_setLandscapeMode", nargs);
}

XOBJSTUB(PrintOMaticXObj::m_doPageSetup, 1)
XOBJSTUB(PrintOMaticXObj::m_doJobSetup, 1)
XOBJSTUBNR(PrintOMaticXObj::m_setDocumentName)
XOBJSTUBNR(PrintOMaticXObj::m_setProgressMsg)
XOBJSTUBNR(PrintOMaticXObj::m_setProgressPict)
XOBJSTUB(PrintOMaticXObj::m_printPreview, 0)
XOBJSTUBV(PrintOMaticXObj::m_printPicts)

void PrintOMaticXObj::m_print(int nargs) {
	PrintOMaticXObject *obj = (PrintOMaticXObject *)g_lingo->_state->me.u.obj;

	g_lingo->dropStack(nargs);

#ifdef USE_PRINTING

	PrintingManager *pm = g_system->getPrintingManager();

	if (!pm)
		return;

	auto lambda = [obj](PrintJob *job) -> void {
		for (auto pageIt = obj->pages.begin(); pageIt != obj->pages.end(); ++pageIt) {
			auto &page = *pageIt;

			job->beginPage();

			for (auto elmIt = page.elements.begin(); elmIt != page.elements.end(); ++elmIt) {
				auto &elm = *elmIt;

				elm->draw(job);
			}

			job->endPage();
		}

		job->endDoc();
	};

	PrintCallback cb = new Common::Functor1Lamb<PrintJob *, void, decltype(lambda)>(lambda);

	pm->printCustom(cb, obj->docName);

#endif
}

PrintOMaticXObject::Page::Page(Page &&old) : elements(Common::move(old.elements)) {
}

PrintOMaticXObject::Page &PrintOMaticXObject::Page::operator=(Page &&old) {
	elements = Common::move(old.elements);
	return *this;
}

void PrintOMaticXObject::Page::drawBitmap(Graphics::ManagedSurface *surf, const Common::Rect &area) {
	this->elements.push_back(Common::ScopedPtr<PageElement>(new BitmapElement(surf, area)));
}

void PrintOMaticXObject::Page::drawText(const Common::String &text, Common::Point pos) {
	this->elements.push_back(Common::ScopedPtr<PageElement>(new TextElement(text, pos)));
}

void PrintOMaticXObject::Page::BitmapElement::draw(PrintJob *job) const {
	job->drawBitmap(*bitmap, drawArea);
}

PrintOMaticXObject::Page::BitmapElement::~BitmapElement() {
	delete bitmap;
}

void PrintOMaticXObject::Page::TextElement::draw(PrintJob *job) const {
	job->drawText(text, pos);
}

PrintOMaticXObject::Page::PageElement::~PageElement() {
}

} // End of namespace Director
