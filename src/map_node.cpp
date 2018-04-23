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

#include "character_class.h"
#include "character.h"

#include "map_node.h"


using namespace lair;


CharacterGroups::CharacterGroups(const MapNode* node)
    : _node(node)
{
	if(!_node) {
		return;
	}

	_characters.resize(node->_characters.size());
	std::copy(node->_characters.begin(), node->_characters.end(),
	          _characters.begin());

	std::stable_sort(_characters.begin(), _characters.end(),
	                 [](CharacterSP c0, CharacterSP c1) {
		if(c0->team() < c1->team())
			return true;
		if(c1->team() < c0->team())
			return false;

		if(c0->place() < c1->place())
			return true;
		return false;
	});

	_indices[0] = 0;

	_indices[1] = _indices[0];
	while(_indices[1] < count() &&
	      get(_indices[1])->team() == BLUE &&
	      get(_indices[1])->place() == BACK)
		_indices[1] += 1;

	_indices[2] = _indices[1];
	while(_indices[2] < count() &&
	      get(_indices[2])->team() == BLUE &&
	      get(_indices[2])->place() == FRONT)
		_indices[2] += 1;

	_indices[3] = _indices[2];
	while(_indices[3] < count() &&
	      get(_indices[3])->team() == RED &&
	      get(_indices[3])->place() == BACK)
		_indices[3] += 1;

	_indices[4] = count();

//	dbgLogger.warning("Groups: ", _indices[0], ", ", _indices[1],
//	        ", ", _indices[2], ", ", _indices[3], ", ", _indices[4]);
//	unsigned i = 0;
//	for(CharacterSP c: _characters) {
//		dbgLogger.info("  ", i++, ": ", c->className(), " ", c->index());
//	}
}


unsigned CharacterGroups::count() const {
	return _characters.size();
}


unsigned CharacterGroups::count(Team team) const {
	return _index(team + 1, 0) - _index(team, 0);
}


unsigned CharacterGroups::count(Team team, Place place) const {
	return _index(team, place + 1) - _index(team, place);
}


unsigned CharacterGroups::count(CharType type, Team team) const {
	unsigned c = 0;
	unsigned teamCount = count(team);
	for(unsigned i = 0; i < teamCount; ++i) {
		if(get(team, i)->type() == type) {
			c += 1;
		}
	}
	return c;
}


CharacterSP CharacterGroups::get(unsigned index) const {
	return _characters.at(index);
}


CharacterSP CharacterGroups::get(Team team, unsigned index) const {
	return _characters.at(_index(team, 0) + index);
}


CharacterSP CharacterGroups::get(Team team, Place place, unsigned index) const {
	return _characters.at(_index(team, place) + index);
}


unsigned CharacterGroups::distanceBetween(CharacterSP c0, CharacterSP c1) const {
	if(!c0->isAlive() || c0->node().get() != _node ||
	   !c1->isAlive() || c1->node().get() != _node)
		return 9999;

	int p0 = c0->placeIndex();
	int p1 = c1->placeIndex();

	if(p0 > p1)
		std::swap(p0, p1);

	unsigned dist = p1 - p0;
	for(int i = p0 + 1; i < p1; ++i) {
		Team team = teamFromPlaceIndex(i);
		Place place = placeFromPlaceIndex(i);
		if(!count(team, place)) {
			dist -= 1;
		}
	}

	return dist;
}


CharacterSP CharacterGroups::pick(Team team, Place place) const {
	unsigned c = count(team, place);
	if(c == 0)
		return CharacterSP();
	return get(team, place, rand() % c);
}


CharacterSP CharacterGroups::pickClosestEnemy(CharacterSP c, int range) const {
	if(range < 0)
		range = c->range();

	Team enemy = c->enemyTeam();

	if(c->place() == BACK && count(c->team(), FRONT)) {
		range -= 1;
	}

	if(count(enemy, FRONT)) {
		if(range > 0 && range == FRONT) {
			return pick(enemy, FRONT);
		}
		else {
			range -= 1;
		}
	}

	if(range > 0 && count(enemy, BACK)) {
		return pick(enemy, BACK);
	}

	return CharacterSP();
}


unsigned CharacterGroups::_index(unsigned team, unsigned place) const {
	return _indices[2 * team + place];
}



const String& MapNode::id() const {
	return _id;
}


const String& MapNode::name() const {
	return _name;
}


const MapNode::NodeMap& MapNode::paths() const {
	return _paths;
}


MapNodeSP MapNode::destination(const String& direction) const {
	for(const auto& pair: _paths) {
		for(const String& dir: pair.second) {
			if(dir == direction) {
				return pair.first->shared_from_this();
			}
		}
	}
	return MapNodeSP();
}


const Path& MapNode::image() const {
	return _image;
}


const Vector2& MapNode::pos() const {
	return _pos;
}


const String& MapNode::tower() const {
	return _tower;
}


const String& MapNode::fonxus() const {
	return _fonxus;
}


CharacterSP MapNode::characterAt(unsigned index) const {
	auto it  = _characters.begin();
	auto end = _characters.end();
	for(unsigned i = 0; it != end && i < index; ++i)
		++it;

	if(it == _characters.end())
		return CharacterSP();
	return *it;
}


const CharacterSet& MapNode::characters() const {
	return _characters;
}


CharacterGroups MapNode::characterGroups() const {
	return CharacterGroups(this);
}


unsigned MapNode::characterIndex(CharacterCSP character) const {
	auto it = _characters.find(std::const_pointer_cast<Character>(character));

	if(it == _characters.end()) {
		dbgLogger.error("MapNode::characterName: character ", character->debugName(),
		                " not found.");
		return 0;
	}

	return std::distance(_characters.begin(), it);
}


void MapNode::addCharacter(CharacterSP character) {
	_characters.emplace(character);
}


void MapNode::removeCharacter(CharacterSP character) {
	_characters.erase(character);
}
