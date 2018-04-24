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

#include "tower_ai.h"


using namespace lair;


TowerAi::TowerAi(CharacterSP character)
    : Ai(character)
{
}


void TowerAi::play() {
	CharacterSP c = character();

	if(!c || !c->isAlive())
		return;

//	dbgLogger.log(c->debugName(), " turn:");

	Team enemy = c->enemyTeam();

	CharacterGroups groups = c->node()->characterGroups();
	if(groups.count(enemy)) {
		CharacterSP target = _target.lock();

		if(!target || !target->isAlive() ||
		        groups.distanceBetween(c, target) > c->range()) {
			target = groups.pickClosestEnemy(c);
		}

		if(target) {
//			dbgLogger.info("  Attack ", target->debugName());
			c->attack(target);
		}

		_target = target;
	}
	else {
		_target.reset();
	}
}
