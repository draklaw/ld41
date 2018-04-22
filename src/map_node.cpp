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


const CharacterSet& MapNode::characters() const {
	return _characters;
}


void MapNode::addCharacter(CharacterSP character) {
	_characters.emplace(character);
}


void MapNode::removeCharacter(CharacterSP character) {
	_characters.erase(character);
}
