#pragma once

#include "engine/point.hpp"
#include "engine/world_tile.hpp"

namespace devilution {

struct Missile;
struct Monster;
struct Player;

enum MinionState : int {
	MinionStateFollow = 0,
	MinionStateChase = 1,
	MinionStateIdle = 2,
};

void InitGolems();
void InitSkeletons();
Monster *PreSpawnSkeleton();
void SpawnGolem(Player &player, Monster &golem, Point position, Missile &missile);
void SpawnSkeleton(Player &player, Monster &skeleton, Point position, Missile &missile);
void GolumAi(Monster &monster);

} // namespace devilution
