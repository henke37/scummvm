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

#ifndef DIRECTOR_LINGO_XLIBS_PRINTOMATICXOBJ_H
#define DIRECTOR_LINGO_XLIBS_PRINTOMATICXOBJ_H

#include "common/array.h"
#include "common/str.h"
#include "common/rect.h"

#include "graphics/managed_surface.h"

#include "backends/printing/printman.h"

namespace Director {

class PrintOMaticXObject : public Object<PrintOMaticXObject> {
public:
	PrintOMaticXObject(ObjectType objType);

	PrintOMaticXObject(const PrintOMaticXObject &);
	void operator=(const PrintOMaticXObject &) = delete;

	AbstractObject *clone() override;

	Common::String docName;
	PrintSettings *settings;

	class Page {
		public:
			Page(){};
			~Page(){};
			Page(Page &&);
			Page(const Page &) = delete;
			Page &operator=(const Page &) = delete;
			Page &operator=(Page &&);

			//Takes ownership of the surface
			void drawBitmap(Graphics::ManagedSurface *, const Common::Rect &);
			void drawText(const Common::String &, Common::Point pos);
			void drawLine(const Common::Point &start, const Common::Point &end);

			class PageElement {
			protected:
				PageElement(){};
			public:
				virtual ~PageElement();
				PageElement(const PageElement &) = delete;
				PageElement &operator=(const PageElement &) = delete;

				virtual void draw(PrintJob *job) const =0;
			};

			class BitmapElement : public PageElement {
			public:
				Graphics::ManagedSurface *bitmap;
				Common::Rect drawArea;

				void draw(PrintJob *job) const override;

				BitmapElement(Graphics::ManagedSurface *bitmap, Common::Rect drawArea) : bitmap(bitmap), drawArea(drawArea) {}
				~BitmapElement();
			};

			class TextElement : public PageElement {
			public:
				Common::String text;
				Common::Point pos;

				void draw(PrintJob *job) const override;

				TextElement(const Common::String &text, Common::Point pos) : text(text), pos(pos) {}
				~TextElement(){};
			};

			Common::Array<Common::ScopedPtr<PageElement> > elements;
	};

	Common::Array<Page> pages;
	Page *currentPage;

	private:
};

namespace PrintOMaticXObj {

extern const char *xlibName;
extern const char *fileNames[];

void open(int type);
void close(int type);

void m_new(int nargs);
void m_dispose(int nargs);
void m_reset(int nargs);
void m_newPage(int nargs);
void m_setPrintableMargins(int nargs);
void m_getPageWidth(int nargs);
void m_getPageHeight(int nargs);
void m_picture(int nargs);
void m_stagePicture(int nargs);
void m_1bitStagePicture(int nargs);
void m_setLandscapeMode(int nargs);
void m_doPageSetup(int nargs);
void m_doJobSetup(int nargs);
void m_setProgressMsg(int nargs);
void m_setProgressPict(int nargs);
void m_setDocumentName(int nargs);
void m_printPreview(int nargs);
void m_printPicts(int nargs);
void m_print(int nargs);
void m_register(int nargs);

} // End of namespace PrintOMaticXObj

} // End of namespace Director

#endif
