# Longan Nano で動くVGM プレーヤー (PSG版)


## これはなに?

何かと話題の RISC-V マイコン Longan Nano (GD32V) を使って VGM ファイルを再生させる試みです。SD カード内のフォルダに保存した vgm 拡張子のファイルを順番に再生します。

## What is this?

This is an application with Longan Nano to play vgm files with a YM2149 PSG chip.

## 配線図 (Schematics and wiring diagram)

<img src="https://user-images.githubusercontent.com/13434151/91420705-01b81f80-e890-11ea-850f-b358487bcf5c.png" width="700">


### 説明

クロックは秋月電子で売っているクロック生成モジュール SI5351 を使っています。VGM ファイル情報からクロック設定を読み込んで切り替えします。大抵の場合は、4MHz か 3.57MHz です。I2C はプルアップします。

操作スイッチは Arduino 用のよくある 5 ボタンキーパッドと同じ構成です。フォルダの前後、ファイルの前後が4つのボタンに割り当てられています。

YM2149 の SEL ピンは内部のクロック 2 分周を切り替えるので GND に落とすと、4MHz が 2MHz で動作します。

VGM ファイルは ``.vgm`` 拡張子をつけて、ルート直下のフォルダに入れてください。ルートや孫フォルダに配置しても認識しません。他に余計なファイルも入れないほうがよいです。VGZ ファイルは圧縮されているので解凍してください。

### Description

The clock signal is generated by SI5351 VFO, which is sold at Akiduki Denshi. If it's not handy for you, you can replace it with a normal Xtal, 3.57MHz for MSX and 4MHz for X1 turbo. I2C signals have to be pulled up.

Control buttons are derived from a general key pad for Arduino, 5 buttons through the ADC input.

VGM files have to be stored in a directory in the root. Any files in the root or a descendat directory will be ignored. A file with a .VGZ extension is zipped, it must be unzipped.


### Longan Nano で使えるピン
<img width="600" alt="pins" src="https://user-images.githubusercontent.com/13434151/91421188-93c02800-e890-11ea-9517-12cd35cec64a.png">
Longan Nano って初めから SD カードと液晶が搭載されてコンパクトにまとまっています。これらを利用する場合、デフォルトで上図のピンが使われます。また、今回は I2C でクロックを設定するのでそれも必要です。

**液晶で使っているピンの内訳**
PA5 - SCK0 = SPI0 クロック
PA6 - MISO0 = SPI0 データ入力（未使用）
PA7 - MOSI0 = SPI0 データ出力
PB0 - TFT_RS = LCD のリセットコマンド（GPIO）
PB1 - TFT_DC = LCD の Write コマンド（GPIO）
PB2 - CS0 = SPI0 の CS（GPIO）

液晶用の MISO0 である PA6 ピンは液晶初期化時に確保はされますが、実際には使われていない模様。そのため、``Lcd_Init();`` 後に PA6 を出力に設定すれば普通に使えます。うちではいまのところ問題はおきていません。PB2 は液晶コネクタにだけつながっている模様。

**SD カードで使っているピンの内訳**
PB15 - SPI1 MOSI
PB14 - SPI1 MISO
PB13 - SPI1 SCLK
PB12 - SPI1 CS_TF


Longan Nano is equipped with a micro SD card reader/writer and a color LCD, that use some pins for I2S communications. In this example, I use I2C to setup the clock generator, two additonal pins are necessary.


## 【困った問題1】Arduino ライブラリが未完成！

Arduiono 互換で開発ができることを謳う [Longanduiono](https://github.com/sipeed/platform-gd32v) ですが、I2C ライブラリや ADC、ファイルアクセスなどが未実装のまま放置されています。中の人が飽きてしまったのか 2019 年の秋頃から更新されていません。ですので、Arduino 気分で気楽に開発は出来ないと思います。

I2C は GD32V のコードを元に Wire 風なものを書いて実装。ファイルアクセスは Bad Apple デモから FatFS で実装。ADC は [こちらのサイト](https://jhalfmoon.com/dbc/2020/07/12/%E9%B3%A5%E3%81%AA%E3%81%8D%E9%87%8C%E3%81%AE%E3%83%9E%E3%82%A4%E3%82%B3%E3%83%B3%E5%B1%8B91-longan-nano%E3%80%81adc-%E5%8D%98%E7%99%BA%E5%A4%89%E6%8F%9B/) を激しく参考にさせていただきました。



## 【困った問題2】SD カードが安定しない！

開発中とにかく安定しない SD カードに悩まされました。筆者の開発力が低いせいもあって、謎の動作に何日もハマることも数回でした。loop 内に関係のないコードを追加すると、なぜか setup で SD カードをマウントできない、見つからないという現象がおきます。いろいろやった結果の注意点というかコツを書いておきます。

### コードが増えると SD カードの準備が遅れる？

これは、SD カードマウント後に書かれたコードも当てはまります。謎です。避けるコツは、セットアップ処理ごとにある程度のディレイを入れる、SD カードのマウントを初期化処理の後半に持ってくる、配列やメモリの確保が間違っていないかを確認、などがあります。

### ファイルの後半にシークするとゴミデータを返す

VGM 内の曲情報 GD3 データはファイルの最後にあるのでこれを取得しようとするときに起きる不具合。はっきりした理由はわかりませんが、処理の順序をずらしたり、一旦その処理だけ別プロジェクトで単体動作させるなどして調整しました。

## Longan Nano のここが Good
安い。液晶ついてる。マイクロ SD リーダーも付いてる。速い。Arduino よりメモリ多い。ノイズ少ないっぽい。

## Longan Nano のここが Bad
ライブラリが未完成。謎のバグでハマる。情報少ない。書き込みの度に Boot と Reset ボタンを押さないといけない。使えるピンが少ないかも。JTAG ないとデバッグほぼ無理。
 

## 実装例
<img width="600" alt="pins" src="https://user-images.githubusercontent.com/13434151/91424323-9e7cbc00-e894-11ea-89fa-3a403285425f.JPG">
