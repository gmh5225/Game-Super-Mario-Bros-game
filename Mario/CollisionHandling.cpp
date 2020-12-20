#include "CollisionHandling.h"

#include <vector>
#include <cmath>
#include "Player.h"
#include "SoundController.h"
#include "Block.h"
#include "Platform.h"
#include "World.h"
#include "Coin.h"
#include "Flower.h"
#include "Mushroom.h"
#include "Star.h"
#include "Shell.h"
#include "Turtle.h"
#include "RedTurtle.h"
#include "FireRocket.h"
#include "Creature.h"
#include "Plant.h"
#include "JumpingFish.h"
#include "CloudBombardier.h"
#include "FireBall.h"
#include "Boss.h"
#include "Position.h"
#include "AnimatedText.h"
#include "Screen.h"
#include "UtilityFunctions.h"
#include "WorldInteractionFunctions.h"


bool isCharacterHittingObject(const WorldObject &figure, const WorldObject &block, Direction direction, int distance)
{
	if (direction == Direction::Right && figure.getX() < block.getX()
		&& figure.getX() + distance + figure.getWidth() / 2 >= block.getX() - block.getWidth() / 2) {

		return true;
	}
	else if (direction == Direction::Left && figure.getX() > block.getX()
		&& figure.getX() - distance - figure.getWidth() / 2 <= block.getX() + block.getWidth() / 2) {

		return true;
	}
	else if (direction == Direction::Up && figure.getY() > block.getY()
		&& figure.getY() - distance - figure.getHeight() / 2 <= block.getY() + block.getHeight() / 2) {

		return true;
	}
	else if (direction == Direction::Down && figure.getY() < block.getY()
		&& figure.getY() + distance + figure.getHeight() / 2 >= block.getY() - block.getHeight() / 2) {

		return true;
	}

	return false;
}

bool isCharacterStandingOnSomething(const WorldObject &figure, const World &world)
{
	std::vector<Block> blocks = world.getBlocks();
	for (auto &block : blocks) {
		if (isElementDirectlyAboveObject(figure, block) && areAtTheSameWidth(figure, block) 
			&& !block.isInvisible()) {

			return true;
		}
	}

	std::vector<Platform> platforms = world.getPlatforms();
	for (auto &platform : platforms) {
		if (isElementDirectlyAboveObject(figure, platform) && areAtTheSameWidth(figure, platform)) {
			return true;
		}
	}

	return false;
}

bool isMonsterStandingOnBlock(const LivingObject &monster, const Block &block)
{
	if (isMonsterCloseAboveBlock(monster, block) && areAtTheSameWidth(monster, block)) {
		return true;
	}

	return false;
}

bool isMushroomStandingOnBlock(const World &world, const Block &block)
{
	std::vector<std::shared_ptr<BonusObject>> bonusElements = world.getBonusElements();
	for (auto &bonusElement : bonusElements) {
		if (std::dynamic_pointer_cast<Mushroom>(bonusElement)) {
			if (isElementDirectlyAboveObject(*bonusElement, block) && areAtTheSameWidth(*bonusElement, block)) {
				return true;
			}
		}
	}

	return false;
}

bool isPlayerCloseToFireRocket(const FireRocket &fireRocket, const World &world)
{
	const Player& player = world.getPlayer();

	if (abs(player.getX() - fireRocket.getX()) < 350) {
		return true;
	}

	return false;
}

bool isPlayerCloseToPlant(const Plant &plant, const World &world)
{
	const Player& player = world.getPlayer();
	int yDifference = plant.getY() - player.getY();

	if (abs(player.getX() - plant.getX()) < 40 && (yDifference > 30 && yDifference < 60)) {
		return true;
	}

	return false;
}

bool isPlayerAheadOfBombardier(const CloudBombardier &bombardier, const World &world)
{
	const Player& player = world.getPlayer();

	if (player.getX() > bombardier.getX()) {
		return true;
	}

	return false;
}

bool isPlayerStandingOnThisPlatform(const Player &player, const Platform &platform)
{
	if (isElementDirectlyAboveObject(player, platform) && areAtTheSameWidth(player, platform)) {
		return true;
	}

	return false;
}

