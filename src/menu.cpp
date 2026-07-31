

#include "menu.hpp"
#include "scene.hpp"
#include <cstdint>

Menu::Menu(Font *font, Arena *arena) : Scene(font, *arena) {}

void Menu::draw(IVector2 *resolution) { Scene::draw(resolution); }

void Menu::update(IVector2 *resolution) { Scene::update(resolution); }

void Menu::init() { Scene::init(); }

void Menu::input() {}

void Menu::resetCameraPos() {}
void Menu::resetScene() {}
void Menu::updateMode(int main, int sec) {}
void Menu::setHoverState(bool hover, uint32_t node_id) {}

void Menu::createAlgorithmInstance(AlgorithmId) {}

void Menu::switchAlgorithm(AlgorithmId) {}
