# Longan Nano で動くVGM プレーヤー (PSG版)

## これはなに?

何かと話題の RISC-V マイコン Longan Nano (GD32V) を遣って VGM ファイルを再生させる試みです。SD カード内のフォルダに保存した vgm 拡張子のファイルを順番に再生します。

## 配線図


<img src="https://user-images.githubusercontent.com/13434151/91420705-01b81f80-e890-11ea-850f-b358487bcf5c.png" width="700">

### 各部の説明

クロックは秋月電子で売っているクロック生成モジュール SI5351 を使っています。VGM ファイル情報からクロック設定を読み込んで切り替えします。大抵の場合は、4MHz か 3.57MHz です。I2C はプルアップします。

操作スイッチは Arduino 用のよくある 5 ボタンキーパッドと同じ構成です。

### Longan Nano で使えるピン
<img width="600" alt="pins" src="https://user-images.githubusercontent.com/13434151/91421188-93c02800-e890-11ea-9517-12cd35cec64a.png">
Longan Nano って初めから SD カードと液晶が搭載されてコンパクトにまとまっています。これらを利用する場合、デフォルトで上図のピンが使われます。また、今回は I2C でクロックを設定するのでそれも必要です。

## Arduino ライブラリが未完成！

Arduiono 互換で開発ができることを謳う [Longanduiono](https://github.com/sipeed/platform-gd32v) ですが、I2C ライブラリや ADC、ファイルアクセスなどが未実装のまま放置されています。中の人が飽きてしまったのか 2019 年の秋頃から更新されていません。ですので、Arduino 気分で気楽に開発は出来ないと思います。

I2C は GD32V のコードを元に Wire 風なものを書いて実装。ファイルアクセスは Bad Apple デモから FatFS で実装。ADC は [こちらのサイト](https://jhalfmoon.com/dbc/2020/07/12/%E9%B3%A5%E3%81%AA%E3%81%8D%E9%87%8C%E3%81%AE%E3%83%9E%E3%82%A4%E3%82%B3%E3%83%B3%E5%B1%8B91-longan-nano%E3%80%81adc-%E5%8D%98%E7%99%BA%E5%A4%89%E6%8F%9B/) を激しく参考にさせていただきました。

## SD カードが安定しない！

開発中とにかく安定しない SD カードに悩まされました。筆者の開発力が低いせいもあって、謎の動作に何日もハマることも数回でした。loop 内に関係のないコードを追加すると、なぜか setup で SD カードをマウントできない、見つからないという現象がおきます。いろいろやった結果の注意点というかコツを書いておきます。

- 処理がいろいろあると SD カードがマウント準備できていない。これは、SD カードマウント後に書かれたコードも当てはまります。謎です。避けるコツは、セットアップ処理ごとにある程度のディレイを入れる、SD カードのマウントを初期化処理の後半に持ってくる、配列やメモリの確保が間違っていないかを確認、などがあります。

- ファイルをオープンしたあとファイルの後半にシークすると、ゴミデータしか返さなくなる。これもはっきりした理由はわかりませんが、処理の順序をずらしたり、一旦その処理だけ別プロジェクトで単体動作させるなどして調整しました。

![IMG_6408](https://user-images.githubusercontent.com/13434151/91424323-9e7cbc00-e894-11ea-89fa-3a403285425f.JPG)
