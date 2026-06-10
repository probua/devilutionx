#include "cooldown_hud.h"

#include <cstdint>

#include <fmt/format.h>

#include "control.h"
#include "diablo.h"
#include "engine/palette.h"
#include "engine/render/primitive_render.hpp"
#include "engine/render/text_render.hpp"
#include "nthread.h"
#include "panels/spell_icons.hpp"
#include "player.h"
#include "spelldat.h"

namespace devilution {

namespace {

constexpr int IconSize = 37;
constexpr int BoxPadding = 2;
constexpr int BoxWidth = IconSize + 2 * BoxPadding;
constexpr int TextHeight = 12;
constexpr int Gap = 4;

struct CooldownEntry {
	SpellID spellId;
	float remaining;
};

} // namespace

void DrawCooldownHud(const Surface &out)
{
	if (leveltype == DTYPE_TOWN)
		return;

	Player &myPlayer = *MyPlayer;
	uint32_t now = gGameTicks;

	CooldownEntry entries[64];
	int count = 0;

	for (int i = 1; i <= static_cast<int>(SpellID::LastDiablo); i++) {
		SpellID id = static_cast<SpellID>(i);
		uint16_t cd = GetSpellData(id).sCooldown;
		if (cd == 0)
			continue;

		uint32_t start = myPlayer.spellCooldownStart[i];
		if (start == 0)
			continue;

		uint32_t elapsed = now - start;
		if (elapsed >= cd) {
			myPlayer.spellCooldownStart[i] = 0;
			continue;
		}

		float ticksPerSecond = 1000.0f / static_cast<float>(gnTickDelay);
		entries[count].spellId = id;
		entries[count].remaining = static_cast<float>(cd - elapsed) / ticksPerSecond;
		count++;
	}

	if (count == 0)
		return;

	const Rectangle &panel = GetMainPanel();
	int boxHeight = IconSize + TextHeight + 2 * BoxPadding;
	int totalWidth = count * BoxWidth + (count - 1) * Gap;
	int startX = panel.position.x + (panel.size.width - totalWidth) / 2;
	int startY = panel.position.y - boxHeight - 4;

	for (int i = 0; i < count; i++) {
		int x = startX + i * (BoxWidth + Gap);
		int y = startY;

		DrawHalfTransparentRectTo(out, x, y, BoxWidth, boxHeight);

		SetSpellTrans(SpellType::Skill);
		Point iconPos { x + BoxPadding, y + BoxPadding + IconSize };
		DrawSmallSpellIcon(out, iconPos, entries[i].spellId);

		char text[16];
		auto end = fmt::format_to(text, "{:.1f}", entries[i].remaining);
		*end = '\0';

		int textWidth = GetLineWidth(text, GameFont12);
		int textX = x + (BoxWidth - textWidth) / 2;
		int textY = y + BoxPadding + IconSize + 2;

		DrawString(out, text, Point { textX, textY }, { .flags = UiFlags::ColorWhite | UiFlags::FontSize12, .spacing = 1 });
	}
}

} // namespace devilution
