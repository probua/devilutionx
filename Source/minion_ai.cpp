#include "minion_ai.h"

#include <cstddef>
#include <cstdlib>
#include <algorithm>

#include "engine/actor_position.hpp"
#include "engine/displacement.hpp"
#include "engine/direction.hpp"
#include "engine/random.hpp"
#include "levels/gendung.h"
#include "missiles.h"
#include "monster.h"
#include "msg.h"
#include "player.h"
#include "spelldat.h"

namespace devilution {

namespace {

constexpr int MaxMinionReturnDistance = 4;
constexpr int MinionEngageRange = 3;
constexpr int MinionIdleDelay = 1;
constexpr int MinionChaseMaxRange = 6;
constexpr int MinionFollowHysteresis = 2;
constexpr int MinionCombatLeash = 6;

Point GetFormationPosition(size_t ownerId, size_t monsterId)
{
	Player &owner = Players[ownerId];
	Direction facing = owner._pdir;
	if (facing == Direction::NoDirection)
		facing = Direction::South;

	Displacement forward(facing);
	forward.deltaX *= 3;
	forward.deltaY *= 3;

	bool isSkeleton = monsterId >= MAX_PLRS;
	Direction lateralDir = isSkeleton ? Right(facing) : Left(facing);
	Displacement lateral(lateralDir);

	return owner.position.future + forward + lateral;
}

bool ScanForEnemy(Monster &minion, size_t ownerId)
{
	Point ownerPos = Players[ownerId].position.tile;
	Point minionPos = minion.position.tile;
	int8_t minionRoom = dTransVal[minionPos.x][minionPos.y];
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
		if (dTransVal[monster.position.tile.x][monster.position.tile.y] != minionRoom)
			continue;
		if (!LineClearMissile(minionPos, monster.position.tile))
			continue;

		int dist = monster.position.tile.WalkingDistance(minionPos);
		if (dist > MaxMinionReturnDistance)
			continue;

		bool isThreat = (monster.flags & MFLAG_NO_ENEMY) == 0
		    && (monster.flags & MFLAG_TARGETS_MONSTER) == 0
		    && monster.enemy == static_cast<int>(ownerId)
		    && monster.position.tile.WalkingDistance(ownerPos) <= MinionEngageRange;

		if (isThreat && !bestIsThreat) {
			bestDist = dist;
			bestEnemy = monsterId;
			bestIsThreat = true;
		} else if (isThreat == bestIsThreat && dist < bestDist) {
			bestDist = dist;
			bestEnemy = monsterId;
		}
	}

	if (bestEnemy != -1) {
		minion.flags &= ~MFLAG_NO_ENEMY;
		minion.flags |= MFLAG_TARGETS_MONSTER;
		minion.enemy = bestEnemy;
		minion.enemyPosition = Monsters[bestEnemy].position.future;
		return true;
	}

	return false;
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

void ClearMinionTarget(Monster &minion)
{
	size_t ownerId = minion.getId();
	if (ownerId >= MAX_PLRS)
		ownerId -= MAX_PLRS;
	minion.flags |= MFLAG_NO_ENEMY;
	minion.flags &= ~MFLAG_TARGETS_MONSTER;
	minion.enemy = static_cast<int>(ownerId);
	minion.enemyPosition = Players[ownerId].position.future;
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
	golem.goal = MonsterGoal::Normal;
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
	skeleton.maxHitPoints = (8 * missile._mispllvl + 3 * player._pLevel + 15) << 6;
	skeleton.hitPoints = skeleton.maxHitPoints;
	skeleton.armorClass = 25;
	skeleton.golemToHit = 5 * (missile._mispllvl + 8) + 2 * player._pLevel;
	skeleton.minDamage = missile._mispllvl;
	skeleton.maxDamage = 2 * missile._mispllvl + 2;
	skeleton.flags |= MFLAG_GOLEM;
	skeleton.ai = MonsterAIID::Golem;
	skeleton.activeForTicks = UINT8_MAX;
	skeleton.goal = MonsterGoal::Normal;
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
	Point formationPos = GetFormationPosition(ownerId, golem.getId());
	int distToFormation = golem.position.tile.WalkingDistance(formationPos);

	switch (golem.goal) {

	case MonsterGoal::Normal: {
		if (ScanForEnemy(golem, ownerId)) {
			golem.goal = MonsterGoal::Attack;
			golem.var1 = MinionStateChase;
			golem.var2 = 0;
			break;
		}
		if (distToFormation > MinionFollowHysteresis) {
			golem.goal = MonsterGoal::Move;
			golem.var1 = MinionStateFollow;
			golem.var2 = 0;
			break;
		}
		golem.var1 = MinionStateIdle;
		golem.direction = GetDirection(golem.position.tile, formationPos);
		break;
	}

	case MonsterGoal::Move: {
		if (ScanForEnemy(golem, ownerId)) {
			golem.goal = MonsterGoal::Attack;
			golem.var1 = MinionStateChase;
			golem.var2 = 0;
			break;
		}
		if (distToFormation <= MinionFollowHysteresis) {
			golem.goal = MonsterGoal::Normal;
			golem.var1 = MinionStateIdle;
			golem.var2 = 0;
			break;
		}
		golem.var1 = MinionStateFollow;
		golem.var2++;
		if (golem.var2 >= MinionIdleDelay) {
			golem.var2 = 0;
			MoveToward(golem, formationPos);
		}
		break;
	}

	case MonsterGoal::Attack: {
		if (distToOwner > MinionCombatLeash) {
			golem.goal = MonsterGoal::Move;
			golem.var1 = MinionStateFollow;
			golem.var2 = 0;
			ClearMinionTarget(golem);
			break;
		}
		Monster &enemy = Monsters[golem.enemy];
		if ((enemy.hitPoints >> 6) <= 0) {
			golem.goal = MonsterGoal::Normal;
			golem.var1 = MinionStateIdle;
			golem.var2 = 0;
			ClearMinionTarget(golem);
			break;
		}
		int distToEnemy = golem.position.tile.WalkingDistance(enemy.position.tile);
		if (distToEnemy > MinionChaseMaxRange) {
			golem.goal = MonsterGoal::Normal;
			golem.var1 = MinionStateIdle;
			golem.var2 = 0;
			ClearMinionTarget(golem);
			break;
		}

		golem.enemyPosition = enemy.position.future;
		golem.direction = GetDirection(golem.position.tile, enemy.position.tile);

		int mex = abs(golem.position.tile.x - enemy.position.future.x);
		int mey = abs(golem.position.tile.y - enemy.position.future.y);
		if (mex < 2 && mey < 2) {
			golem.enemyPosition = enemy.position.tile;
			ActivateNearbyMonsters(golem, enemy);
			StartAttack(golem);
			break;
		}

		MoveToward(golem, enemy.position.tile);
		break;
	}

	default:
		golem.goal = MonsterGoal::Normal;
		golem.var1 = MinionStateIdle;
		golem.var2 = 0;
		break;
	}
}

} // namespace devilution
