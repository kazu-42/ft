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

## セクション B: ストレージと暗号化 (Questions 13-22)

### Q13: LVM の基本概念

**問い**: LVM (Logical Volume Manager) の 3 層構造（PV / VG / LV）を図示し、従来のパーティション方式と比較した利点を 5 つ以上挙げよ。

**回答**:
LVM はディスクストレージを柔軟に管理するための仕組みである。物理ディスクのパーティションを抽象化し、動的なサイズ変更やスナップショットを可能にする。

```
物理ディスク            LVM レイヤー              マウントポイント
+------------+
| /dev/sda1  |---> PV ─┐
+------------+          │
                        ├──> VG (LVMGroup) ──┬──> LV (root)   ──> /
+------------+          │                    ├──> LV (swap)   ──> [swap]
| /dev/sda2  |---> PV ─┘                    ├──> LV (home)   ──> /home
+------------+                               ├──> LV (var)    ──> /var
                                             ├──> LV (var-log)──> /var/log
+------------+                               ├──> LV (srv)    ──> /srv
| /dev/sdb1  |---> PV ─── VG に追加可能 ──>  └──> LV (tmp)    ──> /tmp
+------------+
```

**3 層構造の詳細**:

| 層 | 名称 | 説明 | コマンド例 |
|----|------|------|-----------|
| **PV** (Physical Volume) | 物理ボリューム | 物理パーティションを LVM 用に初期化したもの | `pvcreate /dev/sda2` |
| **VG** (Volume Group) | ボリュームグループ | 1つ以上の PV をまとめたストレージプール | `vgcreate LVMGroup /dev/sda2` |
| **LV** (Logical Volume) | 論理ボリューム | VG から切り出した仮想パーティション | `lvcreate -L 10G -n root LVMGroup` |

**従来のパーティション方式との比較**:

| 項目 | 従来方式 (fdisk) | LVM |
|------|-----------------|-----|
| サイズ変更 | 不可（再フォーマット必要） | オンラインで拡張可能 |
| 複数ディスク統合 | 不可 | VG に複数 PV を追加可能 |
| スナップショット | 不可 | `lvcreate --snapshot` で作成可能 |
| パーティション数上限 | MBR: 4 primary / GPT: 128 | 実質無制限 |
| ストレージの移行 | 困難 | `pvmove` でデータを別 PV に移動可能 |
| シンプロビジョニング | 不可 | 実使用量に応じた割り当て可能 |

**LVM の利点**:
1. **動的サイズ変更**: サービスを停止せずに LV のサイズを拡張できる（`lvextend` + `resize2fs`）
2. **ストレージプール**: 複数の物理ディスクを1つの VG にまとめ、柔軟に LV を配置できる
3. **スナップショット**: ある時点のデータの一貫性あるコピーを瞬時に作成でき、バックアップやテストに利用できる
4. **シンプロビジョニング**: 実際に使用した分だけディスク容量を消費するため、効率的なストレージ利用が可能
5. **オンラインマイグレーション**: `pvmove` でサービス稼働中にデータを別のディスクに移行できる
6. **ストライピング**: 複数の PV にデータを分散して書き込み、I/O パフォーマンスを向上できる

---

### Q14: Born2beRoot の LVM レイアウト

**問い**: Born2beRoot で作成される LVM のパーティションレイアウトを全て列挙し、各論理ボリュームの推奨サイズとファイルシステムの種類を述べよ。

**回答**:
Born2beRoot のボーナスパートのパーティション構成:

```
/dev/sda
├── sda1: /boot (500MB, ext2, 暗号化なし)
└── sda2: 暗号化パーティション (残り全て)
    └── sda5_crypt (LUKS)
        └── LVMGroup (VG)
            ├── root      (10G,   ext4)  → /
            ├── swap      (2.3G,  swap)  → [swap]
            ├── home      (5G,    ext4)  → /home
            ├── var       (3G,    ext4)  → /var
            ├── srv       (3G,    ext4)  → /srv
            ├── tmp       (3G,    ext4)  → /tmp
            └── var--log  (4G,    ext4)  → /var/log
```

**各パーティションの役割と設計理由**:

| パーティション | サイズ | FS | マウントポイント | 設計理由 |
|---------------|--------|-----|-----------------|---------|
| sda1 | 500MB | ext2 | /boot | GRUB がカーネルを読み込むため暗号化不可。ext2 はジャーナルなしで軽量 |
| root | 10GB | ext4 | / | OS の基本システム。パッケージインストール領域 |
| swap | 2.3GB | swap | [swap] | RAM の 1-2 倍。メモリ不足時のスワップ領域 |
| home | 5GB | ext4 | /home | ユーザーデータの分離。クォータ管理が容易 |
| var | 3GB | ext4 | /var | 可変データ（キャッシュ、スプール）の分離 |
| srv | 3GB | ext4 | /srv | サービスデータの分離（Web サーバー等） |
| tmp | 3GB | ext4 | /tmp | 一時ファイル。`noexec,nosuid` でマウント可能 |
| var--log | 4GB | ext4 | /var/log | ログの分離。肥大化しても他に影響しない |

確認コマンド:
```bash
# LVM の全体構造を確認
sudo lsblk

# 各 LV の詳細情報
sudo lvs -o +devices

# VG の空き容量確認
sudo vgs

# PV の情報確認
sudo pvs

# 各パーティションのマウント状態と使用量
df -h

# fstab の確認
cat /etc/fstab
```

---

### Q15: LUKS 暗号化の仕組み

**問い**: LUKS (Linux Unified Key Setup) の暗号化の仕組みを、鍵管理の観点から説明せよ。パスフレーズとマスターキーの関係、Key Slot の仕組みも述べよ。

**回答**:
LUKS は Linux のディスク暗号化標準であり、dm-crypt カーネルモジュールの上に鍵管理レイヤーを提供する。

**暗号化の階層構造**:

```
パスフレーズ（ユーザーが入力）
    |
    | PBKDF2 / Argon2 (鍵導出関数)
    v
Key Slot の暗号化キー
    |
    | AES-256-XTS で復号
    v
マスターキー（Master Key）
    |
    | AES-256-XTS でディスクデータを暗号化/復号
    v
ディスク上のデータ
```

**鍵管理の仕組み**:

1. **マスターキー**: ディスクデータを直接暗号化する鍵。ランダムに生成され、LUKS ヘッダーに暗号化して保存される
2. **Key Slot**: LUKS は最大 8 つの Key Slot を持つ。各 Key Slot にはパスフレーズから派生した鍵で暗号化されたマスターキーのコピーが格納される
3. **PBKDF2 / Argon2**: パスフレーズをそのまま暗号化キーとして使うと辞書攻撃に弱いため、鍵導出関数（Key Derivation Function）で計算コストを上げる

```
LUKS ヘッダー構造:
+------------------+
| Magic Number     |  "LUKS\xba\xbe" (LUKS 署名)
| Version          |  LUKS1 or LUKS2
| Cipher Name      |  aes
| Cipher Mode      |  xts-plain64
| Hash Spec        |  sha256
| Master Key Len   |  512 bits (AES-256-XTS)
| Salt             |  32 bytes (ランダム)
+------------------+
| Key Slot 0       |  パスフレーズ 1 で暗号化されたマスターキー
| Key Slot 1       |  パスフレーズ 2 で暗号化されたマスターキー（任意）
| Key Slot 2       |  (空)
| ...              |
| Key Slot 7       |  (空)
+------------------+
| 暗号化データ領域  |  マスターキーで暗号化された実データ
+------------------+
```

**Key Slot の利点**:
- 複数のパスフレーズを設定できる（管理者用、バックアップ用等）
- パスフレーズ変更時にディスク全体の再暗号化が不要（マスターキーは同じ）
- 特定の Key Slot だけを無効化できる（退職した管理者のパスフレーズ削除等）

```bash
# LUKS ヘッダー情報の表示
sudo cryptsetup luksDump /dev/sda5

# Key Slot の追加（新しいパスフレーズを追加）
sudo cryptsetup luksAddKey /dev/sda5

# Key Slot の削除
sudo cryptsetup luksRemoveKey /dev/sda5

# Key Slot の状態確認
sudo cryptsetup luksDump /dev/sda5 | grep "Key Slot"

# ヘッダーのバックアップ
sudo cryptsetup luksHeaderBackup /dev/sda5 --header-backup-file /root/luks-header.bak
```

**セキュリティ上の注意点**:
1. LUKS ヘッダーが破損するとデータ全損。ヘッダーのバックアップは必須
2. パスフレーズの強度が全体のセキュリティを決定する。十分な長さと複雑さが必要
3. マスターキーがメモリ上に展開されるため、Cold Boot Attack に注意
4. LUKS1 は PBKDF2、LUKS2 は Argon2id を使用。LUKS2 が推奨

---

### Q16: dm-crypt と暗号化レイヤー

**問い**: dm-crypt の動作原理を説明せよ。Device Mapper フレームワークにおける dm-crypt の位置づけ、暗号化モード (XTS) の仕組みも述べよ。

**回答**:
dm-crypt は Linux カーネルの Device Mapper フレームワーク上で動作するブロックデバイスレベルの暗号化モジュールである。

**Device Mapper フレームワーク**:
Device Mapper は仮想ブロックデバイスを作成するカーネルフレームワークで、物理デバイスと論理デバイスの間にマッピングレイヤーを提供する。

```
アプリケーション
    |
ファイルシステム (ext4)
    |
仮想ブロックデバイス (/dev/mapper/sda5_crypt)
    |
+-----------------------------------+
| Device Mapper フレームワーク       |
|   ├── dm-crypt (暗号化)           |  ← ここが dm-crypt
|   ├── dm-linear (LVM の基本)      |
|   ├── dm-snapshot (スナップショット)|
|   ├── dm-mirror (RAID 1)          |
|   └── dm-stripe (RAID 0)          |
+-----------------------------------+
    |
物理ブロックデバイス (/dev/sda5)
```

**Born2beRoot での Device Mapper の使用**:
```
/dev/sda5 (物理パーティション)
    |
    | dm-crypt (LUKS 暗号化)
    v
/dev/mapper/sda5_crypt (復号された仮想デバイス)
    |
    | dm-linear (LVM)
    v
/dev/mapper/LVMGroup-root  → /
/dev/mapper/LVMGroup-home  → /home
/dev/mapper/LVMGroup-var   → /var
...
```

**AES-XTS モードの仕組み**:

XTS (XEX-based Tweaked-codebook mode with ciphertext Stealing) はディスク暗号化に特化した暗号化モードである:

1. **Tweak 値**: ディスクのセクタ番号をベースにした Tweak 値を使用し、同じ平文でもセクタが異なれば異なる暗号文を生成する
2. **2つの鍵**: AES-256-XTS は 512 ビットの鍵を使用し、256 ビットずつ暗号化鍵と Tweak 鍵に分割する
3. **並列処理**: 各セクタを独立して暗号化/復号できるため、ランダムアクセスに適している

```
セクタ N の暗号化:
  平文ブロック ──XOR──> AES暗号化 ──XOR──> 暗号文ブロック
                 ^                    ^
                 |                    |
              Tweak(N) を AES暗号化した値
```

CBC モードとの比較:
- CBC: セクタ内の1ビット変更で後続ブロック全てが変化。Watermarking 攻撃に弱い
- XTS: セクタ内の1ブロックの変更はそのブロックのみに影響。耐タンパー性が高い

---

### Q17: 暗号化パーティションの起動プロセス

**問い**: LUKS 暗号化されたパーティションを使用する場合、ブートプロセスにどのような追加手順が必要か。initramfs の役割と crypttab の設定を説明せよ。

**回答**:

LUKS 暗号化環境では、カーネルがルートファイルシステムをマウントする前に暗号化を解除する必要がある。この「鶏と卵」問題を解決するのが initramfs である。

**暗号化環境のブートプロセス詳細**:

```
[GRUB]
  |
  | vmlinuz + initramfs をメモリにロード
  v
[カーネル初期化]
  |
  | initramfs を一時的なルートファイルシステムとしてマウント
  v
[initramfs 内の処理]
  |
  ├── 1. 必要なカーネルモジュールのロード
  │      - dm-crypt.ko (暗号化モジュール)
  │      - dm-mod.ko (Device Mapper)
  │      - aes.ko, xts.ko (暗号アルゴリズム)
  │      - ext4.ko (ファイルシステム)
  |
  ├── 2. cryptsetup を使用して LUKS パスフレーズを要求
  │      cryptsetup luksOpen /dev/sda5 sda5_crypt
  │      → パスフレーズ入力
  │      → /dev/mapper/sda5_crypt が利用可能に
  |
  ├── 3. LVM の起動
  │      vgscan && vgchange -ay
  │      → /dev/mapper/LVMGroup-* が利用可能に
  |
  ├── 4. ルートファイルシステムのマウント
  │      mount /dev/mapper/LVMGroup-root /sysroot
  |
  └── 5. pivot_root で実際のルートに切り替え
         pivot_root /sysroot /sysroot/initrd
  |
  v
[systemd (PID 1) 起動]
  |
  | /etc/crypttab に基づいて追加の暗号化デバイスを処理
  | /etc/fstab に基づいて残りのパーティションをマウント
```

**crypttab の設定** (`/etc/crypttab`):
```
# <target name>  <source device>         <key file>  <options>
sda5_crypt        UUID=xxxx-xxxx-xxxx    none        luks,discard
```

- `target name`: マッピング名。`/dev/mapper/sda5_crypt` として表示される
- `source device`: 暗号化された物理パーティション。UUID で指定が推奨
- `key file`: `none` はパスフレーズ入力を要求。ファイルパスを指定すると自動復号
- `options`: `luks` は LUKS 形式、`discard` は TRIM コマンドを通過させる（SSD 向け）

**initramfs の更新**:
```bash
# initramfs を再生成（暗号化設定変更後に必要）
sudo update-initramfs -u

# initramfs の内容を確認
lsinitramfs /boot/initrd.img-$(uname -r) | grep crypt
```

---

### Q18: ディスク暗号化の脅威モデル

**問い**: LUKS ディスク暗号化は「何から」データを守るのか。保護できる脅威と保護できない脅威を分類せよ。

**回答**:

ディスク暗号化は**保存データ (Data at Rest)** を保護する技術である。

**保護できる脅威** ✅:

| 脅威 | 説明 | 具体的シナリオ |
|------|------|--------------|
| 物理的盗難 | サーバーやディスクが盗まれた場合 | データセンターからのディスク窃盗 |
| 不正な物理アクセス | 許可なくディスクを取り外して別マシンでマウント | 退社した従業員が旧サーバーのディスクを持ち出し |
| 廃棄ディスクからの漏洩 | 廃棄・リサイクルされたディスクからのデータ復元 | 中古 HDD からの個人情報復元 |
| コールドブートからの部分的保護 | RAM のデータ残留を悪用した攻撃 | LUKS2 + Argon2id で鍵導出の耐性向上 |
| 法規制への準拠 | GDPR, HIPAA 等が要求する暗号化 | 個人データの保存時暗号化義務 |

**保護できない脅威** ❌:

| 脅威 | 理由 | 対策 |
|------|------|------|
| OS 起動後のリモート攻撃 | 起動後はデータが復号されている | ファイアウォール、IDS/IPS |
| マルウェア | OS 上で動作するマルウェアは復号済みデータにアクセス可能 | AppArmor, ウイルス対策 |
| 権限昇格攻撃 | root 権限を奪取されればデータにアクセス可能 | sudo 制限、最小権限の原則 |
| ネットワーク上のデータ | 通信中のデータは暗号化されない | TLS/SSH で通信を暗号化 |
| メモリ上の鍵 | 起動中はマスターキーがメモリに展開される | Cold Boot Attack 対策 |
| Evil Maid Attack | ブートローダーを改ざんしてパスフレーズを窃取 | Secure Boot, TPM |

**多層防御 (Defense in Depth) の考え方**:
```
+------------------------------------------+
| ネットワーク層: ファイアウォール (UFW)      |
|  +--------------------------------------+|
|  | ホスト層: AppArmor, sudo 制限         ||
|  |  +----------------------------------+||
|  |  | アプリ層: PAM, パスワードポリシー  |||
|  |  |  +------------------------------+|||
|  |  |  | データ層: LUKS 暗号化          ||||  ← ここが LUKS の守備範囲
|  |  |  +------------------------------+|||
|  |  +----------------------------------+||
|  +--------------------------------------+|
+------------------------------------------+
```

LUKS は「データ層」の1つの防御手段に過ぎず、他のレイヤーと組み合わせて初めて効果的なセキュリティが実現される。

---

### Q19: LVM のサイズ変更操作

**問い**: `/home` の LV を 5GB から 8GB に拡張する手順を述べよ。逆に縮小する場合の手順と注意点も説明せよ。

**回答**:

**LV の拡張（5GB → 8GB）**:
```bash
# 1. VG の空き容量を確認
sudo vgs
# VFree が 3GB 以上あることを確認

# 2. LV を拡張
sudo lvextend -L 8G /dev/LVMGroup/home
# または増分で指定: sudo lvextend -L +3G /dev/LVMGroup/home

# 3. ファイルシステムを拡張（ext4 はオンラインで拡張可能）
sudo resize2fs /dev/LVMGroup/home

# 4. 結果を確認
df -h /home
sudo lvs /dev/LVMGroup/home
```

`lvextend` と `resize2fs` を一度に行うショートカット:
```bash
sudo lvextend -L 8G --resizefs /dev/LVMGroup/home
```

**LV の縮小（8GB → 5GB）** ⚠️ 危険な操作:
```bash
# 1. 現在の使用量を確認（縮小後のサイズより小さいこと）
df -h /home

# 2. ファイルシステムをアンマウント（オンライン縮小は不可）
sudo umount /home

# 3. ファイルシステムの整合性チェック（必須）
sudo e2fsck -f /dev/LVMGroup/home

# 4. ファイルシステムを先に縮小（必ず LV より先に行う）
sudo resize2fs /dev/LVMGroup/home 5G

# 5. LV を縮小
sudo lvreduce -L 5G /dev/LVMGroup/home

# 6. 再マウント
sudo mount /home
```

**縮小時の注意点**:

| 注意点 | 理由 |
|--------|------|
| **必ず FS → LV の順で縮小** | LV を先に縮小するとデータが切り捨てられて破壊される |
| **オンライン縮小は不可** | ext4 はオンライン拡張は可能だが、オンライン縮小は不可 |
| **バックアップ必須** | 操作ミスでデータ全損の可能性がある |
| **e2fsck が必須** | resize2fs は e2fsck が完了していないと実行を拒否する |
| **使用量の確認** | 現在のデータ量が縮小後のサイズに収まることを確認 |

---

### Q20: swap パーティション

**問い**: swap の役割を説明せよ。swap パーティションと swap ファイルの違い、swappiness パラメータの意味も述べよ。

**回答**:

swap は物理メモリ (RAM) が不足した際に、使用頻度の低いメモリページをディスクに退避する仕組みである。

**swap の動作原理**:
```
RAM (物理メモリ)                    Swap (ディスク上)
+------------------+                +------------------+
| プロセス A (活発) |                | プロセス C のページ|  ← 長時間未使用
| プロセス B (活発) |  --- swap out-->| プロセス D のページ|     のデータを退避
| 空き領域         |  <-- swap in -- |                  |  ← 必要になったら
+------------------+                +------------------+     戻す (page fault)
```

**swap パーティション vs swap ファイル**:

| 項目 | swap パーティション | swap ファイル |
|------|-------------------|-------------|
| 設定場所 | 専用パーティション | 通常のファイルシステム上のファイル |
| パフォーマンス | やや高い（連続領域保証） | やや低い（ファイルシステムのオーバーヘッド） |
| サイズ変更 | パーティション変更が必要 | ファイルサイズ変更で容易 |
| 暗号化 | LUKS で暗号化可能（Born2beRoot） | ファイルシステムの暗号化に依存 |
| 推奨用途 | サーバー、Born2beRoot | デスクトップ、クラウド VM |

