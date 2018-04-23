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

#include "hero_ai.h"


using namespace lair;


HeroAi::HeroAi(CharacterSP character, Lane lane)
    : Ai(character)
    , _status(PUSH_LANE)
    , _lane(lane)
{
}


void HeroAi::play() {
	CharacterSP c = character();

	if(!c || !c->isAlive())
		return;

	_groups = c->node()->characterGroups();

//	dbgLogger.log(c->debugName(), " turn:");

	// Back when low health
	if(c->hp() < c->maxHP() / 4) {
		_status = BACK_TO_BASE;
	}
	if(c->hp() == c->maxHP() && c->mana() == c->maxMana()) {
		// TODO: remember previous status and reset it.
		_status = PUSH_LANE;
	}

	switch(_status) {
	case PUSH_LANE: {
		unsigned enemyCount = _groups.count(c->enemyTeam());
		unsigned towerCount = _groups.count(BUILDING, c->team());
		unsigned redshirtCount = _groups.count(REDSHIRT, c->team());

		if(enemyCount && !redshirtCount && !towerCount) {
			// Back if it doesn't look good.
			move(BACKWARD);
		}
		else if(enemyCount) {
			if(c->range() == 1 && c->place() == BACK) {
				c->goToPlace(FRONT);
			}
			else {
				attackClosest();
			}
		}
		else if(redshirtCount) {
			move(FORWARD);
		}
		break;
	}

	case FOLLOW_PLAYER: {
		// TODO: Follow and mimic player
		break;
	}

	case BACK_TO_BASE: {
		// TODO: TP to base from somewhere safe
		if(!move(BACKWARD)) {
			// Attack enemies at the Fonxus.
			if(_groups.count(c->enemyTeam())) {
				attackClosest();
			}
		}
		break;
	}
	}
}


void HeroAi::attackClosest() {
	// TODO: Attack player target in FOLLOW_PLAYER mode ?

	CharacterSP target = _target.lock();

	if(!target || !target->isAlive() ||
	        _groups.distanceBetween(character(), target) > character()->range()) {
		target = _groups.pickClosestEnemy(character());
	}

	if(target) {
		_target = target;
		character()->attack(target);
	}
}


bool HeroAi::move(Dir direction) {
	Team dir = (direction == FORWARD)?
	               character()->enemyTeam():
	               character()->team();
	MapNodeSP dest = character()->node()->destination(teamName(dir));
	if(!dest) {
		dest = character()->node()->destination(laneName(_lane));
	}

	if(dest) {
		character()->moveTo(dest);
	}

	return bool(dest);
}
