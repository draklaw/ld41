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


#ifndef LD41_CONSOLE_H_
#define LD41_CONSOLE_H_


#include <vector>
#include <deque>

#include <lair/core/lair.h>


class Console {
public:
	typedef std::function<void(const lair::String&)> ExecCommand;

	typedef std::function<void(const lair::String&)> AddLineCallback;
	typedef std::function<void(const lair::String&)> UpdateInputCallback;

public:
	Console();

	unsigned cursorPos() const;
	void setCursorPos(unsigned pos);

	unsigned lineCount() const;
	const lair::String& line(unsigned i) const;
	void writeLine(const lair::String& line);

	const lair::String& input() const;

	const ExecCommand& execCommand() const;
	void setExecCommand(const ExecCommand& execCommand);

	void inputText(const lair::String& text);
	void backspace();
	void moveCursor(int offset);

	void execLine();

public:
	AddLineCallback     onAddLine;
	UpdateInputCallback onUpdateInput;

private:
	typedef std::deque<lair::String> StringDeque;

private:
	unsigned     _cursorPos;
	StringDeque  _lines;
	unsigned     _inputSize;
	lair::String _input;

	ExecCommand _execCommand;
};


class ConsoleView {
public:
	ConsoleView(Console* console, unsigned width);

	unsigned lineCount() const;
	lair::String text(unsigned lineCount, int offset = -1,
	                  lair::Vector2i* cursor = nullptr) const;

	void _addLine(const lair::String& line);
	void _updateInput(const lair::String& input);

private:
	typedef std::deque<unsigned>     UnsignedDeque;
	typedef std::deque<lair::String> StringDeque;

private:
	void _appendLines(StringDeque& lines, const lair::String& line);

private:
	Console*      _console;
	unsigned      _width;
	UnsignedDeque _viewFromConsole;
	StringDeque   _viewLines;
	StringDeque   _inputLines;
};


#endif