**swappiness パラメータ**:
```bash
# 現在の swappiness 値を確認（0-200、デフォルト 60）
cat /proc/sys/vm/swappiness

# 一時的に変更
sudo sysctl vm.swappiness=10

# 永続的に変更
echo "vm.swappiness=10" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

| swappiness 値 | 動作 | 推奨用途 |
|---------------|------|---------|
| 0 | メモリが完全に枯渇するまで swap を使わない | 大容量 RAM のデータベースサーバー |
| 10 | swap をほとんど使わない | 一般的なサーバー |
| 60 | デフォルト。バランスの取れた設定 | デスクトップ |
| 100 | 積極的に swap を使用 | メモリが少ないシステム |

**Born2beRoot での swap サイズの目安**:
- RAM 2GB 以下: RAM の 2 倍
- RAM 2-8GB: RAM と同量
- RAM 8GB 以上: 最低 4GB（ハイバネーション不要なら）

---

### Q21: ファイルシステムの種類

**問い**: ext4, xfs, btrfs の特徴を比較せよ。Born2beRoot で ext4 を選択する理由と、各ファイルシステムの得意分野を述べよ。

**回答**:

| 項目 | ext4 | XFS | Btrfs |
|------|------|-----|-------|
| **開発元** | Linux コミュニティ | SGI → Red Hat | Oracle → SUSE/Facebook |
| **最大ファイルサイズ** | 16TB | 8EB | 16EB |
| **最大ボリュームサイズ** | 1EB | 8EB | 16EB |
| **ジャーナリング** | メタデータ + データ | メタデータのみ | CoW (Copy-on-Write) |
| **オンライン拡張** | 可能 | 可能 | 可能 |
| **オンライン縮小** | 不可 | 不可 | 可能 |
| **スナップショット** | 不可（LVM で対応） | 不可（LVM で対応） | ネイティブサポート |
| **チェックサム** | メタデータのみ | メタデータのみ | データ + メタデータ |
| **RAID サポート** | 外部 (mdadm) | 外部 (mdadm) | ネイティブ (RAID 0/1/10) |
| **小ファイル性能** | 良好 | 普通 | 良好 |
| **大ファイル性能** | 良好 | 非常に良好 | 良好 |
| **安定性** | 非常に高い | 非常に高い | 改善中（RAID 5/6 は注意） |

**Born2beRoot で ext4 を選択する理由**:
1. **安定性**: 20 年以上の実績があり、最も信頼性が高い
2. **互換性**: ほぼ全ての Linux ディストリビューションでデフォルトサポート
3. **回復ツール**: e2fsck, debugfs 等の成熟したツールが揃っている
4. **学習目的**: 最も広く使われているため、学習コストに対する効果が高い
5. **LVM との相性**: LVM 上の ext4 は十分にテストされた組み合わせ

**各ファイルシステムの得意分野**:
- **ext4**: 汎用サーバー、データベース、Web サーバー。「迷ったら ext4」
- **XFS**: 大容量ファイルを扱うメディアサーバー、バックアップストレージ。RHEL のデフォルト
- **Btrfs**: スナップショットが必要な環境、NAS (Synology)。openSUSE のデフォルト

---

### Q22: TRIM と SSD の暗号化

**問い**: SSD で LUKS 暗号化を使用する場合の TRIM (discard) の問題を説明せよ。セキュリティとパフォーマンスのトレードオフについて述べよ。

**回答**:

**TRIM とは**:
SSD は上書きができず、書き込み前にブロックの消去 (Erase) が必要である。TRIM コマンドは OS が「このブロックはもう使っていない」と SSD に通知する仕組みで、SSD のガベージコレクション効率とパフォーマンスを維持する。

```
TRIM なしの SSD:
  OS が削除 → SSD は「使用中」と認識 → 書き込み時に Read-Erase-Write が必要 → 低速化

TRIM ありの SSD:
  OS が削除 → TRIM 通知 → SSD が事前にブロックを消去 → 書き込み時に即座に Write → 高速
```

**LUKS + TRIM のセキュリティ問題**:

TRIM を暗号化パーティションで有効にすると、「どのブロックが未使用か」という情報がディスクの物理層に漏洩する。攻撃者は暗号文を復号できなくても、データの配置パターン（使用領域 vs 未使用領域）を推測できる。

```
TRIM 無効（セキュア）:
  暗号化パーティション全体がランダムデータに見える
  → 使用領域と未使用領域の区別が不可能

TRIM 有効（情報漏洩）:
  未使用ブロックがゼロクリアされる
  → 使用領域のパターンが判別可能
  → ファイルシステムの構造が推測可能
```

**Born2beRoot での設定**:
```bash
# /etc/crypttab で discard オプションを確認
cat /etc/crypttab
# sda5_crypt UUID=xxx none luks,discard

# fstab でも discard を確認
cat /etc/fstab
# /dev/mapper/LVMGroup-root / ext4 discard,errors=remount-ro 0 1
```

**セキュリティ vs パフォーマンスの判断基準**:

| 環境 | TRIM 推奨 | 理由 |
|------|----------|------|
| 個人学習環境 (Born2beRoot) | 有効 | 物理的な攻撃リスクが低い |
| 一般的な企業サーバー | 有効 | パフォーマンス低下の実害が大きい |
| 機密データを扱うサーバー | 無効 | 情報漏洩リスクを最小化 |
| 軍事・政府機関 | 無効 | 最大限のセキュリティが必要 |

---

## セクション C: セキュリティ設定 (Questions 23-36)

### Q23: パスワードポリシーの設定

**問い**: Born2beRoot で設定するパスワードポリシーの全項目と、その設定ファイル・設定値を列挙せよ。各ポリシーのセキュリティ上の根拠も説明せよ。

**回答**:

Born2beRoot のパスワードポリシーは2つのファイルで設定する:

**1. `/etc/login.defs`（パスワード有効期限）**:

```
PASS_MAX_DAYS   30    # パスワードの最大有効日数
PASS_MIN_DAYS   2     # パスワード変更後、再変更までの最小日数
PASS_WARN_AGE   7     # パスワード期限切れの何日前から警告するか
```

| 設定 | 値 | セキュリティ上の根拠 |
|------|-----|---------------------|
| PASS_MAX_DAYS | 30 | 漏洩したパスワードの有効期間を限定。30日で強制変更 |
| PASS_MIN_DAYS | 2 | パスワード履歴の回避を防止。すぐに元のパスワードに戻すことを禁止 |
| PASS_WARN_AGE | 7 | ユーザーに十分な準備期間を与え、突然のロックアウトを防止 |

**2. `/etc/pam.d/common-password`（パスワード品質）**:

```
password requisite pam_pwquality.so retry=3 \
    minlen=10 \
    ucredit=-1 \
    dcredit=-1 \
    lcredit=-1 \
    maxrepeat=3 \
    reject_username \
    difok=7 \
    enforce_for_root
```

| 設定 | 値 | 意味 |
|------|-----|------|
| retry | 3 | パスワード入力の再試行回数 |
| minlen | 10 | 最小文字数 |
| ucredit | -1 | 大文字を最低1文字含む |
| dcredit | -1 | 数字を最低1文字含む |
| lcredit | -1 | 小文字を最低1文字含む |
| maxrepeat | 3 | 同じ文字を連続3文字以上使用禁止 |
| reject_username | - | ユーザー名をパスワードに含めることを禁止 |
| difok | 7 | 前回のパスワードと最低7文字異なること |
| enforce_for_root | - | root にもポリシーを適用 |

**PAM (Pluggable Authentication Modules) の仕組み**:
```
ユーザーがパスワード変更を要求
    |
    v
passwd コマンド
    |
    v
PAM スタック (/etc/pam.d/common-password)
    |
    ├── pam_pwquality.so  → 品質チェック（長さ、複雑さ）
    ├── pam_unix.so       → パスワードのハッシュ化と /etc/shadow への保存
    └── pam_pwhistory.so  → パスワード履歴の確認（オプション）
```

---

### Q24: sudo の設定

**問い**: Born2beRoot の sudoers 設定を全て説明せよ。各設定項目のセキュリティ上の意味と、誤った設定をした場合のリスクも述べよ。

**回答**:

sudo の設定は `/etc/sudoers.d/` ディレクトリに配置する。`visudo` コマンドで構文チェック付きで編集する。

**Born2beRoot の sudo 設定** (`/etc/sudoers.d/sudo_config`):

```
Defaults  passwd_tries=3
Defaults  badpass_message="Wrong password. Access denied."
Defaults  logfile="/var/log/sudo/sudo.log"
Defaults  log_input
Defaults  log_output
Defaults  iolog_dir="/var/log/sudo"
Defaults  requiretty
Defaults  secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin"
```

**各設定の詳細**:

| 設定 | 意味 | セキュリティ効果 |
|------|------|----------------|
| `passwd_tries=3` | パスワード入力を3回まで許可 | ブルートフォース攻撃の制限 |
| `badpass_message` | 間違ったパスワード時のメッセージ | カスタムメッセージで情報漏洩を制限 |
| `logfile` | sudo の実行ログの保存先 | 監査証跡（誰が何をいつ実行したか） |
| `log_input` | 入力（stdin）をログに記録 | コマンドに渡された入力を追跡 |
| `log_output` | 出力（stdout/stderr）をログに記録 | コマンドの結果を追跡 |
| `iolog_dir` | I/O ログの保存先 | 入出力ログの保管場所 |
| `requiretty` | TTY が必要（スクリプトからの実行を制限） | リモートからの自動化された sudo 実行を防止 |
| `secure_path` | sudo 実行時の PATH を制限 | PATH インジェクション攻撃を防止 |

**`requiretty` の重要性**:
```
requiretty なし（危険）:
  攻撃者がリモートコードを実行 → sudo でコマンドを自動実行可能
  例: cron ジョブ経由、Web アプリの脆弱性経由

requiretty あり（安全）:
  TTY（ターミナル端末）が割り当てられたセッションでのみ sudo 可能
  → リモートスクリプトからの sudo 実行が失敗する
```

**`secure_path` の重要性**:
```
secure_path なし（危険）:
  攻撃者が /tmp/ls (悪意あるスクリプト) を作成
  → ユーザーの PATH に /tmp が含まれていると sudo ls で悪意あるスクリプトが root で実行される

secure_path あり（安全）:
  sudo 実行時は指定されたディレクトリのみからコマンドを検索
  → /tmp のスクリプトは実行されない
```

**誤った設定のリスク**:

| 誤った設定 | リスク |
|-----------|--------|
| `NOPASSWD: ALL` | パスワードなしで全コマンド実行可能。セッション乗っ取りで即座に root 権限 |
| `ALL=(ALL) ALL` を全ユーザーに | 最小権限の原則に違反。全ユーザーが root 権限を取得可能 |
| `!requiretty` | スクリプトやリモートからの自動 sudo 実行が可能に |
| `secure_path` 未設定 | PATH 操作による権限昇格の可能性 |
| `/etc/sudoers` を直接編集 | 構文エラーで sudo が使用不能に。`visudo` 必須 |

---

### Q25: SSH の設定と鍵認証

**問い**: Born2beRoot の SSH 設定（`/etc/ssh/sshd_config`）の重要な項目を列挙し、パスワード認証と公開鍵認証の違いを説明せよ。

**回答**:

**Born2beRoot の sshd_config 設定**:

```bash
# ポート番号（デフォルト 22 から変更）
Port 4242

# root ログインの禁止
PermitRootLogin no

# パスワード認証の許可（ボーナスで鍵認証に変更可能）
PasswordAuthentication yes

# 公開鍵認証の有効化
PubkeyAuthentication yes

# 空パスワードの禁止
PermitEmptyPasswords no

# ログイン試行回数の制限
MaxAuthTries 3

# 接続タイムアウト
LoginGraceTime 60

# 使用するプロトコルバージョン
Protocol 2

# X11 フォワーディングの無効化
X11Forwarding no

# 許可するユーザー（任意設定）
AllowUsers kaztakam
```

**パスワード認証 vs 公開鍵認証**:

| 項目 | パスワード認証 | 公開鍵認証 |
|------|-------------|-----------|
| 認証方式 | パスワードを送信 | 秘密鍵で署名を生成 |
| ブルートフォース | 脆弱 | 事実上不可能 |
| パスワード漏洩 | 危険 | パスワードを送信しないため安全 |
| 利便性 | 手動入力が必要 | 鍵ファイルがあれば自動認証 |
| 管理コスト | 低い | 鍵の配布・管理が必要 |
| 中間者攻撃 | パスワードが傍受される可能性 | 秘密鍵は送信されないため安全 |

**公開鍵認証の仕組み**:
```
クライアント                           サーバー
+------------------+                   +------------------+
| 1. 接続要求      | ---- TCP ----->  | 2. サーバー鍵を送信 |
|                  | <--- 公開鍵 ----  |                  |
| 3. チャレンジへの | ---- 署名 ----> | 4. 署名を検証      |
|    署名を生成    |                   |  ~/.ssh/authorized_keys |
|   (秘密鍵で署名)  |                   |  の公開鍵で検証    |
|                  | <--- 認証OK ---- | 5. セッション確立   |
+------------------+                   +------------------+
```

```bash
# 鍵ペアの生成（クライアント側）
ssh-keygen -t ed25519 -C "user@example.com"
# → ~/.ssh/id_ed25519 (秘密鍵)
# → ~/.ssh/id_ed25519.pub (公開鍵)

# 公開鍵をサーバーに転送
ssh-copy-id -p 4242 user@server

# SSH 接続（鍵認証）
ssh -p 4242 user@server
```

---

### Q26: UFW ファイアウォール

**問い**: UFW の仕組みを、iptables / Netfilter との関係から説明せよ。Born2beRoot で必要なファイアウォールルールと、各ルールの意味を述べよ。

**回答**:

**UFW / iptables / Netfilter の関係**:

```
ユーザーインターフェース層:    UFW (Uncomplicated Firewall)
                                |
                                | UFW が iptables ルールを自動生成
                                v
パケットフィルタリング層:      iptables / nftables
                                |
                                | iptables がカーネルにルールを設定
                                v
カーネル層:                    Netfilter (カーネルモジュール)
                                |
                                | パケットを検査・フィルタリング
                                v
                            ネットワークインターフェース
```

**Netfilter のチェーン**:
```
[受信パケット]
      |
   PREROUTING → INPUT → [ローカルプロセス] → OUTPUT → POSTROUTING → [送信]
      |                                                    ^
      └──── FORWARD ───────────────────────────────────────┘
             (ルーティング: 他ホスト宛のパケットを転送)
```

**Born2beRoot の UFW 設定**:

```bash
# UFW を有効化
sudo ufw enable

# デフォルトポリシー: 全受信を拒否、全送信を許可
sudo ufw default deny incoming
sudo ufw default allow outgoing

# SSH (ポート 4242) を許可
sudo ufw allow 4242

# ボーナス: HTTP (80) を許可（WordPress 用）
sudo ufw allow 80

# ボーナス: HTTPS (443) を許可
sudo ufw allow 443

# ルール一覧の確認
sudo ufw status verbose
```

**UFW ルールの意味**:

| ルール | 意味 | セキュリティ効果 |
|--------|------|----------------|
| `default deny incoming` | 明示的に許可されていない全受信を拒否 | 最小権限の原則。不要なポートを閉じる |
| `default allow outgoing` | 全送信を許可 | パッケージ更新、DNS 解決、NTP 同期に必要 |
| `allow 4242` | TCP/UDP ポート 4242 への接続を許可 | SSH アクセスのみを許可 |

**iptables で実際に何が設定されているか確認**:
```bash
# UFW が生成した iptables ルールを確認
sudo iptables -L -n -v

# UFW のルールファイル
cat /etc/ufw/user.rules
```

---

### Q27: sudo グループとユーザー管理

**問い**: 新しいユーザーを作成し、sudo グループと user42 グループに追加する手順を説明せよ。`useradd` と `adduser` の違いも述べよ。

**回答**:

**ユーザー作成と グループ追加の手順**:

```bash
# 1. ユーザー作成
sudo adduser newuser
# 対話的にパスワード、フルネーム等を設定

# 2. sudo グループに追加
sudo usermod -aG sudo newuser

# 3. user42 グループの作成（初回のみ）
sudo groupadd user42

# 4. user42 グループに追加
sudo usermod -aG user42 newuser

# 5. 確認
id newuser
# uid=1001(newuser) gid=1001(newuser) groups=1001(newuser),27(sudo),1002(user42)
groups newuser
# newuser : newuser sudo user42
```

**`useradd` vs `adduser` の違い**:

| 項目 | `useradd` | `adduser` |
|------|-----------|-----------|
| 種類 | 低レベルコマンド（バイナリ） | 高レベルスクリプト（Perl ラッパー） |
| ホームディレクトリ | `-m` フラグが必要 | 自動作成 |
| パスワード設定 | 別途 `passwd` が必要 | 対話的に設定 |
| シェル設定 | `-s` で指定が必要 | `/etc/adduser.conf` のデフォルト使用 |
| スケルトン | `-k` で指定が必要 | 自動で `/etc/skel` をコピー |
| 対話性 | 非対話的 | 対話的（質問に回答） |
| 推奨用途 | スクリプト内での自動化 | 手動でのユーザー作成 |

```bash
# useradd での同等操作
sudo useradd -m -s /bin/bash -G sudo,user42 newuser
sudo passwd newuser
```

**`usermod -aG` の `-a` フラグの重要性**:
```bash
# -a なし（危険！）: 既存のグループが置き換えられる
sudo usermod -G sudo newuser
# → newuser は sudo グループのみになり、user42 から外れる

# -a あり（安全）: 既存のグループに追加される
sudo usermod -aG sudo newuser
# → newuser の既存グループに sudo が追加される
```

---

### Q28: PAM (Pluggable Authentication Modules)

**問い**: PAM の仕組みを説明せよ。PAM の 4 つのモジュールタイプ（auth, account, password, session）と制御フラグ（required, requisite, sufficient, optional）の違いを述べよ。

**回答**:

PAM は Linux の認証フレームワークで、アプリケーションから認証ロジックを分離する。アプリケーション（login, sshd, su, sudo 等）は PAM を通じて統一的に認証を行う。

```
アプリケーション           PAM 設定                    PAM モジュール
+--------+              /etc/pam.d/                  +-------------+
| login  |──────>  /etc/pam.d/login  ──────>  pam_unix.so (認証)
| sshd   |──────>  /etc/pam.d/sshd   ──────>  pam_pwquality.so (品質)
| sudo   |──────>  /etc/pam.d/sudo   ──────>  pam_tally2.so (ロック)
| su     |──────>  /etc/pam.d/su     ──────>  pam_env.so (環境変数)
+--------+                                   +-------------+
```

**4 つのモジュールタイプ**:

| タイプ | 役割 | 実行タイミング | Born2beRoot での例 |
|--------|------|-------------|-------------------|
| **auth** | ユーザーの本人確認 | ログイン時 | pam_unix.so (パスワード確認) |
| **account** | アカウントの有効性確認 | 認証後 | pam_unix.so (期限切れチェック) |
| **password** | パスワードの変更処理 | パスワード変更時 | pam_pwquality.so (品質チェック) |
| **session** | セッションの開始/終了処理 | ログイン/ログアウト時 | pam_limits.so (リソース制限) |

**制御フラグ**:

| フラグ | 失敗時の動作 | 成功時の動作 | 説明 |
|--------|-----------|-----------|------|
| **required** | 最終的に失敗するが、残りのモジュールも実行 | 次のモジュールへ | 必須。失敗しても他のモジュールの結果を収集 |
| **requisite** | 即座に失敗を返し、残りのモジュールは実行しない | 次のモジュールへ | 必須。即座に失敗 |
| **sufficient** | 次のモジュールへ（無視） | 即座に成功（先行の required が全て成功の場合） | これが成功すれば他は不要 |
| **optional** | 無視 | 無視 | 他にモジュールがない場合のみ結果が使われる |

**PAM スタックの処理フロー例** (`/etc/pam.d/common-password`):
```
password  requisite  pam_pwquality.so retry=3 minlen=10 ...
    ↓ 成功
password  [success=1 default=ignore]  pam_unix.so obscure use_authtok try_first_pass yescrypt
    ↓ 成功 (次の 1 モジュールをスキップ)
password  requisite  pam_deny.so  ← スキップされる
    ↓
password  required   pam_permit.so
```

---

### Q29: ホスト名の変更

**問い**: Born2beRoot でホスト名を変更する手順を説明せよ。変更が必要なファイルと、それぞれの役割を述べよ。

**回答**:

Born2beRoot のホスト名は `<login>42` 形式（例: `kaztakam42`）に設定する。

**ホスト名変更の手順**:

```bash
# 1. 現在のホスト名を確認
hostnamectl

# 2. ホスト名を変更（systemd の hostnamectl を使用）
sudo hostnamectl set-hostname kaztakam42

# 3. /etc/hostname を確認（自動的に更新される）
cat /etc/hostname
# kaztakam42

