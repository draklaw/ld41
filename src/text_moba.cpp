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

#include "game.h"
#include "main_state.h"
#include "console.h"
#include "commands.h"

#include "map_node.h"
#include "character_class.h"
#include "character.h"
#include "skill.h"
#include "ai.h"
#include "redshirt_ai.h"
#include "tower_ai.h"
#include "hero_ai.h"
#include "tm_command.h"

#include "text_moba.h"


using namespace lair;


// TODO: Move this to Lair


const Variant& getVarItem(const Variant& var, const String& key, bool* success = nullptr) {
	if(!var.isVarMap()) {
		dbgLogger.error("Expected VarMap for key \"", key, "\", got \"", var.type()->identifier, "\".");
		if(success)
			*success = false;
		return Variant::null;
	}

	return var.get(key);
}

bool getBool(const Variant& var, const String& key, bool defaultValue = false, bool* success = nullptr) {
	const Variant& v = getVarItem(var, key, success);

	if(v.isBool()) {
		return v.asBool();
	}
	else if(!v.isNull()) {
		dbgLogger.error("Expected Bool for key \"", key, "\", got \"", var.type()->identifier, "\".");
		if(success)
			*success = false;
	}

	return defaultValue;
}

int64 getInt(const Variant& var, const String& key, int64 defaultValue = 0, bool* success = nullptr) {
	const Variant& v = getVarItem(var, key, success);

	if(v.isInt()) {
		return v.asInt();
	}
	else if(!v.isNull()) {
		dbgLogger.error("Expected Int for key \"", key, "\", got \"", var.type()->identifier, "\".");
		if(success)
			*success = false;
	}

	return defaultValue;
}

float getFloat(const Variant& var, const String& key, float defaultValue = 0, bool* success = nullptr) {
	const Variant& v = getVarItem(var, key, success);

	if(v.isFloat()) {
		return v.asFloat();
	}
	else if(!v.isNull()) {
		dbgLogger.error("Expected Float for key \"", key, "\", got \"", var.type()->identifier, "\".");
		if(success)
			*success = false;
	}

	return defaultValue;
}

const String& getString(const Variant& var, const String& key, const String& defaultValue = String(), bool* success = nullptr) {
	const Variant& v = getVarItem(var, key, success);

	if(v.isString()) {
		return v.asString();
	}
	else if(!v.isNull()) {
		dbgLogger.error("Expected String for key \"", key, "\", got \"", var.type()->identifier, "\".");
		if(success)
			*success = false;
	}

	return defaultValue;
}

IntVector getIntList(const Variant& var, const String& key, unsigned size, int defaultValue, bool* success = nullptr) {
	const Variant& v = getVarItem(var, key, success);

	if(v.isInt()) {
		return IntVector(size, v.asInt());
	}
	if(v.isVarList()) {
		IntVector stats(size, defaultValue);
		unsigned i = 0;
		for(const Variant& v2: v.asVarList()) {
			if(i < size) {
				stats[i] = v2.asInt();
				i += 1;
			}
			else {
				dbgLogger.warning("Too much value in array ", key);
				break;
			}
		}
		return stats;
	}
	else if(!v.isNull()) {
		dbgLogger.error("Expected Int list.");
		if(success)
			*success = false;
	}

	return IntVector(6, defaultValue);
}

StringVector getStringList(const Variant& var, const String& key, bool* success = nullptr) {
	const Variant& v = getVarItem(var, key, success);

	StringVector strings;
	if(v.isString()) {
		strings.push_back(v.asString());
	}
	if(v.isVarList()) {
		for(const Variant& v2: v.asVarList()) {
			if(v2.isString()) {
				strings.push_back(v2.asString());
			}
			else {
				dbgLogger.warning("Expected String");
			}
		}
	}
	else if(!v.isNull()) {
		dbgLogger.error("Expected String list.");
		if(success)
			*success = false;
	}

	return strings;
}

IntVector getClassStats(const Variant& var, const String& key, bool* success = nullptr) {
	return getIntList(var, key, 6, 0, success);
}



bool CharacterOrder::operator()(const CharacterSP& c0, const CharacterSP& c1) const {
	if(c0->team() < c1->team())
		return true;
	if(c1->team() < c0->team())
		return false;

	if(c0->cClass()->sortIndex() < c1->cClass()->sortIndex())
		return true;
	if(c1->cClass()->sortIndex() < c0->cClass()->sortIndex())
		return false;

	if(c0->index() < c1->index())
		return true;
	return false;
}



