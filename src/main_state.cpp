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


#include <functional>
#include <iomanip>

#include <lair/core/json.h>

#include "game.h"
#include "splash_state.h"

#include "map_node.h"
#include "character_class.h"
#include "character.h"
#include "skill.h"

#include "main_state.h"


#define ONE_SEC (1000000000)


const float TICK_LENGTH_IN_SEC = 1.f / float(TICKS_PER_SEC);

void dumpEntityTree(Logger& log, EntityRef e, unsigned indent = 0) {
	log.info(std::string(indent * 2u, ' '), e.name(), ": ", e.isEnabled(), ", ", e.position3().transpose());
	EntityRef c = e.firstChild();
	while(c.isValid()) {
		dumpEntityTree(log, c, indent + 1);
		c = c.nextSibling();
	}
}


MainState::MainState(Game* game)
	: GameState(game),

      _mainPass(renderer()),
      _guiPass(renderer()),

      _entities(log(), _game->serializer()),
      _spriteRenderer(loader(), renderer()),

      _sprites(assets(), loader(), &_mainPass, &_spriteRenderer),
      _collisions(),
      _texts(loader(), &_mainPass, &_spriteRenderer),
      _tileLayers(loader(), &_mainPass, &_spriteRenderer),

      _inputs(sys(), &log()),

      _console(),
      _consoleView(&_console, 60, 36),

      _camera(),

      _initialized(false),
      _running(false),
      _loop(sys()),
      _fpsTime(0),
      _fpsCount(0),

      _quitInput(nullptr),
      _leftInput(nullptr),
      _rightInput(nullptr),
      _downInput(nullptr),
      _upInput(nullptr),
      _okInput(nullptr),

      _textMoba(this, &_console)
{
	_entities.registerComponentManager(&_sprites);
	_entities.registerComponentManager(&_collisions);
	_entities.registerComponentManager(&_texts);
	_entities.registerComponentManager(&_tileLayers);
}


MainState::~MainState() {
}


void MainState::initialize() {
	using namespace std::placeholders;

	srand(time(nullptr));

	_loop.reset();
	_loop.setTickDuration(    ONE_SEC /  TICKS_PER_SEC);
//	_loop.setFrameDuration(   ONE_SEC /  FRAMES_PER_SEC);
	_loop.setFrameDuration(   0);
	_loop.setMaxFrameDuration(_loop.frameDuration() * 3);
	_loop.setFrameMargin(     _loop.frameDuration() / 2);

	window()->onResize.connect(std::bind(&MainState::resizeEvent, this))
	        .track(_slotTracker);

	game()->sys()->onKeyDown   = std::bind(&MainState::keyDown, this, _1, _2, _3, _4, _5);
	game()->sys()->onKeyUp     = std::bind(&MainState::keyUp,   this, _1, _2, _3, _4, _5);
	game()->sys()->onTextInput = std::bind(&Console::inputText, &_console, _1);

	_quitInput  = _inputs.addInput("quit");
	_leftInput  = _inputs.addInput("left");
	_rightInput = _inputs.addInput("right");
	_downInput  = _inputs.addInput("down");
	_upInput    = _inputs.addInput("up");
	_okInput    = _inputs.addInput("ok");

	_inputs.mapScanCode(_quitInput,  SDL_SCANCODE_ESCAPE);
	_inputs.mapScanCode(_leftInput,  SDL_SCANCODE_LEFT);
	_inputs.mapScanCode(_rightInput, SDL_SCANCODE_RIGHT);
	_inputs.mapScanCode(_downInput,  SDL_SCANCODE_DOWN);
	_inputs.mapScanCode(_upInput,    SDL_SCANCODE_UP);
	_inputs.mapScanCode(_okInput,    SDL_SCANCODE_SPACE);
	_inputs.mapScanCode(_okInput,    SDL_SCANCODE_RETURN);
	_inputs.mapScanCode(_okInput,    SDL_SCANCODE_RETURN2);

	loadEntities("entities.ldl", _entities.root());

	_models       = _entities.findByName("__models__");
	_mapIconModel = _entities.findByName("map_icon", _models);

	_scene        = _entities.findByName("scene");
	_view         = _entities.findByName("fp_view");
	_map          = _entities.findByName("map");
	_statsText    = _entities.findByName("stats_text");
	_text         = _entities.findByName("text");
	_cursor       = _entities.findByName("cursor");

//	loadSound("kittendeath.wav");

//	loadMusic("ending.mp3");

//	AssetSP font = loader()->loadAsset<BitmapFontLoader>("droid_sans_24.json");

	_textMoba.initialize("gameplay.ldl");

	loader()->waitAll();

	// Set to true to debug OpenGL calls
//	renderer()->context()->setLogCalls(true);

	_initialized = true;
}


