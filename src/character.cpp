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


#include <lair/core/log.h>

#include "map_node.h"
#include "character_class.h"

#include "character.h"


using namespace lair;


Character::Character(TextMoba* textMoba, CharacterClassSP cClass, unsigned index, unsigned level)
    : _textMoba(textMoba)
    , _cClass(cClass)
    , _index(index)
    , _node(nullptr)
    , _team(BLUE)
    , _level(level)
    , _xp(0)
    , _hp(cClass->maxHP(level))
    , _mana(cClass->maxMana(level))
    , _deathTime(0)
{
}


CharacterClassSP Character::cClass() const {
	return _cClass;
}


const String& Character::className() const {
	return _cClass->name();
}


unsigned Character::index() const {
	return _index;
}


MapNodeSP Character::node() const {
	return _node;
}


Team Character::team() const {
	return _team;
}


const String& Character::teamName() const {
	static String tn[] = {
	    "blue",
	    "red",
	    "neutral",
	};
	return tn[_team];
}


bool Character::isPlayer() const {
	return this == _textMoba->player().get();
}


unsigned Character::maxHP() const {
	return _cClass->maxHP(_level);
}


unsigned Character::maxMana() const {
	return _cClass->maxMana(_level);
}


unsigned Character::level() const {
	return _level;
}


unsigned Character::xp() const {
	return _xp;
}


unsigned Character::hp() const {
	return _hp;
}


unsigned Character::mana() const {
	return _mana;
}


bool Character::isAlive() const {
	return _deathTime == 0;
}


bool Character::deathTime() const {
	return _deathTime;
}


AiSP Character::ai() const {
	return _ai;
}


void Character::moveTo(MapNodeSP dest) {
	_textMoba->moveCharacter(shared_from_this(), dest);
}