bool isBlockBlockedByAnother(const Block& block, const World& world)
{
	std::vector<Block> blocks = world.getBlocks();
	for (auto &element : blocks) {
		if (block.getX() == element.getX() && block.getY() == (element.getY() + block.getHeight())) {
			return true;
		}
	}

	return false;
}

bool isPlayerJumpingOnMonster(const Player &player, const LivingObject &monster)
{
	return ((monster.getY() - player.getY() > 25) && player.isNotJumpingUp());
}

bool isMushroomStandingOnBlock(const BonusObject &mushroom, const Block &block)
{
	return (isElementDirectlyAboveObject(mushroom, block) && areAtTheSameWidth(mushroom, block));
}

void handleJumpingOnShell(std::shared_ptr<LivingObject> monster, World &world, Player &player, int index)
{
	if (std::dynamic_pointer_cast<Shell>(monster)->isActive()) {
		world.changeShellMovementParameters(index, Direction::None);
	}
	else {
		Direction direction = determineDirection(player, *monster);
		world.changeShellMovementParameters(index, direction);
	}
}

void handleJumpingOnTurtle(std::shared_ptr<LivingObject> monster, World& world, int index)
{
	world.addShell(Position((monster)->getX(), (monster)->getY() + 6));
	world.deleteMonster(index);
	SoundController::playEnemyDestroyedEffect();
}

void handleJumpingOnRedTurtle(std::shared_ptr<LivingObject> monster, World &world, int index)
{
	if (std::dynamic_pointer_cast<RedTurtle>(monster)->isFlying()) {
		std::dynamic_pointer_cast<RedTurtle>(monster)->loseFlyingAbility();
	}
	else {
		world.addShell(Position(monster->getX(), monster->getY() + 6), true);
		world.deleteMonster(index);
	}

	SoundController::playEnemyDestroyedEffect();
}

void handleJumpingOnCreature(std::shared_ptr<LivingObject> monster, World &world, Player &player, int index)
{
	addTextAndPoints(player, world, 100, Position(monster->getX(), monster->getY() - 15));
	world.addCrushedCreature(Position(monster->getX(), monster->getY() + 8));
	world.deleteMonster(index);
	SoundController::playEnemyDestroyedEffect();
}

void handleJumpingOnFish(std::shared_ptr<LivingObject> monster, World &world, Player &player, int index)
{
	addTextAndPoints(player, world, 200, Position(monster->getX(), monster->getY() - 15));
	bool directionFlag = std::dynamic_pointer_cast<JumpingFish>(monster)->isGoingLeft();
	world.addDestroyedFish(monster->getPosition(), directionFlag);
	world.deleteMonster(index);
	SoundController::playEnemyDestroyedEffect();
}

void handleJumpingOnMonster(std::shared_ptr<LivingObject> monster, World &world, Player &player, int index)
{
	player.performAdditionalJump();

	if (std::dynamic_pointer_cast<Shell>(monster)) {
		handleJumpingOnShell(monster, world, player, index);
	}
	else if (std::dynamic_pointer_cast<Turtle>(monster)) {
		handleJumpingOnTurtle(monster, world, index);
	}
	else if (std::dynamic_pointer_cast<RedTurtle>(monster)) {
		handleJumpingOnRedTurtle(monster, world, index);
	}
	else if (std::dynamic_pointer_cast<Creature>(monster)) {
		handleJumpingOnCreature(monster, world, player, index);
	}
	else if (std::dynamic_pointer_cast<JumpingFish>(monster)) {
		handleJumpingOnFish(monster, world, player, index);
	}
}

