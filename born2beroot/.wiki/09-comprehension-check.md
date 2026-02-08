# 09 - 理解度チェック (Comprehension Check)

Born2beRoot の全範囲をカバーする 105 問の質問集。評価の防御練習にも使用できる。基礎知識からシナリオ問題、ネットワークトラブルシューティング、LVM 操作、sshd_config セキュリティ監査、Terraform/Ansible コードリーディング、総合シナリオ問題までを網羅する。

各セクションの構成:
- **セクション A** (Q1-Q12): 仮想化とOS -- 基礎知識
- **セクション B** (Q13-Q22): ストレージと暗号化
- **セクション C** (Q23-Q36): セキュリティ設定
- **セクション D** (Q37-Q46): 監視とスクリプト
- **セクション E** (Q47-Q60): シナリオ問題（実演系）
- **セクション F** (Q61-Q70): ネットワーク問題
- **セクション G** (Q71-Q80): LVM 操作問題
- **セクション H** (Q81-Q90): sshd_config 監査問題
- **セクション I** (Q91-Q97): Terraform/IaC コード読解問題
- **セクション J** (Q98-Q105): 総合シナリオ問題

---

## セクション A: 仮想化とOS (Questions 1-12)

### Q1: 仮想マシンの仕組み

**問い**: 仮想マシン (Virtual Machine) とは何か。Type 1 Hypervisor と Type 2 Hypervisor の違いを、具体的なソフトウェア名を挙げて説明せよ。

**回答**:
仮想マシンは、Hypervisor が物理ハードウェアのリソース（CPU、メモリ、ディスク、ネットワーク）を仮想的に分割・エミュレートして作った独立したコンピューティング環境である。各 VM は独自の OS を実行でき、物理的には1台のマシンでありながら、複数の独立したサーバーとして機能する。

**Type 1 (Bare-metal) Hypervisor** は物理ハードウェア上に直接動作する。ホスト OS を必要としないため、オーバーヘッドが小さく高性能である。

| ソフトウェア | 開発元 | 特徴 |
|-------------|--------|------|
| VMware ESXi | VMware | エンタープライズ市場のデファクト標準。vSphere で管理 |
| Microsoft Hyper-V | Microsoft | Windows Server に統合。Azure の基盤 |
| KVM (Kernel-based Virtual Machine) | Red Hat / Community | Linux カーネルモジュールとして動作。AWS EC2 の基盤 |
| Xen | Linux Foundation | Citrix Hypervisor の基盤。パラ仮想化をサポート |

**Type 2 (Hosted) Hypervisor** は既存の OS 上でアプリケーションとして動作する。ホスト OS を経由するためオーバーヘッドがあるが、手軽に仮想マシンを作成できる。

| ソフトウェア | 開発元 | 特徴 |
|-------------|--------|------|
| VirtualBox | Oracle | オープンソース。クロスプラットフォーム |
| VMware Workstation | VMware | 商用。高度なネットワーク機能 |
| Parallels Desktop | Parallels | macOS 専用。Apple Silicon 対応 |
| QEMU | Community | エミュレーション機能。KVM と組み合わせて使用可能 |

Born2beRoot では VirtualBox（Type 2）を使用する。42 の学校環境でホスト OS 上に手軽に仮想マシンを作成できるためである。実務のクラウド環境では Type 1 Hypervisor（特に KVM）が使用されることを認識しておくことが重要である。

---

### Q2: Debian を選択した理由

**問い**: なぜ Debian を選んだのか。Rocky Linux と比較した場合の利点と欠点を述べよ。

**回答**:
Debian を選択した理由は以下の通り:

