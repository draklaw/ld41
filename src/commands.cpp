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


#include <functional>

#include <lair/core/log.h>
#include <lair/core/text.h>

#include "map_node.h"
#include "character_class.h"
#include "character.h"
#include "text_moba.h"

#include "commands.h"


using namespace lair;


template<typename It>
String join(It begin, It end, const String& joinString = ", ") {
	std::ostringstream out;
	for(It it = begin; it < end; ++it) {
		if(it != begin)
			out << joinString;
		out << *it;
	}
	return out.str();
}

template<typename Range>
String join(const Range& range, const String& joinString = ", ") {
	return join(range.begin(), range.end(), joinString);
}


String toLower(const String& string) {
	char buffer[8];

	String lower;
	lower.reserve(string.size());

	Utf8CodepointIterator cpit(string);
	while(cpit.hasNext()) {
		*utf8FromCodepoint(buffer, std::tolower(cpit.next())) = '\0';
		lower.append(buffer);
	}

	return lower;
}


HelpCommand::HelpCommand(TextMoba* textMoba)
    : TMCommand(textMoba)
{
	_names.emplace_back("help");
	_names.emplace_back("h");
	_names.emplace_back("?");

	_desc = "  Prints this help message.";
}

void HelpCommand::exec(const StringVector& /*args*/) {
	for(auto cmd: tm()->commands()) {
		print(join(cmd->names()));
		print(cmd->desc());
	}
}


LookCommand::LookCommand(TextMoba* textMoba)
    : TMCommand(textMoba)
{
	_names.emplace_back("look");
	_names.emplace_back("l");

	_desc = "  Look around you. Describe the place and what / who is\n"
	        "  here. With a parameter, look at a specific object here.\n"
	        "  Example: look tower (describe a tower)";
}

void LookCommand::exec(const StringVector& args) {
	if(args.size() == 1) {
		MapNodeSP node = player()->node();
		print("You are at ", node->name, ".");

		if(node->characters.size() < 2) {
			print("There is nothing here.");
		}
		else {
			print("Here, there is");
			unsigned i = 0;
			for(CharacterSP c: node->characters) {
				if(c == player())
					continue;
				i += 1;
				print("  ", i, ": a level ", c->level() + 1, " ",
				      c->teamName(), " ", c->className(), " (",
				      c->hp(), " / ", c->maxHP(), ")");
			}
		}
	}
}


DirectionsCommand::DirectionsCommand(TextMoba* textMoba)
    : TMCommand(textMoba)
{
	_names.emplace_back("directions");
	_names.emplace_back("dir");
	_names.emplace_back("d");

	_desc = "  List the destinations you can reach from here.";
}

void DirectionsCommand::exec(const StringVector& /*args*/) {
	print("From here, you can go toward:");
	for(const auto& pair: player()->node()->paths) {
		print("  ", join(pair.second), ": toward ", pair.first->name);
	}
}


GoCommand::GoCommand(TextMoba* textMoba)
    : TMCommand(textMoba)
{
	_names.emplace_back("go");
	_names.emplace_back("g");

	_desc = "  Walk in a given direction. Type \"directions\" to see where\n"
	        "  you can go. Example: go red (go toward the red base)";
}

void GoCommand::exec(const StringVector& args) {
	if(args.size() != 2) {
		print("I don't understand where you want to go. Type");
		print("  ", args[0], " <direction>");
	}
	else {
		String dir = toLower(args[1]);
		MapNodeSP node = player()->node();
		MapNodeSP dest = node->destination(dir);
		if(dest) {
			tm()->moveCharacter(player(), dest);
			tm()->_execCommand("look");
			tm()->nextTurn();
		}
		else {
			print("Unknown direction \"", dir, "\"");
		}
	}
}
