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

struct CandidateSelectionKeysAnnotation : public EnumAnnotation {
  void dumpDescription(RawConfig &config) const {
    EnumAnnotation::dumpDescription(config);
    int length = sizeof(CandidateSelectionKeys_Annotations) /
                 sizeof(CandidateSelectionKeys_Annotations[0]);
    for (int i = 0; i < length; i++) {
      config.setValueByPath("Enum/" + std::to_string(i),
                            CandidateSelectionKeys_Annotations[i]);
      config.setValueByPath("EnumI18n/" + std::to_string(i),
                            _(CandidateSelectionKeys_Annotations[i]));
    }
  }
};

struct InputModeAnnotation : public EnumAnnotation {
  void dumpDescription(RawConfig &config) const {
    EnumAnnotation::dumpDescription(config);
    int length = sizeof(_InputMode_Names) / sizeof(_InputMode_Names[0]);
    for (int i = 0; i < length; i++) {
      // FIXME: bit not sure if path is correct. Might need namespacing per each
      // configuration?
      config.setValueByPath("Enum/" + std::to_string(i), _InputMode_Names[i]);
      config.setValueByPath("EnumI18n/" + std::to_string(i),
                            _(_InputMode_Names[i]));
    }
  }
};

struct FcitxCskkRuleAnnotation : public EnumAnnotation {
  void dumpDescription(RawConfig &config) const {
    EnumAnnotation::dumpDescription(config);
    uint length;
    auto rules = skk_get_rules(&length);
    for (uint i = 0; i < length; i++) {
      config.setValueByPath("Enum/" + std::to_string(i), rules[i].id);
      config.setValueByPath("EnumI18n/" + std::to_string(i), rules[i].name);
    }
    skk_free_rules(rules, length);
  }
};

FCITX_CONFIGURATION(
    FcitxCskkConfig,
    OptionWithAnnotation<std::string, FcitxCskkRuleAnnotation> cskkRule{
        this, "Rule", _("Rule"), "default"};
    OptionWithAnnotation<InputMode, InputModeAnnotation> inputMode{
        this, "InitialInputMode", _("InitialInputMode"), Hiragana};
    OptionWithAnnotation<PeriodStyle, PeriodStyleI18NAnnotation> periodStyle{
        this, "Period style", _("Period style"), PeriodStyle::PeriodJa};
    OptionWithAnnotation<CommaStyle, CommaStyleI18NAnnotation> commaStyle{
        this, "Comma style", _("Comma style"), CommaStyle::CommaJa};
    // Must be >0 because addon cannot hook in on first transition to
    // composition selection mode.
    Option<int, IntConstrain> pageStartIdx{
        this, "Candidates to show before showing in list",
        _("Candidates to show before showing in list"), 3, IntConstrain(1)};
    Option<int, IntConstrain> pageSize{this, "Candidate list page size",
                                       _("Candidate list page size"), 5,
                                       IntConstrain(1, 10)};
    OptionWithAnnotation<CandidateLayoutHint, CandidateLayoutHintI18NAnnotation>
        candidateLayout{this, "Candidate Layout", _("Candidate Layout"),
                        CandidateLayoutHint::Vertical};
    KeyListOption prevCursorKey{
        this,
        "CursorUp",
        _("Cursor Up"),
        {Key(FcitxKey_Up)},
        KeyListConstrain({KeyConstrainFlag::AllowModifierLess})};
    KeyListOption nextCursorKey{
        this,
        "CursorDown",
        _("Cursor Down"),
        {Key(FcitxKey_Down)},
        KeyListConstrain({KeyConstrainFlag::AllowModifierLess})};
    KeyListOption prevPageKey{
        this,
        "CandidatesPageUpKey",
        _("Candidates Page Previous Page"),
        {Key(FcitxKey_Page_Up), Key(FcitxKey_x)},
        KeyListConstrain({KeyConstrainFlag::AllowModifierLess})};
    KeyListOption nextPageKey{
        this,
        "CandidatesPageDownKey",
        _("Candidates Page Next Page"),
        {Key(FcitxKey_Page_Down), Key(FcitxKey_space)},
        KeyListConstrain({KeyConstrainFlag::AllowModifierLess})};

    OptionWithAnnotation<CandidateSelectionKeys,
                         CandidateSelectionKeysAnnotation>
        candidateSelectionKeys{this, "Candidate selection keys",
                               _("Candidate selection keys"),
                               CandidateSelectionKeys::Number};
    ExternalOption dictionary{this, "Dictionary", _("Dictionary"),
                              "fcitx://config/addon/cskk/dictionary_list"};
    //    Option<bool> showAnnotation{this, "ShowAnnotation",
    //                                _("Show Annotation. Fake yet."), true};);
    ) // FCITX_CONFIGURATION
} // namespace fcitx
#endif // FCITX5_CSKK_CSKKCONFIG_H
