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
#include "cskkconfig.h"
#include <fcitx/candidatelist.h>
#include <fcitx/text.h>
#include <memory>
#include <string>
#include <vector>

namespace fcitx {

class FcitxCskkCandidateWord;

class FcitxCskkCandidateList : public CandidateList,
                               public CursorMovableCandidateList,
                               public PageableCandidateList {
public:
  FcitxCskkCandidateList(FcitxCskkEngine *engine, InputContext *ic);
  ~FcitxCskkCandidateList() override;

  // CandidateList
  // disabling [[nodiscard]] lint since it's not clear how this should be used
  // from fcitx5.

  /// idx seems like the 0-origin idx in the shown page.
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  const Text &label(int idx) const override;
  /// idx seems like the 0-origin idx in the shown page.
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

  // PagableCandidateList
  // Need for paging
  // FIXME?: Assumed has previous 'page'. not candidate.
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  bool hasPrev() const override;
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  bool hasNext() const override;
  void prev() override;
  void next() override;
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  bool usedNextBefore() const override;
  // Following are optional.
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  int totalPages() const override;
  // NOLINTNEXTLINE(modernize-use-nodiscard)
  int currentPage() const override;
  void setPage(int /*unused*/) override;

  const auto &to_composite() { return wordToComposite_; }
  /// cursorPosition is the position of the original cskk candidate list.
  void setCursorPosition(int cursorPosition);

private:
  FcitxCskkEngine *engine_;
  InputContext *ic_;
  std::string wordToComposite_;
  std::vector<Text> labels_;
  std::vector<std::unique_ptr<FcitxCskkCandidateWord>> words_;

  int cursorIndex_{};
  unsigned int pageStartOffset_;
  unsigned int pageSize_;
  unsigned int totalSize_;
  unsigned int totalPage_;
  int currentPage_{};

  // Not sure what this is.
  bool usedNextBefore_ = false;

  // the position of the first candidate in this list in whole candidate list
  unsigned int pageFirst_{};
  // the position of the last candidate + 1 in this list in whole candidate
  // list. equals to pageFirst_ of next page if next page exists.
  unsigned int pageLast_{};

  static std::string getLabels(CandidateSelectionKeys candidateSelectionKeys);
};

class FcitxCskkCandidateWord : public CandidateWord {
public:
  FcitxCskkCandidateWord(FcitxCskkEngine *engine, Text text,
                         int cursorPosition);
  void select(InputContext *inputContext) const override;

private:
  FcitxCskkEngine *engine_;
  int cursorPosition_;
};

} // namespace fcitx
#endif // FCITX5_CSKK_CSKKCANDIDATELIST_H
