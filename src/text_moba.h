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
#include <set>

#include <lair/core/lair.h>
#include <lair/core/path.h>
#include <lair/core/parse.h>

#include "console.h"


class MainState;
class Console;


enum Team {
	BLUE,
	RED,
	NEUTRAL,
};

enum Place {
	BACK,
	FRONT,
};

enum Lane {
	TOP,
	BOT,
};

enum CharType {
	HERO,
	REDSHIRT,
	BUILDING,
};


class MapNode;
class CharacterClass;
class Character;
class SkillModel;
class Skill;
class Ai;
class TMCommand;
class TextMoba;

typedef std::shared_ptr<MapNode>         MapNodeSP;
typedef std::weak_ptr<MapNode>           MapNodeWP;
typedef std::shared_ptr<CharacterClass>  CharacterClassSP;
typedef std::shared_ptr<Character>       CharacterSP;
typedef std::shared_ptr<const Character> CharacterCSP;
typedef std::weak_ptr<Character>         CharacterWP;
typedef std::shared_ptr<SkillModel>      SkillModelSP;
typedef std::shared_ptr<Skill>           SkillSP;
typedef std::shared_ptr<Ai>              AiSP;
typedef std::shared_ptr<TMCommand>       TMCommandSP;


typedef std::vector<int>          IntVector;
typedef std::vector<lair::String> StringVector;


struct CharacterOrder {
	bool operator()(const CharacterSP& c0, const CharacterSP& c1) const;
};

typedef std::vector<CharacterSP> CharacterVector;
typedef std::set<CharacterSP, CharacterOrder> CharacterSet;

typedef std::vector<SkillSP> SkillVector;


const lair::String& teamName(Team team);
const lair::String& placeName(Place place);
const lair::String& laneName(Lane lane);
const lair::String& charTypeName(CharType charType);

Team enemyTeam(Team team);
unsigned placeIndex(Team team, Place place);
Team teamFromPlaceIndex(unsigned pi);
Place placeFromPlaceIndex(unsigned pi);


class TextMoba {
public:
	typedef std::vector<TMCommandSP> TMCommandList;

public:
	TextMoba(MainState* mainState, Console* console);

	void initialize(const lair::Path& logicPath);

	MainState* mainState();
	Console* console();

	unsigned heroNextLevel(unsigned level) const;
	unsigned heroXpWorth(unsigned level) const;
	unsigned redshirtXpWorth(unsigned level) const;
	unsigned towerXpWorth(unsigned level) const;

	unsigned nextLevel(CharacterSP character) const;
	unsigned xpWorth(CharacterSP character) const;

	MapNodeSP mapNode(const lair::String& id);
	MapNodeSP fonxus(Team team);
	CharacterClassSP characterClass(const lair::String& id);
	const CharacterSet& characters() const;
	CharacterSP player();
	SkillModelSP skillModel(const lair::String id);

	CharacterSP spawnCharacter(const lair::String& className, Team team,
	                           MapNodeSP node = MapNodeSP());
	CharacterSP spawnRedshirt(Team team, Lane lane);
	void spawnRedshirts(Team team, unsigned count);

	void killCharacter(CharacterSP character, CharacterSP attacker = nullptr);

	void moveCharacter(CharacterSP character, MapNodeSP dest);
	void placeCharacter(CharacterSP character, Place place);

	void attack(CharacterSP attacker, CharacterSP target);
	void dealDamage(CharacterSP target, unsigned damage,
	                CharacterSP attacker = nullptr);
	void healCharacter(CharacterSP target, unsigned amount,
	                   CharacterSP healer = nullptr);

	void useSkillOn(SkillSP skill, const CharacterVector& targets);
	void _useSkillOn(SkillSP skill, CharacterSP target);

	void grantXp(CharacterSP character, unsigned xp);

	void nextTurn();
	void nextTurn(CharacterSP character);

	const TMCommandList& commands() const;
	TMCommand* command(const lair::String& name) const;

	void _addCommand(TMCommandSP command);

	template<typename Cmd>
	void _addCommand() {
		_addCommand(std::make_shared<Cmd>(this));
	}

	bool _execCommand(const lair::String& command);

	template<typename... Args>
	inline void print(Args&&... args) {
		console()->writeLine(lair::cat(std::forward<Args>(args)...));
	}

private:
	typedef std::unordered_map<lair::String, MapNodeSP>        NodeMap;
	typedef std::unordered_map<lair::String, TMCommand*>       TMCommandMap;
	typedef std::unordered_map<lair::String, CharacterClassSP> ClassMap;
	typedef std::unordered_map<lair::String, SkillModelSP>     SkillModelMap;

private:
	void _initialize(std::istream& in, const lair::Path& logicPath);

private:
	MainState*  _mainState;
	Console*    _console;

	TMCommandList _commands;
	TMCommandMap  _commandMap;

	NodeMap       _nodes;
	ClassMap      _classes;
	SkillModelMap _skillModels;

	unsigned     _charIndex;
	CharacterSet _characters;
	CharacterSP  _player;

public:
	unsigned _firstWaveTime;
	unsigned _waveTime;
	unsigned _redshirtPerLane;

	IntVector _heroNextLevel;
	IntVector _heroXpWorth;
	IntVector _redshirtXpWorth;
	IntVector _towerXpWorth;

	unsigned _turn;
	unsigned _nextWaveCounter;

	CharacterVector _heroes;
};


#endif