void MainState::shutdown() {
	_slotTracker.disconnectAll();

	_initialized = false;
}


void MainState::run() {
	lairAssert(_initialized);

	log().log("Starting main state...");
	_running = true;
	_loop.start();
	_fpsTime  = int64(sys()->getTimeNs());
	_fpsCount = 0;

	startGame();

	do {
		switch(_loop.nextEvent()) {
		case InterpLoop::Tick:
			updateTick();
			break;
		case InterpLoop::Frame:
			updateFrame();
			break;
		}
	} while (_running);
	_loop.stop();

	stopGame();
}


void MainState::quit() {
	_running = false;
}


Game* MainState::game() {
	return static_cast<Game*>(_game);
}


void MainState::addMapIcon(CharacterSP character) {
	if(!character->isAlive() || !character->node())
		return;

	unsigned index = 5;
	if(character->type() == REDSHIRT)
		index = 4;
	else if(character->cClass()->id() == "tower")
		index = 3;
	else if(character->cClass()->id() == "ranger")
		index = 0;
	else if(character->cClass()->id() == "warrior")
		index = 1;
	else if(character->cClass()->id() == "mage")
		index = 2;

	if(index == 5)
		return;

	EntityRef e = _entities.cloneEntity(_mapIconModel, _map);
	e.placeAt(character->node()->pos());
	e.updateWorldTransformRec();

	SpriteComponent* s = _sprites.get(e);
	s->setTileIndex(index);
	s->setColor((character->team() == BLUE)?
	                Vector4(.2, .2, .8, 1):
	                Vector4(.8, .2, .2, 1));

	_mapCharMap[character->node().get()].push_back(e);
}


void MainState::exec(const std::string& cmds, EntityRef self) {
	CommandList commands;
	unsigned first = 0;
	for(unsigned ci = 0; ci <= cmds.size(); ++ci) {
		if(ci == cmds.size() || cmds[ci] == '\n' || cmds[ci] == ';') {
			commands.emplace_back(CommandExpr{ String(cmds.begin() + first, cmds.begin() + ci), self });
			first = ci + 1;
		}
	}
	exec(commands);
}


void MainState::exec(const CommandList& commands) {
	bool execNow = _commandList.empty();
	_commandList.insert(_commandList.begin(), commands.begin(), commands.end());
	if(execNow)
		execNext();
}


void MainState::execNext() {
	while(!_commandList.empty()) {
		CommandExpr cmd = _commandList.front();
		_commandList.pop_front();
		if(execSingle(cmd.command, cmd.self) == 0)
			return;
	}
}


int MainState::execSingle(const std::string& cmd, EntityRef self) {
#define MAX_CMD_ARGS 32

	std::string tokens = cmd;
	unsigned    size   = tokens.size();
	int ret = 0;
	for(unsigned ci = 0; ci < size; ) {
		int   argc = 0;
		const char* argv[MAX_CMD_ARGS];
		while(ci < size) {
			bool endLine = false;
			while(ci < size && std::isspace(tokens[ci])) {
				endLine = (tokens[ci] == '\n') || (tokens[ci] == ';');
				tokens[ci] = '\0';
				++ci;
			}
			if(endLine)
				break;

			argv[argc] = tokens.data() + ci;
			++argc;

			while(ci < size && !std::isspace(tokens[ci])) {
				++ci;
			}
		}

		if(argc) {
			ret = exec(argc, argv, self);
		}
	}

	return ret;
}