void handleImmortalPlayerCollisions(std::shared_ptr<LivingObject> monster, World &world, Player &player, int index)
{
	int points = 200;
	Direction direction = determineDirection(player, *monster);

	if (std::dynamic_pointer_cast<Turtle>(monster) || (std::dynamic_pointer_cast<Shell>(monster)
		&& std::dynamic_pointer_cast<Shell>(monster)->isActive())) {

		world.addDestroyedTurtle(monster->getPosition(), direction, std::dynamic_pointer_cast<Shell>(monster)->isRed());
	}
	else if (std::dynamic_pointer_cast<Creature>(monster)) {
		world.addDestroyedCreature(monster->getPosition(), direction);
		points = 100;
	}
	else if (std::dynamic_pointer_cast<RedTurtle>(monster)) {
		world.addDestroyedTurtle(monster->getPosition(), direction, true);
	}
	else if (std::dynamic_pointer_cast<JumpingFish>(monster)) {
		bool directionFlag = std::dynamic_pointer_cast<JumpingFish>(monster)->isGoingLeft();
		world.addDestroyedFish(monster->getPosition(), directionFlag);
	}

	addTextAndPoints(player, world, points, Position(monster->getX(), monster->getY() - 15));
	world.deleteMonster(index);
	SoundController::playEnemyDestroyedEffect();
}

void handlePlayerAndMonstersCollisions(std::shared_ptr<LivingObject> monster, World &world, Player &player, int index)
{
	if (std::dynamic_pointer_cast<Shell>(monster) && !(std::dynamic_pointer_cast<Shell>(monster)->isActive())) {
		Direction direction = determineDirection(player, *monster);
		world.changeShellMovementParameters(index, direction);
		return;
	}

	if (player.isImmortal()) {
		if (!isMonsterResistantToKnocks(monster)) {
			handleImmortalPlayerCollisions(monster, world, player, index);
		}
	}
	else if (!player.isInsensitive()) {
		player.loseBonusOrLife();
	}
}

void handleCollisionsWithMonsters(Player &player, World &world)
{
	std::vector<std::shared_ptr<LivingObject>> monsters = world.getMonsters();
	for (auto it = monsters.begin(); it != monsters.end(); ++it) {
		if (areAtTheSameWidth(player, **it) && areAtTheSameHeight(player, **it)) {
			if (isPlayerJumpingOnMonster(player, **it) && !isMonsterCrushproof(*it)) {
				handleJumpingOnMonster(*it, world, player, it - monsters.begin());
			}
			else {
				handlePlayerAndMonstersCollisions(*it, world, player, it - monsters.begin());
			}
			break;
		}
	}
}

void handleCollisionsWithFireSerpents(Player &player, World &world)
{
	std::vector<FireSerpent> fireSerpents = world.getFireSerpents();
	for (const auto &fireSerpent : fireSerpents) {
		if (areAtTheSameWidth(player, fireSerpent) && areAtTheSameHeight(player, fireSerpent)) {
			if (!player.isImmortal() && !player.isInsensitive()) {
				player.loseBonusOrLife();
				break;
			}
		}
	}
}

void handlePlayerCollisions(Player &player, World &world)
{
	handleCollisionsWithMonsters(player, world);
	handleCollisionsWithFireSerpents(player, world);
}

void handleShellCollisions(const LivingObject &shell, std::shared_ptr<LivingObject> monster, World &world, int * pts)
{
	Direction direction = determineDirection(shell, *monster);

	if (std::dynamic_pointer_cast<Creature>(monster)) {
		world.addDestroyedCreature(monster->getPosition(), direction);
		*pts = 100;
	}
	else if (std::dynamic_pointer_cast<Turtle>(monster) || std::dynamic_pointer_cast<Shell>(monster)) {
		world.addDestroyedTurtle(monster->getPosition(), direction);
	}
	else if (std::dynamic_pointer_cast<RedTurtle>(monster)) {
		world.addDestroyedTurtle(monster->getPosition(), direction, true);
	}
}

void handleShellsAndMonstersCollisions(World &world, Player &player)
{
	std::vector<std::shared_ptr<LivingObject>> monsters = world.getMonsters();
	for (auto it = monsters.begin(); it != monsters.end(); ++it) {
		if (std::dynamic_pointer_cast<Shell>(*it) && std::dynamic_pointer_cast<Shell>(*it)->isActive()) {
			for (auto it2 = monsters.begin(); it2 != monsters.end(); ++it2) {
				if (!isMonsterResistantToCollisionWithShell(*it2) && (areAtTheSameWidth(**it, **it2)
					&& areAtTheSameHeight(**it, **it2))) {

					int points = 200;
					handleShellCollisions(**it, *it2, world, &points);

					world.deleteMonster(it2 - monsters.begin());
					addTextAndPoints(player, world, points, (*it2)->getPosition());
					SoundController::playEnemyDestroyedEffect();
				}
			}
		}
	}
}

