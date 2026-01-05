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
// Check some fcitx utils existence to support some old versions of fcitx5.
#include "cskk.h"
#include "cskkcandidatelist.h"
#include "cskkconfig.h"
#include "log.h"
#include <cstdint>
#include <cstring>
#include <fcitx-config/iniparser.h>
#include <fcitx-config/rawconfig.h>
#include <fcitx-utils/capabilityflags.h>
#if __has_include(<fcitx-utils/fdstreambuf.h>)
#include <fcitx-utils/fdstreambuf.h>
#define FCITX_HAS_FDSTREAMBUF 1
#else
#include <fcitx-utils/misc.h>
#define FCITX_HAS_FDSTREAMBUF 0
#endif
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/log.h>
#if __has_include(<fcitx-utils/standardpaths.h>)
#include <fcitx-utils/standardpaths.h>
#define FCITX_HAS_STANDARDPATHS 1
#else
#include <fcitx-utils/standardpath.h>
#define FCITX_HAS_STANDARDPATHS 0
#endif
#include <fcitx-utils/stringutils.h>
#include <fcitx-utils/textformatflags.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/event.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/inputpanel.h>
#include <fcitx/text.h>
#include <fcitx/userinterface.h>
#include <istream>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

extern "C" {
#include <fcntl.h>
#include <libcskk.h>
}

FCITX_DEFINE_LOG_CATEGORY(cskk_log, "cskk");