int MainState::exec(int argc, const char** argv, EntityRef self) {
	lairAssert(argc > 0);

	std::ostringstream out;
	out << argv[0];
	for(int i = 1; i < argc; ++i)
		out << " " << argv[i];
	dbgLogger.info(out.str());

	auto cmd = _commands.find(argv[0]);
	if(cmd == _commands.end()) {
		dbgLogger.warning("Unknown command \"", argv[0], "\"");
		return -1;
	}
	return cmd->second(this, self, argc, argv);
}


void MainState::loadSound(const Path& sound) {
	loader()->load<SoundLoader>(sound);
}


void MainState::playSound(const Path& sound) {
	AssetSP asset = assets()->getAsset(sound);
	auto aspect = asset->aspect<SoundAspect>();
	aspect->_get().setVolume(game()->config().soundVolume);
	audio()->playSound(asset);
}


void MainState::loadMusic(const Path& sound) {
	loader()->load<MusicLoader>(sound);
}


void MainState::playMusic(const Path& sound) {
	AssetSP asset = assets()->getAsset(sound);
	audio()->setMusicVolume(0.5);
	audio()->playMusic(assets()->getAsset(sound));
}


EntityRef MainState::getEntity(const String& name, const EntityRef& ancestor) {
	EntityRef entity = _entities.findByName(name, ancestor);
	if(!entity.isValid()) {
		log().error("Entity \"", name, "\" not found.");
	}
	return entity;
}


void MainState::startGame() {
	game()->sys()->startTextInput();
	//audio()->playMusic(assets()->getAsset("music.ogg"));
	//audio()->playSound(assets()->getAsset("sound.ogg"), 2);
}


void MainState::stopGame() {
	game()->sys()->stopTextInput();
}


void MainState::keyDown(unsigned scancode, unsigned /*keycode*/, uint16 /*mod*/,
                        bool /*pressed*/, bool /*repeat*/) {
	switch(scancode) {
	case SDL_SCANCODE_RETURN:
	case SDL_SCANCODE_RETURN2:
	case SDL_SCANCODE_KP_ENTER:
		_console.execLine();
		break;
	case SDL_SCANCODE_BACKSPACE:
		_console.backspace();
		break;
	case SDL_SCANCODE_LEFT:
		_console.moveCursor(-1);
		break;
	case SDL_SCANCODE_RIGHT:
		_console.moveCursor(1);
		break;
	case SDL_SCANCODE_PAGEUP:
		_consoleView.scroll(-3);
		break;
	case SDL_SCANCODE_PAGEDOWN:
		_consoleView.scroll(3);
		break;
	}
}


void MainState::keyUp(unsigned /*scancode*/, unsigned /*keycode*/, uint16 /*mod*/,
                      bool /*pressed*/, bool /*repeat*/) {
}


void MainState::updateTick() {
	loader()->finalizePending();

	_inputs.sync();

	_entities.setPrevWorldTransforms();

	if(_quitInput->justPressed()) {
		quit();
	}


	_entities.updateWorldTransforms();
}


String skillDesc(CharacterSP c) {
	std::ostringstream out;
	for(SkillSP skill: c->skills()) {
		out << skill->name() << ": " << skill->manaCost() << " mp";
		if(skill->timeBeforeNextUse())
			out << " (" << skill->timeBeforeNextUse() << "t)";
		out << "\n";
	}
	return out.str();
}

