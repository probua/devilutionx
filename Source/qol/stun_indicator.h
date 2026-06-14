/**
 * @file stun_indicator.h
 *
 * Draws a small indicator above stunned monsters
 */
#pragma once

#include "engine/displacement.hpp"
#include "engine/point.hpp"

namespace devilution {

struct Surface;

void DrawStunIndicators(const Surface &out, Point viewPosition, Displacement offset);

} // namespace devilution
