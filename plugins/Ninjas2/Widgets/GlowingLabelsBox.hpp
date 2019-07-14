/*
 * Copyright (C) 2018-2019 Rob van den Berg <rghvdberg at gmail dot org>
 *
 * This file is part of Ninjas2
 *
 * Nnjas2 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ninjas2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Ninjas2.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef WOLF_GLOWING_LABELS_BOX_HPP_INCLUDED
#define WOLF_GLOWING_LABELS_BOX_HPP_INCLUDED

#include "Widget.hpp"
#include "NanoVG.hpp"
#include "LabelContainer.hpp"
#include <vector>

START_NAMESPACE_DISTRHO

class GlowingLabelsBox : public LabelContainer
{
  public:
    explicit GlowingLabelsBox(Window &parent, Size<uint> size) noexcept;
    explicit GlowingLabelsBox(NanoWidget *widget, Size<uint> size) noexcept;

  protected:
    void onNanoDisplay() override;
};

END_NAMESPACE_DISTRHO

#endif
