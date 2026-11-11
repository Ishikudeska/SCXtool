# 法的情報

> このページは情報提供を目的として提供される翻訳です。**正文は英語版です**。相違がある場合は、英語のテキスト（および実行ファイルの隣に同梱される `LICENSE` と `NOTICE` ファイル）が優先されます。

## Star Citizen / Cloud Imperium

SCXtool は Star Citizen 向けの**非公式コミュニティツール**です。Cloud Imperium Games（CIG）や Roberts Space Industries（RSI）が開発、承認、後援したものではなく、両社とは一切関係がありません。

**Star Citizen®**、**Roberts Space Industries®**、**Cloud Imperium®** は Cloud Imperium Rights LLC および Cloud Imperium Rights Ltd. の登録商標です。`Data.p4k` の内容を含む Star Citizen のゲームデータはすべて、Cloud Imperium Rights LLC の知的財産です。

SCXtool は CIG や RSI のコンテンツを再配布しません。**あなた自身がライセンスを持つ Star Citizen のインストール**からファイルを読み込み、カスタマイズしたテキストをそのインストールに書き戻します。ゲームのデータが SCXtool を通じてあなたのコンピューターの外に出ることはありません。

## ライセンス

SCXtool は Smart Citizen（Apache License 2.0、© Osiris DevWorks およびコントリビューター）と unp4k（MIT License、© dolkensp およびコントリビューター）から派生したものです。ライセンスの全文と帰属表示は、実行ファイルの隣にある `LICENSE` と `NOTICE` ファイルに同梱されています。

サードパーティライブラリ：

- **Qt 6** — The Qt Company。**GNU LGPL v3** の下で使用し、動的リンクしています。
- **zstd** — Meta Platforms、**BSD** ライセンス。
- **zlib** — Jean-loup Gailly と Mark Adler、**zlib** ライセンス。
- **pugixml** — Arseny Kapoulkine、**MIT** ライセンス。

本ソフトウェアは**「現状のまま」**提供され、いかなる種類の保証も条件もありません。

## プライバシー

SCXtool はローカルのデスクトップアプリケーションです。テレメトリ、分析データ、クラッシュレポートは一切送信せず、アカウントもありません。

オンラインに接続するのは、GitHub で SCXtool の新しいバージョンを確認するとき（起動するたび、および **アップデートを確認** をクリックしたとき）とアップデートを選んだ場合のダウンロード、言語を切り替えたときにその言語のコミュニティ製 `global.ini` をダウンロードするとき、そして URL から INI ファイルをインポートするときだけです。アップデートの確認では、アプリのバージョン以外にあなたや PC に関する情報は送信されません。編集内容、設定、バックアップ、キャッシュはあなたのコンピューターに残ります。

- **設定** — `%APPDATA%\SCXtool\settings.ini`（ポータブル版では実行ファイルの隣の `data\settings.ini`）。
- **編集内容、バックアップ、文字列キャッシュ** — 既定では `Documents\SCXtool\<チャンネル>\`（設定 タブで変更できます）。
- **DataForge キャッシュ** — `%LOCALAPPDATA%\SCXtool\<チャンネル>\cache\dataforge\`。
- **ログとクラッシュレポート** — `Documents\SCXtool\logs\`。

## 自己責任でのご利用

SCXtool が変更するのは、CIG がコミュニティ翻訳として認めているローカライズのテキストと、Game Settings タブであなたが選んだゲームのオプションとキー割り当てです。どのように使うかはあなたの責任です。
