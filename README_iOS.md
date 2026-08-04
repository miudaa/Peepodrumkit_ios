# PeepoDrumKit iOS Port

このプロジェクトは、PeepoDrumKit v1.2.0.1240をiOSへ移植したものです。
SDL3とImGuiを使用しており、iOS上での動作が可能です。

## 内容
- `v1.2.0.1240` の最新機能（#SUDDEN対応など）をマージ済み
- SDL3バックエンドによるクロスプラットフォーム対応
- iOS向けの `Info.plist` および `xmake.lua` の設定
- GitHub Actionsによる自動ビルド設定 (`.github/workflows/ios.yml`)

## IPAファイルの作成方法（GitHub Actionsを使用）

Macを持っていない場合でも、GitHub Actionsを使用してIPAファイルを作成できます。

1. このソースコードを自分のGitHubリポジトリにアップロード（プッシュ）します。
2. リポジトリの **Actions** タブを開きます。
3. **Build iOS IPA** ワークフローを選択し、**Run workflow** をクリックします。
4. ビルドが完了すると、`Artifacts` セクションから `PeepoDrumKit-iOS-Unsigned.ipa` がダウンロード可能になります。

## インストール方法

作成されたIPAファイルは署名されていないため、以下のいずれかの方法でインストールしてください。

- **AltStore / Sideloadly**: PCを使用してiPhoneに署名・インストールできます（推奨）。
- **TrollStore**: 脱獄済みまたは対応バージョンのiOSデバイスで使用可能です。
- **TestFlight**: Apple Developer Programに参加している場合、アップロードして配布できます。

## 注意事項
- このビルドは「署名なし」です。そのままではiPhoneにインストールできません。必ずAltStore等のツールを使用して署名してください。
- タッチ操作はマウスエミュレーションとして動作します。
