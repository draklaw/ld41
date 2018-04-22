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


#ifndef LD41_CHARACTER_CLASS_H_
#define LD41_CHARACTER_CLASS_H_


#include <lair/core/lair.h>

#include "text_moba.h"


class CharacterClass {
public:
	lair::String id;
	lair::String name;
	bool         playable;

	IntVector    maxHP;
	IntVector    maxMana;
	IntVector    xp;
	IntVector    damage;
	IntVector    range;
	IntVector    speed;
};


#endif