const String& teamName(Team team) {
	static const String names[] = {
	    "blue",
	    "red",
	    "neutral",
	};
	return names[team];
}

const String& placeName(Place place) {
	static const String names[] = {
	    "back",
	    "front",
	};
	return names[place];
}

const String& laneName(Lane lane) {
	static const String names[] = {
	    "top",
	    "bot",
	};
	return names[lane];
}

const lair::String& charTypeName(CharType charType) {
	static const String names[] = {
	    "hero",
	    "redshirt",
	    "building",
	};
	return names[charType];
}


Team enemyTeam(Team team) {
	return Team(team ^ 0x01);
}


unsigned placeIndex(Team team, Place place) {
	return (team == BLUE)?
	            ((place == BACK)?  0: 1):
	            ((place == FRONT)? 2: 3);
}


Team teamFromPlaceIndex(unsigned pi) {
	return Team(pi / 2);
}


Place placeFromPlaceIndex(unsigned pi) {
	Team team = teamFromPlaceIndex(pi);
	unsigned place = pi & 0x01;
	return Place((team == BLUE)? place: 1 - place);
}



TextMoba::TextMoba(MainState* mainState, Console* console)
    : _mainState(mainState)
    , _console(console)
{
	using namespace std::placeholders;

	_console->setExecCommand(std::bind(&TextMoba::_execCommand, this, _1));

	_addCommand<HelpCommand>();
	_addCommand<LookCommand>();
	_addCommand<DirectionsCommand>();
	_addCommand<WaitCommand>();
	_addCommand<GoCommand>();
	_addCommand<MoveCommand>();
	_addCommand<AttackCommand>();
	_addCommand<UseCommand>();
}


void TextMoba::initialize(const Path& logicPath) {
	VirtualFile file = _mainState->game()->fileSystem()->file(logicPath);

	Path realPath = file.realPath();
	if(!realPath.empty()) {
		Path::IStream in(realPath.native().c_str());
		_initialize(in, logicPath);
	}

	const MemFile* memFile = file.fileBuffer();
	if(memFile) {
		String buffer((const char*)memFile->data, memFile->size);
		std::istringstream in(buffer);
		_initialize(in, logicPath);
	}
}


MainState* TextMoba::mainState() {
	return _mainState;
}


Console* TextMoba::console() {
	return _console;
}


unsigned TextMoba::heroNextLevel(unsigned level) const {
	return _heroNextLevel.at(level);
}


unsigned TextMoba::heroXpWorth(unsigned level) const {
	return _heroXpWorth.at(level);
}


unsigned TextMoba::redshirtXpWorth(unsigned level) const {
	return _redshirtXpWorth.at(level);
}


unsigned TextMoba::towerXpWorth(unsigned level) const {
	return _towerXpWorth.at(level);
}


unsigned TextMoba::nextLevel(CharacterSP character) const {
	return (character->type() == HERO)? heroNextLevel(character->level()): 0;
}


unsigned TextMoba::xpWorth(CharacterSP character) const {
	switch(character->type()) {
	case HERO:
		return heroXpWorth(character->level());
	case REDSHIRT:
		return redshirtXpWorth(character->level());
	case BUILDING:
		return towerXpWorth(character->level());
	}
	return 0;
}


MapNodeSP TextMoba::mapNode(const String& id) {
	auto it = _nodes.find(id);
	if(it == _nodes.end())
		return nullptr;
	return it->second;
}


MapNodeSP TextMoba::fonxus(Team team) {
	return mapNode((team == BLUE)? "bf": "rf");
}


CharacterClassSP TextMoba::characterClass(const lair::String& id) {
	auto it = _classes.find(id);
	if(it == _classes.end())
		return nullptr;
	return it->second;
}


const CharacterSet& TextMoba::characters() const {
	return _characters;
}


CharacterSP TextMoba::player() {
	return _player;
}


SkillModelSP TextMoba::skillModel(const lair::String id) {
	auto it = _skillModels.find(id);
	if(it == _skillModels.end())
		return nullptr;
	return it->second;
}


