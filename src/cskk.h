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

#include "cskkconfig.h"
#include <cstdint>
#include <fcitx-config/configuration.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/key.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/candidatelist.h>
#include <fcitx/event.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <fcitx/text.h>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

extern "C" {
#include <libcskk.h>
}

namespace fcitx {

class FcitxCskkContext;
class FcitxCskkCandidateList;

class FcitxCskkEngine final : public InputMethodEngineV2 {
public:
  explicit FcitxCskkEngine(Instance *instance);
  ~FcitxCskkEngine() override;

  void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
  void activate(const InputMethodEntry & /*entry*/,
                InputContextEvent & /*event*/) override;
  void deactivate(const InputMethodEntry &entry,
                  InputContextEvent &event) override;
  void reset(const InputMethodEntry &entry, InputContextEvent &event) override;
  void save() override;
  std::string subModeIconImpl(const InputMethodEntry & /*unused*/,
                              InputContext & /*unused*/) override;

  // Configuration methods are called from fctix5-configtool via DBus message
  // to fcitx5 server.
  const Configuration *getConfig() const override { return &config_; }
  void setConfig(const RawConfig &config) override;
  void reloadConfig() override;

  const auto &dictionaries() { return dictionaries_; }
  const auto &config() { return config_; }
  FcitxCskkContext *context(InputContext *ic) {
    return ic->propertyFor(&factory_);
  }

  static KeyList
  getSelectionKeys(CandidateSelectionKeys candidateSelectionKeys);

  bool isEngineReady();

private:
  Instance *instance_;
  FactoryFor<FcitxCskkContext> factory_;
  FcitxCskkConfig config_;
  std::vector<CskkDictionaryFfi *> dictionaries_;

  void loadDictionary();
  void freeDictionaries();
};

class FcitxCskkContext final : public InputContextProperty {
public:
  FcitxCskkContext(FcitxCskkEngine *engine, InputContext *ic);
  ~FcitxCskkContext() override;
  // Copy used for sharing state among programs
  void copyTo(InputContextProperty *state) override;

  void keyEvent(KeyEvent &keyEvent);
  void reset();
  void updateUI();
  void applyConfig();

  bool saveDictionary();
  // value castable to InputMode
  int getInputMode();

  // FIXME: Ideally, don't use this context() and prepare public function for
  // each usage to separate the responsibility.
  auto &context() { return context_; }

private:
  // TODO: unique_ptr using some wrapper class for Rust exposed pointer? Need
  // bit more research.
  CskkContext *context_;
  InputContext *ic_;
  FcitxCskkEngine *engine_;
  bool handleCandidateSelection(
      const std::shared_ptr<FcitxCskkCandidateList> &candidateList,
      KeyEvent &keyEvent);
  static std::tuple<Text, Text> formatPreedit(CskkStateInfoFfi *cskkStateInfo,
                                              uint32_t stateLen);
};

class FcitxCskkFactory final : public AddonFactory {
public:
  AddonInstance *create(AddonManager *manager) override;
};

} // namespace fcitx
#endif // FCITX5_CSKK_CSKK_H
