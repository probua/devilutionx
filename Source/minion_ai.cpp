#include "minion_ai.h"

#include <cstddef>
#include <cstdlib>
#include <algorithm>

#include "engine/actor_position.hpp"
#include "engine/random.hpp"
#include "missiles.h"
#include "monster.h"
#include "msg.h"
#include "spelldat.h"

namespace devilution {

namespace {

constexpr int MaxMinionReturnDistance = 8;
constexpr int MinionEngageRange = 5;
constexpr int MinionIdleDelay = 4;

int PickMinionTarget(Monster &minion, size_t ownerId)
{
	Point ownerPos = Players[ownerId].position.tile;
	Point minionPos = minion.position.tile;
	int bestDist = INT_MAX;
	int bestEnemy = -1;
	bool bestIsThreat = false;

	for (size_t i = 0; i < ActiveMonsterCount; i++) {
		int monsterId = ActiveMonsters[i];
		Monster &monster = Monsters[monsterId];

		if ((monster.hitPoints >> 6) <= 0)
			continue;
		if (monster.isPlayerMinion())
			continue;
		if (monster.position.tile == GolemHoldingCell)
			continue;

		bool isThreat = (monster.flags & MFLAG_NO_ENEMY) == 0
		    && (monster.flags & MFLAG_TARGETS_MONSTER) == 0
		    && monster.enemy == static_cast<int>(ownerId)
		    && monster.position.tile.WalkingDistance(ownerPos) <= MinionEngageRange;

		int dist = monster.position.tile.WalkingDistance(minionPos);
		if (!isThreat && dist > MaxMinionReturnDistance)
			continue;

		if (isThreat && !bestIsThreat) {
			bestDist = dist;
			bestEnemy = monsterId;
			bestIsThreat = true;
		} else if (isThreat == bestIsThreat && dist < bestDist) {
			bestDist = dist;
			bestEnemy = monsterId;
		}
	}

	return bestEnemy;
}

void MoveToward(Monster &monster, Point target)
{
	if (AiPlanPathTo(monster, target))
		return;
	Direction dir = GetDirection(monster.position.tile, target);
	if (RandomWalk(monster, dir))
		return;
	Direction md = Left(dir);
	for (int j = 0; j < 8; j++) {
		md = Right(md);
		if (Walk(monster, md))
			break;
	}
}

void ActivateNearbyMonsters(Monster &minion, Monster &enemy)
{
	if (enemy.activeForTicks == 0) {
		enemy.activeForTicks = UINT8_MAX;
		enemy.position.last = minion.position.tile;
		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				int mx = minion.position.tile.x + k - 2;
				int my = minion.position.tile.y + j - 2;
				if (!InDungeonBounds({ mx, my }))
					continue;
				int enemyId = dMonster[mx][my];
				if (enemyId > 0)
					Monsters[enemyId - 1].activeForTicks = UINT8_MAX;
			}
		}
	}
}

} // namespace

void InitGolems()
{
	if (!setlevel) {
		for (int i = 0; i < MAX_PLRS; i++)
			AddMonster(GolemHoldingCell, Direction::South, 0, false);
	}
}

void InitSkeletons()
{
	if (setlevel)
		return;
	for (int i = 0; i < MAX_PLRS; i++)
		AddMonster(GolemHoldingCell, Direction::South, skeletonTypeIndex, false);
}

Monster *PreSpawnSkeleton()
{
	Monster *skeleton = AddSkeleton({ 0, 0 }, Direction::South, false);
	if (skeleton != nullptr)
		M_StartStand(*skeleton, Direction::South);

	return skeleton;
}

void SpawnGolem(Player &player, Monster &golem, Point position, Missile &missile)
{
	dMonster[position.x][position.y] = golem.getId() + 1;
	golem.position.tile = position;
	golem.position.future = position;
	golem.position.old = position;
	golem.pathCount = 0;
	golem.maxHitPoints = 2 * (320 * missile._mispllvl + player._pMaxMana / 3);
	golem.hitPoints = golem.maxHitPoints;
	golem.armorClass = 25;
	golem.golemToHit = 5 * (missile._mispllvl + 8) + 2 * player._pLevel;
	golem.minDamage = 2 * (missile._mispllvl + 4);
	golem.maxDamage = 2 * (missile._mispllvl + 8);
	golem.flags |= MFLAG_GOLEM;
	golem.activeForTicks = UINT8_MAX;
	StartSpecialStand(golem, Direction::South);
	UpdateEnemy(golem);
	if (&player == MyPlayer) {
		NetSendCmdGolem(
		    golem.position.tile.x,
		    golem.position.tile.y,
		    golem.direction,
		    golem.enemy,
		    golem.hitPoints,
		    GetLevelForMultiplayer(player));
	}
}