CharacterSP TextMoba::spawnCharacter(const lair::String& className, Team team,
                                     MapNodeSP node) {
	CharacterClassSP cc = characterClass(className);
	if(!cc) {
		dbgLogger.error("Invalid character class: \"", className, "\"");
		return CharacterSP();
	}

	CharacterSP character = std::make_shared<Character>(this, cc, _charIndex);
	character->_team = team;

	for(const String& skillName: cc->skills()) {
		SkillModelSP sm = skillModel(skillName);
		if(sm) {
			character->addSkill(sm, 1);
		}
		else {
			dbgLogger.warning("Skill model not found: \"", skillName, "\"");
		}
	}

	if(node) {
		moveCharacter(character, node);
	}

	dbgLogger.log("Spawn ", character->teamName(), " ", character->className(),
	              " ", character->index(), " at ", node? node->name(): "<nowhere>");

	_characters.emplace(character);
	++_charIndex;

	return character;
}


CharacterSP TextMoba::spawnRedshirt(Team team, Lane lane) {
	static const String classes[] = {
	    "blueshirt",
	    "redshirt",
	};

	MapNodeSP fonxus = mapNode((team == BLUE)? "bf": "rf");
	CharacterSP redshirt = spawnCharacter(classes[team], team, fonxus);
	redshirt->setAi<RedshirtAi>(lane);
	dbgLogger.info("  RedshirtAi: ", lane);
	return redshirt;
}


void TextMoba::spawnRedshirts(unsigned count) {
	for(unsigned i = 0; i < count; ++i) {
		spawnRedshirt(BLUE, TOP);
		spawnRedshirt(BLUE, BOT);
		spawnRedshirt(RED,  TOP);
		spawnRedshirt(RED,  BOT);
	}
}


void TextMoba::killCharacter(CharacterSP character, CharacterSP attacker) {
	bool printMessage = character->type() == HERO
	                 || character->node() == player()->node();
	if(attacker) {
		dbgLogger.log(attacker->name(), " killed ", character->name(), ".");
		if(printMessage) {
			print(attacker->name(), " killed ", character->name(), ".");
		}
	}
	else {
		dbgLogger.log(character->debugName(), " killed.");
		if(printMessage) {
			print(character->name(), " killed.");
		}
	}

	unsigned xp = xpWorth(character);
	for(CharacterSP c: character->node()->characters()) {
		if(c->team() == character->team() || c->type() != HERO)
			continue;

		grantXp(c, xp);
	}

	if(character->type() == HERO) {
		moveCharacter(character, fonxus(character->team()));
		// TODO: Death & respawn
		character->_hp = character->maxHP();
	}
	else {
		if(character->node()) {
			character->node()->removeCharacter(character);
			character->_node = nullptr;
		}
		_characters.erase(character);
	}
}


void TextMoba::moveCharacter(CharacterSP character, MapNodeSP dest) {
	if(player() && character != player() && player()->isAlive()
	        && character->type() != BUILDING
	        && character->node() == player()->node()) {
		print(character->name(), " leaves the area.");
	}

	if(character->node()) {
		character->node()->removeCharacter(character);
	}

	character->_node = dest;
	character->_place = character->cClass()->defaultPlace();

	if(player() && character != player() && player()->isAlive()
	        && character->type() != BUILDING
	        && character->node() == player()->node()) {
		print(character->name(false), " enters the area.");
	}

	if(dest) {
		dest->addCharacter(character);
	}
}


void TextMoba::placeCharacter(CharacterSP character, Place place) {
	if(player() && player()->isAlive()
	        && character->node() == player()->node()) {
		print(character->name(), " moves to the ", placeName(place), " row.");
	}
	character->_place = place;
}


void TextMoba::attack(CharacterSP attacker, CharacterSP target) {
	unsigned damage = attacker->damage();

	dbgLogger.log(attacker->debugName(), " attack ", target->debugName(),
	              " for ", damage, " damage.");

	if(attacker->node() == player()->node()) {
		print(attacker->name(), " attack ", target->name(), " for ",
		      damage, " damage.");
	}

	dealDamage(target, damage, attacker);
}


void TextMoba::dealDamage(CharacterSP target, unsigned damage, CharacterSP attacker) {
	if(damage >= target->hp()) {
		target->_hp = 0;
		killCharacter(target, attacker);
	}
	else {
		target->_hp -= damage;
	}
}


void TextMoba::healCharacter(CharacterSP target, unsigned amount, CharacterSP /*healer*/) {
	if(target->isAlive()) {
		target->_hp = std::min(target->hp() + amount, target->maxHP());
	}
}


