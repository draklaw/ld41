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
#include <lair/core/parse.h>

#include "console.h"


using namespace lair;


String Console::inputPrefix = "> ";


Console::Console()
    : _cursorPos(inputPrefix.size())
    , _inputSize(inputPrefix.size())
    , _input(inputPrefix)
{
}


unsigned Console::cursorPos() const {
	return _cursorPos;
}


void Console::setCursorPos(unsigned pos) {
	lairAssert(pos < _inputSize + 1);
	_cursorPos = pos;
}


unsigned Console::lineCount() const {
	return _lines.size();
}


const String& Console::line(unsigned i) const {
	return _lines.at(i);
}


void Console::writeLine(const String& line) {
	auto it  = line.begin();
	auto end = line.end();
	auto lineBegin = it;

	for(; it != end; ++it) {
		if(*it == '\n') {
			_lines.emplace_back(lineBegin, it);
			if(onAddLine)
				onAddLine(_lines.back());
			lineBegin = it + 1;
		}
	}

	_lines.emplace_back(lineBegin, it);
	if(onAddLine)
		onAddLine(_lines.back());
}


const String& Console::input() const {
	return _input;
}


const Console::ExecCommand& Console::execCommand() const {
	return _execCommand;
}


void Console::setExecCommand(const ExecCommand& execCommand) {
	_execCommand = execCommand;
}


void Console::inputText(const String& text) {
	auto it = nextCharacter(_input, _input.begin(), _cursorPos);
	unsigned count = charCount(text);
	_input.insert(it, text.begin(), text.end());
	_inputSize += count;
	_cursorPos += count;

	if(onUpdateInput)
		onUpdateInput(_input);

//	int index = nextCharacter(_input, 0, _cursorPos);
//	dbgLogger.warning("Input: ", _input.substr(0, index), "#", _input.substr(index));
}


void Console::backspace() {
	auto end   = nextCharacter(_input, _input.begin(), _cursorPos);
	auto begin = prevCharacter(_input, end);

	if(begin != end && begin - _input.begin() >= int( inputPrefix.size())) {
		_input.erase(begin, end);
		_cursorPos -= 1;
		if(onUpdateInput)
			onUpdateInput(_input);
	}

//	int index = nextCharacter(_input, 0, _cursorPos);
//	dbgLogger.warning("Input: ", _input.substr(0, index), "#", _input.substr(index));
}


void Console::moveCursor(int offset) {
	_cursorPos = clamp<int>(_cursorPos + offset, inputPrefix.size(), _inputSize);

//	int index = nextCharacter(_input, 0, _cursorPos);
//	dbgLogger.warning("Input: ", _input.substr(0, index), "#", _input.substr(index));
}


void Console::execLine() {
	writeLine(_input);
	if(_execCommand)
		_execCommand(_input.substr(inputPrefix.size()));

	_input = inputPrefix;
	_inputSize = 0;
	_cursorPos = inputPrefix.size();

	if(onUpdateInput)
		onUpdateInput(_input);
}



ConsoleView::ConsoleView(Console* console, unsigned width)
    : _console(console)
    , _width(width)
{
	using namespace std::placeholders;

	_console->onAddLine     = std::bind(&ConsoleView::_addLine,     this, _1);
	_console->onUpdateInput = std::bind(&ConsoleView::_updateInput, this, _1);

	_inputLines.emplace_back();

	_updateInput(_console->input());
}


unsigned ConsoleView::lineCount() const {
	return _viewLines.size() + _inputLines.size();
}


String ConsoleView::text(unsigned lineCount, int offset, Vector2i* cursor) const {
	unsigned maxOffset = this->lineCount() - lineCount;
	if(offset < 0) {
		offset = std::max(0, int(maxOffset) + offset + 1);
	}

	if(cursor)
		*cursor = Vector2i(-1, -1);

	String text;
	int end = std::min(offset + int(lineCount), int(_viewLines.size()));
	int i = offset;
	for(; i < end; ++i) {
		text.push_back('\n');
		text.append(_viewLines[i]);
	}

	i -= _viewLines.size();
	unsigned inputPos = 0;
	unsigned consoleCursor = _console->cursorPos();
	for(; i < int(_inputLines.size()); ++i) {
		text.push_back('\n');
		text.append(_inputLines[i]);

		unsigned size = charCount(_inputLines[i]);
		if(cursor && inputPos <= consoleCursor) {
			*cursor = Vector2i(consoleCursor - inputPos,
			                   i + _viewLines.size() - offset);
		}
		inputPos += size + 1;
	}

	end = offset + lineCount - _viewLines.size();
	for(; i < end; ++i)
		text.push_back('\n');

	return text;
}

void ConsoleView::_addLine(const String& line) {
	_viewFromConsole.emplace_back(_viewLines.size());
	_appendLines(_viewLines, line);
}


void ConsoleView::_updateInput(const String& input) {
	_inputLines.clear();
	_appendLines(_inputLines, input);
}


void ConsoleView::_appendLines(StringDeque& lines, const String& line) {
	auto it        = line.begin();
	auto end       = line.end();

	if(it == end) {
		lines.emplace_back();
	}

	while(it != end) {
		auto lineBegin = it;
		auto lineEnd   = it;
		unsigned count = 0;
		while(it != end && count < _width) {
			if(std::isspace(*it))
				lineEnd = it;
			it = nextCharacter(line, it);
			count += 1;
		}

		if(it == end || lineEnd == lineBegin) {
			lineEnd = it;
		}
		else {
			it = nextCharacter(line, lineEnd);
		}

		unsigned b = lineBegin - line.begin();
		unsigned e = lineEnd - line.begin();
		lines.emplace_back(line.substr(b, e - b));
	}
}
