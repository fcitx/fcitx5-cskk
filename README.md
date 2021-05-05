## fcitx5-cskk
Fcitx5でskk入力方式で入力するためのインプットメソッドプラグイン。

libcskk (https://github.com/naokiri/cskk) を用いる。

## インストール方法

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make && make install

## アンインストール方法

    $ make uninstall

## 開発状況
Dogfoodable.

ひらがなにゅうりょくできる

漢字も変換できる。

### じしょ
いまのところとりあえず、レギュラーファイルの $XDG_DATA_HOME/fcitx5-cskk/dictionary/* をまずよみかきじしょとして、 $XDG_DATA_DIRS/fcitx5-cskk/dictionary/* をよみとりせんようじしょにする。このじゅんばんで、おのおのファイルめいじゅん。 utf-8
