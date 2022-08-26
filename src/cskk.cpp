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
#include "cskkcandidatelist.h"
#include "log.h"
#include <cstdlib>
#include <cstring>
#include <fcitx-config/iniparser.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputpanel.h>
#include <filesystem>
#include <string>
#include <vector>
extern "C" {
#include <fcntl.h>
}
using std::getenv;
using std::string;

FCITX_DEFINE_LOG_CATEGORY(cskk_log, "cskk");

namespace fcitx {

/*******************************************************************************
 * FcitxCskkEngine
 ******************************************************************************/
const string FcitxCskkEngine::config_file_path = string{"conf/fcitx5-cskk"};

FcitxCskkEngine::FcitxCskkEngine(Instance *instance)
    : instance_{instance}, factory_([this](InputContext &ic) {
        auto newCskkContext = new FcitxCskkContext(this, &ic);
        newCskkContext->applyConfig();
        return newCskkContext;
      }) {
  reloadConfig();
  instance_->inputContextManager().registerProperty("cskkcontext", &factory_);
}
FcitxCskkEngine::~FcitxCskkEngine() = default;
void FcitxCskkEngine::keyEvent(const InputMethodEntry &, KeyEvent &keyEvent) {
  CSKK_DEBUG() << "Engine keyEvent start: " << keyEvent.rawKey();
  // delegate to context
  auto ic = keyEvent.inputContext();
  auto context = ic->propertyFor(&factory_);
  context->keyEvent(keyEvent);
  CSKK_DEBUG() << "Engine keyEvent end";
}
void FcitxCskkEngine::save() {
  if (factory_.registered()) {
    instance_->inputContextManager().foreach ([this](InputContext *ic) {
      auto context = ic->propertyFor(&factory_);
      skk_context_save_dictionaries(context->context());
      return true;
    });
  }
}
void FcitxCskkEngine::activate(const InputMethodEntry &, InputContextEvent &) {}
void FcitxCskkEngine::deactivate(const InputMethodEntry &entry,
                                 InputContextEvent &event) {
  FCITX_UNUSED(entry);
  reset(entry, event);
}
void FcitxCskkEngine::reset(const InputMethodEntry &entry,
                            InputContextEvent &event) {
  FCITX_UNUSED(entry);
  CSKK_DEBUG() << "Reset";
  auto ic = event.inputContext();
  auto context = ic->propertyFor(&factory_);
  context->reset();
}
void FcitxCskkEngine::setConfig(const RawConfig &config) {
  CSKK_DEBUG() << "Cskk setconfig";
  config_.load(config, true);
  safeSaveAsIni(config_, FcitxCskkEngine::config_file_path);
  reloadConfig();
}
void FcitxCskkEngine::reloadConfig() {
  CSKK_DEBUG() << "Cskkengine reload config";
  readAsIni(config_, FcitxCskkEngine::config_file_path);

  loadDictionary();
  if (factory_.registered()) {
    instance_->inputContextManager().foreach ([this](InputContext *ic) {
      auto context = ic->propertyFor(&factory_);
      context->applyConfig();
      return true;
    });
  }
}

typedef enum _FcitxSkkDictType { FSDT_Invalid, FSDT_File } FcitxSkkDictType;
void FcitxCskkEngine::loadDictionary() {
  freeDictionaries();

  auto dict_config_file = StandardPath::global().open(
      StandardPath::Type::PkgData, "cskk/dictionary_list", O_RDONLY);

  UniqueFilePtr fp(fdopen(dict_config_file.fd(), "rb"));
  if (!fp) {
    return;
  }

  UniqueCPtr<char> buf;
  size_t len = 0;

  while (getline(buf, &len, fp.get()) != -1) {
    const auto trimmed = stringutils::trim(buf.get());
    const auto tokens = stringutils::split(trimmed, ",");

    if (tokens.size() < 3) {
      continue;
    }

    CSKK_DEBUG() << "Load dictionary: " << trimmed;

    FcitxSkkDictType type = FSDT_Invalid;
    int mode = 0;
    std::string path;
    std::string encoding;
    for (auto &token : tokens) {
      auto equal = token.find('=');
      if (equal == std::string::npos) {
        continue;
      }

      auto key = token.substr(0, equal);
      auto value = token.substr(equal + 1);

      // These keys from gui/dictmodel.cpp
      // Couldn't find a good way to make parser common for Qt classes and here.
      if (key == "type") {
        if (value == "file") {
          type = FSDT_File;
        }
      } else if (key == "file") {
        path = value;
      } else if (key == "mode") {
        if (value == "readonly") {
          mode = 1;
        } else if (value == "readwrite") {
          mode = 2;
        }
      } else if (key == "encoding") {
        encoding = value;
      }
    }

    encoding = !encoding.empty() ? encoding : "euc-jp";

    if (type == FSDT_Invalid) {
      continue;
    } else if (type == FSDT_File) {
      if (path.empty() || mode == 0) {
        continue;
      }
      if (mode == 1) {
        // readonly mode
        auto *dict = skk_file_dict_new(path.c_str(), encoding.c_str());
        if (dict) {
          CSKK_DEBUG() << "Adding file dict: " << path;
          dictionaries_.emplace_back(dict);
        }
      } else {
        // read/write mode
        constexpr char configDir[] = "$FCITX_CONFIG_DIR/";
        constexpr auto len = sizeof(configDir) - 1;
        std::string realpath = path;
        if (stringutils::startsWith(path, configDir)) {
          realpath = stringutils::joinPath(
              StandardPath::global().userDirectory(StandardPath::Type::PkgData),
              path.substr(len));
        }
        auto *userdict = skk_user_dict_new(realpath.c_str(), encoding.c_str());
        if (userdict) {
          CSKK_DEBUG() << "Adding user dict: " << realpath;
          dictionaries_.emplace_back(userdict);
        }
      }
    }
  }
}

void FcitxCskkEngine::freeDictionaries() {
  CSKK_DEBUG() << "Cskk free dict";
  for (auto dictionary : dictionaries_) {
    skk_free_dictionary(dictionary);
  }
  dictionaries_.clear();
}
KeyList FcitxCskkEngine::getSelectionKeys(
    CandidateSelectionKeys candidateSelectionKeys) {
  switch (candidateSelectionKeys) {
  case CandidateSelectionKeys::ABCD:
    return fcitx::KeyList{Key(FcitxKey_a), Key(FcitxKey_b), Key(FcitxKey_c),
                          Key(FcitxKey_d), Key(FcitxKey_e), Key(FcitxKey_f),
                          Key(FcitxKey_g), Key(FcitxKey_h), Key(FcitxKey_i),
                          Key(FcitxKey_j)};
  case CandidateSelectionKeys::QwertyCenter:
    return fcitx::KeyList{Key(FcitxKey_a), Key(FcitxKey_s),
                          Key(FcitxKey_d), Key(FcitxKey_f),
                          Key(FcitxKey_g), Key(FcitxKey_h),
                          Key(FcitxKey_j), Key(FcitxKey_k),
                          Key(FcitxKey_l), Key(FcitxKey_semicolon)};
  case CandidateSelectionKeys::Number:
  default:
    return fcitx::KeyList{Key(FcitxKey_1), Key(FcitxKey_2), Key(FcitxKey_3),
                          Key(FcitxKey_4), Key(FcitxKey_5), Key(FcitxKey_6),
                          Key(FcitxKey_7), Key(FcitxKey_8), Key(FcitxKey_9),
                          Key(FcitxKey_0)};
  }
}
std::string FcitxCskkEngine::subModeIconImpl(const InputMethodEntry &,
                                             InputContext &ic) {
  auto context = ic.propertyFor(&factory_);
  auto current_input_mode = skk_context_get_input_mode(context->context());
  switch (current_input_mode) {
  case InputMode::Ascii:
    return "cskk-ascii";
  case InputMode::HankakuKatakana:
    return "cskk-hankakukana";
  case InputMode::Hiragana:
    return "cskk-hiragana";
  case InputMode::Katakana:
    return "cskk-katakana";
  case InputMode::Zenkaku:
    return "cskk-zenei";
  }
  return "";
}

/*******************************************************************************
 * CskkContext
 ******************************************************************************/

FcitxCskkContext::FcitxCskkContext(FcitxCskkEngine *engine, InputContext *ic)
    : context_(skk_context_new(nullptr, 0)), ic_(ic), engine_(engine) {
  CSKK_DEBUG() << "Cskk context new";
  skk_context_set_input_mode(context_, *engine_->config().inputMode);
}
FcitxCskkContext::~FcitxCskkContext() = default;
void FcitxCskkContext::keyEvent(KeyEvent &keyEvent) {
  auto candidateList = std::dynamic_pointer_cast<FcitxCskkCandidateList>(
      ic_->inputPanel().candidateList());
  if (candidateList != nullptr && !candidateList->empty()) {
    if (handleCandidateSelection(candidateList, keyEvent)) {
      updateUI();
      return;
    }
  }

  if (skk_context_get_composition_mode(context_) !=
      CompositionMode::CompositionSelection) {
    CSKK_DEBUG() << "not composition selection. destroy candidate list.";
    ic_->inputPanel().setCandidateList(nullptr);
  }

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
bool FcitxCskkContext::handleCandidateSelection(
    const std::shared_ptr<FcitxCskkCandidateList> &candidateList,
    KeyEvent &keyEvent) {
  if (keyEvent.isRelease()) {
    return false;
  }
  CSKK_DEBUG() << "handleCandidateSelection";
  const auto config = engine_->config();

  if (keyEvent.key().checkKeyList(*config.prevCursorKey)) {
    candidateList->prevCandidate();
    keyEvent.filterAndAccept();
  } else if (keyEvent.key().checkKeyList(*config.nextCursorKey)) {
    candidateList->nextCandidate();
    keyEvent.filterAndAccept();
  } else if (keyEvent.key().checkKeyList(*config.nextPageKey)) {
    candidateList->next();
    keyEvent.filterAndAccept();
  } else if (keyEvent.key().checkKeyList(*config.prevPageKey)) {
    candidateList->prev();
    keyEvent.filterAndAccept();
  } else if (keyEvent.key().check(Key(FcitxKey_Return))) {
    CSKK_DEBUG() << "return key caught in handle candidate";
    candidateList->candidate(candidateList->cursorIndex()).select(ic_);
    keyEvent.filterAndAccept();
  } else {
    KeyList selectionKeys = FcitxCskkEngine::getSelectionKeys(
        engine_->config().candidateSelectionKeys.value());
    if (auto idx = keyEvent.key().keyListIndex(selectionKeys);
        0 <= idx && idx < engine_->config().pageSize.value()) {
      CSKK_DEBUG() << "Select from page. Idx: " << idx;
      candidateList->candidate(idx).select(ic_);
      keyEvent.filterAndAccept();
    }
  }

  return keyEvent.filtered();
}

void FcitxCskkContext::reset() {
  skk_context_reset(context_);
  updateUI();
}
void FcitxCskkContext::updateUI() {
  auto &inputPanel = ic_->inputPanel();

  // Output
  if (auto output = skk_context_poll_output(context_)) {
    CSKK_DEBUG() << "output: " << output;
    if (strlen(output) > 0) {
      ic_->commitString(output);
    }
    skk_free_string(output);
  }

  // Preedit
  auto preedit = skk_context_get_preedit(context_);
  Text preeditText;
  // FIXME: Pretty text format someday.
  preeditText.append(std::string(preedit));
  preeditText.setCursor(static_cast<int>(strlen(preedit)));

  // CandidateList
  int currentCursorPosition =
      skk_context_get_current_candidate_cursor_position(context_);
  if (currentCursorPosition > engine_->config().pageStartIdx.value() - 1) {
    char *current_to_composite = skk_context_get_current_to_composite(context_);
    auto currentCandidateList =
        std::dynamic_pointer_cast<FcitxCskkCandidateList>(
            ic_->inputPanel().candidateList());
    if (currentCandidateList == nullptr || currentCandidateList->empty() ||
        (strcmp(currentCandidateList->to_composite().c_str(),
                current_to_composite) != 0)) {
      // update whole currentCandidateList only if needed
      CSKK_DEBUG() << "Set new candidate list on UI update";
      inputPanel.setCandidateList(
          std::make_unique<FcitxCskkCandidateList>(engine_, ic_));
    } else {
      // Sync UI with actual data
      currentCandidateList->setCursorPosition(
          static_cast<int>(currentCursorPosition));
    }

  } else {
    inputPanel.setCandidateList(nullptr);
  }

  if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
    inputPanel.setClientPreedit(preeditText);
    ic_->updatePreedit();
  } else {
    inputPanel.setPreedit(preeditText);
  }

  // StatusArea for status icon
  ic_->updateUserInterface(UserInterfaceComponent::StatusArea);
  ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
}
void FcitxCskkContext::applyConfig() {
  CSKK_DEBUG() << "apply config";
  auto &config = engine_->config();

  skk_context_set_rule(context_, config.cskkRule->c_str());
  skk_context_set_dictionaries(context_, engine_->dictionaries().data(),
                               engine_->dictionaries().size());
  skk_context_set_period_style(context_, *config.periodStyle);
  skk_context_set_comma_style(context_, *config.commaStyle);
}
void FcitxCskkContext::copyTo(InputContextProperty *context) {
  auto otherContext = dynamic_cast<FcitxCskkContext *>(context);
  skk_context_set_input_mode(otherContext->context(),
                             skk_context_get_input_mode(context_));
}

/*******************************************************************************
 * FcitxCskkFactory
 ******************************************************************************/

AddonInstance *FcitxCskkFactory::create(AddonManager *manager) {
  {
    CSKK_DEBUG() << "**** CSKK FcitxCskkFactory Create ****";
    registerDomain("fcitx5-cskk", FCITX_INSTALL_LOCALEDIR);
    auto engine = new FcitxCskkEngine(manager->instance());
    return engine;
  }
}
} // namespace fcitx

FCITX_ADDON_FACTORY(fcitx::FcitxCskkFactory);
