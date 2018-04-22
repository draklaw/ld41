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

#include "text_moba.h"


using namespace lair;


MapNode* MapNode::destination(const String& direction) const {
	for(const auto& pair: paths) {
		for(const String& dir: pair.second) {
			if(dir == direction) {
				return pair.first;
			}
		}
	}
	return nullptr;
}



TMCommand::TMCommand(TextMoba* textMoba)
    : _textMoba(textMoba)
{
}


TMCommand::~TMCommand() {
}

const StringVector& TMCommand::names() const {
	return _names;
}


const lair::String& TMCommand::desc() const {
	return _desc;
}


TextMoba* TMCommand::tm() {
	return _textMoba;
}


Character* TMCommand::player() {
	return _textMoba->player();
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
	_addCommand<GoCommand>();
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


MapNode* TextMoba::mapNode(const String& id) {
	auto it = _nodes.find(id);
	if(it == _nodes.end())
		return nullptr;
	return it->second.get();
}


Character* TextMoba::player() {
	return _player.get();
}


void TextMoba::moveCharacter(Character* character, MapNode* dest) {
	character->position = dest;
}


void TextMoba::nextTurn() {
	_console->writeLine("Next turn !");
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
	ErrorList errors;
	LdlParser parser(&in, logicPath.utf8String(), &errors, LdlParser::CTX_MAP);

	Variant config;
	if(!ldlRead(parser, config)) {
		dbgLogger.error("Failed to load gameplay data from \"",
		                logicPath.utf8String(), "\"");
		errors.log(dbgLogger);
	}
	errors.log(dbgLogger);

	Variant motd = config.get("motd");
	if(motd.isString()) {
		_console->writeLine(motd.asString());
	}

	Variant nodes = config.get("nodes");
	if(nodes.isVarMap()) {
		for(const auto& pair: nodes.asVarMap()) {
			const String& id = pair.first;
			const Variant& obj = pair.second;

			MapNodeSP node = std::make_shared<MapNode>();

			node->id = id;

			const Variant& nameVar = obj.get("name");
			if(nameVar.isString())
				node->name = nameVar.asString();
			else
				dbgLogger.error("Node without name");

			const Variant& imageVar = obj.get("image");
			if(imageVar.isString())
				node->image = imageVar.asString();
			else
				dbgLogger.error("Node without image");

			const Variant& posVar = obj.get("position");
			if(posVar.isVarList() && posVar.asVarList().size() == 2) {
				const VarList& pos = posVar.asVarList();
				node->pos = Vector2(pos[0].asFloat(), pos[1].asFloat());
			}
			else
				dbgLogger.error("Node without position");

			_nodes.emplace(node->id, node);
		}
	}
	else {
		dbgLogger.error("Expected \"nodes\" VarMap.");
	}

	Variant paths = config.get("paths");
	if(paths.isVarList()) {
		for(const Variant& path: paths.asVarList()) {
			const Variant& fromVar     = path.get("from");
			const Variant& toVar       = path.get("to");
			const Variant& fromDirsVar = path.get("from_dirs");
			const Variant& toDirsVar   = path.get("to_dirs");
			if(fromVar.isString() && toVar.isString() &&
			        fromDirsVar.isVarList() && toDirsVar.isVarList()) {
				MapNode* from = mapNode(fromVar.asString());
				MapNode* to   = mapNode(toVar.asString());

				StringVector& fromDirs =
				        from->paths.emplace(to,   StringVector()).first->second;
				StringVector& toDirs =
				        to  ->paths.emplace(from, StringVector()).first->second;

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

	_player = std::make_shared<Character>();

	_player->position = mapNode("bf");

	_execCommand("look");
}