namespace fcitx {

/*******************************************************************************
 * FcitxCskkEngine
 ******************************************************************************/
constexpr std::string_view config_file_path = "conf/fcitx5-cskk";

FcitxCskkEngine::FcitxCskkEngine(Instance *instance)
    : instance_{instance}, factory_([this](InputContext &ic) {
        auto *newCskkContext = new FcitxCskkContext(this, &ic);
        newCskkContext->applyConfig();
        return newCskkContext;
      }) {
  reloadConfig();
  instance_->inputContextManager().registerProperty("cskkcontext", &factory_);
}

FcitxCskkEngine::~FcitxCskkEngine() = default;

void FcitxCskkEngine::keyEvent(const InputMethodEntry & /*entry*/,
                               KeyEvent &keyEvent) {
  CSKK_DEBUG() << "Engine keyEvent start: " << keyEvent.rawKey();
  // delegate to context
  auto *ic = keyEvent.inputContext();
  auto *context = ic->propertyFor(&factory_);
  context->keyEvent(keyEvent);
  CSKK_DEBUG() << "Engine keyEvent end";
}

void FcitxCskkEngine::save() {
  if (factory_.registered()) {
    instance_->inputContextManager().foreach ([this](InputContext *ic) {
      auto *context = ic->propertyFor(&factory_);
      return context->saveDictionary();
    });
  }
}

void FcitxCskkEngine::activate(const InputMethodEntry & /*entry*/,
                               InputContextEvent & /*event*/) {}

void FcitxCskkEngine::deactivate(const InputMethodEntry &entry,
                                 InputContextEvent &event) {
  reset(entry, event);
}

void FcitxCskkEngine::reset(const InputMethodEntry & /*entry*/,
                            InputContextEvent &event) {
  CSKK_DEBUG() << "Reset";
  auto *ic = event.inputContext();
  auto *context = ic->propertyFor(&factory_);
  context->reset();
}

void FcitxCskkEngine::setConfig(const RawConfig &config) {
  CSKK_DEBUG() << "Cskk setconfig";
  config_.load(config, true);
  safeSaveAsIni(config_, std::string(config_file_path));
  reloadConfig();
}

void FcitxCskkEngine::reloadConfig() {
  CSKK_DEBUG() << "Cskkengine reload config";
  readAsIni(config_, std::string(config_file_path));

  loadDictionary();
  if (factory_.registered()) {
    instance_->inputContextManager().foreach ([this](InputContext *ic) {
      auto *context = ic->propertyFor(&factory_);
      context->applyConfig();
      return true;
    });
  }
}

typedef enum _FcitxSkkDictType { FSDT_Invalid, FSDT_File } FcitxSkkDictType;

void FcitxCskkEngine::loadDictionary() {
  freeDictionaries();

#if FCITX_HAS_STANDARDPATHS
  auto dict_config_file = StandardPaths::global().open(
      StandardPathsType::PkgData, "cskk/dictionary_list");
#else
  auto dict_config_file = StandardPath::global().open(
      StandardPath::Type::PkgData, "cskk/dictionary_list", O_RDONLY);
#endif

  if (!dict_config_file.isValid()) {
    return;
  }

#if FCITX_HAS_FDSTREAMBUF
  IFDStreamBuf buf(dict_config_file.fd());
  std::istream in(&buf);
  std::string line;

  while (std::getline(in, line)) {
    const auto trimmed = stringutils::trimView(line);
#else
  UniqueFilePtr fp(fdopen(dict_config_file.fd(), "rb"));
  if (!fp) {
    return;
  }
  dict_config_file.release();

  UniqueCPtr<char> line;
  size_t len = 0;

  while (getline(line, &len, fp.get()) != -1) {
    const auto trimmed = stringutils::trim(line.get());
#endif
    const auto tokens = stringutils::split(trimmed, ",");

    if (tokens.size() < 3) {
      continue;
    }

    CSKK_DEBUG() << "Load dictionary: " << trimmed;

    FcitxSkkDictType type = FSDT_Invalid;
    int mode = 0;
    std::string path;
    std::string encoding;
    bool complete = false;

    for (const auto &token : tokens) {
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
      } else if (key == "complete") {
        if (value == "true") {
          complete = true;
        }
      }
    }

    // encoding is not optional now, but for compatibility from fcitx5-skk.
    encoding = !encoding.empty() ? encoding : "euc-jp";

    if (type == FSDT_Invalid) {
      CSKK_WARN() << "Dictionary entry has invalid type. Ignored.";
      continue;
    }
    if (path.empty() || mode == 0) {
      CSKK_WARN() << "Invalid dictionary path or mode. Ignored";
      continue;
    }

#if FCITX_HAS_STANDARDPATHS
    std::string_view partialpath = path;
    if (stringutils::consumePrefix(partialpath, "$FCITX_CONFIG_DIR/")) {
      path = StandardPaths::global().userDirectory(StandardPathsType::PkgData) /
             partialpath;
    }
#else
    constexpr char configDir[] = "$FCITX_CONFIG_DIR/";
    constexpr auto var_len = sizeof(configDir) - 1;
    if (stringutils::startsWith(path, configDir)) {
      path = stringutils::joinPath(
          StandardPath::global().userDirectory(StandardPath::Type::PkgData),
          path.substr(var_len));
    }
#endif

    if (mode == 1) {
      // readonly mode
      auto *dict = skk_file_dict_new(path.c_str(), encoding.c_str(), complete);
      if (dict) {
        CSKK_DEBUG() << "Adding file dict: " << path
                     << " complete:" << complete;
        dictionaries_.emplace_back(dict);
      } else {
        CSKK_WARN() << "Static dictionary load error. Ignored: " << path;
      }
    } else {
      // read/write mode
      auto *userdict =
          skk_user_dict_new(path.c_str(), encoding.c_str(), complete);
      if (userdict) {
        CSKK_DEBUG() << "Adding user dict: " << path;
        dictionaries_.emplace_back(userdict);
      } else {
        CSKK_WARN() << "User dictionary load error. Ignored: " << path;
      }
    }
  }
}

void FcitxCskkEngine::freeDictionaries() {
  CSKK_DEBUG() << "Cskk free dict";
  for (auto *dictionary : dictionaries_) {
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

std::string
FcitxCskkEngine::subModeIconImpl(const InputMethodEntry & /*unused*/,
                                 InputContext &ic) {
  auto *context = ic.propertyFor(&factory_);
  auto current_input_mode = context->getInputMode();
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
  default:
    return "";
  }
}

bool FcitxCskkEngine::isEngineReady() { return factory_.registered(); }

/*******************************************************************************
 * CskkContext
 ******************************************************************************/

FcitxCskkContext::FcitxCskkContext(FcitxCskkEngine *engine, InputContext *ic)
    : context_(skk_context_new(nullptr, 0)), ic_(ic), engine_(engine) {
  CSKK_DEBUG() << "Cskk context new";
  if (!context_) {
    // new context wasn't created
    CSKK_ERROR() << "Failed to create new cskk context";
  }
}

FcitxCskkContext::~FcitxCskkContext() { skk_free_context(context_); }

void FcitxCskkContext::keyEvent(KeyEvent &keyEvent) {
  if (!context_) {
    CSKK_ERROR() << "CSKK Context is not setup. Ignored key.";
    return;
  }
  auto candidateList = std::dynamic_pointer_cast<FcitxCskkCandidateList>(
      ic_->inputPanel().candidateList());
  if (candidateList != nullptr && !candidateList->empty()) {
    if (handleCandidateSelection(candidateList, keyEvent)) {
      updateUI();
      return;
    }
  }

  auto composition_mode = skk_context_get_composition_mode(context_);
  if (composition_mode != CompositionMode::CompositionSelection &&
      composition_mode != CompositionMode::Completion) {
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
        0 <= idx && idx < candidateList->size()) {
      CSKK_DEBUG() << "Select from page. Idx: " << idx;
      candidateList->candidate(idx).select(ic_);
      keyEvent.filterAndAccept();
    }
  }

  return keyEvent.filtered();
}

void FcitxCskkContext::reset() {
  if (context_) {
    skk_context_reset(context_);
  }
  updateUI();
}

void FcitxCskkContext::updateUI() {
  if (!context_) {
    CSKK_WARN() << "No context setup";
    return;
  }
  const auto &config = engine_->config();
  auto &inputPanel = ic_->inputPanel();
  inputPanel.reset();

  // Output
  if (auto *output = skk_context_poll_output(context_)) {
    CSKK_DEBUG() << "output: " << output;
    if (strlen(output) > 0) {
      ic_->commitString(output);
    }
    skk_free_string(output);
  }

  // Preedit
  uint32_t stateStackLen;
  auto *preeditDetail =
      skk_context_get_preedit_detail(context_, &stateStackLen);
  auto [mainPreedit, supplementPreedit] =
      FcitxCskkContext::formatPreedit(preeditDetail, stateStackLen);
  skk_free_preedit_detail(preeditDetail, stateStackLen);
  // CandidateList
  int currentCursorPosition =
      skk_context_get_current_candidate_cursor_position(context_);
  bool showCandidateList =
      currentCursorPosition > engine_->config().pageStartIdx.value() - 1;
  if (showCandidateList) {
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
      currentCandidateList->setCursorPosition(currentCursorPosition);
    }

  } else {
    inputPanel.setCandidateList(nullptr);
  }

