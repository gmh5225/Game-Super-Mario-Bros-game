#ifndef UtilityFunctions_H
#define UtilityFunctions_H

#include <memory>

class WorldObject;
class LivingObject;
class IndependentLivingObject;
class Block;
class Player;
enum class Direction;


bool isDifferenceInInterval(int difference, int begin, int shift, int repetitions);

bool areAtTheSameWidth(const WorldObject &firstObject, const WorldObject &secondObject);

bool areAtTheSameHeight(const WorldObject &firstObject, const WorldObject &secondObject);

bool isElementDirectlyAboveObject(const WorldObject &element, const WorldObject &object);

bool isMonsterCloseAboveBlock(const LivingObject &monster, const Block &block);

bool isInactiveShell(IndependentLivingObject &npc);

int determineShift(const LivingObject &object, int base);

int determineShift(Direction direction, int base);

Direction determineDirection(const WorldObject &firstObject, const WorldObject &secondObject);

Direction determineDirection(const Player &player);

#endif //UtilityFunctions_H