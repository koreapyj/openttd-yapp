/* $Id: gfx_layout.cpp 25583 2013-07-10 19:41:31Z rubidium $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file gfx_layout.cpp Handling of laying out text. */

#include "stdafx.h"
#include "gfx_layout.h"
#include "string_func.h"
#include "strings_func.h"

#include "table/control_codes.h"

#ifdef WITH_ICU
#include <unicode/ustring.h>
#endif /* WITH_ICU */


/** Cache of ParagraphLayout lines. */
Layouter::LineCache *Layouter::linecache;

/** Cache of Font instances. */
Layouter::FontColourMap Layouter::fonts[FS_END];


/**
 * Construct a new font.
 * @param size   The font size to use for this font.
 * @param colour The colour to draw this font in.
 */
Font::Font(FontSize size, TextColour colour) :
		fc(FontCache::Get(size)), colour(colour)
{
	assert(size < FS_END);
}

#ifdef WITH_ICU
/* Implementation details of LEFontInstance */

le_int32 Font::getUnitsPerEM() const
{
	return this->fc->GetUnitsPerEM();
}

le_int32 Font::getAscent() const
{
	return this->fc->GetAscender();
}

le_int32 Font::getDescent() const
{
	return -this->fc->GetDescender();
}

le_int32 Font::getLeading() const
{
	return this->fc->GetHeight();
}

float Font::getXPixelsPerEm() const
{
	return (float)this->fc->GetHeight();
}

float Font::getYPixelsPerEm() const
{
	return (float)this->fc->GetHeight();
}

float Font::getScaleFactorX() const
{
	return 1.0f;
}

float Font::getScaleFactorY() const
{
	return 1.0f;
}

const void *Font::getFontTable(LETag tableTag) const
{
	size_t length;
	return this->getFontTable(tableTag, length);
}

const void *Font::getFontTable(LETag tableTag, size_t &length) const
{
	return this->fc->GetFontTable(tableTag, length);
}

LEGlyphID Font::mapCharToGlyph(LEUnicode32 ch) const
{
	if (IsTextDirectionChar(ch)) return 0;
	return this->fc->MapCharToGlyph(ch);
}

void Font::getGlyphAdvance(LEGlyphID glyph, LEPoint &advance) const
{
	advance.fX = glyph == 0xFFFF ? 0 : this->fc->GetGlyphWidth(glyph);
	advance.fY = 0;
}

le_bool Font::getGlyphPoint(LEGlyphID glyph, le_int32 pointNumber, LEPoint &point) const
{
	return FALSE;
}

size_t Layouter::AppendToBuffer(UChar *buff, const UChar *buffer_last, WChar c)
{
	/* Transform from UTF-32 to internal ICU format of UTF-16. */
	int32 length = 0;
	UErrorCode err = U_ZERO_ERROR;
	u_strFromUTF32(buff, buffer_last - buff, &length, (UChar32*)&c, 1, &err);
	return length;
}

ParagraphLayout *Layouter::GetParagraphLayout(UChar *buff, UChar *buff_end, FontMap &fontMapping)
{
	int32 length = buff_end - buff;

	if (length == 0) {
		/* ICU's ParagraphLayout cannot handle empty strings, so fake one. */
		buff[0] = ' ';
		length = 1;
		fontMapping.End()[-1].first++;
	}

	/* Fill ICU's FontRuns with the right data. */
	FontRuns runs(fontMapping.Length());
	for (FontMap::iterator iter = fontMapping.Begin(); iter != fontMapping.End(); iter++) {
		runs.add(iter->second, iter->first);
	}

	LEErrorCode status = LE_NO_ERROR;
	/* ParagraphLayout does not copy "buff", so it must stay valid.
	 * "runs" is copied according to the ICU source, but the documentation does not specify anything, so this might break somewhen. */
	return new ParagraphLayout(buff, length, &runs, NULL, NULL, NULL, _current_text_dir == TD_RTL ? UBIDI_DEFAULT_RTL : UBIDI_DEFAULT_LTR, false, status);
}

#else /* WITH_ICU */

/*** Paragraph layout ***/

