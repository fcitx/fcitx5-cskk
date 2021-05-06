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
#include <cstdlib>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/log.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputpanel.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
using std::getenv;
using std::string;

namespace fcitx {
FCITX_DEFINE_LOG_CATEGORY(cskk_log, "cskk");

#define CSKK_DEBUG() FCITX_LOGC(cskk_log, Debug) << "\t**CSKK** "
#define CSKK_WARN() FCITX_LOGC(cskk_log, Warn) << "\t**CSKK** "

/*******************************************************************************
 * CskkEngine
 ******************************************************************************/
const string CskkEngine::config_file_path = "conf/fcitx5-cskk";
CskkEngine::CskkEngine(Instance *instance)
    : instance_{instance}, factory_([this](InputContext &ic) {
        auto newCskkContext = new FcitxCskkContext(this, &ic);
        newCskkContext->applyConfig();
        return newCskkContext;
      }) {
  reloadConfig();
  instance_->inputContextManager().registerProperty("cskkcontext", &factory_);
}
CskkEngine::~CskkEngine() = default;
void CskkEngine::keyEvent(const InputMethodEntry &, KeyEvent &keyEvent) {
  CSKK_DEBUG() << "Engine keyEvent start: " << keyEvent.rawKey();
  // delegate to context
  auto ic = keyEvent.inputContext();
  auto context = ic->propertyFor(&factory_);
  context->keyEvent(keyEvent);
  CSKK_DEBUG() << "Engine keyEvent end";
}
void CskkEngine::save() {}
void CskkEngine::activate(const InputMethodEntry &, InputContextEvent &) {}
void CskkEngine::deactivate(const InputMethodEntry &entry,
                            InputContextEvent &event) {
  FCITX_UNUSED(entry);
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
void CskkEngine::setConfig(const RawConfig &config) {
  CSKK_DEBUG() << "Cskk setconfig";
  config_.load(config, true);
  safeSaveAsIni(config_, CskkEngine::config_file_path);
  reloadConfig();
}
void CskkEngine::reloadConfig() {
  CSKK_DEBUG() << "Cskkengine reload config";
  readAsIni(config_, CskkEngine::config_file_path);

  loadDictionary();
  if (factory_.registered()) {
    instance_->inputContextManager().foreach ([this](InputContext *ic) {
      auto context = ic->propertyFor(&factory_);
      context->applyConfig();
      return true;
    });
  }
}
void CskkEngine::loadDictionary() {
  freeDictionaries();

  const std::filesystem::directory_options directoryOptions =
      (std::filesystem::directory_options::skip_permission_denied |
       std::filesystem::directory_options::follow_directory_symlink);

  // For time being, just load files from XDG_DATA_{HOME,DIRS} only.
  try {
    std::filesystem::path dataDir = getXDGDataHome();
    dataDir.append("fcitx5-cskk/dictionary");
    CSKK_DEBUG() << dataDir;
    for (const auto &file :
         std::filesystem::directory_iterator(dataDir, directoryOptions)) {
      if (file.is_regular_file()) {
        dictionaries_.emplace_back(
            skk_user_dict_new(file.path().c_str(), "UTF-8"));
      }
    }
  } catch (std::filesystem::filesystem_error &ignored) {
    CSKK_WARN() << "Read userdictionary failed. Skipping.";
  }

  std::vector<string> xdgDataDirs = getXDGDataDirs();

  for (const auto &xdgDataDir : xdgDataDirs) {
    std::filesystem::path dataDir = xdgDataDir;
    dataDir.append("fcitx5-cskk/dictionary");
    CSKK_DEBUG() << dataDir;
    try {
      for (const auto &file :
           std::filesystem::directory_iterator(dataDir, directoryOptions)) {
        if (file.is_regular_file()) {
          dictionaries_.emplace_back(
              skk_file_dict_new(file.path().c_str(), "UTF-8"));
        }
      }
    } catch (std::filesystem::filesystem_error &ignored) {
      CSKK_WARN() << "Read static dictionary failed. Skipping.";
    }
  }
}
std::string CskkEngine::getXDGDataHome() {
  const char *xdgDataHomeEnv = getenv("XDG_DATA_HOME");
  const char *homeEnv = getenv("$HOME");
  if (xdgDataHomeEnv) {
    return string(xdgDataHomeEnv);
  } else if (homeEnv) {
    return string(homeEnv) + "/.local/share";
  }
  return "";
}

std::vector<std::string> CskkEngine::getXDGDataDirs() {
  const char *xdgDataDirEnv = getenv("XDG_DATA_DIRS");
  string rawDirs;

  if (xdgDataDirEnv) {
    rawDirs = string(xdgDataDirEnv);
  } else {
    rawDirs = "/usr/local/share:/usr/share";
  }

  std::vector<string> xdgDataDirs;
  auto offset = 0;
  while (true) {
    auto pos = rawDirs.find(':', offset);

    if (pos == std::string::npos) {
      xdgDataDirs.emplace_back(rawDirs.substr(offset));
      break;
    }
    xdgDataDirs.emplace_back(rawDirs.substr(offset, pos - offset));
    offset = pos + 1;
  }
  CSKK_DEBUG() << xdgDataDirs;
  return xdgDataDirs;
}
void CskkEngine::freeDictionaries() {
  CSKK_DEBUG() << "Cskk free dict";
  for (auto dictionary : dictionaries_) {
    skk_free_dictionary(dictionary);
  }
  dictionaries_.clear();
}

/*******************************************************************************
 * CskkContext
 ******************************************************************************/

FcitxCskkContext::FcitxCskkContext(CskkEngine *engine, InputContext *ic)
    : context_(skk_context_new(nullptr, 0)), ic_(ic), engine_(engine) {
  CSKK_DEBUG() << "Cskk context new";
  skk_context_set_input_mode(context_, *engine_->config().inputMode);
}
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
  ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
}
void FcitxCskkContext::applyConfig() {
  CSKK_DEBUG() << "apply config";
  skk_context_set_dictionaries(context_, engine_->dictionaries().data(),
                               engine_->dictionaries().size());
}
void FcitxCskkContext::copyTo(InputContextProperty *context) {
  auto otherContext = dynamic_cast<FcitxCskkContext *>(context);
  skk_context_set_input_mode(otherContext->context(),
                             skk_context_get_input_mode(context_));
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