void handleFireBallAndBossCollision(std::shared_ptr<LivingObject> monster, World &world, int * pts)
{
	std::dynamic_pointer_cast<Boss>(monster)->decrementHealthPoints();

	if (std::dynamic_pointer_cast<Boss>(monster)->getHealthPoints() != 0) {
		*pts = -1;
	}
	else {
		*pts = 5000;
		world.addDestroyedBoss(monster->getPosition());
	}
}

void handleFireBallCollision(const FireBall &fireball, std::shared_ptr<LivingObject> monster, World &world, int * pts)
{
	Direction direction = fireball.getMovement().getDirection();
	int alignment = (direction == Direction::Left ? -5 : 5);

	if (std::dynamic_pointer_cast<Creature>(monster)) {
		world.addDestroyedCreature(Position(monster->getX() + alignment, monster->getY()), direction);
		*pts = 100;
	}
	else if (std::dynamic_pointer_cast<Turtle>(monster) || std::dynamic_pointer_cast<Shell>(monster)) {
		world.addDestroyedTurtle(Position((monster)->getX() + alignment, (monster)->getY()), direction);
	}
	else if (std::dynamic_pointer_cast<RedTurtle>(monster)) {
		world.addDestroyedTurtle(Position(monster->getX() + alignment, monster->getY()), direction, true);
	}
	else if (std::dynamic_pointer_cast<Boss>(monster)) {
		handleFireBallAndBossCollision(monster, world, pts);
	}
	else if (std::dynamic_pointer_cast<JumpingFish>(monster)) {
		bool directionFlag = std::dynamic_pointer_cast<JumpingFish>(monster)->isGoingLeft();
		world.addDestroyedFish(monster->getPosition(), directionFlag);
	}
}

void handleFireBallDestruction(const FireBall &fireball, World &world, int fireballIndex)
{
	int alignment = (fireball.getMovement().getDirection() == Direction::Left ? -5 : 5);

	world.deleteFireBall(fireballIndex);
	world.addExplosion(Position(fireball.getX() + alignment, fireball.getY()));
}

void handleFireBallsAndMonstersCollisions(World &world, Player &player)
{
	std::vector<FireBall> fireballs = world.getFireBalls();
	std::vector<std::shared_ptr<LivingObject>> monsters = world.getMonsters();
	int index = 0;
	for (auto it = fireballs.begin(); it != fireballs.end(); ++it, ++index) {
		for (auto it2 = monsters.begin(); it2 != monsters.end(); ++it2) {
			if (!isMonsterResistantToFireBalls(*it2) && (areAtTheSameWidth(*it, **it2) 
				&& areAtTheSameHeight(*it, **it2))) {

				int points = 200;
				handleFireBallCollision(fireballs[index], *it2, world, &points);
				handleFireBallDestruction(fireballs[index], world, index);

				if (points != -1) {
					world.deleteMonster(it2 - monsters.begin());
					addTextAndPoints(player, world, points, Position((*it2)->getX(), (*it2)->getY() - 15));
					bool bossFlag = (points == 5000);
					SoundController::playEnemyDestroyedEffect(bossFlag);
				}
			}
		}
	}
}

void handleMonsterDestruction(const Block &block, std::shared_ptr<LivingObject> monster, World &world, int * pts)
{
	Direction direction = determineDirection(block, *monster);

	if (std::dynamic_pointer_cast<Creature>(monster)) {
		world.addDestroyedCreature(monster->getPosition(), direction);
		*pts = 100;
	}
	else if (std::dynamic_pointer_cast<Turtle>(monster)) {
		world.addDestroyedTurtle(monster->getPosition(), direction);
	}
	else if (std::dynamic_pointer_cast<RedTurtle>(monster)) {
		world.addDestroyedTurtle(monster->getPosition(), direction, true);
	}
}