/**
 * Create the visual run.
 * @param font       The font to use for this run.
 * @param chars      The characters to use for this run.
 * @param char_count The number of characters in this run.
 * @param x          The initial x position for this run.
 */
ParagraphLayout::VisualRun::VisualRun(Font *font, const WChar *chars, int char_count, int x) :
		font(font), glyph_count(char_count)
{
	this->glyphs = MallocT<GlyphID>(this->glyph_count);

	/* Positions contains the location of the begin of each of the glyphs, and the end of the last one. */
	this->positions = MallocT<float>(this->glyph_count * 2 + 2);
	this->positions[0] = x;
	this->positions[1] = 0;

	for (int i = 0; i < this->glyph_count; i++) {
		this->glyphs[i] = font->fc->MapCharToGlyph(chars[i]);
		this->positions[2 * i + 2] = this->positions[2 * i] + font->fc->GetGlyphWidth(this->glyphs[i]);
		this->positions[2 * i + 3] = 0;
	}
}

/** Free all data. */
ParagraphLayout::VisualRun::~VisualRun()
{
	free(this->positions);
	free(this->glyphs);
}

/**
 * Get the font associated with this run.
 * @return The font.
 */
Font *ParagraphLayout::VisualRun::getFont() const
{
	return this->font;
}

/**
 * Get the number of glyhps in this run.
 * @return The number of glyphs.
 */
int ParagraphLayout::VisualRun::getGlyphCount() const
{
	return this->glyph_count;
}

/**
 * Get the glyhps of this run.
 * @return The glyphs.
 */
const GlyphID *ParagraphLayout::VisualRun::getGlyphs() const
{
	return this->glyphs;
}

/**
 * Get the positions of this run.
 * @return The positions.
 */
float *ParagraphLayout::VisualRun::getPositions() const
{
	return this->positions;
}

/**
 * Get the height of this font.
 * @return The height of the font.
 */
int ParagraphLayout::VisualRun::getLeading() const
{
	return this->getFont()->fc->GetHeight();
}

/**
 * Get the height of the line.
 * @return The maximum height of the line.
 */
int ParagraphLayout::Line::getLeading() const
{
	int leading = 0;
	for (const VisualRun * const *run = this->Begin(); run != this->End(); run++) {
		leading = max(leading, (*run)->getLeading());
	}

	return leading;
}

/**
 * Get the width of this line.
 * @return The width of the line.
 */
int ParagraphLayout::Line::getWidth() const
{
	if (this->Length() == 0) return 0;

	/*
	 * The last X position of a run contains is the end of that run.
	 * Since there is no left-to-right support, taking this value of
	 * the last run gives us the end of the line and thus the width.
	 */
	const VisualRun *run = this->getVisualRun(this->countRuns() - 1);
	return run->getPositions()[run->getGlyphCount() * 2];
}

/**
 * Get the number of runs in this line.
 * @return The number of runs.
 */
int ParagraphLayout::Line::countRuns() const
{
	return this->Length();
}

/**
 * Get a specific visual run.
 * @return The visual run.
 */
ParagraphLayout::VisualRun *ParagraphLayout::Line::getVisualRun(int run) const
{
	return *this->Get(run);
}

/**
 * Create a new paragraph layouter.
 * @param buffer The characters of the paragraph.
 * @param length The length of the paragraph.
 * @param runs   The font mapping of this paragraph.
 */
ParagraphLayout::ParagraphLayout(WChar *buffer, int length, FontMap &runs) : buffer_begin(buffer), buffer(buffer), runs(runs)
{
	assert(runs.End()[-1].first == length);
}

/**
 * Reset the position to the start of the paragraph.
 */
void ParagraphLayout::reflow()
{
	this->buffer = this->buffer_begin;
}

/**
 * Construct a new line with a maximum width.
 * @param max_width The maximum width of the string.
 * @return A Line, or NULL when at the end of the paragraph.
 */
