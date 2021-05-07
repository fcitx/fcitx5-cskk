/*
 * Copyright (c) 2021 Naoaki Iwakiri
 * This program is released under GNU General Public License version 3 or later
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Creation Date: 2021-05-07
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileType: SOURCE
 * SPDX-FileName: fcitxcskkcandidatelist.h
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */

#ifndef FCITX5_CSKK_CSKKCANDIDATELIST_H
#define FCITX5_CSKK_CSKKCANDIDATELIST_H

#include "cskk.h"
#include <fcitx/candidatelist.h>
#include <string>

namespace fcitx {

class FcitxCskkCandidateWord;

class FcitxCskkCandidateList : public CandidateList,
                               public CursorMovableCandidateList {
  //                               public PageableCandidateList,
public:
  FcitxCskkCandidateList(FcitxCskkEngine *engine, InputContext *ic);
  ~FcitxCskkCandidateList() override;

  // CandidateList
  // disabling [[nodiscard]] lint since it's not clear how this should be used
  // from fcitx5. NOLINTNEXTLINE(modernize-use-nodiscard)
  const Text &label(int idx) const override;
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  const CandidateWord &candidate(int idx) const override;
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  int size() const override;
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  int cursorIndex() const override;
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  CandidateLayoutHint layoutHint() const override;

  // CursorMovableCandidateList
  void prevCandidate() override;
  void nextCandidate() override;

  const auto &to_composite() { return to_composite_; }
  void setCursorIndex(int idx);

private:
  FcitxCskkEngine *engine_;
  InputContext *ic_;
  std::string to_composite_;
  std::vector<Text> labels_;
  std::vector<std::unique_ptr<FcitxCskkCandidateWord>> words_;

  int cursorIndex_ = -1;
  int pageStartOffset_ = -1;
};

class FcitxCskkCandidateWord : public CandidateWord {
public:
  FcitxCskkCandidateWord(FcitxCskkEngine *engine, Text text, int idx);
  void select(InputContext *inputContext) const override;

private:
  FcitxCskkEngine *engine_;
  int idx_;
};

} // namespace fcitx
#endif // FCITX5_CSKK_CSKKCANDIDATELIST_H
