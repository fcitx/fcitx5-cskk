/*
 * Copyright (c) 2021 Naoaki Iwakiri
 * This program is released under GNU General Public License version 3 or later
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>. Creation Date:
 * 2021-04-30
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileType: SOURCE
 * SPDX-FileName: cskk.cpp
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */
#include "cskk.h"
#include <iostream>

namespace fcitx {
CskkEngine::CskkEngine(Instance *instance)
    : instance_{instance}, factory_([this](InputContext &ic) {
        auto newCskkContext = new FcitxCskkContext(this, &ic);
        return newCskkContext;
      }) {
  instance_->inputContextManager().registerProperty("skkState", &factory_);
};
CskkEngine::~CskkEngine() = default;
void CskkEngine::keyEvent(const InputMethodEntry &, KeyEvent &keyEvent) {
  // delegate to state
  auto ic = keyEvent.inputContext();
  auto state = ic->propertyFor(&factory_);
  state->keyEvent(keyEvent);
}
void CskkEngine::save() {}

////////////////////////////////////////////////////////////////////////////////
/// CskkContext
////////////////////////////////////////////////////////////////////////////////
FcitxCskkContext::FcitxCskkContext(CskkEngine *_engine, InputContext *_ic)
    : context_(skk_context_new(nullptr, 0)) {
          FCITX_UNUSED(_engine);
          FCITX_UNUSED(_ic);
}
FcitxCskkContext::~FcitxCskkContext() = default;
void FcitxCskkContext::keyEvent(KeyEvent &keyEvent) {
  // TODO: handleCandidate to utilize fcitx's paged candidate list

  uint32_t modifiers =
      static_cast<uint32_t>(keyEvent.rawKey().states() & KeyState::SimpleMask);

  CskkKeyEvent *cskkKeyEvent = skk_key_event_new_from_fcitx_keyevent(
      keyEvent.rawKey().sym(), modifiers, keyEvent.isRelease());

  if (skk_context_process_key_event(context_, cskkKeyEvent)) {
    keyEvent.filterAndAccept();
  }
}
} // namespace fcitx