# 4. /etc/hosts を手動で更新
sudo vim /etc/hosts
```

**変更が必要なファイル**:

| ファイル | 役割 | 内容 |
|---------|------|------|
| `/etc/hostname` | システムの静的ホスト名 | `kaztakam42` |
| `/etc/hosts` | ローカルの名前解決 | `127.0.1.1  kaztakam42` |

**`/etc/hosts` の内容**:
```
127.0.0.1       localhost
127.0.1.1       kaztakam42

# IPv6
::1             localhost ip6-localhost ip6-loopback
ff02::1         ip6-allnodes
ff02::2         ip6-allrouters
```

**なぜ `/etc/hosts` の更新が必要か**:
`/etc/hosts` は DNS を使わないローカルの名前解決に使用される。多くのコマンド（`sudo` 等）はホスト名を解決する必要があるため、`/etc/hosts` にホスト名がないと以下のエラーが発生する:
```
sudo: unable to resolve host kaztakam42: Name or service not found
```

**変更の確認**:
```bash
hostname         # kaztakam42
hostname -f      # FQDN (完全修飾ドメイン名)
hostnamectl      # 詳細情報
```

---

### Q30: cron と monitoring.sh

**問い**: Born2beRoot の monitoring.sh を cron で 10 分ごとに実行する設定方法を説明せよ。cron の書式と、at コマンドとの違いも述べよ。

**回答**:

**cron の設定**:

```bash
# crontab を編集（root で実行）
sudo crontab -e

# 以下の行を追加（10分ごとに実行）
*/10 * * * * /usr/local/bin/monitoring.sh
```

**cron の書式**:
```
┌───────── 分 (0-59)
│ ┌───────── 時 (0-23)
│ │ ┌───────── 日 (1-31)
│ │ │ ┌───────── 月 (1-12)
│ │ │ │ ┌───────── 曜日 (0-7, 0と7は日曜)
│ │ │ │ │
* * * * * コマンド

例:
*/10 * * * *     → 10分ごと
0 */2 * * *      → 2時間ごと（0分に実行）
30 3 * * 1       → 毎週月曜 3:30
0 0 1 * *        → 毎月1日 0:00
@reboot          → 起動時に1回
```

**cron vs at の違い**:

| 項目 | cron | at |
|------|------|-----|
| 実行頻度 | 繰り返し実行 | 1回のみ |
| スケジュール | 分/時/日/月/曜日で指定 | 日時で指定 |
| 用途 | 定期的なタスク（監視、バックアップ） | 一度きりのタスク（メンテナンス） |
| 管理 | `crontab -e` / `crontab -l` | `at`, `atq`, `atrm` |
| デーモン | crond | atd |

**monitoring.sh のスケジューリング停止方法**（評価時の確認事項）:

```bash
# 方法1: crontab からエントリを削除
sudo crontab -e
# → 該当行を削除またはコメントアウト

# 方法2: cron サービスを停止（全 cron ジョブが停止）
sudo systemctl stop cron

# 方法3: monitoring.sh の実行権限を削除
sudo chmod -x /usr/local/bin/monitoring.sh
```

---

### Q31: /etc/shadow のフォーマット

**問い**: `/etc/shadow` ファイルの各フィールドの意味を説明せよ。パスワードハッシュのアルゴリズムの識別方法も述べよ。

**回答**:

```bash
# /etc/shadow の1行の構造
kaztakam:$y$j9T$salt$hash:19700:2:30:7:::
│        │                  │    │ │  │
│        │                  │    │ │  └── PASS_WARN_AGE (警告開始日数)
│        │                  │    │ └── PASS_MAX_DAYS (最大有効日数)
│        │                  │    └── PASS_MIN_DAYS (最小変更日数)
│        │                  └── 最終パスワード変更日 (1970/1/1 からの日数)
│        └── パスワードハッシュ
└── ユーザー名
```

**全 9 フィールド**:

| # | フィールド | 例 | 説明 |
|---|----------|-----|------|
| 1 | ユーザー名 | kaztakam | ログイン名 |
| 2 | パスワードハッシュ | $y$j9T$... | ハッシュ化されたパスワード |
| 3 | 最終変更日 | 19700 | 最後にパスワードを変更した日（エポックからの日数） |
| 4 | 最小日数 | 2 | 次の変更まで待つ日数 |
| 5 | 最大日数 | 30 | パスワードの有効期限 |
| 6 | 警告日数 | 7 | 期限切れ前の警告開始日数 |
| 7 | 猶予日数 | (空) | 期限切れ後もログイン可能な日数 |
| 8 | アカウント期限 | (空) | アカウントの有効期限（エポックからの日数） |
| 9 | 予約 | (空) | 将来の使用のために予約 |

**パスワードハッシュのアルゴリズム識別**:

| プレフィックス | アルゴリズム | セキュリティ |
|-------------|-----------|------------|
| `$1$` | MD5 | 危険。使用禁止 |
| `$5$` | SHA-256 | 許容可能 |
| `$6$` | SHA-512 | 推奨 |
| `$y$` / `$7$` | yescrypt | 最も推奨（Debian 11+ のデフォルト） |
| `$2b$` | bcrypt | 良好（主に BSD 系で使用） |

**特殊な値**:
- `!` または `*`: パスワードロック（ログイン不可）
- `!!`: パスワード未設定
- 空文字: パスワードなし（危険）

```bash
# ハッシュアルゴリズムの確認
sudo cat /etc/shadow | grep kaztakam

# 現在のデフォルトアルゴリズムの確認
cat /etc/login.defs | grep ENCRYPT_METHOD

# パスワード情報の確認
sudo chage -l kaztakam
```

---

### Q32: AppArmor のプロファイル管理

**問い**: AppArmor の enforce モードと complain モードの違いを説明せよ。カスタムプロファイルの作成手順も述べよ。

**回答**:

**AppArmor のモード**:

| モード | 動作 | 用途 |
|--------|------|------|
| **enforce** | ポリシー違反のアクセスをブロックし、ログに記録 | 本番環境 |
| **complain** | ポリシー違反のアクセスを許可するが、ログに記録 | プロファイル開発・デバッグ |
| **unconfined** | プロファイルなし。制限なし | AppArmor 未対応のプログラム |

```bash
# 現在の AppArmor 状態を確認
sudo aa-status

# 出力例:
# apparmor module is loaded.
# 40 profiles are loaded.
# 20 profiles are in enforce mode.
# 2 profiles are in complain mode.
# 18 profiles are in unconfined mode.

# モード切り替え
sudo aa-enforce /etc/apparmor.d/usr.sbin.sshd    # → enforce モード
sudo aa-complain /etc/apparmor.d/usr.sbin.sshd   # → complain モード
sudo aa-disable /etc/apparmor.d/usr.sbin.sshd    # → 無効化
```

**カスタムプロファイルの作成手順**:

```bash
# 1. プロファイルの自動生成を開始
sudo aa-genprof /usr/local/bin/monitoring.sh

# 2. 別の端末で monitoring.sh を実行
sudo /usr/local/bin/monitoring.sh

# 3. aa-genprof に戻り、ログを分析して許可/拒否を選択
# (S)can → 検出されたアクセスを確認
# (A)llow → 許可
# (D)eny → 拒否
# (F)inish → プロファイルを保存

