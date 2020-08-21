#include "World.h"


void World::changeColoursIfAvailable()
{
	std::chrono::steady_clock::time_point timePoint = std::chrono::steady_clock::now();
	if (std::chrono::duration_cast<std::chrono::milliseconds>(timePoint - lastColoursUpdateTime).count() >= 750) {
		lastColoursUpdateTime = timePoint;
		Coin::changeCoinImage();
		Block::changeBlockImage();
		Flower::changeFlowerImage();
	}
}

void World::setMovementDirectionIfPlayerIsCloseEnough(LivingObject& monster)
{
	if (monster.getX() < player->getX() + 2 * CAMERA_REFERENCE_POINT) {
		if (dynamic_cast<Turtle*>(&monster)) {
			dynamic_cast<Turtle*>(&monster)->setMoveDirection(Left);
		}
		else if (dynamic_cast<Creature*>(&monster)) {
			dynamic_cast<Creature*>(&monster)->setMoveDirection(Left);
		}
	}
}

void World::deleteTemporaryElementsIfTimePassed()
{
	for (unsigned int i = 0; i < temporaryElements.size(); ++i) {
		if (temporaryElements[i]->shouldBeRemoved()) {
			if (dynamic_cast<AnimatedCoin*>(temporaryElements[i])) {
				addAnimatedText(TWO_HUNDRED, new Position(temporaryElements[i]->getX(),
					temporaryElements[i]->getY()));
			}
			temporaryElements.erase(temporaryElements.begin() + i);
		}
	}
}

void World::performBonusElementsActions()
{
	for (unsigned int i = 0; i < bonusElements.size(); ++i) {
		bonusElements[i]->move(*this);
		if (bonusElements[i]->getY() > WORLD_HEIGHT + DISTANCE_FROM_WORLD) {
			deleteLivingElement(i);
		}
	}
}

void World::performMonstersActions()
{
	for (unsigned int i = 0; i < monsters.size(); ++i) {
		if (monsters[i]->getMovement()->getDirection() == None) {
			setMovementDirectionIfPlayerIsCloseEnough(*monsters[i]);
		}

		monsters[i]->move(*this);

		if (dynamic_cast<Shell*>(monsters[i]) && !(dynamic_cast<Shell*>(monsters[i])->isActive())
			&& dynamic_cast<Shell*>(monsters[i])->shouldTurnIntoTurtle()) {

			monsters.push_back(new Turtle(new Position(monsters[i]->getX(), monsters[i]->getY())));
			monsters.erase(monsters.begin() + i);
		}

		if (monsters[i]->getY() > WORLD_HEIGHT + DISTANCE_FROM_WORLD) {
			deleteMonster(i);
		}
	}
}

void World::performFireBallsActions()
{
	for (unsigned int i = 0; i < fireballs.size(); ++i) {
		fireballs[i].move(*this);

		if (fireballs[i].shouldBeRemoved()) {
			int shift = (fireballs[i].getMovement()->getDirection() == Left ? -5 : 5);
			temporaryElements.push_back(new Explosion(new Position(fireballs[i].getX() + shift, fireballs[i].getY())));

			fireballs.erase(fireballs.begin() + i);
		}
		else if (fireballs[i].getY() > WORLD_HEIGHT + DISTANCE_FROM_WORLD) {
			fireballs.erase(fireballs.begin() + i);
		}
	}

	if (fireballStatus && player->isArmed()) {
		SoundController::playFireballPoppedEffect();
		Direction direction = player->isTurnedRight() ? Right : Left;
		fireballs.push_back(FireBall(new Position(player->getX() + 5, player->getY() - 15), direction));

		fireballStatus = false;
	}
}

void World::performWorldActions()
{
	player->move(*this);
	performBonusElementsActions();
	performMonstersActions();
	performFireBallsActions();

	if (slidingCounter) {
		slideBlock();
	}

	if (flag->isActive()) {
		flag->changePosition();
	}
}

void World::slideTemporaryElements()
{
	for (unsigned int i = 0; i < temporaryElements.size(); ++i) {
		temporaryElements[i]->slide();
	}
}

void World::slideBlock()
{
	if (isFlowerStandingOnTheBlock(*this, lastTouchedBlockIndex)) {
		slidingCounter = 0;
	}
	else {
		if (slidingCounter == 124) {
			playBlockSoundEffects();
			subtractCoinFromBlockIfPossible();

			if (isMushroomStandingOnTheBlock(*this, lastTouchedBlockIndex)) {
				raiseUpMushroom();
			}
		}

		slidingCounter--;

		if (slidingCounter % 3 == 0) {
			int height = (slidingCounter <= 60 ? 1 : -1);
			blocks[lastTouchedBlockIndex].addToPositionY(height);

			if (slidingCounter > 60 && blocks[lastTouchedBlockIndex].getModel() == BonusWithGreenMushroom) {
				createGreenMushroom();
			}
		}
		handleIfMonsterCollideWithBlock(*this, blocks[lastTouchedBlockIndex], player);

		if (slidingCounter == 0) {
			createNewBonusIfPossible();
			slideBlockStatus = true;
		}
	}
}

