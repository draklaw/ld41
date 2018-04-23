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


#ifndef LD41_SKILL_H_
#define LD41_SKILL_H_


#include <lair/core/lair.h>

#include "map_node.h"
#include "character.h"

#include "text_moba.h"


enum SkillEffect {
	NO_EFFECT,
	DAMAGE,
	HEAL,
//	ROOT,
//	PUSH,
	DOT,
	HOT
};

enum SkillTarget {
	NO_TARGET,
	SELF,
	SINGLE,
	FRONT_ROW,
	BACK_ROW,
	ANY_ROW,
	BOTH_ROWS,
};


class SkillModel {
public:

	struct Effect {
		SkillEffect type(unsigned level) const;
		unsigned power(unsigned level) const;

		IntVector _type;
		IntVector _power;
	};

	typedef std::vector<Effect> EffectVector;

public:
	const lair::String& id() const;
	const lair::String& name() const;
	const lair::String& desc() const;

	const EffectVector& effects() const;
	SkillTarget target(unsigned level) const;
	unsigned range(unsigned level) const;
	unsigned cooldown(unsigned level) const;
	unsigned manaCost(unsigned level) const;

	void _use(unsigned level, CharacterSP character, CharacterSP target);

public:
	lair::String _id;
	lair::String _name;
	lair::String _desc;

	EffectVector _effects;
	IntVector    _target;
	IntVector    _power;
	IntVector    _range;
	IntVector    _cooldown;
	IntVector    _manaCost;
};


class Skill {
public:
	Skill(SkillModelSP model, unsigned level, CharacterSP character);

	const lair::String& id() const;
	const lair::String& name() const;
	const lair::String& desc() const;

//	Effect effect() const;
	SkillTarget target() const;
//	unsigned power() const;
	unsigned range() const;
	unsigned cooldown() const;
	unsigned manaCost() const;
	unsigned level() const;
	unsigned timeBeforeNextUse() const;

	CharacterSP character() const;

	bool usable() const;
	Team targetTeam() const;

	CharacterVector targets() const;
	CharacterVector targets(Place place) const;
	CharacterVector targets(CharacterSP target) const;

	void use();
	void use(Place place);
	void use(CharacterSP target);

	void useOn(const CharacterVector& chars);

	void _use(CharacterSP target);

	template<typename... Args>
	void print(Args&&... args) const {
		character()->_textMoba->print(std::forward<Args>(args)...);
	}

public:
	SkillModelSP _model;

	unsigned     _level;
	unsigned     _timeBeforeNextUse;

	CharacterWP  _character;
};


SkillEffect parseSkillEffect(const lair::String& str);
SkillTarget parseSkillTarget(const lair::String& str);


#endif
