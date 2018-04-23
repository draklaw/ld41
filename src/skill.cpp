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
#include "character_class.h"
#include "character.h"

#include "skill.h"


using namespace lair;



SkillEffect SkillModel::Effect::type(unsigned level) const {
	if(level == 0)
		return NO_EFFECT;
	return SkillEffect(_type.at(level - 1));
}


unsigned SkillModel::Effect::power(unsigned level) const {
	if(level == 0)
		return 0;
	return _power.at(level - 1);
}


const String& SkillModel::id() const {
	return _id;
}


const String& SkillModel::name() const {
	return _name;
}


const String& SkillModel::desc() const {
	return _name;
}


const SkillModel::EffectVector& SkillModel::effects() const {
	return _effects;
}


SkillTarget SkillModel::target(unsigned level) const {
	if(level == 0)
		return NO_TARGET;
	return SkillTarget(_target.at(level - 1));
}


unsigned SkillModel::range(unsigned level) const {
	if(level == 0)
		return 0;
	return _range.at(level - 1);
}


unsigned SkillModel::cooldown(unsigned level) const {
	if(level == 0)
		return 0;
	return _cooldown.at(level - 1);
}


unsigned SkillModel::manaCost(unsigned level) const {
	if(level == 0)
		return 999999;
	return _manaCost.at(level - 1);
}



Skill::Skill(SkillModelSP model, unsigned level, CharacterSP character)
    : _model(model)
    , _level(level)
    , _timeBeforeNextUse(0)
    , _character(character)
{
}

const String& Skill::id() const {
	return _model->id();
}


const String& Skill::name() const {
	return _model->name();
}


const String& Skill::desc() const {
	return _model->desc();
}


//Skill::Effect Skill::effect() const {
//	if(_level == 0)
//		return NONE;
//	return _effect;
//}


SkillTarget Skill::target() const {
	return _model->target(_level);
}


//unsigned Skill::power() const {
//	return _model->power(_level);
//}


unsigned Skill::range() const {
	return _model->range(_level);
}


unsigned Skill::cooldown() const {
	return _model->cooldown(_level);
}


unsigned Skill::manaCost() const {
	return _model->manaCost(_level);
}


unsigned Skill::level() const {
	return _level;
}


unsigned Skill::timeBeforeNextUse() const {
	return _timeBeforeNextUse;
}


CharacterSP Skill::character() const {
	return _character.lock();
}


bool Skill::usable() const {
	CharacterSP c = character();
	return c->isAlive() && _level && _timeBeforeNextUse == 0
	    && c->mana() >= manaCost();
}


Team Skill::targetTeam() const {
	for(const SkillModel::Effect& effect: _model->effects()) {
		switch(effect.type(_level)) {
		case NO_EFFECT:
			return BLUE;
		case DAMAGE:
		case DOT:
			return character()->enemyTeam();
		case HEAL:
		case HOT:
			return character()->team();
		}
	}
	return BLUE;
}


CharacterVector Skill::targets() const {
	CharacterVector chars;

	if(!usable())
		return chars;

	CharacterSP c = character();
	CharacterGroups groups = c->node()->characterGroups();
	Team team = targetTeam();
	switch(target()) {
	case NO_TARGET:
	case SINGLE:
	case ANY_ROW:
		dbgLogger.error("Invalid Skill::target call");
		break;
	case SELF:
		chars.push_back(c);
		break;
	case FRONT_ROW:
		for(unsigned i = 0; i < groups.count(team, FRONT); ++i) {
			CharacterSP t = groups.get(team, FRONT, i);
			if(groups.distanceBetween(c, t) <= range()) {
				chars.push_back(t);
			}
		}
		break;
	case BACK_ROW:
		for(unsigned i = 0; i < groups.count(team, BACK); ++i) {
			CharacterSP t = groups.get(team, BACK, i);
			if(groups.distanceBetween(c, t) <= range()) {
				chars.push_back(t);
			}
		}
		break;
	case BOTH_ROWS:
		for(unsigned i = 0; i < groups.count(team); ++i) {
			CharacterSP t = groups.get(team, i);
			if(groups.distanceBetween(c, t) <= range()) {
				chars.push_back(t);
			}
		}
		break;
	}

	return chars;
}


CharacterVector Skill::targets(Place place) const {
	CharacterVector chars;

	if(!usable())
		return chars;

	CharacterSP c = character();
	CharacterGroups groups = c->node()->characterGroups();
	Team team = targetTeam();
	if(target() == ANY_ROW) {
		for(unsigned i = 0; i < groups.count(team, place); ++i) {
			CharacterSP t = groups.get(team, place, i);
			if(groups.distanceBetween(c, t) <= range()) {
				chars.push_back(t);
			}
		}
	}
	else {
		dbgLogger.error("Invalid Skill::target(Place) call");
	}

	return chars;
}


CharacterVector Skill::targets(CharacterSP target) const {
	CharacterVector chars;

	if(!usable())
		return chars;

	CharacterSP c = character();
	CharacterGroups groups = c->node()->characterGroups();
	if(this->target() == SINGLE) {
		if(groups.distanceBetween(c, target) <= range()) {
			chars.push_back(target);
		}
	}
	else {
		dbgLogger.error("Invalid Skill::target(CharacterSP) call");
	}

	return chars;
}


void Skill::use() {
	useOn(targets());
}


void Skill::use(Place place) {
	useOn(targets(place));
}


void Skill::use(CharacterSP target) {
	useOn(targets(target));
}


void Skill::useOn(const CharacterVector& chars) {
	character()->_textMoba->useSkillOn(shared_from_this(), chars);
}


SkillEffect parseSkillEffect(const lair::String& str) {
	if(str == "none")
		return NO_EFFECT;
	else if(str == "damage")
		return DAMAGE;
	else if(str == "heal")
		return HEAL;
	else if(str == "dot")
		return DOT;
	else if(str == "hot")
		return HOT;
	dbgLogger.error("Unknown skill effect type \"", str, "\"");
	return NO_EFFECT;
}


SkillTarget parseSkillTarget(const lair::String& str) {
	if(str == "self")
		return SELF;
	else if(str == "single")
		return SINGLE;
	else if(str == "front_row")
		return FRONT_ROW;
	else if(str == "back_row")
		return BACK_ROW;
	else if(str == "any_row")
		return ANY_ROW;
	else if(str == "both_rows")
		return BOTH_ROWS;
	dbgLogger.error("Unknown skill target \"", str, "\"");
	return NO_TARGET;
}
