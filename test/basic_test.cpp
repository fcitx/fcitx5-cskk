/*
 * Copyright (c) 2021 Naoaki Iwakiri
 * This program is released under GNU General Public License version 3 or later
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Creation Date: 2021-05-08
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileType: SOURCE
 * SPDX-FileName: basic_test.cpp
 * SPDX-FileCopyrightText: Copyright (c) 2021 Naoaki Iwakiri
 */

#include "gtest/gtest.h"
// TODO: Add real test
// Test to check that test compiles...
class TestTest : public ::testing::Test {};

TEST_F(TestTest, A) { EXPECT_EQ(1, 1); }
TEST_F(TestTest, B) { EXPECT_EQ(1, 2); }