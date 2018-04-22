/*
 *  Copyright (C) 2018 the authors (see AUTHORS)
 *
 *  This file is part of Draklia's ld41.
 *
 *  lair is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  lair is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with lair.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef LD41_CHARACTER_H_
#define LD41_CHARACTER_H_


#include <lair/core/lair.h>

#include "text_moba.h"


class Character {
public:
	Character(CharacterClassSP cClass, unsigned level = 0);

	CharacterClassSP cClass() const;
	const lair::String& className() const;

	MapNodeSP node() const;

	Team team() const;
	const lair::String& teamName() const;

	unsigned maxHP() const;
	unsigned maxMana() const;
	unsigned level() const;
	unsigned xp() const;
	unsigned hp() const;
	unsigned mana() const;

	bool isAlive() const;
	bool deathTime() const;

public:
	CharacterClassSP _cClass;
	MapNodeSP _node;
	Team     _team;

	unsigned _level;
	unsigned _xp;

	unsigned _hp;
	unsigned _mana;

	unsigned _deathTime;
};


#endif
