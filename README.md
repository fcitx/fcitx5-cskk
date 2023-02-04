## ![cskk logo](https://raw.githubusercontent.com/naokiri/fcitx5-cskk/master/data/icon/48x48/apps/cskk.png) fcitx5-cskk 
[Fcitx5](https://github.com/fcitx/fcitx5) でskk入力方式で入力するためのインプットメソッドプラグイン。

[libcskk](https://github.com/naokiri/cskk) を用いる。

説明書は[CSKK Docs](https://naokiri.github.io/cskk-docs/)に公開。

## インストール方法

### Required libraries

Same as other fcitx5 plugin project.

For example in Debian,

    $ sudo apt install gettext cmake cmake-extra-modules fcitx5-modules-dev qtbase5-dev qtdeclarative5-dev libfcitx5-qt-dev

For full features.

### Install

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
- [x] 優先度、読み書き可不可の辞書リスト設定


### 辞書
辞書の形式は [skk-dev](https://skk-dev.github.io/dict/) で配布されているものを想定している。

デフォルトでは/usr/share/skk/SKK-JISYO.L が euc-jp の読み取り専用辞書として使われる。

辞書はfcitx5のconfigtoolから設定可能。


直接編集する場合は `~/.local/share/fcitx5/cskk/dictionary_list` に保存されている。
','区切りのkey=valueリストで、type,file,mode,encodingを指定する。
例として、

    type=file,file=/usr/share/skk/SKK-JISYO.L,mode=readonly,encoding=euc-jp
    type=file,file=$FCITX_CONFIG_DIR/cskk/user.dict,mode=readwrite

typeはfileのみ。必須。

fileはファイルへのパスを指定する。必須。唯一文頭でのみ$FCITX_CONFIG_DIRのみ変数として使え、fcitx5の設定ディレクトリ(通常は~/.local/share/fcitx5)を指す。

modeはreadonlyまたはreadwrite。必須。

encodingに指定できる内容はlibcskkに準じる。必須。少なくとも"euc-jp"や"utf-8"が使える。



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