void handleBlockAndMonstersCollisions(World &world, const Block &block, Player &player)
{
	std::vector<std::shared_ptr<LivingObject>> monsters = world.getMonsters();
	for (auto it = monsters.begin(); it != monsters.end(); ++it) {
		if (isMonsterStandingOnBlock(**it, block)) {
			int points = 200;
			handleMonsterDestruction(block, *it, world, &points);
		
			addTextAndPoints(player, world, points, Position((*it)->getX(), (*it)->getY() - 15));
			world.deleteMonster(it - monsters.begin());
			SoundController::playEnemyDestroyedEffect();
		}
	}
}

void handleBlockAndCoinsCollisions(World &world, const Block &block, Player &player)
{
	std::vector<std::shared_ptr<InanimateObject>> elements = world.getInanimateElements();
	for (auto it = elements.begin(); it != elements.end(); ++it) {
		if (isElementDirectlyAboveObject(**it, block) && areAtTheSameWidth(**it, block)) {
			collectCoinByCollision(player, world, it - elements.begin());
			return;
		}
	}
}

void handleBlockAndMushroomsCollisions(World &world, const Block &block)
{
	std::vector<std::shared_ptr<BonusObject>> bonusElements = world.getBonusElements();
	for (auto &bonusElement : bonusElements) {
		if (std::dynamic_pointer_cast<Mushroom>(bonusElement)) {
			if (isMushroomStandingOnBlock(*bonusElement, block) && block.canCollideWithMushrooms()) {
				std::dynamic_pointer_cast<Mushroom>(bonusElement)->decreasePositionY();
				std::dynamic_pointer_cast<Mushroom>(bonusElement)->setStepsUp(30);
			}
		}
	}
}

void handleBlockCollisions(World &world, const Block &block, Player &player)
{
	handleBlockAndMonstersCollisions(world, block, player);
	handleBlockAndCoinsCollisions(world, block, player);
	handleBlockAndMushroomsCollisions(world, block);
}

void collectBonusIfPossible(Player &player, World &world)
{
	std::vector<std::shared_ptr<BonusObject>> elements = world.getBonusElements();
	int i = 0;
	for (auto it = elements.begin(); it != elements.end(); ++it, ++i) {
		if (areAtTheSameWidth(player, **it) && areAtTheSameHeight(player, **it) && (*it)->isActive()) {
			if (std::dynamic_pointer_cast<Mushroom>(*it)) {
				collectMushroom(player, world, dynamic_cast<Mushroom&>(**it), i);
			}
			else if (std::dynamic_pointer_cast<Flower>(*it)) {
				collectFlower(player, world, i);
			}
			else if (std::dynamic_pointer_cast<Star>(*it)) {
				collectStar(player, world, i);
			}
		}
	}
}

void collectCoinIfPossible(Player &player, World &world)
{
	std::vector<std::shared_ptr<InanimateObject>> elements = world.getInanimateElements();
	for (auto it = elements.begin(); it != elements.end(); ++it) {
		if (std::dynamic_pointer_cast<Coin>(*it) && (areAtTheSameWidth(player, **it) 
			&& areAtTheSameHeight(player, **it))) {
			
			collectCoin(player, world, it - elements.begin());
			return;
		}
	}
}

void handleBonusCollecting(Player &player, World &world)
{
	collectCoinIfPossible(player, world);
	collectBonusIfPossible(player, world);
}

int getAlignmentForCollisionFromRight(int distance, const WorldObject &object, const Block &block, const World &world)
{
	int alignment = 0;

	if (block.getType() == BlockType::TubeLeftEntry) {
		if (!isCharacterStandingOnSomething(object, world)) {
			alignment = (object.getX() + distance + object.getWidth() / 2) - (block.getX() - block.getWidth() / 2);
		}
	}
	else {
		alignment = (object.getX() + distance + object.getWidth() / 2) - (block.getX() - block.getWidth() / 2);
	}
	
	return (alignment > 0 ? alignment : 0);
}