1. **安定性**: Debian stable は十分なテストを経てリリースされ、サーバー用途に最適である。リリースサイクルは約2年で、各リリースは約5年間サポートされる
2. **パッケージの豊富さ**: 約59,000以上のパッケージが利用可能で、必要なソフトウェアが揃っている
3. **UFW (Uncomplicated Firewall)**: iptables のフロントエンドとして、ファイアウォールの設定がシンプルである
4. **AppArmor**: デフォルトで有効であり、SELinux より学習コストが低い。パスベースのアクセス制御で直感的
5. **歴史とコミュニティ**: 1993年からの長い歴史を持ち、ドキュメントとコミュニティが豊富
6. **完全なフリーソフトウェア**: Debian Social Contract に基づき、自由なソフトウェアのみで構成可能

**Debian vs Rocky Linux の比較**:

| 項目 | Debian | Rocky Linux |
|------|--------|-------------|
| パッケージマネージャ | apt / dpkg | dnf / rpm |
| ファイアウォール | UFW (iptables) | firewalld (nftables) |
| MAC (Mandatory Access Control) | AppArmor | SELinux |
| リリースモデル | コミュニティ主導 | RHEL バイナリ互換 |
| エンタープライズ互換性 | Ubuntu Server が派生 | RHEL と完全互換 |
| パッケージ数 | 約59,000+ | 約8,000+ (base) |
| init システム | systemd | systemd |

Rocky Linux の利点は RHEL 互換であることで、エンタープライズ環境（金融、医療、政府機関）での実務知識が身につく。ただし、Born2beRoot の学習目的（基礎的なサーバー管理）においては Debian のシンプルさが適している。

---

### Q3: apt と aptitude

**問い**: apt と aptitude の違いを 3 つ以上挙げよ。また、それぞれのユースケースを説明せよ。

**回答**:

| 項目 | apt | aptitude |
|------|-----|----------|
| **インターフェース** | コマンドラインのみ | コマンドライン + ncurses TUI |
| **依存関係解決** | 単一の解決策を提示 | 複数の解決策を提案し選択可能 |
| **推奨パッケージ** | デフォルトで Recommends をインストール | デフォルトではインストールしない |
| **未使用パッケージ** | `apt autoremove` が必要 | 自動的に削除を提案 |
| **デフォルトインストール** | Debian に含まれる | 追加インストールが必要 |
| **ログ** | `/var/log/apt/history.log` | 独自の操作履歴を保持 |
| **検索機能** | `apt search` | `aptitude search` + TUI でのブラウジング |

**aptitude の依存関係解決の優位性の具体例**:
パッケージ A が B と C に依存し、B が D>=2.0 に依存、C が D<=1.9 に依存する場合（依存関係の衝突）:
- `apt` はエラーを出して停止する
- `aptitude` は「B を別バージョンに変更する」「C の代替パッケージを使用する」等の複数の解決策を提案する

**ユースケース**:
- **apt**: 日常的なパッケージ管理（インストール、アップデート、削除）。スクリプトからの自動化
- **aptitude**: 複雑な依存関係の問題解決。パッケージの調査・ブラウジング。サーバーの初期構築時の対話的な作業

---

### Q4: AppArmor

**問い**: AppArmor とは何か。DAC (Discretionary Access Control) との違いを説明し、AppArmor と SELinux の違いも述べよ。

**回答**:
AppArmor は Linux の MAC (Mandatory Access Control) セキュリティモジュールである。LSM (Linux Security Modules) フレームワーク上に実装されている。

**DAC vs MAC の違い**:

DAC は従来の Linux パーミッションシステム（rwx / owner / group / others）で、ファイルの所有者がアクセス権を設定する。問題点は以下の通り:
- プログラムが脆弱性を突かれて乗っ取られた場合、そのプログラムの実行ユーザーの権限で任意の操作が可能になる
- 例: Apache が www-data ユーザーで動作している場合、Apache の脆弱性を突かれると www-data が読み書きできる全てのファイルにアクセスされる

AppArmor は MAC として、各プログラムに「プロファイル」を定義し、そのプログラムがアクセスできるファイルやリソースをシステム管理者が制限する。プログラムが乗っ取られても、プロファイルで許可されていない操作は実行できない。

**AppArmor vs SELinux**:

