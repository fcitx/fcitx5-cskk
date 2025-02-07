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
 * SPDX-FileName: fcitxcskkcandidatelist.cpp
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */

#include "cskkcandidatelist.h"
#include "cskk.h"
#include "cskkconfig.h"
#include "log.h"
#include <algorithm>
#include <cstdlib>
#include <fcitx/candidatelist.h>
#include <fcitx/inputcontext.h>
#include <fcitx/text.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

extern "C" {
#include <libcskk.h>
}

namespace fcitx {
/*******************************************************************************
 * FcitxCskkCandidateList
 ******************************************************************************/

FcitxCskkCandidateList::FcitxCskkCandidateList(FcitxCskkEngine *engine,
                                               InputContext *ic)
    : engine_(engine), ic_(ic) {
  CSKK_DEBUG() << "Construct FcitxCskkCandidateList";
  setPageable(this);
  setCursorMovable(this);

  FcitxCskkContext *fcitxCskkContext = engine_->context(ic_);
  CskkContext *cskkContext = fcitxCskkContext->context();

  auto totalSize = skk_context_get_current_candidate_count(cskkContext);
  std::vector<char *> candidates;
  candidates.resize(totalSize);
  auto loadedSize = skk_context_get_current_candidates(
      cskkContext, candidates.data(), totalSize, 0);
  totalSize_ = loadedSize;
  for (auto i = 0; i < static_cast<int>(totalSize_); i++) {
    Text text;
    text.append(candidates[i]);
    words_.emplace_back(
        std::make_unique<FcitxCskkCandidateWord>(engine_, text, i));
  }

  skk_free_candidate_list(candidates.data(), loadedSize);

  const auto &config = engine_->config();
  auto labels =
      FcitxCskkCandidateList::getLabels(config.candidateSelectionKeys.value());
  for (char label : labels) {
    auto theLabel = {label, ' ', '\0'};
    labels_.emplace_back(Text(theLabel));
  }

  char *to_composite = skk_context_get_current_to_composite(cskkContext);
  wordToComposite_ = std::string(to_composite);
  skk_free_string(to_composite);

  pageSize_ = config.pageSize.value();
  pageStartOffset_ = config.pageStartIdx.value() - 1;
  totalPage_ = ((totalSize_ - pageStartOffset_ - 1) / pageSize_);

  auto cursorPosition =
      skk_context_get_current_candidate_cursor_position(cskkContext);
  setCursorPosition(cursorPosition);
}
const Text &FcitxCskkCandidateList::label(int idx) const {
  return labels_[idx];
}
const CandidateWord &FcitxCskkCandidateList::candidate(int idx) const {
  return *words_[pageFirst_ + idx];
}
// number of candidates on the current page.
int FcitxCskkCandidateList::size() const {
  return static_cast<int>(pageLast_ - pageFirst_);
}
int FcitxCskkCandidateList::cursorIndex() const { return cursorIndex_; }
CandidateLayoutHint FcitxCskkCandidateList::layoutHint() const {
  return *engine_->config().candidateLayout;
}
void FcitxCskkCandidateList::prevCandidate() {
  auto newCursorPos = static_cast<int>(cursorIndex_ + pageFirst_ - 1);
  setCursorPosition(newCursorPos);
}
void FcitxCskkCandidateList::nextCandidate() {
  auto newCursorPos = static_cast<int>(cursorIndex_ + pageFirst_ + 1);
  setCursorPosition(newCursorPos);
}
void FcitxCskkCandidateList::setCursorPosition(int cursorPosition) {
  // This might be duplicating call to the same cursor position, but doesn't
  // harm to sync the position.
  FcitxCskkContext *fcitxCskkContext = engine_->context(ic_);
  CskkContext *cskkContext = fcitxCskkContext->context();
  skk_context_select_candidate_at(cskkContext, cursorPosition);

  // Assume totalSize = 27, cursorPosition = 14, pageStartOffset = 3, page_size
  // = 10.
  //
  // 0~3: not in page. 4~13: 0th page. 14~23: 1st page. 24~26: 2nd page.
  currentPage_ =
      static_cast<int>((cursorPosition - pageStartOffset_ - 1) / pageSize_);
  pageFirst_ = currentPage_ * pageSize_ + pageStartOffset_ + 1;
  pageLast_ = std::min(totalSize_, pageFirst_ + pageSize_);
  cursorIndex_ = static_cast<int>(cursorPosition - pageFirst_);

  // assert!(pageFirst_ <= cursorPosition)
  // assert!(cursorPosition < pageLast_)
  // assert!(pageFirst_ + cursorIndex_ < pageLast_)
}
bool FcitxCskkCandidateList::hasPrev() const { return currentPage_ > 1; }
bool FcitxCskkCandidateList::hasNext() const {
  return currentPage_ < static_cast<int>(totalPage_);
}
void FcitxCskkCandidateList::prev() {
  auto newCursorPos = static_cast<int>(pageFirst_ - pageSize_);
  setCursorPosition(newCursorPos);
}
void FcitxCskkCandidateList::next() {
  auto newCursorPos = static_cast<int>(pageFirst_ + pageSize_);
  usedNextBefore_ = true;
  setCursorPosition(newCursorPos);
}
int FcitxCskkCandidateList::totalPages() const {
  return static_cast<int>(totalPage_);
}
int FcitxCskkCandidateList::currentPage() const {
  return static_cast<int>(currentPage_);
}
void FcitxCskkCandidateList::setPage(int page) {
  auto newCursorPos =
      static_cast<int>(pageStartOffset_ + (page * pageSize_) + 1);
  setCursorPosition(newCursorPos);
}
bool FcitxCskkCandidateList::usedNextBefore() const { return usedNextBefore_; }
std::string FcitxCskkCandidateList::getLabels(
    CandidateSelectionKeys candidateSelectionKeys) {
  switch (candidateSelectionKeys) {
  case CandidateSelectionKeys::ABCD:
    return "abcdefghij";
  case CandidateSelectionKeys::QwertyCenter:
    return "asdfghjkl;";
  case CandidateSelectionKeys::Number:
  default:
    return "1234567890";
  }
}

FcitxCskkCandidateList::~FcitxCskkCandidateList() = default;

/*******************************************************************************
 * FcitxCskkCandidateWord
 ******************************************************************************/
FcitxCskkCandidateWord::FcitxCskkCandidateWord(FcitxCskkEngine *engine,
                                               Text text, int cursorPosition)
    : engine_(engine), cursorPosition_(cursorPosition) {
  CSKK_DEBUG() << "candidate word create: " << text.toString();
  setText(std::move(text));
}
void FcitxCskkCandidateWord::select(InputContext *inputContext) const {
  {
    auto *fcitxCskkContext = engine_->context(inputContext);
    skk_context_confirm_candidate_at(fcitxCskkContext->context(),
                                     static_cast<int>(cursorPosition_));
    fcitxCskkContext->updateUI();
  }
}

} // namespace fcitx