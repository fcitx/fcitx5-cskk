## fcitx5-cskk
[Fcitx5](https://github.com/fcitx/fcitx5) でskk入力方式で入力するためのインプットメソッドプラグイン。

[libcskk](https://github.com/naokiri/cskk) を用いる。

## インストール方法

    $ rm -rf ./build
    $ mkdir build
    $ cd build  
    $ cmake ..
    $ make && make install

システムによっては、アイコン類の読み込みのために再起動が必要です。

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
- [x] 変換候補リスト ラベル選択
  
- 設定項目
    - [x] 入力モード初期値設定
    - [x] 漢字変換候補ラベル((a,b,c...), (1,2,3...) etc.)
    - [x] 句読点スタイル ((,.),(、。),(、.)... )
    - [x] 変換候補リスト表示までの変換候補数 
    - [x] 変換候補リストのサイズ
     
### 実装内容・予定不明
- [ ] 優先度、読み書き可不可の辞書リスト設定


### じしょ
現在のところ $XDG_DATA_HOME/fcitx5-cskk/dictionary/\*.dict (多くの環境では$HOME/.local/share/fctix5-cskk/dictionary/\*.dict )  を読み書き可能な辞書として優先して読む。 
$XDG_DATA_DIRS/fcitx5-cskk/dictionary/\* (/usr/local/share/fcitx5-cskk/dictionary/\* 等) を読みとり専用辞書とする。
文字コードはUTF-8のみに対応。ファイル名順。

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

