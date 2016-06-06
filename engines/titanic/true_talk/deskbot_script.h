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

#ifndef TITANIC_DESKBOT_SCRIPT_H
#define TITANIC_DESKBOT_SCRIPT_H

#include "common/array.h"
#include "titanic/true_talk/tt_npc_script.h"

namespace Titanic {

class DeskbotScript : public TTnpcScript {
public:
	DeskbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2);

	virtual void proc7(int v1, int v2);
	virtual int proc10() const;
	virtual int proc15() const;
	virtual bool handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		int val, uint tagId, uint remainder) const;

	/**
	 * Setup range sets
	 */
	virtual bool setupRanges();

	virtual bool proc18() const;
	virtual int proc21(int v1, int v2, int v3);
	virtual int proc22(int id) const;
	virtual int proc23() const;
	virtual const int *getTablePtr(int id);
	virtual int proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const;
	virtual void proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence);
	virtual int proc36(int val) const;
	virtual uint translateId(uint id) const;

	virtual void proc38();
	virtual void proc39();
	virtual void proc40();
	virtual void proc41();
};

} // End of namespace Titanic

#endif /* TITANIC_DESKBOT_SCRIPT_H */
