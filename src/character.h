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


#ifndef LD41_CHARACTER_H_
#define LD41_CHARACTER_H_


#include <lair/core/lair.h>

#include "text_moba.h"

class Buff {
public:
	unsigned ticks;
	int amount;
	char type;
};

typedef std::vector<Buff> BuffVector;

class Character : public std::enable_shared_from_this<Character> {
public:
	Character(TextMoba* textMoba, CharacterClassSP cClass, unsigned index,
	          unsigned level = 0);

	CharacterClassSP cClass() const;
	const lair::String& className() const;

	unsigned index() const;

	CharType type() const;

	lair::String name(bool showIndex = true) const;
	lair::String debugName() const;
	lair::String shortDesc() const;

	MapNodeSP node() const;

	Team team() const;
	Team enemyTeam() const;
	const lair::String& teamName() const;

	bool isPlayer() const;

	Place place() const;
	const lair::String& placeName() const;

	unsigned maxHP() const;
	unsigned maxMana() const;

	unsigned level() const;
	unsigned xp() const;
	unsigned hp() const;
	unsigned mana() const;
	unsigned damage() const;
	unsigned range() const;

	bool isAlive() const;
	bool deathTime() const;

	const SkillVector& skills();
	SkillSP skill(const lair::String& name);
	void addSkill(SkillModelSP model, unsigned level = 1);

	AiSP ai() const;

	template<typename T, typename... Args>
	AiSP setAi(Args&&... args) {
		_ai = std::make_shared<T>(shared_from_this(), std::forward<Args>(args)...);
		return _ai;
	}

	unsigned placeIndex() const;

	void moveTo(MapNodeSP dest);
	void goToPlace(Place place);
	void attack(CharacterSP target);
	void takeDamage(unsigned damage, CharacterSP attacker = nullptr);
	void heal(unsigned amount, CharacterSP healer = nullptr);

public:
	TextMoba* _textMoba;

	CharacterClassSP _cClass;
	unsigned _index;

	MapNodeSP _node;
	Team     _team;

	Place    _place;

	unsigned _level;
	unsigned _xp;

	unsigned _hp;
	unsigned _mana;

	unsigned _deathTime;

	BuffVector _buffs;
	SkillVector _skills;

	AiSP     _ai;
};


#endif
