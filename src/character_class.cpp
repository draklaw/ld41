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
#include "character.h"

#include "character_class.h"


using namespace lair;


const String& CharacterClass::id() const {
	return _id;
}


const String& CharacterClass::name() const {
	return _name;
}


int CharacterClass::sortIndex() const {
	return _sortIndex;
}


bool CharacterClass::playable() const {
	return _playable;
}


Place CharacterClass::defaultPlace() const {
	return _defaultPlace;
}


const IntVector CharacterClass::maxHP() const {
	return _maxHP;
}


const IntVector CharacterClass::maxMana() const {
	return _maxMana;
}


const IntVector CharacterClass::xp() const {
	return _xp;
}


const IntVector CharacterClass::damage() const {
	return _damage;
}


const IntVector CharacterClass::range() const {
	return _range;
}


const IntVector CharacterClass::speed() const {
	return _speed;
}


int CharacterClass::maxHP(unsigned level) const {
	return _maxHP.at(level);
}


int CharacterClass::maxMana(unsigned level) const {
	return _maxMana.at(level);
}


int CharacterClass::xp(unsigned level) const {
	return _xp.at(level);
}


int CharacterClass::damage(unsigned level) const {
	return _damage.at(level);
}


int CharacterClass::range(unsigned level) const {
	return _range.at(level);
}


int CharacterClass::speed(unsigned level) const {
	return _speed.at(level);
}