ParagraphLayout::Line *ParagraphLayout::nextLine(int max_width)
{
	/* Simple idea:
	 *  - split a line at a newline character, or at a space where we can break a line.
	 *  - split for a visual run whenever a new line happens, or the font changes.
	 */
	if (this->buffer == NULL) return NULL;

	Line *l = new Line();

	if (*this->buffer == '\0') {
		/* Only a newline. */
		this->buffer = NULL;
		*l->Append() = new VisualRun(this->runs.Begin()->second, this->buffer, 0, 0);
		return l;
	}

	const WChar *begin = this->buffer;
	const WChar *last_space = NULL;
	const WChar *last_char = begin;
	int width = 0;

	int offset = this->buffer - this->buffer_begin;
	FontMap::iterator iter = this->runs.Begin();
	while (iter->first <= offset) {
		iter++;
		assert(iter != this->runs.End());
	}

	const FontCache *fc = iter->second->fc;
	const WChar *next_run = this->buffer_begin + iter->first;

	for (;;) {
		WChar c = *this->buffer;
		last_char = this->buffer;

		if (c == '\0') {
			this->buffer = NULL;
			break;
		}

		if (this->buffer == next_run) {
			int w = l->getWidth();
			*l->Append() = new VisualRun(iter->second, begin, this->buffer - begin, w);
			iter++;
			assert(iter != this->runs.End());

			next_run = this->buffer_begin + iter->first;
			begin = this->buffer;

			last_space = NULL;
		}

		if (IsWhitespace(c)) last_space = this->buffer;

		if (IsPrintable(c) && !IsTextDirectionChar(c)) {
			int char_width = GetCharacterWidth(fc->GetSize(), c);
			width += char_width;
			if (width > max_width) {
				/* The string is longer than maximum width so we need to decide
				 * what to do with it. */
				if (width == char_width) {
					/* The character is wider than allowed width; don't know
					 * what to do with this case... bail out! */
					this->buffer = NULL;
					return l;
				}

				if (last_space == NULL) {
					/* No space has been found. Just terminate at our current
					 * location. This usually happens for languages that do not
					 * require spaces in strings, like Chinese, Japanese and
					 * Korean. For other languages terminating mid-word might
					 * not be the best, but terminating the whole string instead
					 * of continuing the word at the next line is worse. */
					last_char = this->buffer;
				} else {
					/* A space is found; perfect place to terminate */
					this->buffer = last_space + 1;
					last_char = last_space;
				}
				break;
			}
		}

		this->buffer++;
	}

	if (l->Length() == 0 || last_char - begin != 0) {
		int w = l->getWidth();
		*l->Append() = new VisualRun(iter->second, begin, last_char - begin, w);
	}
	return l;
}

/**
 * Appand a wide character to the internal buffer.
 * @param buff        The buffer to append to.
 * @param buffer_last The end of the buffer.
 * @param c           The character to add.
 * @return The number of buffer spaces that were used.
 */
size_t Layouter::AppendToBuffer(WChar *buff, const WChar *buffer_last, WChar c)
{
	*buff = c;
	return 1;
}

/**
 * Get the actual ParagraphLayout for the given buffer.
 * @param buff The begin of the buffer.
 * @param buff_end The location after the last element in the buffer.
 * @param fontMapping THe mapping of the fonts.
 * @return The ParagraphLayout instance.
 */
ParagraphLayout *Layouter::GetParagraphLayout(WChar *buff, WChar *buff_end, FontMap &fontMapping)
{
	return new ParagraphLayout(buff, buff_end - buff, fontMapping);
}
#endif /* !WITH_ICU */

/**
 * Create a new layouter.
 * @param str      The string to create the layout for.
 * @param maxw     The maximum width.
 * @param colour   The colour of the font.
 * @param fontsize The size of font to use.
 */