| 項目 | AppArmor | SELinux |
|------|----------|---------|
| アクセス制御方式 | パスベース（ファイルパスで制御） | ラベルベース（セキュリティコンテキストで制御） |
| 学習コスト | 低い（プロファイルが読みやすい） | 高い（ポリシー言語が複雑） |
| 粒度 | 中程度 | 非常に細かい |
| デフォルト採用 | Debian, Ubuntu, SUSE | RHEL, CentOS, Fedora |
| プロファイル作成 | `aa-genprof` で対話的に作成可能 | `audit2allow` でポリシー生成 |

```bash
# AppArmor の状態確認
sudo aa-status

# プロファイルの例（/etc/apparmor.d/usr.sbin.sshd より抜粋）
/usr/sbin/sshd {
  /etc/ssh/** r,
  /proc/*/fd/ r,
  /run/sshd.pid rw,
  network inet stream,
}
```

---

### Q5: Linux カーネル

**問い**: Kernel space と User space の違いを説明せよ。System Call はどのような役割を果たすか。CPU の保護リングとの関係も説明せよ。

**回答**:
**Kernel space** は OS の中核部分が動作する特権モードの領域である。CPU の Ring 0（最高特権レベル）で動作し、以下にアクセスできる:
- ハードウェアへの直接アクセス（ディスク、ネットワークカード、メモリコントローラ）
- 全メモリ空間へのアクセス
- プロセス管理（スケジューリング、作成、終了）
- 割り込み処理

**User space** はアプリケーションが動作する非特権モードの領域である。CPU の Ring 3（最低特権レベル）で動作する:
- ハードウェアに直接アクセスできない
- 自プロセスのメモリ空間のみアクセス可能
- カーネルの機能を利用するには System Call を経由する必要がある

**System Call** は User space からカーネルの機能にアクセスするためのインターフェースである。

```
User space                      Kernel space
+-----------+                   +------------------+
| アプリ    |  --- syscall ---> | カーネル         |
| (Ring 3)  |  <-- 結果 -----  | (Ring 0)         |
+-----------+                   +------------------+
     |                                |
     |  open(), read(), write()       |  ハードウェア制御
     |  fork(), exec(), exit()        |  メモリ管理
     |  socket(), connect()           |  プロセススケジューリング
```

代表的な System Call:
- `open()` / `close()`: ファイルのオープン/クローズ
- `read()` / `write()`: データの読み書き
- `fork()`: プロセスの作成
- `exec()`: プログラムの実行
- `mmap()`: メモリマッピング
- `socket()` / `connect()`: ネットワーク通信

この仕組みにより、不正なアプリケーションがハードウェアを直接操作することを防ぎ、システムの安定性とセキュリティを確保している。カーネルはゲートキーパーとして、全てのハードウェアアクセスを仲介・検証する。

---

### Q6: パッケージ管理

**問い**: dpkg と apt の関係を説明せよ。`apt install vim` を実行した時に内部で何が起こるか。依存関係の地獄 (dependency hell) とは何か。

**回答**:
dpkg は Debian の低レベルパッケージマネージャで、.deb ファイルの直接的なインストール、削除、情報表示を行う。しかし依存関係の自動解決はしない。

apt は dpkg のフロントエンドで、リポジトリからのパッケージ検索、依存関係の自動解決、ダウンロードを行い、最終的に dpkg を呼び出してインストールする。

```
            apt (高レベル)
             |
             |  リポジトリ検索、依存解決、ダウンロード
             |
            dpkg (低レベル)
             |
             |  .deb ファイルのインストール
             |
        ファイルシステム
```

**`apt install vim` の内部動作**:

