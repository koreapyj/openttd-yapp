/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file clipboard_widget.h Types related to the clipboard widgets. */

#ifndef WIDGETS_CLIPBOARD_WIDGET_H
#define WIDGETS_CLIPBOARD_WIDGET_H

/** Widgets of the #ClipboardToolbarWindow class. */
enum ClipboardToolbarWidgets {
	WID_CT_CLIPBOARD_1,                                     ///< Button to switch to clipboard #1
	WID_CT_CLIPBOARD_2,                                     ///< Button to switch to clipboard #2
	WID_CT_CLIPBOARD_3,                                     ///< Button to switch to clipboard #3
	WID_CT_CLIPBOARD_4,                                     ///< Button to switch to clipboard #4

	WID_CT_COPY,                                            ///< Copy button (single player)
	WID_CT_PASTE,                                           ///< Paste button (single player)

	WID_CT_PASTE_FLAG_BUTTON_BEGIN,                         ///< First button to toggle copy-paste flag
	WID_CT_WITH_RAIL = WID_CT_PASTE_FLAG_BUTTON_BEGIN,      ///< Toggle rails button
	WID_CT_WITH_ROAD,                                       ///< Toggle roads button
	WID_CT_WITH_WATER,                                      ///< Toggle water button
	WID_CT_WITH_AIR,                                        ///< Toggle air button
	WID_CT_MIRROR_SIGNALS,                                  ///< Toggle signal mirrorig button
	WID_CT_UPGRADE_BRIDGES,                                 ///< Toggle bridge upgrading button
	WID_CT_PASTE_FLAG_BUTTON_END,                           ///< Past-the-last button to toggle copy-paste flag

	WID_CT_CONVERT_RAILTYPE = WID_CT_PASTE_FLAG_BUTTON_END, ///< Button to select railtype to convert to

	WID_CT_TERRAFORM,                                       ///< Button to select terraforming mode

	WID_CT_TRANSFORMATION,                                  ///< Button to show/reset clipboard transformation
	WID_CT_ROTATE_LEFT,                                     ///< Rotate left button
	WID_CT_ROTATE_RIGHT,                                    ///< Rotate right button
	WID_CT_REFLECT_NE_SW,                                   ///< Reflect against NE-SW axis button
	WID_CT_REFLECT_NW_SE,                                   ///< Reflect against NW-SE axis button

	WID_CT_HEIGHT_DIFF_GLYPH,                               ///< Image in front of buttons to increase/decrease height level
	WID_CT_HEIGHT_DIFF,                                     ///< Panel with buttons to increase/decrease height level
	WID_CT_HEIGHT_DIFF_INCREASE,                            ///< Button to increase height level
	WID_CT_HEIGHT_DIFF_DECREASE,                            ///< Button to decrease height level
};

#endif /* WIDGETS_CLIPBOARD_WIDGET_H */
