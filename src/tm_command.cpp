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

#include "tm_command.h"


using namespace lair;


TMCommand::TMCommand(TextMoba* textMoba, bool hidden)
    : _textMoba(textMoba)
    , _hidden(hidden)
{
}


TMCommand::~TMCommand() {
}

const StringVector& TMCommand::names() const {
	return _names;
}


const lair::String& TMCommand::desc() const {
	return _desc;
}


TextMoba* TMCommand::tm() {
	return _textMoba;
}


CharacterSP TMCommand::player() {
	return _textMoba->player();
}