1. `/etc/apt/sources.list` および `/etc/apt/sources.list.d/` で定義されたリポジトリからパッケージインデックス（`/var/lib/apt/lists/`）を参照し、vim パッケージ情報を検索
2. vim の依存パッケージツリーを構築する（例: vim-common, vim-runtime, libgpm2, libpython3.9 等）
3. 既にインストール済みのパッケージを除外し、必要なパッケージのみをリストアップ
4. 各 .deb ファイルをリポジトリからダウンロードし `/var/cache/apt/archives/` に保存
5. ダウンロードした .deb ファイルの MD5/SHA256 チェックサムを検証
6. dpkg を呼び出して依存関係の順序で各 .deb ファイルをインストール
7. 各パッケージの `preinst` スクリプト（インストール前処理）を実行
8. ファイルをシステムに展開
9. 各パッケージの `postinst` スクリプト（インストール後処理）を実行
10. パッケージデータベース (`/var/lib/dpkg/status`) を更新

**依存関係の地獄 (Dependency Hell)**:
パッケージ A が B>=2.0 に依存し、パッケージ C（既にインストール済み）が B<=1.9 に依存する場合、A をインストールすると C が壊れる。このような依存関係の衝突が連鎖的に発生し、解決困難になる状態を指す。apt の依存解決エンジンはこの問題を自動的に処理しようとするが、完全には解決できない場合がある。

---

### Q7: ブートプロセス

**問い**: 電源投入から SSH でログインできるようになるまでのブートプロセスを、Born2beRoot の構成に即して詳細に説明せよ。

**回答**:

```
電源投入
  |
  v
[1] BIOS/UEFI (ファームウェア)
  |  - POST (Power-On Self-Test): CPU, RAM, デバイスの動作確認
  |  - ブートデバイスの検索（HDD/SSD の MBR を読み込む）
  |
  v
[2] GRUB Stage 1 (MBR: 最初の 446 バイト)
  |  - MBR に格納された GRUB のコードが実行される
  |  - /boot パーティションの場所を特定
  |
  v
[3] GRUB Stage 2 (/boot パーティション)
  |  - GRUB メニューを表示（カーネル選択画面）
  |  - カーネルイメージ (vmlinuz) と initramfs をメモリにロード
  |  - カーネルパラメータを渡して制御をカーネルに移譲
  |
  v
[4] Linux カーネルの初期化
  |  - initramfs（初期RAMファイルシステム）をマウント
  |  - 必要なカーネルモジュールをロード（dm-crypt, lvm2 等）
  |  - initramfs 内の init スクリプトが実行される
  |
  v
[5] LUKS パスフレーズの要求
  |  - cryptsetup が暗号化パーティションのパスフレーズを要求
  |  - 正しいパスフレーズが入力されると dm-crypt がマスターキーを復号
  |  - 暗号化パーティションが /dev/mapper/sda5_crypt として利用可能になる
  |
  v
[6] LVM の起動
  |  - vgscan: ボリュームグループを検索
  |  - vgchange -ay: ボリュームグループをアクティブ化
  |  - 各論理ボリュームが /dev/mapper/ に表示される
  |
  v
[7] ルートファイルシステムのマウント
  |  - LVM の root ボリュームが / にマウントされる
  |  - initramfs から実際のルートファイルシステムに pivot_root
  |
  v
[8] systemd の起動 (PID 1)
  |  - /etc/fstab に基づいて残りのパーティションをマウント
  |  - 各サービスを依存関係に基づいて並列起動:
  |    - apparmor.service: AppArmor プロファイルのロード
  |    - networking.service: ネットワークインターフェースの設定
  |    - ufw.service: ファイアウォールルールの適用
  |    - ssh.service (sshd): SSH デーモンの起動
  |    - cron.service: cron デーモンの起動
  |
  v
[9] ログインプロンプト / SSH 受付開始
     - getty がコンソールにログインプロンプトを表示
     - sshd がポート 4242 で接続を待機
     - SSH クライアントから接続可能な状態
```

各段階でエラーが発生した場合の影響:
- BIOS/UEFI エラー: ビープ音、画面に何も表示されない
- GRUB エラー: "GRUB rescue>" プロンプト
- LUKS エラー: パスフレーズ入力画面でループ（3回失敗でシェルに落ちる）
- LVM エラー: "Volume group not found" エラー
- systemd エラー: emergency mode に移行