# 4. 生成されたプロファイルを確認
cat /etc/apparmor.d/usr.local.bin.monitoring.sh
```

**プロファイルの構文例**:
```
/usr/local/bin/monitoring.sh {
  # 実行許可
  /usr/bin/bash ix,
  /usr/bin/wc ix,
  /usr/bin/grep ix,
  /usr/bin/awk ix,
  /usr/bin/wall ix,
  /usr/bin/free ix,
  /usr/bin/df ix,
  /usr/sbin/lsblk ix,

  # 読み取り許可
  /proc/** r,
  /sys/** r,
  /etc/hostname r,

  # ネットワークアクセス拒否
  deny network,

  # 書き込み拒否（監視スクリプトは読み取り専用であるべき）
  deny /home/** w,
  deny /etc/** w,
}
```

---

### Q33: Fail2ban（ボーナス）

**問い**: Fail2ban の仕組みと Born2beRoot での設定方法を説明せよ。SSH ブルートフォース攻撃からの保護の仕組みを述べよ。

**回答**:

Fail2ban はログファイルを監視し、認証失敗などの不審なパターンを検出した IP アドレスを自動的にブロックするツールである。

**動作原理**:
```
[SSH ログイン試行]
      |
      v
/var/log/auth.log に記録
      |
      v
[Fail2ban デーモン]
  |
  ├── ログを定期的にスキャン
  ├── フィルタ正規表現でマッチング
  ├── 失敗回数をカウント
  └── 閾値超過 → アクション実行
          |
          v
      [iptables / UFW で IP をブロック]
      "ufw insert 1 deny from <ip>"
```

**Born2beRoot での設定**:

```bash
# インストール
sudo apt install fail2ban

# 設定ファイルのコピー（jail.local を使用。jail.conf は直接編集しない）
sudo cp /etc/fail2ban/jail.conf /etc/fail2ban/jail.local
```

**`/etc/fail2ban/jail.local` の SSH 設定**:
```ini
[sshd]
enabled  = true
port     = 4242
filter   = sshd
logpath  = /var/log/auth.log
maxretry = 3
findtime = 600
bantime  = 600
action   = ufw
```

| 設定 | 値 | 意味 |
|------|-----|------|
| enabled | true | この jail を有効化 |
| port | 4242 | SSH のポート番号 |
| filter | sshd | 使用するフィルタ（`/etc/fail2ban/filter.d/sshd.conf`） |
| logpath | /var/log/auth.log | 監視するログファイル |
| maxretry | 3 | この回数失敗したらブロック |
| findtime | 600 | この秒数内の失敗をカウント（10分） |
| bantime | 600 | ブロックする秒数（10分。-1 で永久） |
| action | ufw | ブロックに使用するツール |

```bash
# Fail2ban の状態確認
sudo fail2ban-client status
sudo fail2ban-client status sshd

# ブロックされた IP の確認
sudo fail2ban-client get sshd banned

# 手動でブロック解除
sudo fail2ban-client set sshd unbanip 192.168.1.100
```

---

### Q34: SELinux と AppArmor の深い比較

**問い**: SELinux のラベルベースのアクセス制御と AppArmor のパスベースのアクセス制御の違いを、具体例を交えて説明せよ。

**回答**:

**SELinux のラベルベース**:

SELinux は全てのプロセスとファイルに「セキュリティコンテキスト（ラベル）」を付与し、ラベル間のアクセスルールを定義する。

```bash
# SELinux のラベル確認 (RHEL/CentOS の場合)
ls -Z /var/www/html/index.html
# system_u:object_r:httpd_sys_content_t:s0

#  user   : role    : type                : level
# system_u:object_r:httpd_sys_content_t  :s0
```

```
SELinux のアクセス制御:
  プロセス (httpd_t) → ファイル (httpd_sys_content_t) → 許可
  プロセス (httpd_t) → ファイル (user_home_t)         → 拒否

  ※ ファイルのパスに関係なく、ラベルで判断
```

**AppArmor のパスベース**:

AppArmor はファイルパスに基づいてアクセスルールを定義する。

```
# AppArmor プロファイル
/usr/sbin/httpd {
  /var/www/html/** r,     # このパスは読み取り可能
  /home/** deny,          # このパスは拒否
}
```

```
AppArmor のアクセス制御:
  プロセス (/usr/sbin/httpd) → /var/www/html/index.html → 許可 (パスが一致)
  プロセス (/usr/sbin/httpd) → /home/user/secret.txt    → 拒否 (パスが一致)

  ※ ファイルのラベルは関係なく、パスで判断
```

**具体的な違い**:

| 操作 | SELinux | AppArmor |
|------|---------|----------|
| ファイルを別の場所に移動 | ラベルが維持されるため、元のポリシーが適用 | パスが変わるため、別のルールが適用される |
| ハードリンクの作成 | 同じラベルを共有 | パスごとに異なるルールが適用される可能性 |
| 新規ファイルの作成 | 親ディレクトリのタイプを継承 | パスに基づくルールが自動適用 |
| ポリシーの可読性 | 低い（ラベルとポリシー言語の理解が必要） | 高い（ファイルパスで直感的） |

---

### Q35: ネットワークセキュリティの基礎

**問い**: TCP/IP モデルの 4 層を説明し、SSH 接続時に各層で何が起こるかを述べよ。3-way handshake の仕組みも説明せよ。

**回答**:

**TCP/IP モデルの 4 層**:

```
+------------------+  SSH 接続時の動作
| アプリケーション層  |  SSH プロトコル（鍵交換、認証、暗号化通信）
| (Application)    |  → ユーザーのコマンドを暗号化して送受信
+------------------+
| トランスポート層   |  TCP（ポート 4242 への信頼性ある通信）
| (Transport)      |  → 3-way handshake、シーケンス番号管理
+------------------+
| インターネット層   |  IP（送信元/宛先 IP アドレスでルーティング）
| (Internet)       |  → パケットをネクストホップに転送
+------------------+
| ネットワーク      |  Ethernet / Wi-Fi（物理フレームの送受信）
| アクセス層        |  → MAC アドレスで隣接ノードに配送
| (Network Access) |
+------------------+
```

**3-way handshake**:

```
クライアント                           サーバー (ポート 4242)
    |                                      |
    |  1. SYN (seq=100)                    |
    |  ------------------------------------->
    |  「接続したい。シーケンス番号は 100」     |
    |                                      |
    |  2. SYN-ACK (seq=300, ack=101)       |
    |  <-------------------------------------
    |  「了解。私の番号は 300。101 を待つ」    |
    |                                      |
    |  3. ACK (seq=101, ack=301)           |
    |  ------------------------------------->
    |  「了解。301 を待つ。接続確立」          |
    |                                      |
    |  ===== TCP 接続確立 =====              |
    |                                      |
    |  4. SSH バージョン交換                  |
    |  SSH-2.0-OpenSSH_9.2p1 ----------->  |
    |  <----------- SSH-2.0-OpenSSH_9.2p1  |
    |                                      |
    |  5. 鍵交換 (Key Exchange)             |
    |  Diffie-Hellman 鍵共有               |
    |  → セッション鍵の生成                  |
    |                                      |
    |  6. ユーザー認証                       |
    |  パスワード or 公開鍵認証              |
    |                                      |
    |  ===== SSH セッション確立 =====         |
```

**各層の具体的な処理**:

| 層 | SSH 接続時の具体的処理 | プロトコル |
|----|----------------------|----------|
| アプリケーション | SSH バージョン交換、鍵交換、認証、暗号化通信 | SSH |
| トランスポート | 3-way handshake、データのセグメント化、再送制御 | TCP |
| インターネット | ルーティングテーブルに基づくパケット転送 | IP |
| ネットワークアクセス | ARP で MAC アドレス解決、Ethernet フレーム送信 | Ethernet |

---

### Q36: 最小権限の原則

**問い**: 最小権限の原則 (Principle of Least Privilege) とは何か。Born2beRoot でこの原則がどのように適用されているか、5 つ以上の具体例を挙げて説明せよ。

**回答**:

最小権限の原則とは「ユーザーやプロセスに対して、その業務や機能に必要な最小限の権限のみを付与する」というセキュリティの基本原則である。

**Born2beRoot での適用例**:

| # | 設定 | 最小権限の適用 |
|---|------|-------------|
| 1 | `PermitRootLogin no` | root での SSH ログインを禁止。必要時は sudo で権限昇格 |
| 2 | `sudo` グループ制限 | 全ユーザーではなく、特定ユーザーのみ sudo を許可 |
| 3 | `secure_path` | sudo 実行時の PATH を必要なディレクトリのみに制限 |
| 4 | UFW デフォルト deny | 全ポートを閉じ、必要なポート（4242）のみ開放 |
| 5 | AppArmor プロファイル | 各プログラムに必要なファイル/リソースのみアクセス許可 |
| 6 | パーティション分離 | `/tmp` に `noexec` を設定し、一時ファイルの実行を禁止 |
| 7 | ファイルパーミッション | `/etc/shadow` は root と shadow グループのみ読み取り可能（640） |
| 8 | SSH 鍵の権限 | `~/.ssh/authorized_keys` は所有者のみ読み書き可能（600） |
| 9 | `requiretty` | TTY なしでの sudo 実行を禁止し、自動化攻撃を制限 |
| 10 | cron の root 実行 | monitoring.sh は root の crontab で実行し、一般ユーザーの cron を制限 |

**最小権限の原則の階層**:
```
+---------------------------------------------------+
| ネットワーク: UFW で必要なポートのみ開放              |
|  +-----------------------------------------------+|
|  | ユーザー: sudo グループで必要なユーザーのみ権限昇格 ||
|  |  +-------------------------------------------+||
|  |  | プロセス: AppArmor で各プロセスの権限を制限    |||
|  |  |  +---------------------------------------+|||
|  |  |  | ファイル: パーミッションで最小限のアクセス  ||||
|  |  |  +---------------------------------------+|||
|  |  +-------------------------------------------+||
|  +-----------------------------------------------+|
+---------------------------------------------------+
```

この原則を徹底することで、仮に1つの層が突破されても、次の層で攻撃を制限できる（多層防御 / Defense in Depth）。

---

## セクション D: 監視とスクリプト (Questions 37-46)

### Q37: monitoring.sh の全内容

**問い**: Born2beRoot の monitoring.sh が収集する全ての情報を列挙し、各情報を取得するためのコマンドと仕組みを説明せよ。

**回答**:

monitoring.sh は wall コマンドで全ユーザーの端末に以下の情報をブロードキャストする:

```bash
#!/bin/bash

# 1. OS アーキテクチャとカーネルバージョン
ARCH=$(uname -a)

# 2. 物理 CPU 数
PCPU=$(grep "physical id" /proc/cpuinfo | sort -u | wc -l)

# 3. 仮想 CPU (vCPU) 数
VCPU=$(grep "^processor" /proc/cpuinfo | wc -l)

# 4. RAM 使用量
RAM_TOTAL=$(free -m | awk '/Mem:/ {print $2}')
RAM_USED=$(free -m | awk '/Mem:/ {print $3}')
RAM_PERC=$(free -m | awk '/Mem:/ {printf("%.2f"), $3/$2*100}')

# 5. ディスク使用量
DISK_TOTAL=$(df -BG --total | awk '/total/ {print $2}')
DISK_USED=$(df -BM --total | awk '/total/ {print $3}')
DISK_PERC=$(df --total | awk '/total/ {print $5}')

# 6. CPU 負荷
CPU_LOAD=$(top -bn1 | grep "Cpu(s)" | awk '{printf("%.1f%%"), $2+$4}')

# 7. 最終起動日時
LAST_BOOT=$(who -b | awk '{print $3" "$4}')

# 8. LVM の使用状態
LVM=$(if [ $(lsblk | grep "lvm" | wc -l) -gt 0 ]; then echo yes; else echo no; fi)

# 9. TCP 接続数
TCP=$(ss -t | grep ESTAB | wc -l)

# 10. ログインユーザー数
USER_LOG=$(who | wc -l)

# 11. IP アドレスと MAC アドレス
IP=$(hostname -I | awk '{print $1}')
MAC=$(ip link show | awk '/ether/ {print $2}')

# 12. sudo 実行回数
SUDO_LOG=$(journalctl _COMM=sudo | grep COMMAND | wc -l)

# wall で全端末にブロードキャスト
wall "
    #Architecture: $ARCH
    #CPU physical: $PCPU
    #vCPU: $VCPU
    #Memory Usage: $RAM_USED/${RAM_TOTAL}MB ($RAM_PERC%)
    #Disk Usage: $DISK_USED/${DISK_TOTAL} ($DISK_PERC)
    #CPU load: $CPU_LOAD
    #Last boot: $LAST_BOOT
    #LVM use: $LVM
    #Connections TCP: $TCP ESTABLISHED
    #User log: $USER_LOG
    #Network: IP $IP ($MAC)
    #Sudo: $SUDO_LOG cmd
"
```

**各コマンドの解説**:

| 情報 | コマンド | 仕組み |
|------|---------|--------|
| アーキテクチャ | `uname -a` | カーネル名、ホスト名、カーネルバージョン、CPU アーキテクチャ等を表示 |
| 物理 CPU | `/proc/cpuinfo` の `physical id` | 物理 CPU ソケット数。sort -u で重複を排除 |
| 仮想 CPU | `/proc/cpuinfo` の `processor` | 論理プロセッサ数（物理 CPU × コア数 × ハイパースレッド） |
| RAM | `free -m` | メモリ情報を MB 単位で表示。/proc/meminfo を読み取る |
| ディスク | `df -BG --total` | ファイルシステムのディスク使用量。--total で合計行を追加 |
| CPU 負荷 | `top -bn1` | -b はバッチモード、-n1 は1回のみ。us（ユーザー）+ sy（システム）の CPU 使用率 |
| 最終起動 | `who -b` | /var/run/utmp から最終ブート時刻を取得 |
| LVM | `lsblk` | ブロックデバイスのタイプに "lvm" が含まれるか確認 |
| TCP | `ss -t` | ss (socket statistics) で TCP 接続の ESTABLISHED 状態をカウント |
| ログインユーザー | `who` | /var/run/utmp からログイン中のユーザーを表示 |
| IP/MAC | `hostname -I` / `ip link` | ネットワークインターフェースの IP と MAC アドレス |
| sudo 回数 | `journalctl _COMM=sudo` | systemd ジャーナルから sudo コマンドの実行履歴をカウント |

---

### Q38: wall コマンド

**問い**: wall コマンドの仕組みを説明せよ。どのユーザーの端末にメッセージが表示されるか。mesg コマンドとの関係も述べよ。

**回答**:

`wall` (Write ALL) は全てのログイン中ユーザーの端末にメッセージをブロードキャストするコマンドである。

```
wall "Hello everyone!"

端末1 (kaztakam - tty1)     → メッセージ表示
端末2 (kaztakam - pts/0)    → メッセージ表示 (SSH セッション)
端末3 (otheruser - pts/1)   → メッセージ表示 (SSH セッション)
端末4 (otheruser - pts/2)   → mesg n の場合、表示されない
```

**wall の動作原理**:
1. `/var/run/utmp` からログイン中のユーザーとその端末デバイスを取得
2. 各端末デバイス（例: `/dev/pts/0`）を開いてメッセージを書き込む
3. 端末デバイスに書き込み権限がない場合はスキップ

**mesg コマンド**:
```bash
# メッセージの受信を拒否
mesg n
# → 端末デバイスの group write 権限を削除 (crw--w---- → crw-------)

# メッセージの受信を許可
mesg y
# → 端末デバイスの group write 権限を付与 (crw------- → crw--w----)

# 現在の状態を確認
mesg
# is y  または  is n
```

**注意点**:
- root ユーザーから送信された wall メッセージは `mesg n` でもブロックできない
- monitoring.sh が root の cron で実行される場合、全端末にメッセージが届く

---

### Q39: /proc ファイルシステム

**問い**: /proc ファイルシステムの仕組みを説明せよ。monitoring.sh が参照する /proc 配下のファイルとその内容を述べよ。

**回答**:

/proc は仮想ファイルシステム (procfs) で、カーネルがリアルタイムにシステム情報を提供する。ディスク上にデータは存在せず、読み取り時にカーネルが動的に内容を生成する。

```
/proc/
├── 1/              ← PID 1 (systemd) のプロセス情報
│   ├── cmdline     ← 実行コマンドライン
│   ├── status      ← プロセス状態（メモリ、UID 等）
│   ├── fd/         ← オープンしているファイルディスクリプタ
│   └── maps        ← メモリマッピング
├── cpuinfo         ← CPU 情報（monitoring.sh で使用）
├── meminfo         ← メモリ情報（free コマンドのソース）
├── uptime          ← 起動からの経過時間
├── loadavg         ← ロードアベレージ
├── net/
│   ├── tcp         ← TCP 接続情報
│   └── dev         ← ネットワークインターフェース統計
├── sys/
│   ├── vm/swappiness  ← スワップ設定
│   └── kernel/hostname ← ホスト名
├── version         ← カーネルバージョン（uname のソース）
└── partitions      ← パーティション情報
```

**monitoring.sh が参照する /proc ファイル**:

| ファイル | コマンド | 情報 |
|---------|---------|------|
| `/proc/cpuinfo` | `grep "physical id"` / `grep "processor"` | CPU 数 |
| `/proc/meminfo` | `free` コマンドが内部で参照 | メモリ使用量 |
| `/proc/stat` | `top` コマンドが内部で参照 | CPU 使用率 |
| `/proc/net/tcp` | `ss` コマンドが内部で参照 | TCP 接続 |
| `/proc/version` | `uname` が内部で参照 | カーネルバージョン |

```bash
# /proc/cpuinfo の内容例
cat /proc/cpuinfo
# processor     : 0
# vendor_id     : GenuineIntel
# model name    : Intel(R) Core(TM) i7-8700
# physical id   : 0
# cpu cores     : 1

# /proc/meminfo の内容例
cat /proc/meminfo
# MemTotal:       1024000 kB
# MemFree:         512000 kB
# MemAvailable:    768000 kB
# Buffers:          32000 kB
# Cached:          128000 kB
```

---

### Q40: シェルスクリプトの基礎

**問い**: monitoring.sh で使用されている以下のシェルスクリプト構文を説明せよ: 変数展開 `$()`, パイプ `|`, `awk`, `grep`, `sed`, リダイレクト。

**回答**:

**コマンド置換 `$()`**:
```bash
# $() 内のコマンドを実行し、その出力を変数に代入
VCPU=$(grep "^processor" /proc/cpuinfo | wc -l)
# 1. grep が /proc/cpuinfo から "processor" 行を抽出
# 2. wc -l で行数をカウント
# 3. 結果が VCPU 変数に代入される

# バッククォート `` は古い書式。ネスト不可なので $() を推奨
VCPU=`grep "^processor" /proc/cpuinfo | wc -l`  # 非推奨
```

**パイプ `|`**:
```bash
# コマンドの標準出力を次のコマンドの標準入力に接続
grep "physical id" /proc/cpuinfo | sort -u | wc -l
# 1. grep: "physical id" を含む行を抽出
# 2. sort -u: ユニークな行のみ残す
# 3. wc -l: 行数をカウント

# パイプの内部動作:
# grep → FIFO → sort → FIFO → wc
# 各コマンドは並列に実行される（プロデューサー・コンシューマー）
```

**awk**:
```bash
# フィールド処理言語。スペース区切りのフィールドを操作
free -m | awk '/Mem:/ {printf("%.2f"), $3/$2*100}'
# /Mem:/ : "Mem:" を含む行にマッチ
# $2     : 2番目のフィールド（合計メモリ）
# $3     : 3番目のフィールド（使用中メモリ）
# printf : フォーマット付き出力

# awk のフィールド番号
# $0 = 行全体
# $1 = 最初のフィールド
# $NF = 最後のフィールド
```

**grep**:
```bash
# パターンマッチでテキストをフィルタリング
grep "^processor" /proc/cpuinfo
# ^ : 行頭にマッチ（正規表現）

# よく使うオプション
grep -i "pattern"   # 大文字小文字を無視
grep -v "pattern"   # マッチしない行を表示
grep -c "pattern"   # マッチした行数を表示
grep -E "pat1|pat2" # 拡張正規表現（OR）
```

**リダイレクト**:
```bash
# 標準出力をファイルに書き込み
command > file      # 上書き
command >> file     # 追記

# 標準エラー出力をリダイレクト
command 2> /dev/null  # エラーを破棄
command 2>&1          # エラーを標準出力に合流

# monitoring.sh での例
journalctl _COMM=sudo 2>/dev/null | grep COMMAND | wc -l
# journalctl のエラー出力を破棄し、正常な出力のみパイプに渡す
```

---

### Q41: systemd タイマー vs cron

**問い**: systemd タイマーと cron の違いを説明し、monitoring.sh を systemd タイマーで実行する設定例を示せ。

**回答**:

| 項目 | cron | systemd タイマー |
|------|------|-----------------|
| 設定方式 | crontab (1行) | .timer + .service (2ファイル) |
| ログ | 標準ログなし | journalctl で確認可能 |
| 依存関係 | なし | 他の Unit に依存可能 |
| 精度 | 分単位 | 秒単位（マイクロ秒も可） |
| 漏れた実行 | スキップ | Persistent=true で起動後に実行 |
| リソース制御 | なし | cgroup で CPU/メモリ制限可能 |
| 起動後の遅延 | @reboot | OnBootSec= で秒単位指定 |

**systemd タイマーでの monitoring.sh 設定例**:

`/etc/systemd/system/monitoring.service`:
```ini
[Unit]
Description=Born2beRoot Monitoring Script
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/local/bin/monitoring.sh
StandardOutput=journal
```

`/etc/systemd/system/monitoring.timer`:
```ini
[Unit]
Description=Run monitoring every 10 minutes

[Timer]
OnBootSec=0min
OnUnitActiveSec=10min
AccuracySec=1s
Persistent=true

[Install]
WantedBy=timers.target
```

```bash
# 有効化と起動
sudo systemctl daemon-reload
sudo systemctl enable --now monitoring.timer

# 状態確認
sudo systemctl status monitoring.timer
sudo systemctl list-timers

# ログ確認
sudo journalctl -u monitoring.service
```

Born2beRoot では cron の使用が要件であるため、実際には cron を使用する。ただし systemd タイマーの方が柔軟でモダンなアプローチであることを理解しておくとよい。

---

### Q42: ログ管理

**問い**: Linux のログ管理システムを説明せよ。rsyslog と journald の違い、重要なログファイルの場所と内容を述べよ。

**回答**:

**ログシステムの構成**:
```
アプリケーション / カーネル
    |
    ├── syslog() API ──> rsyslog ──> /var/log/ 配下のテキストファイル
    |                        |
    └── sd_journal_print() ──> journald ──> /var/log/journal/ (バイナリ)
                                  |
                                  └──> rsyslog に転送可能
```

**rsyslog vs journald**:

| 項目 | rsyslog | journald |
|------|---------|----------|
| 形式 | テキストファイル | バイナリ（構造化データ） |
| 保存場所 | /var/log/ | /var/log/journal/ |
| 閲覧 | cat, less, grep | journalctl |
| フィルタリング | grep でテキスト検索 | メタデータ（ユニット名、PID 等）で高速検索 |
| ログローテーション | logrotate | journald 自体の設定 |
| ブートログ | 含まない | ブート単位で管理 |
| パフォーマンス | 低い（テキスト処理） | 高い（インデックス付きバイナリ） |

**重要なログファイル**:

| ファイル | 内容 | 用途 |
|---------|------|------|
| `/var/log/auth.log` | 認証関連ログ（SSH、sudo、PAM） | 不正ログイン検知 |
| `/var/log/syslog` | 全般的なシステムログ | トラブルシューティング |
| `/var/log/kern.log` | カーネルログ | ハードウェア問題の調査 |
| `/var/log/dpkg.log` | パッケージ管理のログ | インストール/更新の履歴 |
| `/var/log/apt/history.log` | apt の操作履歴 | 何がいつインストールされたか |
| `/var/log/sudo/sudo.log` | sudo の実行ログ（Born2beRoot 設定） | 監査証跡 |
| `/var/log/ufw.log` | UFW ファイアウォールのログ | ブロックされた接続の確認 |

```bash
# journalctl の便利な使い方
sudo journalctl -u sshd --since "1 hour ago"  # SSH の直近1時間のログ
sudo journalctl -p err                         # エラー以上のログのみ
sudo journalctl --disk-usage                   # ジャーナルのディスク使用量
sudo journalctl --vacuum-size=100M             # 100MB まで削減
```

---

### Q43: シグナルとプロセス管理

**問い**: Linux のシグナルの仕組みを説明せよ。SIGTERM, SIGKILL, SIGHUP, SIGINT の違いと、プロセスの状態遷移（R, S, D, Z, T）も述べよ。

**回答**:

**シグナル** はプロセス間通信 (IPC) の一種で、プロセスに対して非同期的にイベントを通知する仕組みである。

| シグナル | 番号 | デフォルト動作 | 説明 |
|---------|------|-------------|------|
| SIGTERM | 15 | 終了 | 正常終了要求。プロセスがクリーンアップ可能 |
| SIGKILL | 9 | 即座に終了 | 強制終了。プロセスは処理できない |
| SIGHUP | 1 | 終了 | 端末切断。デーモンでは設定リロードに使用 |
| SIGINT | 2 | 終了 | Ctrl+C による割り込み |
| SIGSTOP | 19 | 停止 | プロセスを一時停止。処理できない |
| SIGCONT | 18 | 再開 | SIGSTOP で停止したプロセスを再開 |
| SIGUSR1 | 10 | 終了 | ユーザー定義シグナル 1 |
| SIGCHLD | 17 | 無視 | 子プロセスの状態変化を親に通知 |

```bash
# シグナルの送信
kill -SIGTERM 1234      # PID 1234 に SIGTERM を送信
kill -15 1234           # 同じ
kill -9 1234            # SIGKILL (最後の手段)
killall sshd            # プロセス名で指定
pkill -f "monitoring"   # パターンマッチで指定
```

**プロセスの状態遷移**:

```
                    SIGSTOP
    ┌──────────────────────────────┐
    |                              v
[Created] → [R: Running] ─────> [T: Stopped]
               |    ^              |
               |    | スケジュール  | SIGCONT
               v    |              v
            [S: Sleeping] ←──── [R: Running]
               |
               | I/O 待ち
               v
            [D: Disk Sleep]  ← kill -9 でも終了できない（I/O 完了待ち）
               |
               v
            [R: Running] → exit() → [Z: Zombie]
                                        |
                                        | 親が wait() を呼ぶ
                                        v
                                    [消滅]
```

| 状態 | 記号 | 説明 | `ps` での表示 |
|------|------|------|-------------|
| Running | R | CPU で実行中 or 実行可能キューで待機 | R |
| Sleeping | S | イベント待ち（割り込み可能） | S |
| Disk Sleep | D | I/O 完了待ち（割り込み不可） | D |
| Zombie | Z | 終了済みだが親が wait() していない | Z |
| Stopped | T | シグナルで停止中 | T |

---

### Q44: bash のデバッグ

**問い**: monitoring.sh にバグがある場合のデバッグ方法を 3 つ以上示せ。

**回答**:

**方法 1: set -x でトレース実行**
```bash
#!/bin/bash
set -x  # スクリプト全体をトレース

# または特定の区間だけ
set -x
VCPU=$(grep "^processor" /proc/cpuinfo | wc -l)
set +x

# 出力例:
# + grep '^processor' /proc/cpuinfo
# + wc -l
# + VCPU=1
```

**方法 2: bash -x で実行**
```bash
# スクリプトを変更せずにトレース
bash -x /usr/local/bin/monitoring.sh

# 詳細なトレース（ネストされた関数呼び出しも表示）
bash -xv /usr/local/bin/monitoring.sh
```

**方法 3: set -euo pipefail でエラー検出**
```bash
#!/bin/bash
set -e          # エラーが発生したら即座に終了
set -u          # 未定義変数の使用をエラーにする
set -o pipefail # パイプ内のエラーを検出

# 組み合わせ（ベストプラクティス）
set -euo pipefail
```

**方法 4: 各変数の値を出力**
```bash
echo "DEBUG: VCPU=$VCPU" >&2
echo "DEBUG: RAM_TOTAL=$RAM_TOTAL" >&2
# >&2 で標準エラー出力に出す（wall の出力に影響しない）
```

**方法 5: shellcheck による静的解析**
```bash
# shellcheck をインストール
sudo apt install shellcheck

# スクリプトを解析
shellcheck /usr/local/bin/monitoring.sh

# 出力例:
# Line 10: VCPU=$(grep ^processor /proc/cpuinfo | wc -l)
#          ^-- SC2126: Consider using grep -c instead of grep | wc -l
```

**方法 6: trap で終了時のデバッグ情報出力**
```bash
#!/bin/bash
trap 'echo "ERROR at line $LINENO: $BASH_COMMAND" >&2' ERR

# エラー発生時に行番号と実行しようとしたコマンドが表示される
```

---

### Q45: 環境変数と設定ファイル

**問い**: Linux の環境変数の仕組みを説明せよ。`/etc/profile`, `~/.bashrc`, `~/.profile` の読み込み順序と違いも述べよ。

**回答**:

**環境変数** はプロセスに関連付けられたキーと値のペアで、子プロセスに継承される。

```bash
# 環境変数の設定
export PATH="/usr/local/bin:$PATH"

# 環境変数の確認
echo $PATH
env           # 全環境変数を表示
printenv PATH # 特定の環境変数

# シェル変数（環境変数ではない。子プロセスに継承されない）
MY_VAR="hello"   # export なし → シェル変数
export MY_VAR    # export あり → 環境変数に昇格
```

**設定ファイルの読み込み順序**:

```
ログインシェル (ssh ログイン, su - の場合):
  1. /etc/profile          ← 全ユーザー共通の設定
  2. /etc/profile.d/*.sh   ← 追加の全ユーザー設定
  3. ~/.bash_profile       ← ユーザー個別（存在すれば）
     または ~/.bash_login  ← ↑がなければ
     または ~/.profile     ← ↑もなければ

非ログインシェル (ターミナルエミュレータ, bash の場合):
  1. /etc/bash.bashrc      ← 全ユーザー共通
  2. ~/.bashrc             ← ユーザー個別
```

| ファイル | 種類 | 読み込みタイミング | 用途 |
|---------|------|-----------------|------|
| `/etc/profile` | ログインシェル | ログイン時 | 全ユーザーの PATH、umask |
| `/etc/profile.d/*.sh` | ログインシェル | ログイン時 | パッケージ固有の環境設定 |
| `~/.profile` | ログインシェル | ログイン時 | ユーザーの環境変数 |
| `~/.bashrc` | 非ログインシェル | bash 起動時 | エイリアス、プロンプト設定 |
| `/etc/bash.bashrc` | 非ログインシェル | bash 起動時 | 全ユーザーの bash 設定 |
| `/etc/environment` | 全シェル | PAM 経由 | システム全体の環境変数 |

**sudo での環境変数**:
```bash
# sudo は secure_path で PATH を上書きする
sudo env | grep PATH
# PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

# 元の環境変数を保持したい場合
sudo -E command    # 環境変数を保持（secure_path は除く）
sudo env "PATH=$PATH" command  # 特定の変数を渡す
```

---

### Q46: スクリプトの権限とセキュリティ

**問い**: monitoring.sh を安全に実行するために必要な権限設定を説明せよ。スクリプトの SUID ビットの危険性と、安全な代替手段も述べよ。

**回答**:

**monitoring.sh の推奨権限設定**:
```bash
# 所有者: root、権限: rwxr-xr-x (755)
ls -la /usr/local/bin/monitoring.sh
-rwxr-xr-x 1 root root 1234 Jan 1 00:00 /usr/local/bin/monitoring.sh

# root の crontab で実行（root 権限が必要なコマンドがあるため）
sudo crontab -l
# */10 * * * * /usr/local/bin/monitoring.sh
```

**SUID ビットの危険性**:
```bash
# 絶対にやってはいけない設定
chmod u+s /usr/local/bin/monitoring.sh  # SUID ビットを設定

# なぜ危険か:
# 1. SUID シェルスクリプトは多くの攻撃ベクトルを持つ
# 2. IFS 変数の操作で任意のコマンドを実行される可能性
# 3. LD_PRELOAD でライブラリを差し替えられる可能性
# 4. レースコンディション（TOCTOU）攻撃の可能性
# 5. Linux カーネルは SUID スクリプトを無視する（セキュリティ対策）
```

**安全な代替手段**:

| 方法 | 説明 | セキュリティ |
|------|------|------------|
| root の crontab | root として実行。cron デーモンが管理 | 最も一般的で安全 |
| sudoers で特定コマンドのみ許可 | 必要なコマンドだけ NOPASSWD で許可 | 最小権限の原則に適合 |
| systemd service | Unit ファイルで権限を細かく制御 | 最もモダンで柔軟 |
| capabilities | 必要な能力のみ付与（root 不要） | 最小権限のベストプラクティス |

```bash
# capabilities の例（root 不要で特定の操作を許可）
sudo setcap cap_net_raw+ep /usr/local/bin/monitoring.sh
# → root でなくても raw ソケットの操作が可能
```

---

## セクション E: シナリオ問題 (Questions 47-60)

### Q47: SSH 接続ができない

**問い**: Born2beRoot の VM に SSH で接続できない。考えられる原因を全て列挙し、各原因の確認方法と修正方法を述べよ。

**回答**:

SSH 接続失敗のトラブルシューティングフローチャート:

```
SSH 接続失敗
  |
  ├── [1] ネットワーク層の問題
  │     ├── VM が起動していない → VirtualBox で確認
  │     ├── Port Forwarding 未設定 → VirtualBox 設定確認
  │     └── ゲスト OS のネットワーク未起動 → ip addr で確認
  |
  ├── [2] SSH デーモンの問題
  │     ├── sshd が起動していない → systemctl status sshd
  │     ├── ポート番号が違う → /etc/ssh/sshd_config 確認
  │     └── 設定エラー → sshd -t で構文チェック
  |
  ├── [3] ファイアウォールの問題
  │     ├── UFW がポートをブロック → sudo ufw status
  │     └── 許可ルールがない → sudo ufw allow 4242
  |
  ├── [4] 認証の問題
  │     ├── パスワードが間違い → 直接コンソールで確認
  │     ├── PermitRootLogin no → 一般ユーザーで接続
  │     └── AllowUsers に含まれていない → sshd_config 確認
  |
  └── [5] クライアント側の問題
        ├── ポート番号の指定忘れ → ssh -p 4242 user@localhost
        ├── known_hosts の不一致 → ~/.ssh/known_hosts を編集
        └── ホスト/ポートの間違い → 接続先を確認
```

**各原因の確認コマンド**:

```bash
# [1] ネットワーク確認（ゲスト OS 内）
ip addr show                    # IP アドレスの確認
ping -c 3 8.8.8.8              # 外部接続テスト

# [2] SSH デーモン確認
sudo systemctl status sshd     # サービス状態
sudo sshd -t                   # 設定ファイルの構文チェック
sudo ss -tlnp | grep 4242     # ポートのリスニング状態

# [3] ファイアウォール確認
sudo ufw status verbose        # UFW ルール一覧

# [4] 認証確認（サーバー側のリアルタイムログ）
sudo journalctl -u sshd -f    # SSH のログを監視

# [5] クライアント側
ssh -v -p 4242 user@localhost  # -v で詳細な接続ログを表示
```

---

### Q48: ディスク容量不足

**問い**: `/var/log` パーティションが 100% になった。原因の特定方法と、即座の対応、恒久的な対策を述べよ。

**回答**:

**即座の対応**:

```bash
# 1. ディスク使用量の確認
df -h /var/log

# 2. 大きなファイルの特定
sudo du -sh /var/log/* | sort -rh | head -10

# 3. 古いログの削除（即座の対応）
sudo journalctl --vacuum-size=50M    # journald のログを 50MB に削減
sudo journalctl --vacuum-time=7d     # 7日より古いログを削除

# 4. 圧縮されていないログの圧縮
sudo gzip /var/log/syslog.1
sudo gzip /var/log/auth.log.1

# 5. 不要なログファイルの削除
sudo truncate -s 0 /var/log/syslog    # ファイルを空にする（削除はしない）
# ※ rm で削除すると、まだファイルハンドルを持つプロセスがディスクを解放しない
```

**原因の特定**:

| 原因 | 確認方法 | 対策 |
|------|---------|------|
| ログローテーション未設定 | `cat /etc/logrotate.d/rsyslog` | logrotate を設定 |
| 大量のエラーログ | `tail -100 /var/log/syslog` で内容確認 | エラーの根本原因を修正 |
| 監査ログの肥大化 | `du -sh /var/log/sudo/` | sudo ログのローテーション設定 |
| 攻撃による大量のログ | `grep "Failed password" /var/log/auth.log | wc -l` | Fail2ban の導入 |

**恒久的な対策**:

```bash
# logrotate の設定確認（/etc/logrotate.d/rsyslog）
/var/log/syslog {
    rotate 7        # 7世代保持
    daily           # 毎日ローテーション
    missingok       # ファイルがなくてもエラーにしない
    notifempty      # 空のファイルはローテーションしない
    compress        # 古いログを gzip 圧縮
    delaycompress   # 1世代前は圧縮しない
    postrotate
        /usr/lib/rsyslog/rsyslog-rotate  # ローテーション後のスクリプト
    endscript
}
```

**LVM での容量拡張**（根本解決）:
```bash
# VG に空き容量がある場合
sudo lvextend -L +2G /dev/LVMGroup/var--log
sudo resize2fs /dev/LVMGroup/var--log
```

---

### Q49: パスワードを忘れた

**問い**: Born2beRoot で一般ユーザーのパスワードを忘れた場合の回復手順を説明せよ。GRUB からのシングルユーザーモードの使い方も述べよ。

**回答**:

**方法 1: sudo が使える別のユーザーからリセット**
```bash
# sudo 権限を持つ別のユーザーでログイン
sudo passwd kaztakam
# 新しいパスワードを2回入力
```

**方法 2: GRUB からシングルユーザーモード**

```
1. VM を再起動
2. GRUB メニューが表示されたら 'e' キーを押す（設定編集）
3. "linux" で始まる行を見つける:
   linux /vmlinuz-5.10.0-28-amd64 root=/dev/mapper/LVMGroup-root ro quiet
4. 行末の "ro quiet" を以下に変更:
   rw init=/bin/bash
5. Ctrl+X または F10 で起動

# root シェルが起動する
root@(none):/#

# パスワードをリセット
passwd kaztakam
# 新しいパスワードを2回入力

# ファイルシステムを再マウント（読み書き可能に）
mount -o remount,rw /

# 再起動
exec /sbin/init
# または
reboot -f
```

**注意: LUKS 暗号化環境での制限**:
Born2beRoot では LUKS 暗号化を使用しているため、GRUB でのシングルユーザーモード起動前にパスフレーズの入力が求められる。LUKS パスフレーズも忘れた場合、データへのアクセスは事実上不可能である。

**GRUB パスワードの設定（セキュリティ対策）**:
```bash
# GRUB にパスワードを設定して、不正な起動パラメータ変更を防止
sudo grub-mkpasswd-pbkdf2
# パスワードを入力 → ハッシュが生成される

# /etc/grub.d/40_custom に追加
set superusers="admin"
password_pbkdf2 admin grub.pbkdf2.sha512.10000.HASH...

# GRUB 設定を更新
sudo update-grub
```

---

### Q50: 新しいサービスの追加

**問い**: Born2beRoot に Lighttpd + MariaDB + PHP (WordPress 用) をセットアップする手順を述べよ。各サービスのセキュリティ設定も含めよ。

**回答**:

**1. Lighttpd のインストールと設定**:
```bash
# インストール
sudo apt install lighttpd

# FastCGI モジュールの有効化
sudo lighty-enable-mod fastcgi
sudo lighty-enable-mod fastcgi-php

# 設定確認
cat /etc/lighttpd/lighttpd.conf

# UFW でポート 80 を開放
sudo ufw allow 80

# 起動と有効化
sudo systemctl enable --now lighttpd

# 動作確認
curl http://localhost
```

**2. MariaDB のインストールと設定**:
```bash
# インストール
sudo apt install mariadb-server

# セキュリティ初期設定
sudo mysql_secure_installation
# - root パスワード設定: Y
# - 匿名ユーザー削除: Y
# - リモート root ログイン禁止: Y
# - テストデータベース削除: Y
# - 権限テーブルリロード: Y

# WordPress 用データベース作成
sudo mysql -u root -p
CREATE DATABASE wordpress;
CREATE USER 'wpuser'@'localhost' IDENTIFIED BY 'StrongP@ssw0rd';
GRANT ALL PRIVILEGES ON wordpress.* TO 'wpuser'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

**3. PHP のインストール**:
```bash
sudo apt install php-cgi php-mysql php-fpm
```

**4. WordPress のインストール**:
```bash
cd /var/www/html
sudo wget https://wordpress.org/latest.tar.gz
sudo tar -xzf latest.tar.gz
sudo chown -R www-data:www-data /var/www/html/wordpress
```

**セキュリティチェックリスト**:

| サービス | 設定 | 理由 |
|---------|------|------|
| Lighttpd | `server.port = 80` | ボーナスで必要なポートのみ開放 |
| MariaDB | `bind-address = 127.0.0.1` | リモート接続を禁止（ローカルのみ） |
| PHP | `expose_php = Off` | PHP バージョン情報の非表示 |
| WordPress | ファイル権限 755/644 | 書き込み権限を最小化 |
| UFW | ポート 80 のみ追加 | 必要最小限のポート開放 |

---

### Q51: AppArmor プロファイル違反

**問い**: あるプログラムが AppArmor のプロファイル違反でブロックされた。ログの確認方法、原因の特定、プロファイルの修正手順を述べよ。

**回答**:

```bash
# 1. 違反ログの確認
sudo journalctl | grep "apparmor.*DENIED"
# または
sudo dmesg | grep "apparmor.*DENIED"
# または
sudo cat /var/log/kern.log | grep "apparmor.*DENIED"

# ログの例:
# audit: type=1400 audit(...): apparmor="DENIED" operation="open"
#   profile="/usr/sbin/sshd" name="/etc/custom/config" pid=1234
#   comm="sshd" requested_mask="r" denied_mask="r"

# 2. ログの読み方:
# apparmor="DENIED"    → ブロックされた
# operation="open"     → ファイルのオープン操作
# profile="/usr/sbin/sshd" → sshd のプロファイルで発生
# name="/etc/custom/config" → アクセスしようとしたファイル
# requested_mask="r"   → 読み取りを要求
# denied_mask="r"      → 読み取りが拒否された

# 3. プロファイルを complain モードに変更して動作確認
sudo aa-complain /etc/apparmor.d/usr.sbin.sshd

# 4. プログラムを実行して必要なアクセスをログに記録
sudo systemctl restart sshd

# 5. ログからプロファイルを更新
sudo aa-logprof
# → 検出されたアクセスを確認し、許可/拒否を選択

# 6. enforce モードに戻す
sudo aa-enforce /etc/apparmor.d/usr.sbin.sshd
```

---

### Q52: LVM スナップショットの活用

**問い**: システムアップデート前に LVM スナップショットを作成し、問題が発生した場合にロールバックする手順を述べよ。

**回答**:

```bash
# 1. アップデート前にスナップショットを作成
sudo lvcreate --size 2G --snapshot --name root-backup /dev/LVMGroup/root

# --size 2G: スナップショット用の領域（変更量を見積もって決定）
# --snapshot: スナップショット LV であることを指定
# --name: スナップショットの名前

# 2. スナップショットの確認
sudo lvs
#  LV          VG        Attr       LSize  Origin
#  root        LVMGroup  owi-a-s--- 10.00g
#  root-backup LVMGroup  swi-a-s---  2.00g root

# 3. システムアップデートを実行
sudo apt update && sudo apt upgrade

# 4a. 問題なし → スナップショットを削除
sudo lvremove /dev/LVMGroup/root-backup

# 4b. 問題あり → スナップショットからロールバック
# ⚠️ ロールバックにはアンマウントが必要
sudo umount /
sudo lvconvert --merge /dev/LVMGroup/root-backup
# → 再起動後にマージが実行される
sudo reboot
```

**スナップショットの仕組み (Copy-on-Write)**:
```
スナップショット作成時:
  元の LV: [A][B][C][D][E]
  スナップ: (変更なし → メタデータのみ)

ブロック B が変更された時:
  元の LV: [A][B'][C][D][E]  ← B が B' に変更
  スナップ: [B]               ← 変更前の B をコピー (CoW)

読み取り時:
  元の LV からは A, B', C, D, E が読める
  スナップからは A, B, C, D, E が読める（B はスナップ領域から）
```

---

### Q53: UFW ルールの追加と削除

**問い**: 以下のシナリオに対する UFW の設定手順を述べよ。(a) 特定 IP からのみ SSH を許可、(b) HTTP/HTTPS を追加、(c) 誤って追加したルールの削除。

**回答**:

**(a) 特定 IP からのみ SSH を許可**:
```bash
# 既存の SSH 許可ルールを削除
sudo ufw delete allow 4242

# 特定の IP からのみ許可
sudo ufw allow from 192.168.1.100 to any port 4242

# サブネットで指定する場合
sudo ufw allow from 192.168.1.0/24 to any port 4242

# 確認
sudo ufw status numbered
```

**(b) HTTP/HTTPS の追加**:
```bash
# ポート番号で指定
sudo ufw allow 80
sudo ufw allow 443

# サービス名で指定
sudo ufw allow http
sudo ufw allow https

# プロファイル（アプリケーション）で指定
sudo ufw allow 'WWW Full'

# 利用可能なプロファイル一覧
sudo ufw app list
```

**(c) ルールの削除**:
```bash
# 方法 1: 番号で削除
sudo ufw status numbered
# [1] 4242     ALLOW IN    Anywhere
# [2] 80       ALLOW IN    Anywhere

sudo ufw delete 2   # 番号 2 のルールを削除

# 方法 2: ルールを指定して削除
sudo ufw delete allow 80

# 方法 3: 全ルールをリセット
sudo ufw reset   # ⚠️ 全ルールが削除される
```

---

### Q54: パスワードポリシーの変更

**問い**: 評価中に「パスワードポリシーを変更して、新しいユーザーを作成し、ポリシーが適用されることを確認せよ」と言われた。手順を述べよ。

**回答**:

```bash
# 1. 現在のポリシーを確認
grep -E "^PASS" /etc/login.defs
sudo cat /etc/pam.d/common-password | grep pam_pwquality

# 2. ポリシーの確認後、新しいユーザーを作成
sudo adduser testuser

# 3. パスワード入力時にポリシーが適用されることを確認
# 例: "abc" を入力 → エラー「パスワードが短すぎます」
# 例: "abcdefghij" → エラー「大文字が含まれていません」
# 例: "Abcdefghi1" → 成功（minlen=10, ucredit=-1, dcredit=-1 を満たす）

# 4. パスワード有効期限の確認
sudo chage -l testuser
# Last password change           : Feb 09, 2026
# Password expires               : Mar 11, 2026 (30日後)
# Minimum number of days between password change : 2
# Maximum number of days between password change : 30
# Number of days of warning before password expires : 7

# 5. 既存ユーザーにもポリシーを適用（login.defs は新規ユーザーのみ）
sudo chage -M 30 -m 2 -W 7 existinguser

# 6. グループへの追加
sudo usermod -aG sudo testuser
sudo usermod -aG user42 testuser

# 7. 確認
id testuser
groups testuser
```

---

### Q55: サービスの起動トラブル

**問い**: `systemctl start sshd` が失敗した。原因の調査と修復の手順を述べよ。

**回答**:

```bash
# 1. エラー状態の確認
sudo systemctl status sshd
# ● ssh.service - OpenBSD Secure Shell server
#    Loaded: loaded (/lib/systemd/system/ssh.service; enabled)
#    Active: failed (Result: exit-code)
#    Process: 1234 ExecStartPre=/usr/sbin/sshd -t (code=exited, status=255/EXCEPTION)

# 2. 詳細なログの確認
sudo journalctl -u sshd -n 50 --no-pager

# 3. 設定ファイルの構文チェック
sudo sshd -t
# /etc/ssh/sshd_config: line 42: Bad configuration option: PermitRootLoginn
# → タイプミスを発見

# 4. 設定ファイルの修正
sudo vim /etc/ssh/sshd_config
# PermitRootLoginn → PermitRootLogin に修正

# 5. 再度構文チェック
sudo sshd -t
# （エラーなし）

# 6. サービスの再起動
sudo systemctl start sshd

# 7. 状態確認
sudo systemctl status sshd
# Active: active (running)

# 8. ポートのリスニング確認
sudo ss -tlnp | grep 4242
# LISTEN  0  128  0.0.0.0:4242  0.0.0.0:*  users:(("sshd",pid=1234,fd=3))
```

**よくある原因と対策**:

| 原因 | エラーメッセージ | 対策 |
|------|----------------|------|
| 設定ファイルの構文エラー | `Bad configuration option` | `sshd -t` で修正 |
| ポートが既に使用中 | `Address already in use` | `ss -tlnp` で確認し、競合プロセスを停止 |
| 鍵ファイルの権限不正 | `bad permissions` | `chmod 600 /etc/ssh/ssh_host_*_key` |
| 鍵ファイルがない | `Could not load host key` | `ssh-keygen -A` で再生成 |

---

### Q56: ファイアウォールのデバッグ

**問い**: UFW を有効にしたら外部との通信が全てブロックされた。DNS 解決もできない。原因と対策を述べよ。

**回答**:

```bash
# 1. 現在のルールを確認
sudo ufw status verbose
# Default: deny (incoming), deny (outgoing), disabled (routed)
#                           ^^^^
# → 問題: outgoing も deny になっている！

# 2. 原因: outgoing のデフォルトポリシーが deny
# UFW はデフォルトで incoming=deny, outgoing=allow だが
# 誤って outgoing=deny に設定した場合、全送信がブロックされる

# 3. DNS が解決できないことの確認
nslookup google.com
# ;; connection timed out; no servers could be reached

# 4. 修正: outgoing を allow に変更
sudo ufw default allow outgoing

# 5. 特定のポートのみ outgoing を許可する場合（厳密な設定）
sudo ufw default deny outgoing
sudo ufw allow out 53     # DNS
sudo ufw allow out 80     # HTTP
sudo ufw allow out 443    # HTTPS
sudo ufw allow out 123    # NTP (時刻同期)

# 6. 修正後の確認
sudo ufw status verbose
ping -c 3 8.8.8.8
nslookup google.com
sudo apt update
```

---

### Q57: LUKS パスフレーズの変更

**問い**: LUKS の暗号化パスフレーズを変更する手順を述べよ。安全に変更するための注意点も述べよ。

**回答**:

```bash
# 1. 現在の Key Slot の状態を確認
sudo cryptsetup luksDump /dev/sda5
# Key Slot 0: ENABLED
# Key Slot 1: DISABLED
# ...

# 2. 新しいパスフレーズを追加（既存のパスフレーズ入力が必要）
sudo cryptsetup luksAddKey /dev/sda5
# Enter any existing passphrase: (現在のパスフレーズ)
# Enter new passphrase for key slot: (新しいパスフレーズ)
# Verify passphrase: (新しいパスフレーズ)

# 3. 新しいパスフレーズで復号できることを確認
sudo cryptsetup luksOpen --test-passphrase /dev/sda5
# Enter passphrase for /dev/sda5: (新しいパスフレーズ)
# → エラーなし = 成功

# 4. 古いパスフレーズを削除
sudo cryptsetup luksRemoveKey /dev/sda5
# Enter passphrase to be deleted: (古いパスフレーズ)

# 5. 最終確認
sudo cryptsetup luksDump /dev/sda5
# Key Slot 0: ENABLED (新しいパスフレーズ)
# Key Slot 1: DISABLED
```

**注意点**:

| 注意点 | 理由 |
|--------|------|
| 必ず新しい Key を追加してから古い Key を削除 | 順序を間違えると全 Key Slot が空になりデータ全損 |
| LUKS ヘッダーのバックアップを取る | ヘッダー破損時の復旧用 |
| 新しいパスフレーズでの復号テスト | 追加した Key が正常に動作することを確認 |
| 再起動してパスフレーズを確認 | ブート時に新しいパスフレーズで復号できることを確認 |

---

### Q58: ユーザーのグループ管理

**問い**: 評価者から「新しいグループ evaluating を作成し、ユーザーを追加して、そのグループでしかアクセスできないディレクトリを作成せよ」と言われた。手順を述べよ。

**回答**:

```bash
# 1. グループの作成
sudo groupadd evaluating

# 2. ユーザーをグループに追加
sudo usermod -aG evaluating kaztakam
sudo usermod -aG evaluating testuser

# 3. 確認
getent group evaluating
# evaluating:x:1003:kaztakam,testuser

# 4. グループ専用ディレクトリの作成
sudo mkdir /srv/evaluating
sudo chown root:evaluating /srv/evaluating
sudo chmod 2770 /srv/evaluating
# 2: SGID ビット → 新規ファイルが evaluating グループを継承
# 7: 所有者 (root) に rwx
# 7: グループ (evaluating) に rwx
# 0: その他に権限なし

# 5. 動作確認
# evaluating グループのメンバーとして
touch /srv/evaluating/testfile   # → 成功
ls -la /srv/evaluating/testfile
# -rw-r--r-- 1 kaztakam evaluating ... testfile
# → グループが evaluating になっている（SGID の効果）

# evaluating グループ以外のユーザーとして
su - otheruser
ls /srv/evaluating/               # → Permission denied
touch /srv/evaluating/testfile2   # → Permission denied
```

---

### Q59: カーネルパニック

**問い**: Born2beRoot の VM がカーネルパニックで起動しなくなった。考えられる原因と回復手順を述べよ。

**回答**:

**考えられる原因**:

| 原因 | 症状 | 対策 |
|------|------|------|
| カーネルアップデートの失敗 | 新しいカーネルでパニック | GRUB で前のカーネルを選択 |
| initramfs の破損 | "Unable to mount root fs" | GRUB から initramfs を再生成 |
| fstab の誤設定 | "mount: wrong fs type" | レスキューモードで fstab 修正 |
| ディスクの破損 | "I/O error" | fsck でファイルシステム修復 |
| メモリ不足 | "Out of memory" | swap の追加、メモリ増設 |

**回復手順**:

```
1. GRUB メニューで前のカーネルバージョンを選択
   - "Advanced options for Debian" を選択
   - 前のカーネルバージョンを選択して起動

2. 起動に成功したら:
   # 壊れたカーネルを削除
   sudo apt remove linux-image-<壊れたバージョン>

   # initramfs を再生成
   sudo update-initramfs -u

   # GRUB を更新
   sudo update-grub

3. GRUB で起動できない場合:
   - VirtualBox のスナップショットから復元
   - Debian インストール DVD でレスキューモードを使用
```

---

### Q60: 監査とコンプライアンス

**問い**: Born2beRoot の設定が CIS Benchmark に準拠しているかを確認する方法を述べよ。確認すべき主要な項目を 10 個以上挙げよ。

**回答**:

**CIS (Center for Internet Security) Benchmark** はサーバーのセキュリティ設定の推奨基準である。

**Born2beRoot の CIS 準拠チェックリスト**:

| # | チェック項目 | 確認コマンド | 期待値 |
|---|-------------|-------------|--------|
| 1 | パーティション分離 | `df -h` | /tmp, /var, /var/log が独立 |
| 2 | /tmp の noexec マウント | `mount | grep /tmp` | noexec,nosuid,nodev |
| 3 | パスワード最大有効日数 | `grep PASS_MAX_DAYS /etc/login.defs` | 30 以下 |
| 4 | パスワード最小文字数 | `grep minlen /etc/pam.d/common-password` | 10 以上 |
| 5 | SSH root ログイン禁止 | `grep PermitRootLogin /etc/ssh/sshd_config` | no |
| 6 | SSH ポート変更 | `grep Port /etc/ssh/sshd_config` | 22 以外 |
| 7 | ファイアウォール有効 | `sudo ufw status` | Status: active |
| 8 | AppArmor 有効 | `sudo aa-status` | loaded, enforce |
| 9 | SUID ファイルの監査 | `find / -perm -4000 -type f 2>/dev/null` | 最小限 |
| 10 | 空パスワードの禁止 | `sudo awk -F: '$2==""' /etc/shadow` | 出力なし |
| 11 | sudo のログ記録 | `grep logfile /etc/sudoers.d/*` | log_input, log_output |
| 12 | 不要サービスの無効化 | `systemctl list-units --type=service` | 必要なもののみ |
| 13 | NTP の設定 | `timedatectl status` | NTP synchronized: yes |
| 14 | core dump の無効化 | `ulimit -c` | 0 |

---

## セクション F: ネットワーク問題 (Questions 61-70)

### Q61: ネットワーク診断コマンド

**問い**: 以下のネットワーク診断コマンドの用途と出力の読み方を説明せよ: `ip addr`, `ss`, `ping`, `traceroute`, `dig`, `netstat`。

**回答**:

| コマンド | 用途 | 代替 |
|---------|------|------|
| `ip addr` | ネットワークインターフェースと IP アドレスの表示 | `ifconfig`（非推奨） |
| `ss` | ソケット統計（接続状態、リスニングポート） | `netstat`（非推奨） |
| `ping` | ICMP Echo による疎通確認 | - |
| `traceroute` | パケットの経路を表示 | `tracepath` |
| `dig` | DNS クエリ | `nslookup`, `host` |

```bash
# ip addr の出力の読み方
ip addr show
# 2: enp0s3: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500
#     link/ether 08:00:27:xx:xx:xx  ← MAC アドレス
#     inet 10.0.2.15/24 brd 10.0.2.255  ← IPv4 アドレス / サブネットマスク
#     inet6 fe80::xxxx/64  ← IPv6 リンクローカルアドレス

# ss の主要なオプション
ss -tlnp    # TCP (-t) リスニング (-l) ポートを数値 (-n) で表示、プロセス名付き (-p)
ss -tunap   # TCP+UDP の全接続をプロセス名付きで表示
# State    Recv-Q  Send-Q  Local Address:Port  Peer Address:Port  Process
# LISTEN   0       128     0.0.0.0:4242        0.0.0.0:*          sshd

# ping の出力
ping -c 3 8.8.8.8
# PING 8.8.8.8: 64 bytes from 8.8.8.8: icmp_seq=1 ttl=117 time=3.45 ms
#                                        ^^^^^^^^       ^^^       ^^^^
#                                        応答元         TTL値     応答時間

# traceroute の出力
traceroute 8.8.8.8
# 1  10.0.2.2  0.5ms   ← VirtualBox NAT ゲートウェイ
# 2  192.168.1.1  1.2ms ← ホスト OS のルーター
# 3  ...
# → パケットが通過する各ルーターを表示

# dig の出力
dig google.com
# ;; ANSWER SECTION:
# google.com.  300  IN  A  142.250.190.14
#              ^^^      ^  ^^^^^^^^^^^^^^^^
#              TTL     タイプ  IP アドレス
```

---

### Q62: サブネットマスクと CIDR

**問い**: サブネットマスクと CIDR 記法を説明せよ。10.0.2.15/24 の場合、ネットワークアドレス、ブロードキャストアドレス、利用可能なホスト数を求めよ。

**回答**:

**CIDR (Classless Inter-Domain Routing)** はサブネットをプレフィックス長で表記する方式。

```
10.0.2.15/24 の計算:

IP アドレス（2進数）:    00001010.00000000.00000010.00001111
サブネットマスク (/24):   11111111.11111111.11111111.00000000
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^  ^^^^^^^^
                          ネットワーク部 (24ビット)     ホスト部 (8ビット)

ネットワークアドレス:     10.0.2.0   (ホスト部が全て 0)
ブロードキャストアドレス: 10.0.2.255 (ホスト部が全て 1)
利用可能ホスト数:         2^8 - 2 = 254 (10.0.2.1 〜 10.0.2.254)
                          ※ ネットワークアドレスとブロードキャストを除く
```

**よく使うサブネット**:

| CIDR | サブネットマスク | ホスト数 | 用途 |
|------|----------------|---------|------|
| /32 | 255.255.255.255 | 1 | ホスト指定 |
| /24 | 255.255.255.0 | 254 | 小規模ネットワーク |
| /16 | 255.255.0.0 | 65,534 | 中規模ネットワーク |
| /8 | 255.0.0.0 | 16,777,214 | 大規模ネットワーク |

**プライベート IP アドレス範囲**:

| クラス | 範囲 | CIDR | 用途 |
|--------|------|------|------|
| A | 10.0.0.0 - 10.255.255.255 | 10.0.0.0/8 | 大企業、クラウド（VirtualBox NAT） |
| B | 172.16.0.0 - 172.31.255.255 | 172.16.0.0/12 | 中規模ネットワーク |
| C | 192.168.0.0 - 192.168.255.255 | 192.168.0.0/16 | 家庭、小規模オフィス |

---

### Q63: NAT の仕組み

**問い**: NAT (Network Address Translation) の仕組みを説明せよ。SNAT, DNAT, NAPT (PAT) の違いと、VirtualBox の NAT モードとの関係も述べよ。

**回答**:

NAT はプライベート IP アドレスとパブリック IP アドレスの間の変換を行う。

```
NAPT (Network Address Port Translation) の動作:

内部ネットワーク                NAT ルーター                 外部
[10.0.2.15:12345] ─────> [変換テーブル] ─────> [203.0.113.1:50001]
[10.0.2.16:12345] ─────> [変換テーブル] ─────> [203.0.113.1:50002]

変換テーブル:
| 内部 IP:ポート    | 外部 IP:ポート      |
|10.0.2.15:12345   | 203.0.113.1:50001  |
|10.0.2.16:12345   | 203.0.113.1:50002  |
```

| 種類 | 変換対象 | 説明 | 用途 |
|------|---------|------|------|
| **SNAT** | 送信元 IP | 内部 → 外部通信で送信元 IP を変換 | 内部ネットワークからの外部アクセス |
| **DNAT** | 宛先 IP | 外部 → 内部通信で宛先 IP を変換 | Port Forwarding（外部から内部サービスへ） |
| **NAPT/PAT** | IP + ポート | SNAT + ポート番号の変換。1つの外部 IP を共有 | 一般的な家庭/オフィスの NAT |

**VirtualBox の NAT モード**:
```
VirtualBox は NAPT を使用:
  ゲスト OS (10.0.2.15) → VirtualBox NAT Engine → ホスト OS のIP

Port Forwarding は DNAT:
  ホスト (127.0.0.1:4242) → VirtualBox DNAT → ゲスト (10.0.2.15:4242)
```

---

### Q64: DNS の仕組み

**問い**: DNS の名前解決プロセスを、Born2beRoot の VM が `apt update` を実行する場合を例に説明せよ。

**回答**:

```
apt update が deb.debian.org にアクセスする場合:

[Born2beRoot VM]
    |
    | 1. /etc/hosts を確認 → 見つからない
    | 2. /etc/resolv.conf で DNS サーバーを確認
    |    nameserver 10.0.2.3 (VirtualBox の DNS プロキシ)
    |
    v
[VirtualBox DNS プロキシ (10.0.2.3)]
    |
    | ホスト OS の DNS 設定を使用
    |
    v
[ホスト OS の DNS リゾルバ]
    |
    | キャッシュに deb.debian.org がなければ
    |
    v
[再帰的 DNS 解決]
    |
    ├── 1. ルート DNS サーバー (.)
    │      → "org の NS は ns.org に聞いて"
    |
    ├── 2. org の DNS サーバー
    │      → "debian.org の NS は ns.debian.org に聞いて"
    |
    └── 3. debian.org の DNS サーバー
           → "deb.debian.org は 151.101.x.x"
```

```bash
# DNS 設定の確認
cat /etc/resolv.conf
# nameserver 10.0.2.3

# 名前解決の順序 (/etc/nsswitch.conf)
grep hosts /etc/nsswitch.conf
# hosts: files dns
# → まず /etc/hosts (files)、次に DNS を使用

# DNS クエリのテスト
dig deb.debian.org
nslookup deb.debian.org
host deb.debian.org
```

---

### Q65: SSH ポートフォワーディング

**問い**: SSH のポートフォワーディング（ローカル、リモート、ダイナミック）の仕組みと用途を説明せよ。

**回答**:

| 種類 | コマンド | 用途 |
|------|---------|------|
| ローカル | `ssh -L` | リモートサービスにローカルからアクセス |
| リモート | `ssh -R` | ローカルサービスをリモートからアクセス |
| ダイナミック | `ssh -D` | SOCKS プロキシとして使用 |

**ローカルフォワーディング** (`ssh -L`):
```
ローカル PC                SSH トンネル              サーバー
[localhost:8080] ────> [暗号化トンネル] ────> [server:3306 (MySQL)]

ssh -L 8080:localhost:3306 user@server -p 4242
# ローカルの 8080 にアクセスすると、サーバーの MySQL (3306) に転送
# mysql -h 127.0.0.1 -P 8080 で接続可能
```

**リモートフォワーディング** (`ssh -R`):
```
ローカル PC                SSH トンネル              サーバー
[localhost:3000] <──── [暗号化トンネル] <──── [server:8080]

ssh -R 8080:localhost:3000 user@server -p 4242
# サーバーの 8080 にアクセスすると、ローカルの 3000 に転送
# NAT 内のローカルサービスを外部に公開する場合に使用
```

**ダイナミックフォワーディング** (`ssh -D`):
```
ssh -D 1080 user@server -p 4242
# ローカルに SOCKS5 プロキシ (1080) を作成
# ブラウザのプロキシ設定で使用すると、全通信が SSH トンネル経由
```

---

### Q66: iptables のチェーンとテーブル

**問い**: iptables の 5 つのチェーン（INPUT, OUTPUT, FORWARD, PREROUTING, POSTROUTING）と 4 つのテーブル（filter, nat, mangle, raw）の関係を説明せよ。

**回答**:

```
パケットの流れと各テーブル/チェーンの処理順序:

[受信パケット]
      |
  raw:PREROUTING → mangle:PREROUTING → nat:PREROUTING (DNAT)
      |
  ルーティング判断
      |
      ├── 自分宛 ──────────────────────────────────────┐
      │     |                                           |
      │   mangle:INPUT → filter:INPUT ──> [ローカルプロセス]
      │                                        |
      │                                   [ローカルプロセスが送信]
      │                                        |
      │   raw:OUTPUT → mangle:OUTPUT → nat:OUTPUT → filter:OUTPUT
      │                                        |
      │                               mangle:POSTROUTING → nat:POSTROUTING (SNAT)
      │                                        |
      └── 他ホスト宛 ──> mangle:FORWARD → filter:FORWARD ──┘
                                                           |
                                                    [送信パケット]
```

**4 つのテーブル**:

| テーブル | 用途 | 主なターゲット |
|---------|------|-------------|
| **filter** | パケットフィルタリング（許可/拒否） | ACCEPT, DROP, REJECT |
| **nat** | アドレス変換 (NAT) | SNAT, DNAT, MASQUERADE |
| **mangle** | パケットヘッダの変更 | TOS, TTL, MARK |
| **raw** | 接続追跡の免除 | NOTRACK |

**Born2beRoot での UFW → iptables の対応**:
```bash
# UFW のルール
sudo ufw allow 4242

# 実際に生成される iptables ルール
sudo iptables -L -n | grep 4242
# Chain ufw-user-input (1 references)
# ACCEPT  tcp  --  0.0.0.0/0  0.0.0.0/0  tcp dpt:4242
```

---

### Q67: ARP と MAC アドレス

**問い**: ARP (Address Resolution Protocol) の仕組みを説明せよ。ARP スプーフィング攻撃とその対策も述べよ。

**回答**:

ARP は IP アドレスから MAC アドレスを解決するプロトコルである。同一ネットワーク内の通信に必要。

```
ARP の動作:

[Host A: 10.0.2.15]                    [Host B: 10.0.2.1]
     |                                        |
     | 1. ARP Request (ブロードキャスト)         |
     | "10.0.2.1 の MAC アドレスは？"             |
     | FF:FF:FF:FF:FF:FF (全員に送信) --------->|
     |                                        |
     | 2. ARP Reply (ユニキャスト)               |
     |<---- "10.0.2.1 は 08:00:27:xx:xx:xx"   |
     |                                        |
     | 3. ARP キャッシュに保存                    |
     | 10.0.2.1 → 08:00:27:xx:xx:xx           |
```

```bash
# ARP キャッシュの確認
ip neigh show
# 10.0.2.2 dev enp0s3 lladdr 52:54:00:12:35:02 REACHABLE

# ARP テーブルの表示（旧コマンド）
arp -n
```

**ARP スプーフィング攻撃**:
攻撃者が偽の ARP Reply を送信し、ターゲットの ARP キャッシュを汚染する。これにより中間者攻撃 (MITM) が可能になる。

```
正常時:
  Host A → Router (10.0.2.2 = MAC_Router) → Internet

攻撃時:
  攻撃者が「10.0.2.2 は MAC_Attacker」と偽の ARP Reply を送信
  Host A → Attacker (10.0.2.2 = MAC_Attacker) → Router → Internet
  → 攻撃者が通信を盗聴・改ざん可能
```

**対策**:
- 静的 ARP エントリの設定
- arpwatch で ARP テーブルの変化を監視
- 802.1X 認証でネットワークアクセスを制限
- SSH/TLS で通信を暗号化（MITM でも内容は読めない）

---

### Q68: TCP と UDP の違い

**問い**: TCP と UDP の違いを、SSH (TCP) と DNS (UDP) の具体例で説明せよ。

**回答**:

| 項目 | TCP | UDP |
|------|-----|-----|
| 接続 | コネクション型（3-way handshake） | コネクションレス |
| 信頼性 | 保証あり（再送、順序保証） | 保証なし |
| フロー制御 | あり（ウィンドウサイズ） | なし |
| ヘッダサイズ | 20バイト | 8バイト |
| 用途 | SSH, HTTP, FTP, SMTP | DNS, NTP, DHCP, VoIP |

**SSH (TCP) の場合**:
```
SSH はコマンドの入出力を正確に送受信する必要がある
→ パケットの欠落や順序の入れ替わりは許されない
→ TCP の信頼性保証が必須

1. 3-way handshake で接続確立
2. データをセグメントに分割して送信
3. 受信確認 (ACK) を待つ
4. ACK が来なければ再送
5. シーケンス番号で順序を保証
```

**DNS (UDP) の場合**:
```
DNS クエリは小さなパケット1つで完結する
→ 接続のオーバーヘッドは無駄
→ UDP の軽量さが適している

1. クエリを1パケットで送信
2. レスポンスを1パケットで受信
3. 応答がなければクライアントが再送
→ 接続確立なし、高速

※ DNS over TCP: 512バイトを超える応答や、ゾーン転送は TCP を使用
```

---

### Q69: VPN と SSH トンネルの違い

**問い**: VPN (Virtual Private Network) と SSH トンネルの違いを説明せよ。Born2beRoot の環境で VPN を構築する場合の考慮事項も述べよ。

**回答**:

| 項目 | VPN | SSH トンネル |
|------|-----|------------|
| レイヤー | L2/L3（ネットワーク全体） | L4-L7（アプリケーション単位） |
| 対象 | 全トラフィック | 特定のポート/接続 |
| プロトコル | OpenVPN, WireGuard, IPsec | SSH |
| 設定の複雑さ | 高い | 低い |
| パフォーマンス | 高い（カーネルレベル） | 低い（ユーザースペース） |
| 用途 | 拠点間接続、リモートワーク | 単一サービスへの安全なアクセス |

Born2beRoot では SSH トンネルで十分な場合がほとんどである。VPN はネットワーク全体を暗号化する必要がある場合（複数のサービスに同時アクセスする場合等）に使用する。

---

### Q70: ネットワークのトラブルシューティング手順

**問い**: Born2beRoot の VM で `apt update` が失敗する。体系的なトラブルシューティング手順を述べよ。

**回答**:

```
レイヤー別診断（下位層から確認）:

[L1: 物理層] ─── VM のネットワークアダプタは有効か？
  $ ip link show enp0s3
  → state UP であること

[L2: データリンク層] ─── MAC アドレスは正常か？
  $ ip link show enp0s3 | grep ether

[L3: ネットワーク層] ─── IP アドレスは割り当てられているか？
  $ ip addr show enp0s3
  → inet 10.0.2.15/24 があること
  $ ping -c 3 10.0.2.2  (ゲートウェイ)
  → 応答があること
  $ ping -c 3 8.8.8.8  (外部 IP)
  → 応答があること

[L4: トランスポート層] ─── ポートは開いているか？
  $ ss -tlnp  (ローカルのリスニングポート)

[L7: アプリケーション層] ─── DNS は動作しているか？
  $ dig deb.debian.org
  → IP アドレスが返ること
  $ curl -I http://deb.debian.org/debian/
  → HTTP 200 が返ること
```

```bash
# よくある原因と対策:
# 1. DHCP が取得できていない
sudo dhclient enp0s3

# 2. DNS が設定されていない
echo "nameserver 8.8.8.8" | sudo tee /etc/resolv.conf

# 3. UFW が送信をブロック
sudo ufw default allow outgoing

# 4. sources.list の URL が間違い
sudo cat /etc/apt/sources.list

# 5. プロキシ設定が必要（学校環境）
export http_proxy="http://proxy:8080"
export https_proxy="http://proxy:8080"
```

---

## セクション G: LVM 操作問題 (Questions 71-80)

### Q71: LVM 情報の確認

**問い**: 以下のコマンドの出力の違いを説明せよ: `pvs`, `vgs`, `lvs`, `pvdisplay`, `vgdisplay`, `lvdisplay`。

**回答**:

| コマンド | 出力内容 | 詳細度 |
|---------|---------|--------|
| `pvs` | PV の概要（名前、VG、サイズ） | 低（一覧表示） |
| `pvdisplay` | PV の詳細（UUID、PE サイズ等） | 高 |
| `vgs` | VG の概要（名前、PV数、LV数、サイズ） | 低 |
| `vgdisplay` | VG の詳細（UUID、PE サイズ等） | 高 |
| `lvs` | LV の概要（名前、VG、サイズ、属性） | 低 |
| `lvdisplay` | LV の詳細（UUID、セグメント等） | 高 |

```bash
# 概要表示（スクリプト向き）
sudo pvs
# PV         VG        Fmt  Attr PSize   PFree
# /dev/mapper/sda5_crypt LVMGroup lvm2 a-- 30.80g  0

sudo vgs
# VG        #PV #LV #SN Attr   VSize  VFree
# LVMGroup    1   7   0 wz--n- 30.80g    0

sudo lvs
# LV      VG        Attr       LSize
# home    LVMGroup  -wi-ao----  5.00g
# root    LVMGroup  -wi-ao---- 10.00g
# ...

# 詳細表示（デバッグ向き）
sudo pvdisplay /dev/mapper/sda5_crypt
# --- Physical volume ---
# PV Name               /dev/mapper/sda5_crypt
# VG Name               LVMGroup
# PV Size               30.80 GiB
# PE Size               4.00 MiB
# Total PE              7885
# Allocated PE          7885
# Free PE               0
```

---

### Q72: LV の作成と削除

**問い**: VG に空き容量がある場合に、新しい LV を作成してマウントする手順を述べよ。

**回答**:

```bash
# 1. VG の空き容量を確認
sudo vgs
# VFree が 0 でないことを確認

# 2. 新しい LV を作成
sudo lvcreate -L 1G -n data LVMGroup

# 3. ファイルシステムを作成
sudo mkfs.ext4 /dev/LVMGroup/data

# 4. マウントポイントを作成
sudo mkdir /mnt/data

# 5. マウント
sudo mount /dev/LVMGroup/data /mnt/data

# 6. 永続化（/etc/fstab に追加）
echo "/dev/LVMGroup/data /mnt/data ext4 defaults 0 2" | sudo tee -a /etc/fstab

# 7. fstab の確認
sudo mount -a  # エラーがないことを確認

# LV の削除:
sudo umount /mnt/data
sudo lvremove /dev/LVMGroup/data
# /etc/fstab からエントリを削除
```

---

### Q73: VG の拡張

**問い**: 新しい物理ディスクを VG に追加して、既存の LV を拡張する手順を述べよ。

**回答**:

```bash
# 1. 新しいディスクの確認
sudo fdisk -l /dev/sdb

# 2. ディスクを PV として初期化
sudo pvcreate /dev/sdb1

# 3. 既存の VG に PV を追加
sudo vgextend LVMGroup /dev/sdb1

# 4. VG の空き容量を確認
sudo vgs
# VFree が増加していることを確認

# 5. LV を拡張
sudo lvextend -L +5G /dev/LVMGroup/home

# 6. ファイルシステムを拡張
sudo resize2fs /dev/LVMGroup/home

# 7. 確認
df -h /home
```

**PV の移動（ディスク交換時）**:
```bash
# 古いディスクから新しいディスクにデータを移動
sudo pvmove /dev/sda2 /dev/sdb1

# 古い PV を VG から削除
sudo vgreduce LVMGroup /dev/sda2

# PV を解放
sudo pvremove /dev/sda2
```

---

### Q74: PE (Physical Extent) の理解

**問い**: PE (Physical Extent) と LE (Logical Extent) の関係を説明せよ。PE サイズが LVM のパフォーマンスに与える影響も述べよ。

**回答**:

PE は LVM がストレージを管理する最小単位である。デフォルトは 4MB。

```
VG は PV を PE に分割して管理:

PV (/dev/sda2):
[PE0][PE1][PE2][PE3][PE4][PE5][PE6][PE7]...

LV (root) は PE を集めて構成:
  root: [PE0][PE1][PE2]...[PE2559]  = 2560 × 4MB = 10GB
  home: [PE2560][PE2561]...[PE3839] = 1280 × 4MB = 5GB

PE と LE の対応:
  LV の LE 0 → PV の PE 2560
  LV の LE 1 → PV の PE 2561
  （PE は物理的な位置、LE は論理的な位置）
```

| PE サイズ | メリット | デメリット |
|----------|---------|-----------|
| 小さい (1MB) | 細かいサイズ調整が可能 | メタデータが大きい |
| デフォルト (4MB) | バランスが良い | - |
| 大きい (16MB+) | 大容量ディスクで効率的 | 最小 LV サイズが大きくなる |

---

### Q75: LVM シンプロビジョニング

**問い**: LVM のシンプロビジョニング (Thin Provisioning) の仕組みと利点を説明せよ。

**回答**:

```
通常のプロビジョニング (Thick):
  VG 空き容量: 30GB
  LV 作成: 10GB → 10GB が即座に確保される
  実際の使用量: 2GB → 残り 8GB は無駄

シンプロビジョニング (Thin):
  VG 空き容量: 30GB
  Thin Pool 作成: 20GB
  Thin LV 作成: 50GB (!) → メタデータのみ。実データは使用時に確保
  実際の使用量: 2GB → Thin Pool から 2GB のみ使用
  → 合計 50GB を「見せかけ」で提供（オーバーコミット）
```

```bash
# Thin Pool の作成
sudo lvcreate --type thin-pool -L 20G -n thinpool LVMGroup

# Thin LV の作成（Thin Pool のサイズを超えることも可能）
sudo lvcreate --type thin -V 50G --thinpool thinpool -n thinvol LVMGroup

# 使用状況の確認
sudo lvs -a
```

利点: ストレージの効率的な利用、オーバーコミット可能
注意: Thin Pool が満杯になると全 Thin LV が書き込み不可になる。監視が必須。

---

### Q76: LVM ストライピングとミラーリング

**問い**: LVM のストライピングとミラーリングの仕組みを説明せよ。RAID との違いも述べよ。

**回答**:

**ストライピング（RAID 0 相当）**:
```
データを複数の PV に分散して書き込み → I/O パフォーマンス向上

PV1:  [A1][A3][A5]
PV2:  [A2][A4][A6]
→ 2倍の書き込み速度（並列書き込み）
→ 冗長性なし（1台壊れると全データ損失）

sudo lvcreate -L 10G -n striped -i 2 LVMGroup
# -i 2: 2つの PV にストライプ
```

**ミラーリング（RAID 1 相当）**:
```
データを複数の PV に同時コピー → 冗長性確保

PV1:  [A1][A2][A3]  (コピー1)
PV2:  [A1][A2][A3]  (コピー2)
→ 1台壊れてもデータは安全
→ 読み取り性能は向上（並列読み取り）

sudo lvcreate -L 10G -m 1 -n mirrored LVMGroup
# -m 1: 1つのミラーコピー（合計2コピー）
```

LVM のストライピング/ミラーリングは mdadm (ソフトウェア RAID) より柔軟だが、パフォーマンスはやや劣る。本番環境ではハードウェア RAID または mdadm が推奨される。

---

### Q77: LVM のトラブルシューティング

**問い**: `vgchange -ay` で "Volume group not found" エラーが発生した場合の対処法を述べよ。

**回答**:

```bash
# 1. PV のスキャン
sudo pvscan
# → PV が見つかるか確認

# 2. PV が見つからない場合
sudo pvs --all
# → 全デバイスをスキャン

# 3. LUKS が開いているか確認（Born2beRoot の場合）
sudo dmsetup ls
# → sda5_crypt が表示されるか

# 4. LUKS を開く（開いていない場合）
sudo cryptsetup luksOpen /dev/sda5 sda5_crypt

# 5. VG を再スキャン
sudo vgscan
sudo vgchange -ay

# 6. LVM メタデータのバックアップから復元
ls /etc/lvm/backup/
sudo vgcfgrestore LVMGroup
```

---

### Q78: fstab の詳細

**問い**: `/etc/fstab` の各フィールドの意味を説明し、Born2beRoot の fstab の内容を解説せよ。

**回答**:

```bash
# fstab の各フィールド:
# <device>        <mount point>  <type>  <options>       <dump> <pass>
/dev/mapper/LVMGroup-root  /         ext4   errors=remount-ro  0      1
/dev/mapper/LVMGroup-home  /home     ext4   defaults           0      2
/dev/mapper/LVMGroup-var   /var      ext4   defaults           0      2
/dev/mapper/LVMGroup-srv   /srv      ext4   defaults           0      2
/dev/mapper/LVMGroup-tmp   /tmp      ext4   defaults,noexec,nosuid  0  2
/dev/mapper/LVMGroup-var--log /var/log ext4  defaults          0      2
/dev/mapper/LVMGroup-swap  none      swap   sw                 0      0
UUID=xxxx                  /boot     ext2   defaults           0      2
```

| フィールド | 説明 |
|-----------|------|
| device | デバイス名または UUID |
| mount point | マウント先のディレクトリ |
| type | ファイルシステムの種類 |
| options | マウントオプション |
| dump | dump によるバックアップ対象（0=対象外） |
| pass | fsck のチェック順序（0=チェックなし、1=最優先、2=通常） |

**重要なマウントオプション**:

| オプション | 意味 |
|-----------|------|
| `defaults` | rw, suid, dev, exec, auto, nouser, async |
| `noexec` | このパーティションでの実行を禁止 |
| `nosuid` | SUID/SGID ビットを無視 |
| `nodev` | デバイスファイルを無視 |
| `ro` | 読み取り専用 |
| `errors=remount-ro` | エラー発生時に読み取り専用で再マウント |

---

### Q79: ディスク I/O の監視

**問い**: ディスク I/O のパフォーマンスを監視するコマンドとその読み方を説明せよ。

**回答**:

```bash
# iostat: ディスク I/O 統計
sudo apt install sysstat
iostat -x 1
# Device  r/s    w/s   rkB/s   wkB/s  await  %util
# sda     5.00  10.00  20.00  160.00   2.50  15.00
#         ^      ^      ^       ^       ^      ^
#         読み取り 書き込み 読みKB/s 書きKB/s 平均待ち時間 使用率

# iotop: プロセスごとの I/O 使用量（top のディスク版）
sudo apt install iotop
sudo iotop -o  # I/O が発生しているプロセスのみ表示

# vmstat: 仮想メモリ統計（I/O 含む）
vmstat 1
# bi: ブロック入力 (read)
# bo: ブロック出力 (write)
# wa: I/O 待ちの CPU 時間 (%)
```

---

### Q80: LVM のベストプラクティス

**問い**: LVM を使用する場合のベストプラクティスを 5 つ以上述べよ。

**回答**:

| # | ベストプラクティス | 理由 |
|---|-------------------|------|
| 1 | VG に空き容量を残す | 将来の LV 拡張やスナップショット用 |
| 2 | スナップショットを活用 | アップデート前のバックアップ |
| 3 | 用途別に LV を分離 | `/var/log` の肥大化が他に影響しない |
| 4 | PE サイズを適切に設定 | 大容量ディスクでは大きめに |
| 5 | LVM メタデータをバックアップ | `/etc/lvm/backup/` の定期バックアップ |
| 6 | 監視を設定 | LV の使用率を監視し、閾値超過でアラート |
| 7 | ドキュメントを残す | LV の用途と設計理由を記録 |

---

## セクション H: sshd_config 監査問題 (Questions 81-90)

### Q81: sshd_config の全設定項目

**問い**: Born2beRoot で設定すべき sshd_config の全項目を列挙し、各項目のデフォルト値と推奨値を述べよ。

**回答**:

| 設定項目 | デフォルト値 | Born2beRoot 推奨値 | セキュリティ効果 |
|---------|------------|-------------------|----------------|
| Port | 22 | 4242 | ポートスキャン回避 |
| PermitRootLogin | prohibit-password | no | root 直接ログイン禁止 |
| PasswordAuthentication | yes | yes (基本) / no (ボーナス) | 鍵認証のみ許可 |
| PubkeyAuthentication | yes | yes | 公開鍵認証を有効化 |
| PermitEmptyPasswords | no | no | 空パスワードを禁止 |
| MaxAuthTries | 6 | 3 | ブルートフォース制限 |
| LoginGraceTime | 120 | 60 | 接続待ちタイムアウト短縮 |
| X11Forwarding | no | no | X11 転送を無効化 |
| AllowUsers | (なし) | kaztakam | 特定ユーザーのみ許可 |
| Protocol | 2 | 2 | SSH v1 を無効化 |
| UsePAM | yes | yes | PAM 認証を使用 |
| ChallengeResponseAuthentication | yes | no | チャレンジレスポンス認証無効化 |
| ClientAliveInterval | 0 | 300 | 無操作時の接続維持確認（5分） |
| ClientAliveCountMax | 3 | 3 | 接続維持の最大試行回数 |
| Banner | none | /etc/ssh/banner | ログイン前の警告バナー |

---

### Q82: SSH 鍵の種類

**問い**: SSH 鍵の種類（RSA, Ed25519, ECDSA）を比較し、推奨される鍵の種類と理由を述べよ。

**回答**:

| 鍵の種類 | 鍵長 | セキュリティ | パフォーマンス | 推奨度 |
|---------|------|------------|-------------|--------|
| RSA | 2048-4096 bit | 良好（4096推奨） | 遅い（鍵生成・署名） | △ |
| ECDSA | 256/384/521 bit | 良好 | 速い | △（NIST 曲線の懸念） |
| Ed25519 | 256 bit | 非常に高い | 非常に速い | ◎（最推奨） |

```bash
# Ed25519 鍵の生成（推奨）
ssh-keygen -t ed25519 -C "user@born2beroot"

# RSA 4096 鍵の生成（互換性が必要な場合）
ssh-keygen -t rsa -b 4096 -C "user@born2beroot"
```

Ed25519 が推奨される理由:
1. **短い鍵長で高いセキュリティ**: 256 bit で RSA 3072 bit 相当
2. **高速な署名/検証**: RSA の数十倍速い
3. **サイドチャネル攻撃耐性**: 定数時間実装
4. **鍵の短さ**: authorized_keys ファイルがコンパクト

---

### Q83: SSH ログの監査

**問い**: SSH のログインログを分析し、不正アクセスの兆候を発見する方法を述べよ。

**回答**:

```bash
# 1. 認証失敗のログを確認
sudo grep "Failed password" /var/log/auth.log | tail -20

# 2. 失敗した IP アドレスの集計
sudo grep "Failed password" /var/log/auth.log | \
  awk '{print $(NF-3)}' | sort | uniq -c | sort -rn | head -10
#   150 192.168.1.50    ← 同一 IP から大量の失敗 = ブルートフォース
#     5 10.0.2.2
#     1 10.0.2.15

# 3. 成功したログインの確認
sudo grep "Accepted" /var/log/auth.log | tail -20

# 4. 不正な時間帯のログイン
sudo grep "Accepted" /var/log/auth.log | awk '{print $1,$2,$3}'

# 5. root ログイン試行
sudo grep "root" /var/log/auth.log | grep -i "failed"

# 6. 存在しないユーザーでの試行
sudo grep "Invalid user" /var/log/auth.log
```

**不正アクセスの兆候**:
- 同一 IP からの大量の失敗 → ブルートフォース攻撃
- 深夜の成功ログイン → 不正アクセスの可能性
- 存在しないユーザー名での試行 → 辞書攻撃
- 短時間に複数の異なる IP からの試行 → ボットネット攻撃

---

### Q84: SSH のセキュリティ強化

**問い**: sshd_config 以外で SSH のセキュリティを強化する方法を 5 つ以上述べよ。

**回答**:

| # | 方法 | 効果 |
|---|------|------|
| 1 | Fail2ban の導入 | ブルートフォース攻撃の自動ブロック |
| 2 | 公開鍵認証のみ許可 | パスワード認証を無効化 |
| 3 | 2要素認証 (2FA) | Google Authenticator + SSH |
| 4 | Port Knocking | 特定のポートシーケンスで SSH ポートを開放 |
| 5 | AllowUsers/AllowGroups | 特定のユーザー/グループのみ SSH 許可 |
| 6 | SSH CA (認証局) | 証明書ベースの認証 |
| 7 | TCP Wrappers | `/etc/hosts.allow` / `/etc/hosts.deny` で IP 制限 |
| 8 | アイドルタイムアウト | ClientAliveInterval で無操作セッションを切断 |

---

### Q85: sshd_config の構文チェック

**問い**: sshd_config を変更した後に行うべき確認手順を述べよ。設定ミスによるロックアウトを防ぐ方法も述べよ。

**回答**:

```bash
# 1. 構文チェック（必須）
sudo sshd -t
# エラーがなければ出力なし

# 2. 設定の詳細確認
sudo sshd -T | grep -i "port\|permitroot\|passwordauth"

# 3. 既存の SSH セッションを維持したまま sshd を再起動
sudo systemctl reload sshd
# ※ restart ではなく reload を使用。既存セッションは切断されない

# 4. 別のターミナルから接続テスト（既存セッションは閉じない！）
ssh -p 4242 user@localhost

# ロックアウト防止策:
# - 変更前に必ず sshd -t で構文チェック
# - 既存の SSH セッションを維持したまま再起動
# - VirtualBox のコンソールからもアクセス可能であることを確認
# - at コマンドで5分後に元の設定に戻すスケジュールを設定
sudo cp /etc/ssh/sshd_config /etc/ssh/sshd_config.bak
echo "cp /etc/ssh/sshd_config.bak /etc/ssh/sshd_config && systemctl reload sshd" | at now + 5 minutes
```

---

### Q86-Q90: sshd_config 監査シナリオ

### Q86: 脆弱な sshd_config の特定

**問い**: 以下の sshd_config の問題点を全て指摘せよ。

```
Port 22
PermitRootLogin yes
PasswordAuthentication yes
PermitEmptyPasswords yes
MaxAuthTries 10
X11Forwarding yes
Protocol 2,1
```

**回答**:

| 行 | 問題点 | リスク | 修正 |
|----|-------|--------|------|
| `Port 22` | デフォルトポート | 自動スキャンの対象 | `Port 4242` |
| `PermitRootLogin yes` | root 直接ログイン許可 | 権限昇格が不要 | `PermitRootLogin no` |
| `PermitEmptyPasswords yes` | 空パスワード許可 | パスワードなしでログイン可能 | `PermitEmptyPasswords no` |
| `MaxAuthTries 10` | 試行回数が多すぎ | ブルートフォースに弱い | `MaxAuthTries 3` |
| `X11Forwarding yes` | X11 転送が有効 | X11 の脆弱性を攻撃面に | `X11Forwarding no` |
| `Protocol 2,1` | SSH v1 が有効 | SSHv1 は既知の脆弱性あり | `Protocol 2` |

---

### Q87: SSH 接続の暗号化

**問い**: SSH 接続で使用される暗号化アルゴリズムを確認する方法と、弱い暗号を無効化する設定を述べよ。

**回答**:

```bash
# 現在の接続で使用されている暗号を確認
ssh -v -p 4242 user@localhost 2>&1 | grep "cipher\|kex\|mac"

# sshd が提供する暗号一覧
sudo sshd -T | grep -E "^ciphers|^macs|^kexalgorithms"

# 強い暗号のみを許可する設定 (/etc/ssh/sshd_config):
Ciphers chacha20-poly1305@openssh.com,aes256-gcm@openssh.com,aes128-gcm@openssh.com
MACs hmac-sha2-512-etm@openssh.com,hmac-sha2-256-etm@openssh.com
KexAlgorithms curve25519-sha256,curve25519-sha256@libssh.org
HostKeyAlgorithms ssh-ed25519,rsa-sha2-512,rsa-sha2-256
```

---

### Q88: SSH のバナー設定

**問い**: SSH 接続前にバナーメッセージを表示する設定方法と、表示すべき内容、表示すべきでない内容を述べよ。

**回答**:

```bash
# /etc/ssh/sshd_config
Banner /etc/ssh/banner

# /etc/ssh/banner の内容例:
# ====================================================
# WARNING: Unauthorized access to this system is
# prohibited. All activities are monitored and logged.
# ====================================================
```

| 表示すべき内容 | 表示すべきでない内容 |
|-------------|-----------------|
| 不正アクセスの警告 | OS 名やバージョン |
| 監視している旨 | 内部 IP アドレス |
| 法的な告知 | ホスト名の詳細 |
| 組織名（任意） | ソフトウェアのバージョン |

---

### Q89: SSH の多要素認証

**問い**: SSH に Google Authenticator を使った 2 要素認証を設定する手順を述べよ。

**回答**:

```bash
# 1. インストール
sudo apt install libpam-google-authenticator

# 2. ユーザーごとに設定
google-authenticator
# → QR コードが表示される。スマートフォンの認証アプリでスキャン

# 3. PAM 設定（/etc/pam.d/sshd に追加）
auth required pam_google_authenticator.so

# 4. sshd_config を変更
ChallengeResponseAuthentication yes
AuthenticationMethods publickey,keyboard-interactive
# → 公開鍵 + TOTP の2要素認証

# 5. sshd を再起動
sudo systemctl restart sshd
```

---

### Q90: SSH のセッション管理

**問い**: 現在の SSH セッション一覧を確認し、不正なセッションを強制切断する方法を述べよ。

**回答**:

```bash
# 1. 現在のログインセッション一覧
who
# kaztakam  pts/0  2024-01-01 10:00 (10.0.2.2)
# unknown   pts/1  2024-01-01 03:00 (192.168.1.50)  ← 不審

# 2. SSH プロセスの確認
ps aux | grep sshd
# root  1234 sshd: kaztakam [priv]
# root  5678 sshd: unknown [priv]  ← 不審なセッション

# 3. 不正なセッションを切断
sudo kill 5678  # SSH プロセスを終了

# 4. 特定ユーザーの全セッションを切断
sudo pkill -u unknown

# 5. last コマンドで過去のログイン履歴を確認
last -20
# unknown  pts/1  192.168.1.50  Mon Jan 1 03:00 - 03:30 (00:30)
```

---

## セクション I: Terraform/IaC コード読解問題 (Questions 91-97)

### Q91: Terraform の基本構文

**問い**: 以下の Terraform コードが何をするか説明せよ。

```hcl
resource "virtualbox_vm" "born2beroot" {
  name   = "Born2beRoot"
  cpus   = 1
  memory = "1024 mib"
  image  = "debian-12.img"
}
```

**回答**:

このコードは VirtualBox の仮想マシンを宣言的に定義する。

- `resource`: Terraform のリソースブロック（インフラの構成要素を定義）
- `"virtualbox_vm"`: リソースタイプ（VirtualBox VM プロバイダー）
- `"born2beroot"`: リソース名（Terraform 内での識別子）
- `name`: VM の表示名
- `cpus`: 仮想 CPU 数
- `memory`: RAM サイズ（1024 MiB = 1GB）
- `image`: ベースイメージファイル

**宣言的 vs 命令的**:
```
命令的（手動操作）:
  1. VirtualBox を開く
  2. 「新規」をクリック
  3. 名前を入力
  4. CPU を 1 に設定
  5. メモリを 1024MB に設定

宣言的（Terraform）:
  「こういう VM が存在すべき」と定義
  → Terraform が現在の状態との差分を計算して適用
```

---

### Q92: Terraform 変数とモジュール

**問い**: Terraform の変数（variable）、出力（output）、モジュール（module）の使い方を説明せよ。

**回答**:

```hcl
# variables.tf - 変数定義
variable "vm_name" {
  type        = string
  default     = "Born2beRoot"
  description = "VM の名前"
}

variable "vm_memory" {
  type    = string
  default = "1024 mib"
}

# main.tf - 変数の使用
resource "virtualbox_vm" "born2beroot" {
  name   = var.vm_name      # 変数の参照
  memory = var.vm_memory
}

# outputs.tf - 出力定義
output "vm_ip" {
  value       = virtualbox_vm.born2beroot.network_adapter.0.ipv4_address
  description = "VM の IP アドレス"
}

# module の使用例
module "born2beroot" {
  source    = "./modules/vm"
  vm_name   = "Born2beRoot"
  vm_memory = "1024 mib"
}
```

変数を使うことで、同じ構成を異なる環境（開発・本番）で再利用できる。モジュールは関連するリソースをパッケージ化する仕組みで、コードの再利用性を高める。

---

### Q93: Terraform のステート管理

**問い**: Terraform の state ファイルの役割と、state が壊れた場合の復旧方法を説明せよ。

**回答**:

**State ファイル** (`terraform.tfstate`) は Terraform が管理するインフラの現在の状態を記録する JSON ファイル。

```
Terraform の動作:
  1. terraform plan: 設定ファイル (.tf) と state の差分を計算
  2. terraform apply: 差分を実際のインフラに適用し、state を更新

  設定ファイル (.tf)  ←→  State (.tfstate)  ←→  実際のインフラ
  「あるべき姿」          「現在の記録」          「実際の状態」
```

**State が壊れた場合**:
```bash
# 1. バックアップから復元
cp terraform.tfstate.backup terraform.tfstate

# 2. 実際のリソースから state を再インポート
terraform import virtualbox_vm.born2beroot <vm-id>

# 3. state の手動編集（最終手段）
terraform state list
terraform state show virtualbox_vm.born2beroot
terraform state rm virtualbox_vm.born2beroot  # state から削除（実リソースは残る）
```

**State のベストプラクティス**:
- リモートバックエンド（S3 等）に保存
- state のロック機能を使用（同時変更防止）
- `.gitignore` に追加（機密情報を含む場合がある）

---

### Q94: Ansible との比較

**問い**: Terraform と Ansible の違いを説明し、Born2beRoot の自動化でそれぞれが担う役割を述べよ。

**回答**:

| 項目 | Terraform | Ansible |
|------|-----------|---------|
| 用途 | インフラのプロビジョニング | 設定管理・アプリケーションデプロイ |
| アプローチ | 宣言的 | 手続き的 + 宣言的 |
| 状態管理 | State ファイル | ステートレス |
| エージェント | なし（API ベース） | なし（SSH ベース） |
| 冪等性 | 組み込み | モジュールによる |
| 言語 | HCL | YAML |

**Born2beRoot での役割分担**:
```
Terraform: VM の作成、ディスク、ネットワーク設定
    ↓
Ansible: OS の設定、パッケージインストール、セキュリティ設定
    ↓
完成した Born2beRoot VM
```

---

### Q95: IaC のメリット

**問い**: Infrastructure as Code のメリットを 5 つ以上挙げ、Born2beRoot を IaC で管理する場合の具体的な利点を述べよ。

**回答**:

| # | メリット | Born2beRoot での具体例 |
|---|---------|---------------------|
| 1 | 再現性 | 同じ設定の VM を何度でも作成可能。評価前の環境リセット |
| 2 | バージョン管理 | Git で設定変更の履歴を追跡。変更理由の記録 |
| 3 | コードレビュー | PR でセキュリティ設定の変更をレビュー |
| 4 | 自動テスト | CI/CD で設定の検証を自動化 |
| 5 | ドキュメント化 | コード自体がインフラのドキュメント |
| 6 | 標準化 | チームメンバー全員が同じ手順で構築 |
| 7 | 災害復旧 | コードから環境を再構築可能 |

---

### Q96: Terraform のライフサイクル

**問い**: `terraform init`, `terraform plan`, `terraform apply`, `terraform destroy` の各コマンドの動作を説明せよ。

**回答**:

```
terraform init
  → プロバイダープラグインのダウンロード
  → バックエンドの初期化
  → モジュールのダウンロード

terraform plan
  → 設定ファイルと state の差分を計算
  → 作成/変更/削除されるリソースを表示
  → 実際のインフラは変更しない（ドライラン）

terraform apply
  → plan の内容を実行
  → インフラを作成/変更/削除
  → state ファイルを更新
  → 確認プロンプト（-auto-approve で省略可能）

terraform destroy
  → state に記録された全リソースを削除
  → state ファイルを更新（空に）
  → 確認プロンプト
```

---

### Q97: Ansible Playbook の読解

**問い**: 以下の Ansible Playbook が何をするか説明せよ。

```yaml
- name: Configure Born2beRoot Security
  hosts: born2beroot
  become: yes
  tasks:
    - name: Install required packages
      apt:
        name: "{{ item }}"
        state: present
      loop:
        - ufw
        - libpam-pwquality
        - sudo

    - name: Configure SSH
      lineinfile:
        path: /etc/ssh/sshd_config
        regexp: "^Port"
        line: "Port 4242"
      notify: restart sshd

  handlers:
    - name: restart sshd
      service:
        name: sshd
        state: restarted
```

**回答**:

この Playbook は Born2beRoot のセキュリティ設定を自動化する:

1. **ホスト**: `born2beroot` グループの全ホストに対して実行
2. **権限昇格**: `become: yes` で root 権限で実行 (sudo)
3. **タスク 1**: `ufw`, `libpam-pwquality`, `sudo` パッケージをインストール
   - `loop` で複数パッケージを繰り返しインストール
   - `state: present` で「インストール済み」を保証（冪等性）
4. **タスク 2**: `/etc/ssh/sshd_config` の `Port` 行を `Port 4242` に変更
   - `regexp` で既存の Port 行を検索
   - `line` で置換内容を指定
   - `notify` で変更があった場合のみ handler を呼び出し
5. **Handler**: sshd サービスを再起動（タスク 2 で変更があった場合のみ）

---

## セクション J: 総合シナリオ問題 (Questions 98-105)

### Q98: 完全な VM セットアップ

**問い**: Born2beRoot の VM を最初からセットアップする場合の全手順を、大きな順序で述べよ。

**回答**:

```
Phase 1: VM 作成
  1. VirtualBox で新規 VM を作成（Debian 64-bit、RAM 1GB、ディスク 30GB）
  2. Debian ISO をマウントして起動

Phase 2: OS インストール
  3. 言語・キーボード設定
  4. ホスト名設定（<login>42）
  5. root パスワード設定
  6. 一般ユーザー作成
  7. ディスクパーティション設定
     - /boot: 500MB (暗号化なし)
     - LUKS 暗号化パーティション作成
     - LVM 設定（root, swap, home, var, srv, tmp, var-log）
  8. 基本システムのインストール

Phase 3: セキュリティ設定
  9. sudo のインストールと設定（/etc/sudoers.d/）
  10. パスワードポリシー設定（login.defs + pam_pwquality）
  11. UFW の設定（ポート 4242 のみ許可）
  12. SSH の設定（ポート 4242、root ログイン禁止）
  13. AppArmor の確認

Phase 4: 監視とユーザー
  14. monitoring.sh の作成
  15. cron の設定（10分ごと）
  16. user42 グループの作成
  17. ユーザーのグループ設定

Phase 5: ボーナス（任意）
  18. Lighttpd + MariaDB + PHP + WordPress
  19. 追加サービスの設定
```

---

### Q99: セキュリティインシデント対応

**問い**: Born2beRoot で不正アクセスの兆候（大量の SSH ログイン失敗）を検出した。インシデント対応の手順を述べよ。

**回答**:

**インシデント対応の 6 ステップ**:

| ステップ | 行動 | Born2beRoot での実施 |
|---------|------|---------------------|
| 1. 検知 | 異常の確認 | `grep "Failed password" /var/log/auth.log | wc -l` |
| 2. 封じ込め | 被害の拡大防止 | `sudo ufw deny from <攻撃者IP>` |
| 3. 根絶 | 攻撃手段の除去 | 侵入されていないか確認、不正ユーザーの削除 |
| 4. 復旧 | 正常な状態への復帰 | LVM スナップショットからの復元（必要な場合） |
| 5. 事後分析 | 原因の特定 | ログの分析、タイムラインの作成 |
| 6. 改善 | 再発防止策 | Fail2ban の導入、鍵認証への移行 |

```bash
# 即座の対応:
# 1. 攻撃者の IP を特定
sudo grep "Failed password" /var/log/auth.log | \
  awk '{print $(NF-3)}' | sort | uniq -c | sort -rn

# 2. 攻撃者の IP をブロック
sudo ufw deny from 192.168.1.50

# 3. 不正なセッションの確認と切断
who
sudo pkill -u suspicious_user

# 4. パスワードの変更
sudo passwd kaztakam

# 5. ログの保全（証拠保存）
sudo cp /var/log/auth.log /root/incident-$(date +%Y%m%d).log
```

---

### Q100: パフォーマンスチューニング

**問い**: Born2beRoot の VM のパフォーマンスが遅い。原因の調査方法とチューニング手順を述べよ。

**回答**:

```bash
# CPU の確認
top -bn1 | head -5
# %Cpu(s): 95.0 us → CPU 使用率が高い

# メモリの確認
free -m
# Mem: total=1024, used=980 → メモリ不足

# ディスク I/O の確認
iostat -x 1 3
# %util: 99% → ディスクが飽和

# プロセスの確認
ps aux --sort=-%mem | head -10  # メモリ使用量順
ps aux --sort=-%cpu | head -10  # CPU 使用量順
```

**チューニング方法**:

| 問題 | 対策 |
|------|------|
| CPU 高負荷 | 不要なプロセスの停止、cron の頻度を減らす |
| メモリ不足 | swap の追加、不要なサービスの無効化 |
| ディスク I/O | LVM でディスクを追加、ログの圧縮 |
| ネットワーク | MTU の最適化、不要な接続の閉鎖 |

---

### Q101: バックアップ戦略

**問い**: Born2beRoot のバックアップ戦略を設計せよ。フル、差分、増分バックアップの違いと、LVM スナップショットの活用方法を述べよ。

**回答**:

| バックアップ種類 | 説明 | 復元速度 | ストレージ |
|---------------|------|---------|----------|
| **フル** | 全データをコピー | 最速 | 最大 |
| **差分** | 前回のフルからの変更分 | 中程度 | 中程度 |
| **増分** | 前回のバックアップからの変更分 | 最遅 | 最小 |

```
推奨スケジュール:
  日曜: フルバックアップ
  月-土: 増分バックアップ

復元手順（水曜のデータを復元）:
  フル: 日曜のフルバックアップを復元
  差分: 日曜のフル + 水曜の差分
  増分: 日曜のフル + 月曜増分 + 火曜増分 + 水曜増分
```

**LVM スナップショットの活用**:
```bash
# バックアップ前にスナップショット作成
sudo lvcreate --size 2G --snapshot --name backup-snap /dev/LVMGroup/root

# スナップショットをマウントしてバックアップ
sudo mkdir /mnt/backup-snap
sudo mount -o ro /dev/LVMGroup/backup-snap /mnt/backup-snap
sudo tar czf /backup/root-$(date +%Y%m%d).tar.gz -C /mnt/backup-snap .

# クリーンアップ
sudo umount /mnt/backup-snap
sudo lvremove /dev/LVMGroup/backup-snap
```

---

### Q102: 権限昇格攻撃

**問い**: Linux サーバーで権限昇格 (Privilege Escalation) 攻撃が行われる手法を 5 つ以上挙げ、Born2beRoot でそれぞれを防ぐ設定を述べよ。

**回答**:

| # | 攻撃手法 | Born2beRoot の防御策 |
|---|---------|---------------------|
| 1 | SUID バイナリの悪用 | `find / -perm -4000` で監査、不要な SUID を削除 |
| 2 | sudo の設定ミス | `requiretty`, `secure_path`, 最小限の権限付与 |
| 3 | カーネルの脆弱性 | `apt upgrade` で定期的にカーネルを更新 |
| 4 | cron ジョブの操作 | cron スクリプトの権限を root:root 755 に |
| 5 | PATH 操作 | `secure_path` で sudo の PATH を制限 |
| 6 | LD_PRELOAD 攻撃 | AppArmor でライブラリの読み込みを制限 |
| 7 | /tmp ディレクトリの悪用 | `noexec,nosuid` でマウント |
| 8 | パスワードファイルの操作 | `/etc/shadow` の権限を 640 に |

---

### Q103: コンプライアンスレポート

**問い**: Born2beRoot の設定状態をスクリプトで自動チェックし、コンプライアンスレポートを生成する方法を述べよ。

**回答**:

```bash
#!/bin/bash
# compliance_check.sh - Born2beRoot コンプライアンスチェッカー

echo "=== Born2beRoot Compliance Report ==="
echo "Date: $(date)"
echo ""

# 1. SSH 設定
echo "[SSH Configuration]"
PORT=$(grep "^Port" /etc/ssh/sshd_config | awk '{print $2}')
[ "$PORT" = "4242" ] && echo "PASS: SSH Port = 4242" || echo "FAIL: SSH Port = $PORT"

ROOT_LOGIN=$(grep "^PermitRootLogin" /etc/ssh/sshd_config | awk '{print $2}')
[ "$ROOT_LOGIN" = "no" ] && echo "PASS: Root login disabled" || echo "FAIL: Root login = $ROOT_LOGIN"

# 2. ファイアウォール
echo ""
echo "[Firewall]"
UFW_STATUS=$(sudo ufw status | head -1)
echo "$UFW_STATUS" | grep -q "active" && echo "PASS: UFW is active" || echo "FAIL: UFW is inactive"

# 3. パスワードポリシー
echo ""
echo "[Password Policy]"
MAX_DAYS=$(grep "^PASS_MAX_DAYS" /etc/login.defs | awk '{print $2}')
[ "$MAX_DAYS" -le 30 ] && echo "PASS: PASS_MAX_DAYS = $MAX_DAYS" || echo "FAIL: PASS_MAX_DAYS = $MAX_DAYS"

# 4. AppArmor
echo ""
echo "[AppArmor]"
aa-status 2>/dev/null | grep -q "apparmor module is loaded" && echo "PASS: AppArmor loaded" || echo "FAIL: AppArmor not loaded"

# 5. sudo 設定
echo ""
echo "[Sudo Configuration]"
grep -q "requiretty" /etc/sudoers.d/* 2>/dev/null && echo "PASS: requiretty set" || echo "FAIL: requiretty not set"
grep -q "secure_path" /etc/sudoers.d/* 2>/dev/null && echo "PASS: secure_path set" || echo "FAIL: secure_path not set"
```

---

### Q104: 災害復旧計画

**問い**: Born2beRoot の VM が完全に壊れた場合の災害復旧 (Disaster Recovery) 計画を策定せよ。RTO (Recovery Time Objective) と RPO (Recovery Point Objective) の概念も説明せよ。

**回答**:

**RTO / RPO の定義**:
- **RTO**: 障害発生からサービス復旧までの目標時間（例: 1時間以内に復旧）
- **RPO**: 許容できるデータ損失の時間（例: 最大1時間分のデータ損失を許容）

```
         RPO                  RTO
  <──────────────>    <──────────────>
  最終バックアップ      障害発生        復旧完了
  ─────┼─────────────────┼──────────────┼───── 時間軸
       |    この間のデータ |              |
       |    は失われる    |              |
```

**Born2beRoot の DR 計画**:

| 優先度 | 復旧方法 | RTO | RPO |
|--------|---------|-----|-----|
| 1 | VirtualBox スナップショットから復元 | 5分 | スナップショット時点 |
| 2 | LVM スナップショットから復元 | 15分 | スナップショット時点 |
| 3 | バックアップから復元 | 30分 | 最終バックアップ時点 |
| 4 | IaC (Terraform+Ansible) で再構築 | 1時間 | コードリポジトリの最新 |
| 5 | 手動で再構築 | 数時間 | ドキュメント次第 |

IaC の最大の利点は、手順書がコードとして存在するため、手動の再構築よりも確実で高速な復旧が可能なことである。

---

### Q105: 総合理解度テスト

**問い**: 以下の全てを 1 つの回答にまとめよ: Born2beRoot プロジェクトで最も重要な学びは何か。このプロジェクトで得た知識が実務でどのように活かされるか。42 の次のプロジェクトにどう繋がるか。

**回答**:

**最も重要な学び**:

Born2beRoot の核心は「サーバーセキュリティの多層防御 (Defense in Depth)」の実践的理解である。

```
多層防御の実践:
  物理層:     LUKS ディスク暗号化 → データの保護
  ネットワーク: UFW ファイアウォール → アクセス制御
  ホスト:     AppArmor → プロセスの制限
  認証:       SSH + パスワードポリシー → 本人確認
  権限:       sudo + 最小権限 → 権限昇格の防止
  監視:       monitoring.sh + ログ → 異常の検知
```

**実務での活用**:

| Born2beRoot の知識 | 実務での活用 |
|-------------------|-------------|
| LVM | クラウド (AWS EBS) のボリューム管理 |
| LUKS | 顧客データの暗号化（GDPR/HIPAA 準拠） |
| SSH | リモートサーバー管理、CI/CD パイプライン |
| UFW/iptables | AWS Security Groups、GCP Firewall Rules |
| AppArmor | Kubernetes の Pod Security、コンテナ隔離 |
| systemd | マイクロサービスのデプロイ・監視 |
| monitoring.sh | Prometheus、Grafana、Datadog による監視 |
| IaC (Terraform) | クラウドインフラの自動構築 |

**42 の次のプロジェクトへの繋がり**:

```
Born2beRoot (サーバー基礎)
    |
    ├── ft_irc (ネットワーク) → TCP/IP、ソケット通信
    ├── Inception (Docker) → コンテナ、Docker Compose
    ├── webserv (Web サーバー) → HTTP、CGI、システムコール
    ├── ft_transcendence (フルスタック) → 全知識の統合
    └── Cloud-1 (クラウド) → Terraform、Kubernetes
```

Born2beRoot は「OS の中で何が起きているかを理解する」最初のステップであり、この理解がなければ上位のプロジェクトで発生する問題をデバッグすることは困難である。根本的な理解こそが、優れたエンジニアとそうでないエンジニアの違いを生む。

---

*End of Comprehension Check - 105 Questions Complete*
