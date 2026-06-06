#include "minionstatus.h"

#include <string_view>

#include "control.h"
#include "engine/palette.h"
#include "engine/render/primitive_render.hpp"
#include "engine/render/text_render.hpp"
#include "levels/gendung.h"
#include "missiles.h"
#include "monster.h"
#include "multi.h"
#include "panels/spell_icons.hpp"
#include "player.h"
#include "utils/str_cat.hpp"

namespace devilution {

namespace {

constexpr int BoxWidth = 220;
constexpr int BoxHeight = 36;
constexpr int IconWidth = 37;
constexpr int BarMaxWidth = 120;
constexpr int BarHeight = 3;

#ifdef _DEBUG
constexpr int DebugMaxMinionChaseDistance = 4;
constexpr int DebugMaxMinionReturnDistance = 8;
constexpr int DebugMinionEngageRange = 5;

string_view GetMinionAiState(const Monster &minion)
{
	if (minion.mode == MonsterMode::Death)
		return "DEAD";
	if (minion.mode == MonsterMode::MeleeAttack)
		return "ATTACK";
	if (IsMonsterModeMove(minion.mode))
		return "WALK";

	size_t ownerId = minion.getId();
	if (ownerId >= MAX_PLRS)
		ownerId -= MAX_PLRS;

	int distToOwner = minion.position.tile.WalkingDistance(Players[ownerId].position.future);

	if (distToOwner > DebugMaxMinionReturnDistance)
		return "URGENT";

	bool enemyNearby = false;
	if ((minion.flags & MFLAG_NO_ENEMY) == 0) {
		int distToEnemy = minion.position.tile.WalkingDistance(Monsters[minion.enemy].position.tile);
		enemyNearby = distToEnemy <= DebugMinionEngageRange;
	}

	if (enemyNearby) {
		if (distToOwner <= DebugMaxMinionChaseDistance)
			return "CHASE";
		return "RETREAT";
	}

	if (distToOwner > DebugMaxMinionChaseDistance)
		return "FOLLOW";
	return "IDLE";
}
#endif

uint8_t GetHpBarColor(int hp, int maxHp)
{
	int pct = (maxHp > 0) ? (100 * hp / maxHp) : 0;
	if (pct > 60)
		return PAL16_BEIGE + 8;
	if (pct > 30)
		return PAL8_YELLOW;
	return PAL8_RED;
}

void DrawMinionBox(const Surface &out, Point pos, const Monster &minion, SpellID spell, string_view name)
{
	DrawHalfTransparentRectTo(out, pos.x, pos.y, BoxWidth, BoxHeight);

	SetSpellTrans(SpellType::Skill);
	DrawSmallSpellIcon(out, { pos.x + 1, pos.y + BoxHeight - 1 }, spell);

	int textX = pos.x + IconWidth + 4;
	int textY = pos.y + 2;

	TextRenderOptions nameOpts {};
	nameOpts.flags = UiFlags::ColorWhite | UiFlags::FontSize12;
	nameOpts.spacing = 1;
	DrawString(out, name, { textX, textY }, nameOpts);

#ifdef _DEBUG
	string_view aiState = GetMinionAiState(minion);
	if (!aiState.empty()) {
		int stateX = textX + GetLineWidth(name, GameFont12) + 4;
		TextRenderOptions stateOpts {};
		stateOpts.flags = UiFlags::ColorWhitegold | UiFlags::FontSize12;
		stateOpts.spacing = 1;
		DrawString(out, StrCat("[", aiState, "]"), { stateX, textY }, stateOpts);
	}
#endif

	int barX = pos.x + IconWidth + 4;
	int barY = pos.y + 16;
	int fillWidth = (minion.maxHitPoints > 0) ? (BarMaxWidth * minion.hitPoints / minion.maxHitPoints) : 0;
	fillWidth = std::max(0, std::min(fillWidth, BarMaxWidth));

	uint8_t color = GetHpBarColor(minion.hitPoints, minion.maxHitPoints);

	if (fillWidth > 0) {
		UnsafeDrawHorizontalLine(out, { barX, barY }, fillWidth, color + 1);
		UnsafeDrawHorizontalLine(out, { barX, barY + 1 }, fillWidth, color);
		UnsafeDrawHorizontalLine(out, { barX, barY + 2 }, fillWidth, color - 1);
	}

	UnsafeDrawHorizontalLine(out, { barX, barY }, BarMaxWidth, PAL16_GRAY + 5);
	UnsafeDrawHorizontalLine(out, { barX, barY + BarHeight - 1 }, BarMaxWidth, PAL16_GRAY + 5);
	UnsafeDrawVerticalLine(out, { barX, barY }, BarHeight, PAL16_GRAY + 5);
	UnsafeDrawVerticalLine(out, { barX + BarMaxWidth - 1, barY }, BarHeight, PAL16_GRAY + 5);

	int hp = minion.hitPoints >> 6;
	int maxHp = minion.maxHitPoints >> 6;
	DrawString(out, StrCat(hp, "/", maxHp),
	    { { barX, barY + BarHeight + 2 }, { BarMaxWidth, 12 } },
	    { UiFlags::ColorWhite | UiFlags::FontSize12 | UiFlags::AlignCenter, 1 });
}

} // namespace

void DrawMinionStatus(const Surface &out)
{
	if (leveltype == DTYPE_TOWN)
		return;

	Monster &skeleton = Monsters[MAX_PLRS + MyPlayerId];
	Monster &golem = Monsters[MyPlayerId];

	bool hasSkeleton = skeleton.position.tile != GolemHoldingCell;
	bool hasGolem = golem.position.tile != GolemHoldingCell;

	if (!hasSkeleton && !hasGolem)
		return;

	const Rectangle &panel = GetMainPanel();
	constexpr int gap = 4;

	int totalWidth = (hasSkeleton && hasGolem) ? 2 * BoxWidth + gap : BoxWidth;
	int x = panel.position.x + (panel.size.width - totalWidth) / 2;
	int y = panel.position.y - BoxHeight - 4;

	if (hasSkeleton) {
		DrawMinionBox(out, { x, y }, skeleton, SpellID::Skeleton, "Skeleton");
		x += BoxWidth + gap;
	}

	if (hasGolem)
		DrawMinionBox(out, { x, y }, golem, SpellID::Golem, "Golem");
}

} // namespace devilution
