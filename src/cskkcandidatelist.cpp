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
#include "log.h"
#include <algorithm>

namespace fcitx {
/*******************************************************************************
 * FcitxCskkCandidateList
 ******************************************************************************/

FcitxCskkCandidateList::FcitxCskkCandidateList(FcitxCskkEngine *engine,
                                               InputContext *ic)
    : engine_(engine), ic_(ic) {
  CSKK_DEBUG() << "Construct FcitxCskkCandidateList";
  //  setPageable(this);
  setCursorMovable(this);
  // FIXME: とりあえず100個越える候補は無視して実装進めている
  uint bufsize = 100;
  char **candidates = static_cast<char **>(malloc(sizeof(char *) * bufsize));
  FcitxCskkContext *fcitxCskkContext = engine_->context(ic_);
  CskkContext *cskkContext = fcitxCskkContext->context();
  uint pageStartOffset = FcitxCskkEngine::pageStartIdx;
  pageStartOffset_ = static_cast<int>(pageStartOffset);
  uint actual_size = skk_context_get_current_candidates(
      cskkContext, candidates, bufsize, pageStartOffset_);

  cursorIndex_ =
      static_cast<int>(
          skk_context_get_current_canddiate_selection_at(cskkContext)) -
      pageStartOffset_;

  char *to_composite = skk_context_get_current_to_composite(cskkContext);

  to_composite_ = std::string(to_composite);
  skk_free_string(to_composite);

  for (int i = 0; i < static_cast<int>(actual_size); i++) {
    Text text;
    text.append(candidates[i]);
    labels_.emplace_back(Text(""));
    words_.emplace_back(
        std::make_unique<FcitxCskkCandidateWord>(engine_, text, i));
  }

  skk_free_candidate_list(candidates, actual_size);
  free(candidates);
}
const Text &FcitxCskkCandidateList::label(int idx) const {
  return labels_[idx];
}
const CandidateWord &FcitxCskkCandidateList::candidate(int idx) const {
  return *words_[idx];
}
int FcitxCskkCandidateList::size() const {
  return static_cast<int>(words_.size());
}
int FcitxCskkCandidateList::cursorIndex() const { return cursorIndex_; }
CandidateLayoutHint FcitxCskkCandidateList::layoutHint() const {
  return FcitxCskkEngine::layoutHint;
}
void FcitxCskkCandidateList::prevCandidate() {
  CskkContext *cskkContext = engine_->context(ic_)->context();
  cursorIndex_ -= 1;
  skk_context_select_candidate_at(cskkContext, cursorIndex_ + pageStartOffset_);
}
void FcitxCskkCandidateList::nextCandidate() {
  CskkContext *cskkContext = engine_->context(ic_)->context();
  cursorIndex_ += 1;
  skk_context_select_candidate_at(cskkContext, cursorIndex_ + pageStartOffset_);
}
void FcitxCskkCandidateList::setCursorIndex(int idx) { cursorIndex_ = idx; }

FcitxCskkCandidateList::~FcitxCskkCandidateList() = default;

/*******************************************************************************
 * FcitxCskkCandidateWord
 ******************************************************************************/
FcitxCskkCandidateWord::FcitxCskkCandidateWord(FcitxCskkEngine *engine,
                                               Text text, int idx)
    : CandidateWord(), engine_(engine), idx_(idx) {
  CSKK_DEBUG() << "candidate word create: " << text.toString();
  setText(std::move(text));
}
void FcitxCskkCandidateWord::select(InputContext *inputContext) const {
  {
    auto fcitxCskkContext = engine_->context(inputContext);
    skk_context_confirm_candidate_at(
        fcitxCskkContext->context(),
        static_cast<int>(idx_ + FcitxCskkEngine::pageStartIdx));
    fcitxCskkContext->updateUI();
  }
}

} // namespace fcitx