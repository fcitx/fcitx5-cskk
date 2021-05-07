/*
 * Copyright (c) 2021 Naoaki Iwakiri
 * This program is released under GNU General Public License version 3 or later
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Creation Date: 2021-05-07
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileType: SOURCE
 * SPDX-FileName: log.h
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */

#ifndef FCITX5_CSKK_LOG_H
#define FCITX5_CSKK_LOG_H
#include <fcitx-utils/log.h>

FCITX_DECLARE_LOG_CATEGORY(cskk_log);
#define CSKK_DEBUG() FCITX_LOGC(cskk_log, Debug) << "\t**CSKK** "
#define CSKK_WARN() FCITX_LOGC(cskk_log, Warn) << "\t**CSKK** "

#endif // FCITX5_CSKK_LOG_H