  if (ic_->capabilityFlags().test(CapabilityFlag::Preedit)) {
    inputPanel.setClientPreedit(mainPreedit);
    if ((config.showAnnotationCondition.value() ==
         ShowAnnotationCondition::Always) ||
        (config.showAnnotationCondition.value() ==
             ShowAnnotationCondition::SingleCandidate &&
         !showCandidateList)) {
      inputPanel.setPreedit(supplementPreedit);
    }
    ic_->updatePreedit();
  } else {
    inputPanel.setPreedit(mainPreedit);
  }

  // StatusArea for status icon
  ic_->updateUserInterface(UserInterfaceComponent::StatusArea);
  ic_->updateUserInterface(UserInterfaceComponent::InputPanel);
}

void FcitxCskkContext::applyConfig() {
  CSKK_DEBUG() << "apply config";
  if (!context_) {
    CSKK_WARN() << "No context setup. Ignoring config.";
    return;
  }
  const auto &config = engine_->config();

  skk_context_set_rule(context_, config.cskkRule->c_str());
  skk_context_set_input_mode(context_, *config.inputMode);
  skk_context_set_dictionaries(context_, engine_->dictionaries().data(),
                               engine_->dictionaries().size());
  skk_context_set_period_style(context_, *config.periodStyle);
  skk_context_set_comma_style(context_, *config.commaStyle);
}

void FcitxCskkContext::copyTo(InputContextProperty * /*unused*/) {
  // auto otherContext = dynamic_cast<FcitxCskkContext *>(context);
  // Ignored.
  // Even if fcitx5 global option is set to share input state、it only shares
  // the selection of the addon for this input method.
  // fcitx5-cskk will just hold each cskkcontext in each input context's
  // property and shares nothing.
}

