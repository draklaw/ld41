/*
 *  Copyright (C) 2015, 2018 the authors (see AUTHORS)
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


#include <lair/meta/property.h>

#include <lair/render_gl3/texture_set.h>

#include <lair/ec/collision_component.h>

#include "main_state.h"
#include "splash_state.h"

#include "game.h"


GameConfig::GameConfig()
	: GameConfigBase()
{
}

void GameConfig::setFromArgs(int& argc, char** argv) {
	GameConfigBase::setFromArgs(argc, argv);
}

const PropertyList& GameConfig::properties() const {
	return GameConfig::staticProperties();
}

const PropertyList& GameConfig::staticProperties() {
	return GameConfigBase::staticProperties();
}



Game::Game(int argc, char** argv)
    : GameBase(argc, argv),
      _mainState(),
      _splashState() {
	serializer().registerType<Shape2D>();
	serializer().registerType<Shape2DVector>();
	serializer().registerType<TextureSetCSP>(
	    [this](TextureSetCSP& value, const Variant& var, Logger& logger) {
		    return varRead(value, var, _renderer, _loader.get(), logger);
	    },
	    static_cast<bool(*)(Variant&, const TextureSetCSP&, Logger&)>(varWrite)
	);
}


Game::~Game() {
}


void Game::initialize() {
	GameBase::initialize(_config);

	window()->setUtf8Title(PROJECT_NAME);

	_splashState.reset(new SplashState(this));
	_mainState.reset(new MainState(this));

	_splashState->initialize();
	_splashState->setNextState(_mainState.get());
	_splashState->addSplash("TitleScreen.png");

	_mainState->initialize();
}


void Game::shutdown() {
	_mainState->shutdown();
	_splashState->shutdown();

	// Required to ensure everything is freed
	_splashState.reset();
	_mainState.reset();

	GameBase::shutdown();
}


GameConfig& Game::config() {
	return _config;
}


SplashState* Game::splashState() {
	return _splashState.get();
}


MainState* Game::mainState() {
	return _mainState.get();
}
