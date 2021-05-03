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
 * SPDX-FileName: cskk.h
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */

#ifndef FCITX5_CSKK_CSKK_H
#define FCITX5_CSKK_CSKK_H

extern "C" {
#include <cskk/libcskk.h>
}

#include <fcitx-config/configuration.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>

namespace fcitx {
FCITX_DEFINE_LOG_CATEGORY(cskk_log, "cskk");

#define CSKK_DEBUG() FCITX_LOGC(cskk_log, Debug)

class FcitxCskkContext;
// TODO: Check how to use i18n annotation creation macro
FCITX_CONFIG_ENUM_NAME(InputMode, "Hiragana", "Katakana", "HankakuKana",
                       "Zenkaku", "Ascii");

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
FCITX_CONFIGURATION(
    CskkConfig,
    OptionWithAnnotation<InputMode, InputModeAnnotation> inputMode{
        this, "InitialInputMode", _("InitialInputMode. Fake yet."), Hiragana};
    Option<bool> showAnnotation{this, "ShowAnnotation",
                                _("Show Annotation. Fake yet."), true};);

class CskkEngine final : public InputMethodEngine {
public:
  CskkEngine(Instance *instance);
  ~CskkEngine() override;

  void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
  void activate(const InputMethodEntry &, InputContextEvent &) override;
  void deactivate(const InputMethodEntry &entry,
                  InputContextEvent &event) override;
  void reset(const InputMethodEntry &entry, InputContextEvent &event) override;
  void save() override;

  const Configuration *getConfig() const override { return &config_; }
  void setConfig(const RawConfig &config) override {
    CSKK_DEBUG() << "*** Cskk setconfig";
    config_.load(config, true);
    // TODO: Save. Any file name convention etc?
  }

private:
  Instance *instance_;
  FactoryFor<FcitxCskkContext> factory_;
  CskkConfig config_;
};

class FcitxCskkContext final : public InputContextProperty {
public:
  FcitxCskkContext(CskkEngine *engine, InputContext *ic);
  ~FcitxCskkContext() override;
  void keyEvent(KeyEvent &keyEvent);
  void commitPreedit();
  void reset();
  void updateUI();

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