Layouter::Layouter(const char *str, int maxw, TextColour colour, FontSize fontsize)
{
	FontState state(colour, fontsize);
	WChar c = 0;

	do {
		/* Scan string for end of line */
		const char *lineend = str;
		for (;;) {
			size_t len = Utf8Decode(&c, lineend);
			if (c == '\0' || c == '\n') break;
			lineend += len;
		}

		LineCacheItem& line = GetCachedParagraphLayout(str, lineend - str, state);
		if (line.layout != NULL) {
			/* Line is in cache */
			str = lineend + 1;
			state = line.state_after;
			line.layout->reflow();
		} else {
			/* Line is new, layout it */
			const CharType *buffer_last = lastof(line.buffer);
			CharType *buff_begin = line.buffer;
			CharType *buff = buff_begin;
			FontMap &fontMapping = line.runs;
			Font *f = GetFont(state.fontsize, state.cur_colour);

			/*
			 * Go through the whole string while adding Font instances to the font map
			 * whenever the font changes, and convert the wide characters into a format
			 * usable by ParagraphLayout.
			 */
			for (; buff < buffer_last;) {
				c = Utf8Consume(const_cast<const char **>(&str));
				if (c == '\0' || c == '\n') {
					break;
				} else if (c >= SCC_BLUE && c <= SCC_BLACK) {
					state.SetColour((TextColour)(c - SCC_BLUE));
				} else if (c == SCC_PREVIOUS_COLOUR) { // Revert to the previous colour.
					state.SetPreviousColour();
				} else if (c == SCC_TINYFONT) {
					state.SetFontSize(FS_SMALL);
				} else if (c == SCC_BIGFONT) {
					state.SetFontSize(FS_LARGE);
				} else {
					buff += AppendToBuffer(buff, buffer_last, c);
					continue;
				}

				if (!fontMapping.Contains(buff - buff_begin)) {
					fontMapping.Insert(buff - buff_begin, f);
				}
				f = GetFont(state.fontsize, state.cur_colour);
			}

			/* Better safe than sorry. */
			*buff = '\0';

			if (!fontMapping.Contains(buff - buff_begin)) {
				fontMapping.Insert(buff - buff_begin, f);
			}
			line.layout = GetParagraphLayout(buff_begin, buff, fontMapping);
			line.state_after = state;
		}

		/* Copy all lines into a local cache so we can reuse them later on more easily. */
		ParagraphLayout::Line *l;
		while ((l = line.layout->nextLine(maxw)) != NULL) {
			*this->Append() = l;
		}

	} while (c != '\0');
}

/**
 * Get the boundaries of this paragraph.
 * @return The boundaries.
 */
Dimension Layouter::GetBounds()
{
	Dimension d = { 0, 0 };
	for (ParagraphLayout::Line **l = this->Begin(); l != this->End(); l++) {
		d.width = max<uint>(d.width, (*l)->getWidth());
		d.height += (*l)->getLeading();
	}
	return d;
}

/**
 * Get a static font instance.
 */
Font *Layouter::GetFont(FontSize size, TextColour colour)
{
	FontColourMap::iterator it = fonts[size].Find(colour);
	if (it != fonts[size].End()) return it->second;

	Font *f = new Font(size, colour);
	*fonts[size].Append() = FontColourMap::Pair(colour, f);
	return f;
}

/**
 * Reset cached font information.
 * @param size Font size to reset.
 */
void Layouter::ResetFontCache(FontSize size)
{
	for (FontColourMap::iterator it = fonts[size].Begin(); it != fonts[size].End(); ++it) {
		delete it->second;
	}
	fonts[size].Clear();

	/* We must reset the linecache since it references the just freed fonts */
	ResetLineCache();
}

/**
 * Get reference to cache item.
 * If the item does not exist yet, it is default constructed.
 * @param str Source string of the line (including colour and font size codes).
 * @param len Length of \a str in bytes (no termination).
 * @param state State of the font at the beginning of the line.
 * @return Reference to cache item.
 */
Layouter::LineCacheItem &Layouter::GetCachedParagraphLayout(const char *str, size_t len, const FontState &state)
{
	if (linecache == NULL) {
		/* Create linecache on first access to avoid trouble with initialisation order of static variables. */
		linecache = new LineCache();
	}

	LineCacheKey key;
	key.state_before = state;
	key.str.assign(str, len);
	return (*linecache)[key];
}

/**
 * Clear line cache.
 */
void Layouter::ResetLineCache()
{
	if (linecache != NULL) linecache->clear();
}

/**
 * Reduce the size of linecache if necessary to prevent infinite growth.
 */
void Layouter::ReduceLineCache()
{
	if (linecache != NULL) {
		/* TODO LRU cache would be fancy, but not exactly necessary */
		if (linecache->size() > 4096) ResetLineCache();
	}
}