---

### Q8: ファイルシステム階層

**問い**: FHS (Filesystem Hierarchy Standard) に基づいて、Born2beRoot で使用される主要なディレクトリの役割を説明せよ。`/var` と `/tmp` を別パーティションにする理由も述べよ。

**回答**:
FHS は Linux のディレクトリ構造を標準化した仕様である。Born2beRoot で重要なディレクトリ:

```
/                   ルートファイルシステム。全ての起点
├── /boot           カーネルイメージ (vmlinuz)、initramfs、GRUB 設定
│                   Born2beRoot では暗号化されていない独立パーティション
├── /etc            システム設定ファイル
│   ├── /etc/ssh/       SSH デーモンの設定
│   ├── /etc/pam.d/     PAM モジュールの設定
│   ├── /etc/sudoers.d/ sudo の追加設定
│   ├── /etc/login.defs パスワードポリシー
│   └── /etc/fstab      マウント設定
├── /home           ユーザーのホームディレクトリ（独立パーティション）
├── /var            可変データ
│   ├── /var/log/       ログファイル（独立パーティション: /var/log）
│   ├── /var/mail/      メールスプール
│   ├── /var/lib/       アプリケーションの状態データ
│   └── /var/cache/     キャッシュデータ
├── /tmp            一時ファイル（独立パーティション）
│                   再起動時にクリアされる。全ユーザーが書き込み可能
├── /srv            サービスが提供するデータ（独立パーティション）
│                   Web サーバーのドキュメントルート等
├── /usr            ユーザーランドのプログラムとデータ（読み取り専用）
│   ├── /usr/bin/       一般ユーザーのコマンド
│   ├── /usr/sbin/      管理者コマンド
│   └── /usr/local/     ローカルにインストールしたソフトウェア
├── /proc           仮想FS。カーネル情報（CPU、メモリ、プロセス）
├── /sys            仮想FS。デバイス情報
└── /dev            デバイスファイル
    ├── /dev/sda        ディスク
    └── /dev/mapper/    dm-crypt / LVM のマッピング
```

**`/var` と `/tmp` を別パーティションにする理由**:

1. **リソース分離**: `/var/log` のログが肥大化しても、`/`（ルート）パーティションの空き容量に影響しない。ルートパーティションが満杯になるとシステムが正常に動作しなくなる
2. **セキュリティ**: `/tmp` に `noexec` マウントオプションを設定することで、一時ファイルとして配置された悪意あるプログラムの実行を防止できる。また `nosuid` で SUID ビットを無効化できる
3. **サービス拒否 (DoS) 防止**: あるユーザーが `/tmp` に大量のファイルを書き込んでも、別パーティションなのでシステム全体のディスク容量には影響しない
4. **バックアップとリカバリ**: パーティションごとに異なるバックアップ戦略を適用できる（`/var/log` は頻繁にバックアップ、`/tmp` はバックアップ不要）

---

### Q9: コンテナと VM の違い

**問い**: Docker コンテナと VirtualBox の仮想マシンの違いを、分離レベル・リソース消費・起動時間・セキュリティの観点から説明せよ。Born2beRoot をコンテナで実施できない理由も述べよ。

**回答**:

| 観点 | 仮想マシン (VirtualBox) | コンテナ (Docker) |
|------|------------------------|-------------------|
| **分離レベル** | ハードウェアレベル。各 VM が独自のカーネルを持つ | OS レベル。ホストのカーネルを共有 |
| **分離技術** | Hypervisor が完全に分離 | namespace + cgroup で分離 |
| **リソース消費** | 各 VM に OS 全体のメモリ（数百MB〜数GB）が必要 | アプリと必要なライブラリのみ（数MB〜数百MB） |
| **起動時間** | OS のブートプロセス全体。数十秒〜数分 | プロセスの起動のみ。ミリ秒〜数秒 |
| **ディスク使用量** | 数GB〜数十GB | 数MB〜数GB |
| **セキュリティ** | 強い分離。VM 間の影響が極めて小さい | カーネル共有のため、カーネル脆弱性の影響を受ける |
| **ポータビリティ** | OVA/OVF 形式。大きなファイルサイズ | Docker イメージ。レイヤー構造で効率的 |