bool FcitxCskkContext::saveDictionary() {
  if (!context_) {
    CSKK_WARN() << "No cskk context setup. Ignored dictionary save.";
    return false;
  }
  skk_context_save_dictionaries(context_);
  // cskk v0.8 doesn't return value on save dict, return true for now.
  return true;
}

int FcitxCskkContext::getInputMode() {
  if (!context_) {
    CSKK_WARN() << "No cskk context setup. No inputmode.";
    return -1;
  }
  return skk_context_get_input_mode(context_);
}

/**
 * format preedit state into Text.
 * Returns tuple of <Main content Text, Supplement content Text>
 * Main content is something you always want to show, supplement content is
 * something you may show when you have space.
 */
std::tuple<Text, Text>
FcitxCskkContext::formatPreedit(CskkStateInfoFfi *cskkStateInfoArray,
                                uint32_t stateLen) {
  std::string precomposition_marker = "▽";
  std::string selection_marker = "▼";
  std::string completion_marker = "■";
  Text mainContent;
  Text supplementContent;
  size_t mainCursorIdx = 0;
  for (uint32_t i = 0; i < stateLen; i++) {
    auto cskkStateInfo = cskkStateInfoArray[i];
    switch (cskkStateInfo.tag) {
    case DirectStateInfo: {
      auto directStateInfo = cskkStateInfo.direct_state_info;
      if (directStateInfo.confirmed) {
        mainCursorIdx += strlen(directStateInfo.confirmed);
        mainContent.append(directStateInfo.confirmed, TextFormatFlag::NoFlag);
      }
      if (directStateInfo.unconverted) {
        mainCursorIdx += strlen(directStateInfo.unconverted);
        mainContent.append(directStateInfo.unconverted,
                           TextFormatFlag::Underline);
      }
    } break;
    case PreCompositionStateInfo: {
      auto precompositionStateInfo = cskkStateInfo.pre_composition_state_info;
      mainCursorIdx += precomposition_marker.length();
      mainContent.append(precomposition_marker, TextFormatFlag::DontCommit);
      if (precompositionStateInfo.confirmed) {
        mainCursorIdx += strlen(precompositionStateInfo.confirmed);
        mainContent.append(precompositionStateInfo.confirmed,
                           TextFormatFlag::NoFlag);
      }
      if (precompositionStateInfo.kana_to_composite) {
        mainCursorIdx += strlen(precompositionStateInfo.kana_to_composite);
        mainContent.append(precompositionStateInfo.kana_to_composite,
                           TextFormatFlag::Underline);
      }
      if (precompositionStateInfo.unconverted) {
        mainCursorIdx += strlen(precompositionStateInfo.unconverted);
        mainContent.append(precompositionStateInfo.unconverted,
                           TextFormatFlag::Underline);
      }
    } break;
    case PreCompositionOkuriganaStateInfo: {
      auto precompositionOkuriganaStateInfo =
          cskkStateInfo.pre_composition_okurigana_state_info;
      mainContent.append(precomposition_marker, TextFormatFlag::DontCommit);
      mainCursorIdx += precomposition_marker.length();
      if (precompositionOkuriganaStateInfo.confirmed) {
        mainCursorIdx += strlen(precompositionOkuriganaStateInfo.confirmed);
        mainContent.append(precompositionOkuriganaStateInfo.confirmed,
                           TextFormatFlag::NoFlag);
      }
      if (precompositionOkuriganaStateInfo.kana_to_composite) {
        mainCursorIdx +=
            strlen(precompositionOkuriganaStateInfo.kana_to_composite);
        mainContent.append(precompositionOkuriganaStateInfo.kana_to_composite,
                           TextFormatFlag::Underline);
      }
      if (precompositionOkuriganaStateInfo.unconverted) {
        mainContent.append("*", TextFormatFlags{TextFormatFlag::Underline,
                                                TextFormatFlag::DontCommit});
        mainCursorIdx +=
            strlen(precompositionOkuriganaStateInfo.unconverted) + 1;
        mainContent.append(precompositionOkuriganaStateInfo.unconverted,
                           TextFormatFlag::Underline);
      }
    } break;
    case CompositionSelectionStateInfo: {
      auto compositionSelectionStateInfo =
          cskkStateInfo.composition_selection_state_info;

      if (compositionSelectionStateInfo.confirmed) {
        mainContent.append(compositionSelectionStateInfo.confirmed);
        mainCursorIdx += strlen(compositionSelectionStateInfo.confirmed);
      }
      mainContent.append(selection_marker, TextFormatFlag::DontCommit);
      mainCursorIdx += selection_marker.length();

      std::string tmpContentString;
      if (compositionSelectionStateInfo.composited) {
        mainCursorIdx += strlen(compositionSelectionStateInfo.composited);
        tmpContentString.append(compositionSelectionStateInfo.composited);
      }
      if (compositionSelectionStateInfo.okuri) {
        mainCursorIdx += strlen(compositionSelectionStateInfo.okuri);
        tmpContentString.append(compositionSelectionStateInfo.okuri);
      }
      mainContent.append(tmpContentString, TextFormatFlag::Underline);

      if (compositionSelectionStateInfo.annotation) {
        supplementContent.append(compositionSelectionStateInfo.annotation,
                                 TextFormatFlag::DontCommit);
      }
    } break;
    case RegisterStateInfo: {
      auto registerStateInfo = cskkStateInfo.register_state_info;
      mainContent.append(selection_marker, TextFormatFlag::DontCommit);
      mainCursorIdx += selection_marker.length();
      if (registerStateInfo.confirmed) {
        mainCursorIdx += strlen(registerStateInfo.confirmed);
        mainContent.append(registerStateInfo.confirmed,
                           TextFormatFlag::DontCommit);
      }
      if (registerStateInfo.kana_to_composite) {
        mainCursorIdx += strlen(registerStateInfo.kana_to_composite);
        mainContent.append(registerStateInfo.kana_to_composite,
                           TextFormatFlag::DontCommit);
      }
      if (registerStateInfo.okuri) {
        mainCursorIdx += strlen(registerStateInfo.okuri);
        mainContent.append(registerStateInfo.okuri, TextFormatFlag::DontCommit);
      }
      if (registerStateInfo.postfix) {
        mainCursorIdx += strlen(registerStateInfo.postfix);
        mainContent.append(registerStateInfo.postfix,
                           TextFormatFlag::DontCommit);
      }
      mainCursorIdx += strlen("【");
      mainContent.append("【", TextFormatFlag::DontCommit);
    } break;
    case CompleteStateInfo: {
      auto completeStateInfo = cskkStateInfo.complete_state_info;

      if (completeStateInfo.confirmed) {
        mainContent.append(completeStateInfo.confirmed);
        mainCursorIdx += strlen(completeStateInfo.confirmed);
      }
      mainContent.append(completion_marker, TextFormatFlag::DontCommit);
      mainCursorIdx += completion_marker.length();

      if (completeStateInfo.completed) {
        mainCursorIdx += strlen(completeStateInfo.completed);
        mainContent.append(completeStateInfo.completed,
                           TextFormatFlag::Underline);
      }
      if (completeStateInfo.completed_midashi) {
        supplementContent.append(completeStateInfo.completed_midashi,
                                 TextFormatFlag::DontCommit);
      }
    } break;
    }
  }
  // FIXME: Silently assuming length is less than int_max here. May fail when
  // length is over UINT_MAX. very unlikely, not high priority.
  mainContent.setCursor((int)mainCursorIdx);
  // Starting from 1 on purpose. No paren for submodes on top of the stack.
  for (uint32_t i = 1; i < stateLen; i++) {
    mainContent.append("】", TextFormatFlag::DontCommit);
  }

  return {mainContent, supplementContent};
}

/*******************************************************************************
 * FcitxCskkFactory
 ******************************************************************************/

AddonInstance *FcitxCskkFactory::create(AddonManager *manager) {
  {
    CSKK_DEBUG() << "**** CSKK FcitxCskkFactory Create ****";
    registerDomain("fcitx5-cskk", FCITX_INSTALL_LOCALEDIR);
    auto *engine = new FcitxCskkEngine(manager->instance());
    if (engine->isEngineReady()) {
      return engine;
    }
    return nullptr;
  }
}
} // namespace fcitx

#ifdef FCITX_ADDON_FACTORY_V2
FCITX_ADDON_FACTORY_V2(cskk, fcitx::FcitxCskkFactory)
#else
FCITX_ADDON_FACTORY(fcitx::FcitxCskkFactory)
#endif