void MainState::updateFrame() {
	// Update

	BitmapTextComponent* text = _texts.get(_text);
	if(text) {
		Vector2i cursor(-1, -1);
		text->setText(_consoleView.text(&cursor));

		const BitmapFont& font = text->font()->get();

		Vector3 pos(
		    cursor(0) * font.glyph('m').advance,
		    -cursor(1) * font.height() + 1020,
		    0.1
		);
		_cursor.setEnabled(cursor(0) >= 0);
		_cursor.placeAt(pos);
		_cursor.computeWorldTransform();
	}

	CharacterSP player = _textMoba.player();
	String stats = cat(
	    "lvl ", player->level() + 1, " ", player->teamName(), " ", player->className(), "\n",
	    "\n",
	    "hp:   ", std::setw(4), player->hp(),   " / ", player->maxHP(), "\n",
	    "mana: ", std::setw(4), player->mana(), " / ", player->maxMana(), "\n",
	    "xp:   ", std::setw(4), player->xp(),   " / ", _textMoba.nextLevel(player), "\n",
	    "\n",
	    "Skills:\n",
	    skillDesc(player),
	    "\n",
	    "TODO: alies info\n"
	);
	BitmapTextComponent* statsText = _texts.get(_statsText);
	if(statsText) {
		statsText->setText(stats);
	}


	_mapCharMap.clear();
	while(_map.firstChild().isValid()) {
		_map.firstChild().destroy();
	}
	for(CharacterSP c: _textMoba.characters()) {
		addMapIcon(c);
	}

	const float offset = 16;
	for(const auto& pair: _mapCharMap) {
		const EntityVector& entities = pair.second;

		if(entities.size() < 2)
			continue;

		int width = std::ceil(std::sqrt(entities.size()));
		int height = (entities.size() - 1) / width + 1;
		Vector2 base = entities[0].position2() - Vector2(width - 1, -height + 1) * offset / 2;

		int i = 0;
		for(EntityRef e: entities) {
			e.placeAt(Vector2(base + Vector2(i % width, -i / width) * offset));
			i += 1;
		}
	}


	// Rendering
	Context* glc = renderer()->context();

	_texts.createTextures();
	_tileLayers.createTextures();
	renderer()->uploadPendingTextures();
	_spriteRenderer.finalizeShaders();

	glc->clear(gl::COLOR_BUFFER_BIT | gl::DEPTH_BUFFER_BIT);

	bool buffersFilled = false;
	while(!buffersFilled) {
		_mainPass.clear();
		_guiPass.clear();

		_spriteRenderer.beginRender();

		_sprites.render(_entities.root(), _loop.frameInterp(), _camera);
		_texts.render(_entities.root(), _loop.frameInterp(), _camera);
		_tileLayers.render(_entities.root(), _loop.frameInterp(), _camera);

		buffersFilled = _spriteRenderer.endRender();
	}

	_mainPass.render();

	glc->disable(gl::DEPTH_TEST);
	_guiPass.render();
	glc->enable(gl::DEPTH_TEST);


	window()->swapBuffers();
//	glc->setLogCalls(true);

	int64 now = int64(sys()->getTimeNs());
	++_fpsCount;
	int64 etime = now - _fpsTime;
	if(etime >= ONE_SEC) {
		log().info("Fps: ", _fpsCount * float(ONE_SEC) / etime);
		_fpsTime  = now;
		_fpsCount = 0;
	}
}


void MainState::resizeEvent() {
	Box3 viewBox(Vector3(0, 0, 0),
	             Vector3(VIEW_WIDTH,
	                     VIEW_HEIGHT, 1));
	_camera.setViewBox(viewBox);
}


bool MainState::loadEntities(const Path& path, EntityRef parent, const Path& cd) {
	Path localPath = makeAbsolute(cd, path);
	log().info("Load entity \"", localPath, "\"");

	Path realPath = game()->dataPath() / localPath;
	Path::IStream in(realPath.native().c_str());
	if(!in.good()) {
		log().error("Unable to read \"", localPath, "\".");
		return false;
	}
	ErrorList errors;
	LdlParser parser(&in, localPath.utf8String(), &errors, LdlParser::CTX_MAP);

	bool success = _entities.loadEntitiesFromLdl(parser, parent);

	errors.log(log());

	return success;
}
