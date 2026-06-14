#include "qol/stun_indicator.h"

#include <cmath>

#include "diablo.h"
#include "engine/palette.h"
#include "engine/render/primitive_render.hpp"
#include "engine/surface.hpp"
#include "monster.h"
#include "options.h"

namespace devilution {

namespace {

constexpr int SwirlRadius = 8;
constexpr int SwirlStarCount = 3;
constexpr float Pi = 3.14159265f;
constexpr float AnglePerStar = 2.0f * Pi / SwirlStarCount;
constexpr float RotationSpeed = 0.3f;
constexpr float IsoRatio = 0.5f;

constexpr Displacement StarPixels[] = {
	{ 0, -3 },
	{ -1, -2 }, { 0, -2 }, { 1, -2 },
	{ -3, -1 }, { -2, -1 }, { -1, -1 }, { 0, -1 }, { 1, -1 }, { 2, -1 }, { 3, -1 },
	{ -2, 0 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { 2, 0 },
	{ -1, 1 }, { 0, 1 }, { 1, 1 },
	{ -2, 2 }, { -1, 2 }, { 1, 2 }, { 2, 2 },
};

void DrawSwirlStar(const Surface &out, Point center, uint8_t color, int scale)
{
	for (const auto &pixel : StarPixels) {
		Point pos = center + pixel * scale;
		if ((pixel.deltaX + pixel.deltaY) % 2 == 0)
			out.SetPixel(pos, color);
		else
			SetHalfTransparentPixel(out, pos, color);
	}
}

} // namespace

void DrawStunIndicators(const Surface &out, Point viewPosition, Displacement offset)
{
	float baseAngle = static_cast<float>(gGameTicks) * RotationSpeed;

	for (size_t i = 0; i < ActiveMonsterCount; i++) {
		Monster &monster = Monsters[ActiveMonsters[i]];
		if (!(monster.flags & MFLAG_STUNNED))
			continue;
		if (monster.hitPoints <= 0)
			continue;

		Point monsterTile = monster.position.tile;
		Displacement worldOffset = viewPosition - monsterTile;
		worldOffset = worldOffset.worldToScreen()
		    + offset
		    + Displacement { TILE_WIDTH / 2, -TILE_HEIGHT / 2 };

		int spriteHeight = 0;
		if (monster.animInfo.sprites) {
			spriteHeight = monster.animInfo.currentSprite().height();
		}
		worldOffset.deltaY -= spriteHeight / 2 + 4;

		int scale = 1;
		int radius = SwirlRadius;
		if (*sgOptions.Graphics.zoom) {
			worldOffset *= 2;
			radius *= 2;
			scale = 2;
		}

		Point screenCenter { worldOffset.deltaX, worldOffset.deltaY };

		for (int s = 0; s < SwirlStarCount; s++) {
			float starAngle = baseAngle + static_cast<float>(s) * AnglePerStar;
			int dx = static_cast<int>(std::round(std::cos(starAngle) * radius));
			int dy = static_cast<int>(std::round(std::sin(starAngle) * radius * IsoRatio));
			DrawSwirlStar(out, screenCenter + Displacement { dx, dy }, PAL8_YELLOW, scale);
		}
	}
}

} // namespace devilution
