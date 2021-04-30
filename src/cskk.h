/*
 * Copyright (c) 2021 Naoaki Iwakiri
 * This program is released under GNU General Public License version 3 or later
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>. Creation Date:
 * 2021-04-30
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileType: SOURCE
 * SPDX-FileName: cskk.h
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */

#ifndef FCITX5_CSKK_CSKK_H
#define FCITX5_CSKK_CSKK_H

extern "C" {
#include <cskk/libcskk.h>
}

#include <fcitx-utils/i18n.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>

namespace fcitx {
class FcitxCskkContext;
class CskkEngine final : public InputMethodEngine {
public:
  CskkEngine(Instance *instance);
  ~CskkEngine() override;

  void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
  void activate(const InputMethodEntry &, InputContextEvent &) override;
  void deactivate(const InputMethodEntry &entry,
                  InputContextEvent &event) override;
  void reset(const InputMethodEntry &entry,
             InputContextEvent &event) override;
  void save() override;

private:
  Instance *instance_;
  FactoryFor<FcitxCskkContext> factory_;
};

class FcitxCskkContext final : public InputContextProperty {
public:
  FcitxCskkContext(CskkEngine *engine, InputContext *ic);
  ~FcitxCskkContext() override;
  void keyEvent(KeyEvent &keyEvent);
  void commitPreedit();
  void reset();

private:
  // TODO: unique_ptr using some wrapper class for Rust exposed pointer? Need
  // bit more research.
  CskkContext *context_;
  InputContext *ic_;
  CskkEngine *engine_;
};

class CskkFactory final : public AddonFactory {
public:
  AddonInstance *create(AddonManager *manager) override;
};

} // namespace fcitx
#endif // FCITX5_CSKK_CSKK_H
