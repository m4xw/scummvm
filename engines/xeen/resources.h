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

#ifndef XEEN_RESOURCES_H
#define XEEN_RESOURCES_H

#include "common/scummsys.h"
#include "gui/debugger.h"

namespace Xeen {

extern const char *const CREDITS;

extern const char *const OPTIONS_TITLE;

extern const byte SYMBOLS[20][64];

extern const byte TEXT_COLORS[40][4];

extern const char *const DIRECTION_TEXT[4];

extern const char *const RACE_NAMES[5];

extern const char *const CLASS_NAMES[11];

extern const char *const ALIGNMENT_NAMES[3];

extern const char *const SEX_NAMES[2];

extern const char *const CONDITION_NAMES[18];

extern const char *const IN_PARTY;

extern const char *const PARTY_DETAILS;

extern const int FACE_CONDITION_FRAMES[17];

extern const int CHAR_FACES_X[6];

extern const int HP_BARS_X[6];

extern const char *const NO_ONE_TO_ADVENTURE_WITH;

extern const byte BACKGROUND_XLAT[];

extern const char *const YOUR_ROSTER_IS_FULL;

extern const char *const PLEASE_WAIT;

extern const char *const OOPS;

} // End of namespace Xeen

#endif	/* XEEN_RESOURCES_H */
