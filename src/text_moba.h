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


#ifndef LD41_TEXT_MOBA_H_
#define LD41_TEXT_MOBA_H_


#include <utility>
#include <unordered_map>

#include <lair/core/lair.h>
#include <lair/core/path.h>
#include <lair/core/parse.h>

#include "console.h"


class MainState;
class Console;


enum Team {
	TEAM_BLUE,
	TEAM_RED,
};


typedef std::vector<lair::String> StringVector;


class MapNode {
public:
	typedef std::unordered_map<MapNode*, StringVector> NodeMap;

public:
	MapNode* destination(const lair::String& direction) const;

public:
	lair::String  id;
	lair::String  name;
	NodeMap       paths;
	lair::Path    image;
	lair::Vector2 pos;
};

typedef std::shared_ptr<MapNode> MapNodeSP;


class Character {
public:
	enum Status {
		ALIVE,
		DEAD,
	};

public:
	MapNode* position;
	Team     team;

	unsigned maxHP;
	unsigned maxMana;

	unsigned level;
	unsigned xp;

	unsigned hp;
	unsigned mana;

	unsigned deathTime;
};

typedef std::shared_ptr<Character> CharacterSP;


class TextMoba;

class TMCommand {
public:
	TMCommand(TextMoba* textMoba);
	virtual ~TMCommand();

	const StringVector& names() const;
	const lair::String& desc() const;

	virtual void exec(const StringVector& args) = 0;

	template<typename... Args>
	inline void print(Args&&... args) const;

	TextMoba* tm();
	Character* player();

protected:
	TextMoba*    _textMoba;
	StringVector   _names;
	lair::String _desc;
};

typedef std::shared_ptr<TMCommand> TMCommandSP;


class TextMoba {
public:
	typedef std::vector<TMCommandSP> TMCommandList;

public:
	TextMoba(MainState* mainState, Console* console);

	void initialize(const lair::Path& logicPath);

	MainState* mainState();
	Console* console();

	MapNode* mapNode(const lair::String& id);
	Character* player();

	void moveCharacter(Character* character, MapNode* dest);
	void nextTurn();

	const TMCommandList& commands() const;
	TMCommand* command(const lair::String& name) const;

	void _addCommand(TMCommandSP command);

	template<typename Cmd>
	void _addCommand() {
		_addCommand(std::make_shared<Cmd>(this));
	}

	bool _execCommand(const lair::String& command);

private:
	typedef std::unordered_map<lair::String, MapNodeSP>  NodeMap;
	typedef std::unordered_map<lair::String, TMCommand*> TMCommandMap;

private:
	void _initialize(std::istream& in, const lair::Path& logicPath);

private:
	MainState*  _mainState;
	Console*    _console;

	TMCommandList _commands;
	TMCommandMap  _commandMap;

	NodeMap     _nodes;
	CharacterSP _player;
};


template<typename... Args>
inline void TMCommand::print(Args&&... args) const {
	_textMoba->console()->writeLine(lair::cat(std::forward<Args>(args)...));
}


#endif