**Born2beRoot をコンテナで実施できない理由**:

コンテナはホストのカーネルを共有するため、以下の Born2beRoot の学習項目が経験できない:
1. **ブートプロセス**: BIOS → GRUB → カーネルロードの流れがない
2. **LUKS 暗号化**: ディスク暗号化はカーネルレベルの機能であり、コンテナ内では設定できない
3. **LVM**: 論理ボリューム管理はカーネルモジュールであり、コンテナ内での操作は制限される
4. **カーネルモジュール**: AppArmor のプロファイル管理等、カーネルレベルの設定ができない
5. **systemd**: 多くのコンテナイメージでは systemd が使用されない（PID 1 問題）

Born2beRoot の本質は「OS のインストールとカーネルレベルの管理を学ぶこと」であり、コンテナでは実現できない。

---

### Q10: VirtualBox のネットワーク

**問い**: VirtualBox の NAT モードで、ホスト OS からゲスト OS に SSH するために Port Forwarding が必要な理由を説明せよ。他のネットワークモード（Bridged, Host-only, Internal）との違いも述べよ。

**回答**:
**NAT モード**では、ゲスト OS はプライベートネットワーク（10.0.2.0/24）に配置される。VirtualBox が NAT ルーターとして機能する。

```
[ホスト OS]                    [VirtualBox NAT Engine]              [ゲスト OS]
192.168.1.100  <------->  NAT (10.0.2.2 = ゲートウェイ)  <------->  10.0.2.15
                           |
                           |  外向き通信: ゲスト → 外部 は NAT で可能
                           |  内向き通信: 外部 → ゲスト はデフォルトで不可
                           |  Port Forwarding で明示的に許可が必要
```

Port Forwarding 設定例:
- ホスト IP: 127.0.0.1、ホストポート: 4242
- ゲスト IP: 10.0.2.15、ゲストポート: 4242
- これにより `ssh user@127.0.0.1 -p 4242` でゲスト OS に接続可能

**各ネットワークモードの比較**:

| モード | ゲスト→外部 | 外部→ゲスト | ゲスト同士 | ホスト→ゲスト | 用途 |
|--------|-----------|-----------|----------|------------|------|
| NAT | 可能 | Port Forwarding 必要 | 不可 | Port Forwarding 必要 | Born2beRoot（シンプルで安全） |
| Bridged | 可能 | 可能 | 可能 | 可能 | 物理ネットワークに参加 |
| Host-only | 不可 | 不可 | 可能 | 可能 | ホストとの通信のみ |
| Internal | 不可 | 不可 | 可能 | 不可 | VM 間の閉じたネットワーク |
| NAT Network | 可能 | Port Forwarding 必要 | 可能 | Port Forwarding 必要 | 複数 VM で NAT 共有 |

---

### Q11: systemd

**問い**: systemd とは何か。SysVinit との違い、主要なコマンド（systemctl, journalctl）の使い方を説明せよ。

**回答**:
systemd は Linux の init システムおよびサービスマネージャである。PID 1 として起動し、システムの全プロセスとサービスを管理する。

**SysVinit との比較**:

| 項目 | SysVinit | systemd |
|------|----------|---------|
| 起動方式 | シーケンシャル（順次起動） | 並列起動（依存関係に基づく） |
| 設定 | シェルスクリプト (/etc/init.d/) | Unit ファイル (.service, .socket 等) |
| 起動速度 | 遅い | 高速（並列化のため） |
| プロセス追跡 | PID ファイルベース | cgroup ベース（確実な追跡） |
| ログ | syslog (テキストファイル) | journald (バイナリ + 構造化ログ) |
| 依存関係管理 | ランレベル (0-6) の順序 | ターゲットと依存関係グラフ |

