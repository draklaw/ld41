/*
 *  Copyright (C) 2017-2018 the authors (see AUTHORS)
 *
 *  This file is part of ld41.
 *
 *  Kitten Keeper is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kitten Keeper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kitten Keeper.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef LD_41_MAIN_STATE_H_
#define LD_41_MAIN_STATE_H_


#include <lair/core/signal.h>

#include <lair/utils/game_state.h>
#include <lair/utils/interp_loop.h>
#include <lair/utils/input.h>
#include <lair/utils/tile_map.h>

#include <lair/render_gl3/orthographic_camera.h>
#include <lair/render_gl3/render_pass.h>

#include <lair/ec/entity.h>
#include <lair/ec/entity_manager.h>
#include <lair/ec/collision_component.h>
#include <lair/ec/sprite_component.h>
#include <lair/ec/bitmap_text_component.h>
#include <lair/ec/tile_layer_component.h>

#include "console.h"


using namespace lair;


class Game;
class MainState;

enum {
	TICKS_PER_SEC  = 60,
	FRAMES_PER_SEC = 60,
	VIEW_WIDTH     = 1920,
	VIEW_HEIGHT    = 1080,
};

extern const float TICK_LENGTH_IN_SEC;

typedef int (*Command)(MainState* state, EntityRef self, int argc, const char** argv);
typedef std::unordered_map<std::string, Command> CommandMap;

struct CommandExpr {
	String    command;
	EntityRef self;
};
typedef std::deque<CommandExpr> CommandList;


class MainState : public GameState {
public:
	MainState(Game* game);
	virtual ~MainState();

	virtual void initialize();
	virtual void shutdown();

	virtual void run();
	virtual void quit();

	Game* game();

	void exec(const std::string& cmd, EntityRef self = EntityRef());
	void exec(const CommandList& commands);
	void execNext();
	int execSingle(const std::string& cmd, EntityRef self = EntityRef());
	int exec(int argc, const char** argv, EntityRef self = EntityRef());

	void loadSound(const Path& sound);
	void playSound(const Path& sound);
	void loadMusic(const Path& sound);
	void playMusic(const Path& music);

	EntityRef getEntity(const String& name, const EntityRef& ancestor = EntityRef());

	void startGame();
	void stopGame();

	void keyDown(unsigned scancode, unsigned keycode, uint16 mod,
	             bool pressed, bool repeat);
	void keyUp(unsigned scancode, unsigned keycode, uint16 mod,
	           bool pressed, bool repeat);

	void updateTick();
	void updateFrame();

	void resizeEvent();

	bool loadEntities(const Path& path, EntityRef parent = EntityRef(),
	                  const Path& cd = Path());

public:
	// More or less system stuff

	RenderPass                 _mainPass;
	RenderPass                 _guiPass;

	EntityManager              _entities;
	SpriteRenderer             _spriteRenderer;

	SpriteComponentManager     _sprites;
	CollisionComponentManager  _collisions;
	BitmapTextComponentManager _texts;
	TileLayerComponentManager  _tileLayers;

	InputManager               _inputs;

	Console     _console;
	ConsoleView _consoleView;

	SlotTracker _slotTracker;

	OrthographicCamera _camera;

	bool        _initialized;
	bool        _running;
	InterpLoop  _loop;
	int64       _fpsTime;
	unsigned    _fpsCount;

	CommandMap  _commands;
	CommandList _commandList;

	Input*      _quitInput;
	Input*      _leftInput;
	Input*      _rightInput;
	Input*      _downInput;
	Input*      _upInput;
	Input*      _okInput;

	EntityRef   _models;
	EntityRef   _scene;

	EntityRef   _view;
	EntityRef   _map;
	EntityRef   _statsText;
	EntityRef   _text;
	EntityRef   _cursor;
};


#endif