void TextMoba::grantXp(CharacterSP character, unsigned xp) {
	unsigned nextLevelXp = nextLevel(character);
	if(nextLevelXp == 0)
		return;

	if(character == player()) {
		print(character->name(), " gains ", xp, " xp.");
	}

	character->_xp += xp;
	if(character->xp() < nextLevelXp)
		return;

	// Level-up !
	if(character->team() == BLUE) {
		float hpRatio = float(character->hp()) / float(character->maxHP());
		float manaRatio = float(character->mana()) / float(character->maxMana());

		character->_level += 1;
		character->_xp    -= nextLevelXp;
		print(character->name(), " reaches lvl ", character->level() + 1);

		character->_hp   = character->maxHP()   * hpRatio;
		character->_mana = character->maxMana() * manaRatio;

		if(nextLevel(character) == 0)
			character->_xp = 0;
	}
}


void TextMoba::nextTurn() {
	_turn += 1;

	// NPC turns
	for(CharacterSP c: _characters) {
		// Fonxus regen
		if (c->className() == "fonxus")
			for (SkillSP s: c->skills())
				s->use();

		// Hero regens
		if (c->type() == HERO)
		{
			c->heal(1);
			c->_mana = std::min(c->mana() + 2, c->maxMana());
		}

		// Buffs
		BuffVector nb;
		for (Buff b: c->_buffs)
		{
			switch (b.type) {
				case 'h':
					c->heal(b.amount);
					break;
				case 'd':
					c->takeDamage(b.amount);
					break;
				default:
					dbgLogger.warning("Unknown buff type : '", b.type,"'.");
			}

			if (--b.ticks)
				nb.push_back(b);
		}
		c->_buffs.swap(nb);

		// Skip player
		if (c == player())
			continue;

		// Cooldowns
		for(SkillSP skill: c->skills()) {
			if(skill->timeBeforeNextUse() != 0)
				skill->_timeBeforeNextUse -= 1;
		}

		// AI
		if(c->ai()) {
			c->ai()->play();
		}
	}

	// Minion waves.
	_nextWaveCounter -= 1;
	if(_nextWaveCounter == 0) {
		_console->writeLine("A new batch of redshirts is leaving the fonxus.");
		spawnRedshirts(_redshirtPerLane);
		_nextWaveCounter = _waveTime;
	}

	// Player cooldowns
	for(SkillSP skill: player()->skills()) {
		if(skill->timeBeforeNextUse() != 0)
			skill->_timeBeforeNextUse -= 1;
	}

	_console->writeLine(cat("End of turn ", _turn));
	_execCommand("look");
}


const TextMoba::TMCommandList& TextMoba::commands() const {
	return _commands;
}


TMCommand* TextMoba::command(const lair::String& name) const {
	auto it = _commandMap.find(name);
	if(it == _commandMap.end())
		return nullptr;
	return it->second;
}


void TextMoba::_addCommand(TMCommandSP command) {
	_commands.emplace_back(command);

	for(const String& id: command->names()) {
		_commandMap.emplace(id, command.get());
		dbgLogger.info("Register command \"", id, "\"");
	}
}


bool TextMoba::_execCommand(const String& command) {
	dbgLogger.log("Exec: ", command);

	StringVector args;

	auto it  = command.begin();
	auto end = command.end();

	while(it != end && std::isspace(*it))
		++it;

	while(it != end) {
		auto argBegin = it;
		while(it != end && !std::isspace(*it))
			++it;

		args.emplace_back(String(argBegin, it));

		while(it != end && std::isspace(*it))
			++it;
	}

	if(args.empty())
		return false;

	TMCommand* tmCommand = this->command(args[0]);

	if(!tmCommand) {
		console()->writeLine(cat("Command \"", args[0], "\" do not exists. Type \"h\" for help."));
	}
	else {
		tmCommand->exec(args);
	}

	return true;
}