void World::raiseUpMushroom()
{
	std::vector<BonusObject*> elements = getBonusElements();
	for (auto it = elements.begin(); it != elements.end(); ++it) {
		if (dynamic_cast<Mushroom*>(*it)) {
			if (((*it)->getY() + (*it)->getHeight() / 2) == blocks[lastTouchedBlockIndex].getY()
				- blocks[lastTouchedBlockIndex].getHeight() / 2 && areAtTheSameWidth(*it, blocks[lastTouchedBlockIndex])) {

				dynamic_cast<Mushroom*>(*it)->decreasePositionY();
				dynamic_cast<Mushroom*>(*it)->setStepsUp(30);
				return;
			}
		}
	}
}

void World::addShards(Position* position)
{
	temporaryElements.push_back(new Shards(position));
}

void World::subtractCoinFromBlockIfPossible()
{
	if (blocks[lastTouchedBlockIndex].getModel() == Monetary) {
		blocks[lastTouchedBlockIndex].setAvailableCoins(blocks[lastTouchedBlockIndex].getAvailableCoins() - 1);
		player->incrementCoins();
		player->addPoints(200);
		temporaryElements.push_back(new AnimatedCoin(new Position(blocks[lastTouchedBlockIndex].getX(),
			blocks[lastTouchedBlockIndex].getY() - 56)));

		if (!blocks[lastTouchedBlockIndex].getAvailableCoins()) {
			blocks[lastTouchedBlockIndex].setModel(Empty);
		}
	}
	else if (blocks[lastTouchedBlockIndex].getModel() == BonusWithCoin) {
		player->incrementCoins();
		player->addPoints(200);
		temporaryElements.push_back(new AnimatedCoin(new Position(blocks[lastTouchedBlockIndex].getX(),
			blocks[lastTouchedBlockIndex].getY() - 50)));
		blocks[lastTouchedBlockIndex].setModel(Empty);
	}
}

void World::createNewBonusIfPossible()
{
	if (blocks[lastTouchedBlockIndex].getModel() == BonusWithRedMushroom) {
		bonusElements.push_back(new Mushroom(new Position(blocks[lastTouchedBlockIndex].getX(), blocks[lastTouchedBlockIndex].getY()), false));
		blocks[lastTouchedBlockIndex].setModel(Empty);
	}
	else if (blocks[lastTouchedBlockIndex].getModel() == BonusWithFlower) {
		if (player->isSmall()) {
			bonusElements.push_back(new Mushroom(new Position(blocks[lastTouchedBlockIndex].getX(), blocks[lastTouchedBlockIndex].getY()), false));
		}
		else {
			bonusElements.push_back(new Flower(new Position(blocks[lastTouchedBlockIndex].getX(), blocks[lastTouchedBlockIndex].getY())));
		}

		blocks[lastTouchedBlockIndex].setModel(Empty);
	}
	else if (blocks[lastTouchedBlockIndex].getModel() == BonusWithStar) {
		bonusElements.push_back(new Star(new Position(blocks[lastTouchedBlockIndex].getX(), blocks[lastTouchedBlockIndex].getY())));
		blocks[lastTouchedBlockIndex].setModel(Empty);
	}
}

void World::createGreenMushroom()
{
	bonusElements.push_back(new Mushroom(new Position(blocks[lastTouchedBlockIndex].getX(),
		blocks[lastTouchedBlockIndex].getY()), true));
	blocks[lastTouchedBlockIndex].setModel(Empty);
}

void World::playBlockSoundEffects()
{
	if (blocks[lastTouchedBlockIndex].getModel() >= BonusWithGreenMushroom &&
		blocks[lastTouchedBlockIndex].getModel() <= BonusWithStar) {
		SoundController::playBonusAppeardEffect();
	}
	else if (blocks[lastTouchedBlockIndex].getModel() >= Monetary &&
		blocks[lastTouchedBlockIndex].getModel() <= BonusWithCoin) {
		SoundController::playCoinCollectedEffect();
	}
	else {
		SoundController::playBlockHittedEffect();
	}
}

World::World()
{
	gameCounter = 0;
	lastColoursUpdateTime = std::chrono::steady_clock::now();
	lastTouchedBlockIndex = 0;
	player = nullptr;
	flag = nullptr;
	screen = nullptr;
	slidingCounter = 0;
	slideBlockStatus = true;
	fireballStatus = false;
}

std::vector<Block> World::getBlocks() const
{
	return blocks;
}

