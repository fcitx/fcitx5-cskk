## fcitx5-cskk
Fcitx5でskk入力方式で入力するためのインプットメソッドプラグイン。

libcskk (https://github.com/naokiri/cskk) を用いる。

## インストール方法
fcitx5をソースからインストールしている場合、/usr/local/lib 以下のディレクトリにインストールする。

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make && make install

fcitx5をバイナリパッケージでインストールしている場合、現在はライブラリをシステムディレクトリの/usr/lib 以下にインストールしなければならない。(非推奨)

    $ mkdir build
    $ cd build
    $ cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    $ make && make install


    $ make uninstall

## 開発状況
Dogfoodable.

ひらがなにゅうりょくできる

漢字も変換できる。

### じしょ
いまのところとりあえず、レギュラーファイルの $XDG_DATA_HOME/fcitx5-cskk/dictionary/* をまずよみかきじしょとして、 $XDG_DATA_DIRS/fcitx5-cskk/dictionary/* をよみとりせんようじしょにする。このじゅんばんで、おのおのファイルめいじゅん。 utf-8
