#include "Game.h"
namespace MyGame {
	bool Game::Initialize() {
		m_gameplayState.Enter();
		return m_gameplayState.IsReady();
	}
	void Game::Update(float deltaTime) {
		m_gameplayState.Update(deltaTime);
	}
	void Game::Shutdown() {
		m_gameplayState.Exit();
	}
	void Game::Render() {
		m_gameplayState.Render();
	}
}