**主要な systemctl コマンド**:
```bash
# サービスの状態確認
sudo systemctl status sshd

# サービスの起動・停止・再起動
sudo systemctl start sshd
sudo systemctl stop sshd
sudo systemctl restart sshd

# 設定変更後のリロード（プロセス再起動なし）
sudo systemctl reload sshd

# 自動起動の有効化・無効化
sudo systemctl enable sshd
sudo systemctl disable sshd

# 全サービスの状態一覧
systemctl list-units --type=service

# 起動に失敗したサービスの一覧
systemctl --failed
```

**主要な journalctl コマンド**:
```bash
# 全ログの表示
sudo journalctl

# 特定サービスのログ
sudo journalctl -u sshd

# 直近のログ（末尾 50 行）
sudo journalctl -u sshd -n 50

# リアルタイム追跡（tail -f 相当）
sudo journalctl -u sshd -f

# 時間範囲の指定
sudo journalctl --since "2024-01-01" --until "2024-01-02"

# ブート単位でのログ表示
sudo journalctl -b    # 現在のブート
sudo journalctl -b -1 # 前回のブート

# ディスク使用量の確認
sudo journalctl --disk-usage
```

---

### Q12: Linux のファイルパーミッション

**問い**: Linux のファイルパーミッションの仕組みを説明せよ。SUID, SGID, Sticky Bit とは何か。セキュリティ上の注意点も述べよ。

**回答**:
Linux のファイルパーミッションは、所有者 (Owner)、グループ (Group)、その他 (Others) の3つのカテゴリに対して、読み取り (r=4)、書き込み (w=2)、実行 (x=1) の権限を設定する。

```bash
# パーミッションの表示
ls -la /etc/shadow
-rw-r----- 1 root shadow 1234 Jan 1 00:00 /etc/shadow
│├──┤├──┤├──┤
│ │    │    └── Others: --- (権限なし)
│ │    └── Group (shadow): r-- (読み取りのみ)
│ └── Owner (root): rw- (読み書き)
└── ファイルタイプ (- = 通常ファイル, d = ディレクトリ, l = シンボリックリンク)

# 数値表記
chmod 640 /etc/shadow  # rw-r----- と同じ
# 6 = r(4) + w(2) = 所有者
# 4 = r(4)        = グループ
# 0 = ---          = その他
```

**特殊パーミッション**:

| ビット | ファイルに設定した場合 | ディレクトリに設定した場合 |
|--------|----------------------|------------------------|
| **SUID** (4000) | 実行時にファイル所有者の権限で動作 | (効果なし) |
| **SGID** (2000) | 実行時にファイルグループの権限で動作 | 新規ファイルがディレクトリのグループを継承 |
| **Sticky Bit** (1000) | (効果なし) | 所有者のみがファイルを削除可能 |

**具体例**:
```bash
# SUID の例: passwd コマンド
ls -la /usr/bin/passwd
-rwsr-xr-x 1 root root 63960 /usr/bin/passwd
#   ^-- s = SUID ビット
# 一般ユーザーが passwd を実行すると、root 権限で /etc/shadow を更新する

# Sticky Bit の例: /tmp ディレクトリ
ls -ld /tmp
drwxrwxrwt 10 root root 4096 /tmp
#        ^-- t = Sticky Bit
# 全ユーザーが書き込み可能だが、自分のファイルしか削除できない
```

**セキュリティ上の注意点**:
1. SUID が設定された root 所有のプログラムは、脆弱性を突かれると権限昇格に悪用される可能性がある
2. 定期的に `find / -perm -4000 -type f 2>/dev/null` で SUID ファイルを監査すべき
3. `/tmp` や `/home` パーティションには `nosuid` マウントオプションを設定すべき
4. 不要な SUID ビットは `chmod u-s /path/to/file` で削除する

---