void SpawnSkeleton(Player &player, Monster &skeleton, Point position, Missile &missile)
{
	size_t skelType = GetMonsterTypeIndex(MT_WSKELAX);
	if (skelType >= LevelMonsterTypeCount)
		skelType = AddMonsterType(MT_WSKELAX, PLACE_SPECIAL);
	if (skeleton.levelType != skelType)
		InitMonster(skeleton, Direction::South, skelType, GolemHoldingCell);

	dMonster[position.x][position.y] = skeleton.getId() + 1;
	skeleton.position.tile = position;
	skeleton.position.future = position;
	skeleton.position.old = position;
	skeleton.pathCount = 0;
	skeleton.maxHitPoints = 2 * (320 * missile._mispllvl + player._pMaxMana / 3);
	skeleton.hitPoints = skeleton.maxHitPoints;
	skeleton.armorClass = 25;
	skeleton.golemToHit = 5 * (missile._mispllvl + 8) + 2 * player._pLevel;
	skeleton.minDamage = 2 * (missile._mispllvl + 4);
	skeleton.maxDamage = 2 * (missile._mispllvl + 8);
	skeleton.flags |= MFLAG_GOLEM;
	skeleton.ai = MonsterAIID::Golem;
	skeleton.activeForTicks = UINT8_MAX;
	StartSpecialStand(skeleton, Direction::South);
	UpdateEnemy(skeleton);
	if (&player == MyPlayer) {
		NetSendCmdSkeleton(
		    skeleton.position.tile.x,
		    skeleton.position.tile.y,
		    skeleton.direction,
		    skeleton.enemy,
		    skeleton.hitPoints,
		    GetLevelForMultiplayer(player));
	}
}

void GolumAi(Monster &golem)
{
	if (golem.position.tile.x == 1 && golem.position.tile.y == 0)
		return;

	if (IsAnyOf(golem.mode, MonsterMode::Death, MonsterMode::SpecialStand, MonsterMode::MeleeAttack) || golem.isWalking())
		return;

	size_t ownerId = golem.getId();
	if (ownerId >= MAX_PLRS)
		ownerId -= MAX_PLRS;

	int distToOwner = golem.position.tile.WalkingDistance(Players[ownerId].position.future);

	if (distToOwner > MaxMinionReturnDistance) {
		golem.var1 = MinionStateFollow;
		golem.var2++;
		if (golem.var2 < MinionIdleDelay)
			return;
		golem.var2 = 0;
		MoveToward(golem, Players[ownerId].position.tile);
		return;
	}

	int target = PickMinionTarget(golem, ownerId);

	if (target == -1) {
		golem.flags |= MFLAG_NO_ENEMY;
		golem.flags &= ~MFLAG_TARGETS_MONSTER;
		golem.var1 = MinionStateIdle;
		golem.var2 = 0;
		golem.direction = GetDirection(golem.position.tile, Players[ownerId].position.tile);
		return;
	}

	golem.flags &= ~MFLAG_NO_ENEMY;
	golem.flags |= MFLAG_TARGETS_MONSTER;
	golem.enemy = target;
	golem.enemyPosition = Monsters[target].position.future;
	golem.var1 = MinionStateChase;
	golem.var2 = 0;
	Monster &enemy = Monsters[target];
	golem.direction = GetDirection(golem.position.tile, enemy.position.tile);

	int mex = abs(golem.position.tile.x - enemy.position.future.x);
	int mey = abs(golem.position.tile.y - enemy.position.future.y);
	if (mex < 2 && mey < 2) {
		golem.enemyPosition = enemy.position.tile;
		ActivateNearbyMonsters(golem, enemy);
		StartAttack(golem);
		return;
	}

	MoveToward(golem, enemy.position.tile);
}

} // namespace devilution
