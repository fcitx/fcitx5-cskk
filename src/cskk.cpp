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
/*******************************************************************************
 * CskkEngine
 ******************************************************************************/

CskkEngine::CskkEngine(Instance *instance)
    : instance_{instance}, factory_([this](InputContext &ic) {
        auto newCskkContext = new FcitxCskkContext(this, &ic);
        return newCskkContext;
      }) {
  instance_->inputContextManager().registerProperty("cskkcontext", &factory_);
};
CskkEngine::~CskkEngine() = default;
void CskkEngine::keyEvent(const InputMethodEntry &, KeyEvent &keyEvent) {
  // delegate to context
  auto ic = keyEvent.inputContext();
  auto context = ic->propertyFor(&factory_);
  context->keyEvent(keyEvent);
}
void CskkEngine::save() {}
void CskkEngine::activate(const InputMethodEntry &, InputContextEvent &) {}
void CskkEngine::deactivate(const InputMethodEntry &entry,
                            InputContextEvent &event) {
  FCITX_UNUSED(entry);
  auto context = event.inputContext()->propertyFor(&factory_);
  context->commitPreedit();
}
void CskkEngine::reset(const InputMethodEntry &entry,
                       InputContextEvent &event) {
  FCITX_UNUSED(entry);
  auto ic = event.inputContext();
  auto context = ic->propertyFor(&factory_);
  context->reset();
}

/*******************************************************************************
 * CskkContext
 ******************************************************************************/

FcitxCskkContext::FcitxCskkContext(CskkEngine *engine, InputContext *ic)
    : context_(skk_context_new(nullptr, 0)), ic_(ic), engine_(engine) {}
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
void FcitxCskkContext::commitPreedit() {
  auto str = skk_context_get_preedit(context_);
  ic_->commitString(str);
  skk_free_string(str);
}
void FcitxCskkContext::reset() { skk_context_reset(context_); }

/*******************************************************************************
 * CskkFactory
 ******************************************************************************/

AddonInstance *CskkFactory::create(AddonManager *manager) {
  {
    registerDomain("fcitx5-cskk", FCITX_INSTALL_LOCALEDIR);
    return new CskkEngine(manager->instance());
  }
}
} // namespace fcitx

FCITX_ADDON_FACTORY(fcitx::CskkFactory);