std::vector<InanimateObject*> World::getInanimateElements() const
{
	return inanimateElements;
}

std::vector<BonusObject*> World::getBonusElements() const
{
	return bonusElements;
}

std::vector<LivingObject*> World::getMonsters() const
{
	return monsters;
}

std::vector<FireBall> World::getFireBalls() const
{
	return fireballs;
}

int World::getLastTouchedBlockIndex() const
{
	return lastTouchedBlockIndex;
}

int World::getBlockModel(int index) const
{
	return blocks[index].getModel();
}

bool World::isFlagDown() const
{
	return flag->isDown();
}

Screen* World::getScreen() const
{
	return screen;
}

void World::setPlayer(Player* player)
{
	this->player = player;
}

void World::setScreen(Screen* screen)
{
	this->screen = screen;
}

void World::setLastTouchedBlock(int index)
{
	if (!slidingCounter) {
		lastTouchedBlockIndex = index;
	}
}

void World::hitBlock()
{
	if ((blocks[lastTouchedBlockIndex].getModel() >= Destructible
		&& blocks[lastTouchedBlockIndex].getModel() <= BonusWithStar) && blocks[lastTouchedBlockIndex].canBeHitted()) {

		slidingCounter = 124;
	}
}

void World::setFireballStatus()
{
	if (fireballs.size() != 3) {
		fireballStatus = true;
	}
}

void World::switchOnFlag()
{
	flag->setActiveState();
}

void World::changeShellMovementParameters(int index, Direction direction)
{
	dynamic_cast<Shell*>(monsters[index])->setMovementDirectionAndActiveState(direction);
	dynamic_cast<Shell*>(monsters[index])->resetCreationTime();
}

void World::performBlockRemovalActions(int index)
{
	addShards(new Position(blocks[index].getX(), blocks[index].getY()));
	handleIfMonsterCollideWithBlock(*this, blocks[index], player);
	blocks.erase(blocks.begin() + index);
	player->addPoints(50);

	SoundController::playBlockDestroyedEffect();
}

void World::deleteInanimateElement(int index)
{
	inanimateElements.erase(inanimateElements.begin() + index);
}

void World::deleteLivingElement(int index)
{
	bonusElements.erase(bonusElements.begin() + index);
}

void World::deleteMonster(int index)
{
	monsters.erase(monsters.begin() + index);
}

void World::deleteFireBall(int index)
{
	fireballs.erase(fireballs.begin() + index);
}

void World::addShell(Position* position)
{
	monsters.push_back(new Shell(position));
}

void World::addCrushedCreature(Position* position)
{
	temporaryElements.push_back(new CrushedCreature(position));
}

void World::addDestroyedCreature(Position* position)
{
	temporaryElements.push_back(new DestroyedCreature(position));
}

void World::addDestroyedTurtle(Position* position)
{
	temporaryElements.push_back(new DestroyedTurtle(position));
}

void World::addExplosion(Position* position)
{
	temporaryElements.push_back(new Explosion(position));
}

void World::addAnimatedText(TextType type, Position* position)
{
	temporaryElements.push_back(new AnimatedText(type, position));
}

void World::performActions()
{
	gameCounter++;
	//if (gameCounter % 3 == 0) {
	performWorldActions();
	deleteTemporaryElementsIfTimePassed();
	slideTemporaryElements();

	collectBonusIfPossible(player, *this);
	handleIfCollisionWithMonsterOccurs(player, *this);
	handleIfShellCollideWithMonsters(*this, player);
	handleIfFireBallCollideWithMonsters(*this, player);

	changeColoursIfAvailable();
	//}
}

void World::draw(SDL_Surface* display, int beginningOfScreen, int endOfScreen, bool playerFlag)
{
	for (unsigned int i = 0; i < inanimateElements.size(); ++i) {
		inanimateElements[i]->draw(display, beginningOfScreen, endOfScreen);
	}

	for (unsigned int i = 0; i < bonusElements.size(); ++i) {
		bonusElements[i]->draw(display, beginningOfScreen, endOfScreen);
	}

	for (unsigned int i = 0; i < monsters.size(); ++i) {
		monsters[i]->draw(display, beginningOfScreen, endOfScreen);
	}

	for (auto it = fireballs.begin(); it != fireballs.end(); ++it) {
		it->draw(display, beginningOfScreen, endOfScreen);
	}

	for (auto it = blocks.begin(); it != blocks.end(); ++it) {
		it->draw(display, beginningOfScreen, endOfScreen);
	}

	for (unsigned int i = 0; i < temporaryElements.size(); ++i) {
		temporaryElements[i]->draw(display, beginningOfScreen, endOfScreen);
	}

	flag->draw(display, beginningOfScreen, endOfScreen);

	if (playerFlag) {
		player->draw(display, beginningOfScreen, endOfScreen);
	}
}

