/*
 * Copyright (c) 2021 Naoaki Iwakiri
 * This program is released under GNU General Public License version 3 or later
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Creation Date: 2021-05-15
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileType: SOURCE
 * SPDX-FileName: cskkconfig.h
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */

#ifndef FCITX5_CSKK_CSKKCONFIG_H
#define FCITX5_CSKK_CSKKCONFIG_H

extern "C" {
#include <cskk/libcskk.h>
}
#include "log.h"
#include <fcitx-config/configuration.h>
#include <fcitx-config/enum.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/candidatelist.h>

namespace fcitx {

// TODO: use i18n annotation creation macro for ease
FCITX_CONFIG_ENUM_NAME(InputMode, "Hiragana", "Katakana", "HankakuKana",
                       "Zenkaku", "Ascii")
FCITX_CONFIG_ENUM_NAME_WITH_I18N(PeriodStyle, "。", "．")
FCITX_CONFIG_ENUM_NAME_WITH_I18N(CommaStyle, "、", "，")

FCITX_CONFIG_ENUM_NAME_WITH_I18N(CandidateLayoutHint, N_("Not set"),
                                 N_("Vertical"), N_("Horizontal"));

FCITX_CONFIG_ENUM(CandidateSelectionKeys, Number, ABCD, QwertyCenter)
static constexpr const char *CandidateSelectionKeys_Annotations[] = {
    "Number (1,2,3,...)", "ABCD (a,b,c,d,...)",
    "Qwerty Center row (a,s,d,f,...)"};
}
#endif // FCITX5_CSKK_CSKKCONFIG_H
