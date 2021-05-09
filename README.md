## fcitx5-cskk
[Fcitx5](https://github.com/fcitx/fcitx5) でskk入力方式で入力するためのインプットメソッドプラグイン。

[libcskk](https://github.com/naokiri/cskk) を用いる。

## インストール方法

    $ rm -rf ./build
    $ mkdir build
    $ cd build  
    $ cmake ..
    $ make && make install

## アンインストール方法

    $ cd build
    $ make uninstall

## テスト実行方法

    $ rm -rf ./build 
    $ mkdir build
    $ cd build
    $ cmake -DGOOGLETEST=on ..
    $ make runTest 
    $ ./test/runTest

GOOGLETESTフラグはキャッシュされるのでライブラリ生成時には注意が必要

## 開発状況
### 実装予定(いつかは)
- [x] ひらがな・カタカナ・漢字入力
- [x] 変換候補リスト表示
- [ ] 変換候補リスト ラベル選択
  
- 設定項目
    - [x] 入力モード初期値設定
    - [ ] 漢字変換候補ラベル((a,b,c...), (1,2,3...) etc.)
    - [ ] 句読点スタイル ((,.),(、。),(、.)... )
    - [ ] 変換候補リスト表示までの変換候補数 
    - [ ] 変換候補リストのサイズ
     
### 実装内容・予定不明
- [ ] 優先度、読み書き可不可の辞書リスト設定


### じしょ
いまのところとりあえず、レギュラーファイルの $XDG_DATA_HOME/fcitx5-cskk/dictionary/* をまずよみかきじしょとして、 $XDG_DATA_DIRS/fcitx5-cskk/dictionary/* をよみとりせんようじしょにする。このじゅんばんで、おのおのファイルめいじゅん。 utf-8

## 著作権表示

Copyright (C) 2021 Naoaki Iwakiri

## ライセンス
GNU GPL v3 or later.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not,
see <https://www.gnu.org/licenses/>.

