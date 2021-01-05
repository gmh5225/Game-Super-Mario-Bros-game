#include "Turtle.h"

#include "Movement.h"
#include "Size.h"
#include "Position.h"
#include "CollisionHandling.h"
#include "SDL_Utility.h"
#include "World.h"
#include "LayoutStyle.h"
#include "SoundController.h"
#include "Player.h"


std::array<SDL_Surface*, 8> Turtle::turtleImages;

int Turtle::computeImageIndex() const
{
	int baseIndex = (World::LAYOUT_STYLE == LayoutStyle::OpenWorld ? 0 : 4);
	if (movement.getDirection() == Direction::Right) {
		baseIndex += 2;
	}

	return baseIndex + (model - 1);
}

void Turtle::changeModel()
{
	++changeModelCounter;
	if (changeModelCounter % 30 == 0) {
		model += (model & 1 ? 1 : -1);
	}
}

Turtle::Turtle(Position position)
{
	size = Size(26, 44);
	movement = Movement(1, 3, Direction::None);
	this->position = position;
	healthPoints = 1;
	model = 1;
	stepsCounter = 0;
	changeModelCounter = 0;
}

void Turtle::loadTurtleImages(SDL_Surface* display)
{
	turtleImages[0] = loadPNG("./img/npc_imgs/turtle_left1.png", display);
	turtleImages[1] = loadPNG("./img/npc_imgs/turtle_left2.png", display);
	turtleImages[2] = loadPNG("./img/npc_imgs/turtle_right1.png", display);
	turtleImages[3] = loadPNG("./img/npc_imgs/turtle_right2.png", display);
	turtleImages[4] = loadPNG("./img/npc_imgs/turtle_left3.png", display);
	turtleImages[5] = loadPNG("./img/npc_imgs/turtle_left4.png", display);
	turtleImages[6] = loadPNG("./img/npc_imgs/turtle_right3.png", display);
	turtleImages[7] = loadPNG("./img/npc_imgs/turtle_right4.png", display);
}

bool Turtle::shouldStartMoving(const Player &player) const
{
	if (movement.getDirection() == Direction::None && position.getX() < player.getX() + 480) {
		return true;
	}

	return false;
}

bool Turtle::isResistantToCollisionWithShell() const
{
	return false;
}

bool Turtle::isResistantToCollisionWithBlock() const
{
	return false;
}

void Turtle::draw(SDL_Surface* display, int beginningOfCamera, int endOfCamera) const
{
	if (isWithinRangeOfCamera(beginningOfCamera, endOfCamera)) {
		SDL_Surface* turtleImg = turtleImages[computeImageIndex()];
		drawSurface(display, turtleImg, position.getX() - beginningOfCamera, position.getY());
	}
}

void Turtle::move(World &world)
{
	if (movement.getDirection() != Direction::None && stepsCounter % 3 == 0) {
		if (isCharacterStandingOnSomething(*this, world)) {
			moveHorizontally(world);
			changeModel();
		}
		else {
			moveDiagonally(world);
		}
	}
	++stepsCounter;
}

void Turtle::startMoving()
{
	movement.setDirection(Direction::Left);
}

void Turtle::crush(World &world, int index)
{
	world.addShell(Position(position.getX(), position.getY() + 6));
	world.deleteNpc(index);

	SoundController::playEnemyDestroyedEffect();
}

void Turtle::destroy(World &world, Direction direction)
{
	world.addDestroyedTurtle(position, direction);
}