/*
 * Copyright (c) 2021 Naoaki Iwakiri
 * This program is released under GNU General Public License version 3 or later
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Creation Date: 2021-04-30
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileType: SOURCE
 * SPDX-FileName: cskk.cpp
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */
#include "cskk.h"
#include <fcitx-utils/log.h>
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
  CSKK_DEBUG() << "instance created";
}
CskkEngine::~CskkEngine() = default;
void CskkEngine::keyEvent(const InputMethodEntry &, KeyEvent &keyEvent) {
  CSKK_DEBUG() << "**** CSKK Engine keyEvent start: " << keyEvent.rawKey();
  // delegate to context
  auto ic = keyEvent.inputContext();
  auto context = ic->propertyFor(&factory_);
  context->keyEvent(keyEvent);
  CSKK_DEBUG() << "**** CSKK Engine keyEvent end";
}
void CskkEngine::save() {}
void CskkEngine::activate(const InputMethodEntry &, InputContextEvent &) {}
void CskkEngine::deactivate(const InputMethodEntry &entry,
                            InputContextEvent &event) {
  FCITX_UNUSED(entry);
  auto context = event.inputContext()->propertyFor(&factory_);
  context->commitPreedit();
  reset(entry, event);
}
void CskkEngine::reset(const InputMethodEntry &entry,
                       InputContextEvent &event) {
  FCITX_UNUSED(entry);
  CSKK_DEBUG() << "Reset";
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
    CSKK_DEBUG() << "Key processed in context.";
    keyEvent.filterAndAccept();
  }
  if (keyEvent.filtered()) {
    updateUI();
  }
}
void FcitxCskkContext::commitPreedit() {
  auto str = skk_context_get_preedit(context_);
  ic_->commitString(str);
  skk_free_string(str);
}
void FcitxCskkContext::reset() { skk_context_reset(context_); }
void FcitxCskkContext::updateUI() {
  auto &inputPanel = ic_->inputPanel();

  // Output
  if (auto output = skk_context_poll_output(context_)) {
    FCITX_DEBUG() << "**** CSKK output " << output;
    if (strlen(output) > 0) {
      ic_->commitString(output);
    }
    skk_free_string(output);
  }

  // Preedit
  // TODO: candidate in preedit?
  inputPanel.reset();
  auto preedit = skk_context_get_preedit(context_);
  Text preeditText;
  // FIXME: Pretty text format someday.
  preeditText.append(std::string(preedit));
  // FIXME: Narrowing conversion without check
  preeditText.setCursor(static_cast<int>(strlen(preedit)));

  if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
    inputPanel.setClientPreedit(preeditText);
    ic_->updatePreedit();
  } else {
    inputPanel.setPreedit(preeditText);
  }
}

/*******************************************************************************
 * CskkFactory
 ******************************************************************************/

AddonInstance *CskkFactory::create(AddonManager *manager) {
  {
    CSKK_DEBUG() << "**** CSKK CskkFactory Create ****";
    registerDomain("fcitx5-cskk", FCITX_INSTALL_LOCALEDIR);
    auto engine = new CskkEngine(manager->instance());
    return engine;
  }
}
} // namespace fcitx

FCITX_ADDON_FACTORY(fcitx::CskkFactory);