void TextMoba::_initialize(std::istream& in, const lair::Path& logicPath) {
	// Cleanup

	_heroes.clear();


	// Parse ldl

	ErrorList errors;
	LdlParser parser(&in, logicPath.utf8String(), &errors, LdlParser::CTX_MAP);

	Variant config;
	if(!ldlRead(parser, config)) {
		dbgLogger.error("Failed to load gameplay data from \"",
		                logicPath.utf8String(), "\"");
		errors.log(dbgLogger);
	}
	errors.log(dbgLogger);

	// Read gameplay.ldl

	Variant motd = config.get("motd");
	if(motd.isString()) {
		_console->writeLine(motd.asString());
	}

	_firstWaveTime   = getInt(config, "first_wave_time");
	_waveTime        = getInt(config, "wave_time");
	_redshirtPerLane = getInt(config, "redshirt_per_lane");

	_heroNextLevel   = getClassStats(config, "hero_next_level");
	_heroXpWorth     = getClassStats(config, "hero_xp_worth");
	_redshirtXpWorth = getClassStats(config, "redshirt_xp_worth");
	_towerXpWorth    = getClassStats(config, "tower_xp_worth");

	const Variant& nodes = config.get("nodes");
	if(nodes.isVarMap()) {
		for(const auto& pair: nodes.asVarMap()) {
			const String& id = pair.first;
			const Variant& obj = pair.second;

			MapNodeSP node = std::make_shared<MapNode>();

			node->_id = id;

			const Variant& nameVar = obj.get("name");
			if(nameVar.isString())
				node->_name = nameVar.asString();
			else
				dbgLogger.error("Node without name");

			node->_images = getStringList(obj, "images");
			for(const String& img: node->_images) {
				_mainState->loader()->load<ImageLoader>(img);
			}

			const Variant& posVar = obj.get("position");
			if(posVar.isVarList() && posVar.asVarList().size() == 2) {
				const VarList& pos = posVar.asVarList();
				node->_pos = Vector2(pos[0].asFloat(), pos[1].asFloat());
			}
			else
				dbgLogger.error("Node without position");

			node->_tower  = getString(obj, "tower");
			node->_fonxus = getString(obj, "fonxus");

			_nodes.emplace(node->id(), node);
		}
	}
	else {
		dbgLogger.error("Expected \"nodes\" VarMap.");
	}

	const Variant& paths = config.get("paths");
	if(paths.isVarList()) {
		for(const Variant& path: paths.asVarList()) {
			const Variant& fromVar     = path.get("from");
			const Variant& toVar       = path.get("to");
			const Variant& fromDirsVar = path.get("from_dirs");
			const Variant& toDirsVar   = path.get("to_dirs");
			if(fromVar.isString() && toVar.isString() &&
			        fromDirsVar.isVarList() && toDirsVar.isVarList()) {
				MapNodeSP from = mapNode(fromVar.asString());
				MapNodeSP to   = mapNode(toVar.asString());

				StringVector& fromDirs =
				        from->_paths.emplace(to.get(),   StringVector()).first->second;
				StringVector& toDirs =
				        to  ->_paths.emplace(from.get(), StringVector()).first->second;

				for(const Variant& dirVar: fromDirsVar.asVarList()) {
					if(dirVar.isString()) {
						fromDirs.emplace_back(dirVar.asString());
					}
				}

				for(const Variant& dirVar: toDirsVar.asVarList()) {
					if(dirVar.isString()) {
						toDirs.emplace_back(dirVar.asString());
					}
				}
			}
			else {
				dbgLogger.error("Invalid path.");
			}
		}
	}
	else {
		dbgLogger.error("Expected \"paths\" VarList.");
	}

	const Variant& classes = config.get("classes");
	if(classes.isVarMap()) {
		for(const auto& pair: classes.asVarMap()) {
			const String& id = pair.first;
			const Variant& obj = pair.second;

			CharacterClassSP cClass = std::make_shared<CharacterClass>();

			cClass->_id        = id;

			String type = getString(obj, "type");
			if(type == "hero")
				cClass->_type = HERO;
			else if(type == "redshirt")
				cClass->_type = REDSHIRT;
			else if(type == "building")
				cClass->_type = BUILDING;
			else {
				dbgLogger.error("Unexpected CharType: \"", type, "\"");
				cClass->_type = BUILDING;
			}

			cClass->_name      = getString(obj, "name", "<fixme_no_name>");
			cClass->_sortIndex = getInt(obj, "sort_index", 9999);

			String defaultPlace = getString(obj, "default_place", "back");
			cClass->_defaultPlace = (defaultPlace == "back")? BACK: FRONT;

			cClass->_maxHP     = getClassStats(obj, "max_hp");
			cClass->_maxMana   = getClassStats(obj, "max_mana");
			cClass->_damage    = getClassStats(obj, "damage");
			cClass->_range     = getClassStats(obj, "range");
			cClass->_skills    = getStringList(obj, "skills");

			cClass->_image     = getString(obj, "image");
			if(cClass->_image.size()) {
				_mainState->loader()->load<ImageLoader>(cClass->_image);
			}

			_classes.emplace(cClass->id(), cClass);
		}
	}
	else {
		dbgLogger.error("Expected \"classes\" VarMap.");
	}

	const Variant& skills = config.get("skills");
	if(skills.isVarMap()) {
		for(const auto& pair: skills.asVarMap()) {
			const String& id = pair.first;
			const Variant& obj = pair.second;

			SkillModelSP skill = std::make_shared<SkillModel>();

			skill->_id        = id;
			skill->_name      = getString(obj, "name", "<fixme_no_name>");
			skill->_desc      = getString(obj, "desc");

			const Variant& effectsVar = obj.get("effects");
			if(effectsVar.isVarList()) {
				for(const Variant& effectVar: effectsVar.asVarList()) {
					SkillModel::Effect effect;

					const Variant& typeVar = effectVar.get("type");
					if(typeVar.isString())
						effect._type = IntVector(2, parseSkillEffect(typeVar.asString()));
					else if(typeVar.isVarList()){
						const VarList& list = typeVar.asVarList();
						if(list.size() == 2) {
							for(const Variant& v: list) {
								effect._type.push_back(parseSkillEffect(v.asString()));
							}
						}
						else
							dbgLogger.error("Skill effect type must be of size 2");
					}
					else
						dbgLogger.error("Invalid skill effect type");

					effect._power = getIntList(effectVar, "power", 2, 0);

					skill->_effects.push_back(effect);
				}
			}
			else
				dbgLogger.error("Invalid skill effect");

			const Variant& targetVar = obj.get("target");
			skill->_target = IntVector(2, NO_TARGET);
			if(targetVar.isString()) {
				skill->_target = IntVector(2, parseSkillTarget(targetVar.asString()));
			}
			else if(targetVar.isVarList()) {
				const VarList& list = targetVar.asVarList();
				unsigned count = list.size();
				if(count == 2) {
					for(unsigned i = 0; i < count; ++i) {
						skill->_target[i] = parseSkillTarget(list[i].asString());
					}
				}
				else {
					dbgLogger.error("Skill target array must contain exactly 2 values");
				}
			}
			else {
				dbgLogger.error("Invalid skill target.");
			}

			skill->_range    = getIntList(obj, "range", 2, 3);
			skill->_cooldown = getIntList(obj, "cooldown", 2, 0);
			skill->_manaCost = getIntList(obj, "mana_cost", 2, 99999);

			_skillModels.emplace(skill->id(), skill);
		}
	}
	else {
		dbgLogger.error("Expected \"skills\" VarMap.");
	}

	// Setup

	_turn = 0;
	_nextWaveCounter = _firstWaveTime;

	// Player *must* have charIndex 0
	_charIndex = 0;
	_player = spawnCharacter("ranger", BLUE, mapNode("bf"));
	_heroes.push_back(_player);

	_heroes.push_back(spawnCharacter("warrior", BLUE, mapNode("bf")));
	_heroes.push_back(spawnCharacter("mage", BLUE, mapNode("bf")));

	_heroes.push_back(spawnCharacter("ranger", RED, mapNode("rf")));
	_heroes.push_back(spawnCharacter("warrior", RED, mapNode("rf")));
	_heroes.push_back(spawnCharacter("mage", RED, mapNode("rf")));

	for(unsigned i = 1; i < _heroes.size(); ++i) {
		CharacterSP c = _heroes[i];
		c->setAi<HeroAi>((c->className() == "ranger")? TOP: BOT);
	}

	for(const auto& pair: _nodes) {
		MapNodeSP node = pair.second;

		if(node->fonxus().size()) {
			spawnCharacter("fonxus", (node->fonxus() == "blue")? BLUE: RED, node);
		}
		if(node->tower().size()) {
			CharacterSP tower = spawnCharacter("tower", (node->tower() == "blue")? BLUE: RED, node);
			tower->setAi<TowerAi>();
		}
	}

	// Starts the game with a description of the environement
	_execCommand("look");
}
