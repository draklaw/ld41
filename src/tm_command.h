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


#ifndef LD41_TM_COMMAND_H_
#define LD41_TM_COMMAND_H_


#include "text_moba.h"


class TMCommand {
public:
	TMCommand(TextMoba* textMoba);
	virtual ~TMCommand();

	const StringVector& names() const;
	const lair::String& desc() const;

	virtual void exec(const StringVector& args) = 0;

	template<typename... Args>
	inline void print(Args&&... args) const {
		_textMoba->console()->writeLine(lair::cat(std::forward<Args>(args)...));
	}

	TextMoba* tm();
	CharacterSP player();

protected:
	TextMoba*    _textMoba;
	StringVector   _names;
	lair::String _desc;
};


#endif