int getHorizontalAlignmentForCollisionWithBlocks(Direction direction, int distance, const WorldObject &object, const World &world)
{
	int alignment = 0;
	std::vector<Block> blocks = world.getBlocks();
	
	for (auto &block : blocks) {
		if (areAtTheSameHeight(object, block) && isCharacterHittingObject(object, block, direction, distance)
			&& !block.isInvisible()) {

			if (direction == Direction::Right) {
				alignment = getAlignmentForCollisionFromRight(distance, object, block, world);
			}
			else if (direction == Direction::Left) {
				alignment = (block.getX() + block.getWidth() / 2) - (object.getX() - distance - object.getWidth() / 2);
			}
			break;
		}
	}

	return (alignment > 0 ? alignment : 0);
}

int getHorizontalAlignmentForCollisionWithPlatforms(Direction direction, int distance, const WorldObject &object, const World &world)
{
	int alignment = 0;
	std::vector<Platform> platforms = world.getPlatforms();

	for (auto &platform : platforms) {
		if (areAtTheSameHeight(object, platform) && isCharacterHittingObject(object, platform, direction, distance)) {
			if (direction == Direction::Right) {
				alignment = (object.getX() + distance + object.getWidth() / 2) - (platform.getX() - platform.getWidth() / 2);
			}
			else if (direction == Direction::Left) {
				alignment = (platform.getX() + platform.getWidth() / 2) - (object.getX() - distance - object.getWidth() / 2);
			}
			break;
		}
	}

	return (alignment > 0 ? alignment : 0);
}

int computeHorizontalAlignment(Direction direction, int distance, const WorldObject &object, const World &world)
{
	int firstAlignment = getHorizontalAlignmentForCollisionWithBlocks(direction, distance, object, world);
	int secondAlignment = getHorizontalAlignmentForCollisionWithPlatforms(direction, distance, object, world);

	return (firstAlignment >= secondAlignment ? firstAlignment : secondAlignment);
}

// this method additionaly set the last touched block's index
int getVerticalAlignmentForCollisionWithBlocks(Direction direction, int distance, const WorldObject &object, World &world)
{
	int alignment = 0;
	std::vector<Block> blocks = world.getBlocks();

	for (auto it = blocks.begin(); it != blocks.end(); ++it) {
		if (areAtTheSameWidth(object, *it) && isCharacterHittingObject(object, *it, direction, distance)) {
			if (direction == Direction::Up) {
				alignment = (it->getY() + it->getHeight() / 2) - (object.getY() - distance - object.getHeight() / 2);

				if (alignment > 0) {
					world.setLastTouchedBlock(it - blocks.begin());	// 
				}
			}
			else if ( direction == Direction::Down && !it->isInvisible()) {
				alignment = (object.getY() + distance + object.getHeight() / 2) - (it->getY() - it->getHeight() / 2);
			}
			break;
		}
	}

	return (alignment > 0 ? alignment : 0);
}

// this method additionaly set the last touched block's index to -1
int getVerticalAlignmentForCollisionWithPlatforms(Direction direction, int distance, const WorldObject &object, World &world)
{
	int alignment = 0;
	std::vector<Platform> platforms = world.getPlatforms();

	for (auto it = platforms.begin(); it != platforms.end(); ++it) {
		if (areAtTheSameWidth(object, *it) && isCharacterHittingObject(object, *it, direction, distance)) {
			if (direction == Direction::Up) {
				alignment = (it->getY() + it->getHeight() / 2) - (object.getY() - distance - object.getHeight() / 2);

				if (alignment > 0) {
					world.setLastTouchedBlock(-1);	// 
				}
			}
			else if (direction == Direction::Down) {
				alignment = (object.getY() + distance + object.getHeight() / 2) - (it->getY() - it->getHeight() / 2);
			}
			break;
		}
	}

	return (alignment > 0 ? alignment : 0);
}

int computeVerticalAlignment(Direction direction, int distance, const WorldObject &object, World &world)
{
	int firstAlignment = getVerticalAlignmentForCollisionWithBlocks(direction, distance, object, world);
	int secondAlignment = getVerticalAlignmentForCollisionWithPlatforms(direction, distance, object, world);

	return (firstAlignment >= secondAlignment ? firstAlignment : secondAlignment);
}