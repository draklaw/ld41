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


#ifndef LD41_COMMANDS_H_
#define LD41_COMMANDS_H_


#include <lair/core/lair.h>

#include "tm_command.h"


#define DECL_COMMAND(_name) \
	class _name : public TMCommand { \
	public: \
	    _name(TextMoba* textMoba); \
	    virtual void exec(const StringVector& args) override; \
	};


DECL_COMMAND(HelpCommand)
DECL_COMMAND(LookCommand)
DECL_COMMAND(DirectionsCommand)
DECL_COMMAND(WaitCommand)
DECL_COMMAND(GoCommand)
DECL_COMMAND(MoveCommand)
DECL_COMMAND(AttackCommand)


#endif
