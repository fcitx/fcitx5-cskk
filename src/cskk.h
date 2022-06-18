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
#include "cskkconfig.h"
#include <fcitx/addonfactory.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>
#include <string>

namespace fcitx {

class FcitxCskkContext;
class FcitxCskkCandidateList;

class FcitxCskkEngine final : public InputMethodEngineV2 {
public:
  explicit FcitxCskkEngine(Instance *instance);
  ~FcitxCskkEngine() override;

  void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
  void activate(const InputMethodEntry &, InputContextEvent &) override;
  void deactivate(const InputMethodEntry &entry,
                  InputContextEvent &event) override;
  void reset(const InputMethodEntry &entry, InputContextEvent &event) override;
  void save() override;
  std::string subModeIconImpl(const InputMethodEntry &,
                              InputContext &) override;

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

private:
  Instance *instance_;
  FactoryFor<FcitxCskkContext> factory_;
  FcitxCskkConfig config_;
  std::vector<CskkDictionaryFfi *> dictionaries_;

  // TODO: Change to string_view when fcitx5 moved to C++17 or later
  static const std::string config_file_path;

  void loadDictionary();
  static std::string getXDGDataHome();
  static std::vector<std::string> getXDGDataDirs();

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
};

class FcitxCskkFactory final : public AddonFactory {
public:
  AddonInstance *create(AddonManager *manager) override;
};

} // namespace fcitx
#endif // FCITX5_CSKK_CSKK_H
