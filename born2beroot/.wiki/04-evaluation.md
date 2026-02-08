# 04 - 評価基準と防御準備 (Evaluation Criteria & Defense)

Born2beRoot の評価では、設定の確認だけでなく、仕組みの理解と説明能力が問われる。
本ドキュメントでは、評価の流れ、55問以上の質問と模範回答（短い版・詳細版）、デモンストレーション手順、よくある間違い回答とその修正を網羅的に提供する。

---

## 1. 評価の流れ

### 1.1 一般的な評価手順

1. **signature.txt の検証**: VM のディスクハッシュが一致するか確認
2. **VM の起動**: LUKS パスフレーズの入力、正常な起動の確認
3. **基本設定の確認**: ホスト名、ユーザー、グループ
4. **各要件の動作確認と質疑応答**: SSH、UFW、パスワードポリシー、sudo、monitoring.sh
5. **新規ユーザー作成**: 評価中にユーザーを作成して全設定が適用されるか確認
6. **ホスト名の変更**: 評価中にホスト名を変更し、再起動後に反映・元に戻せるか確認
7. **Bonus の確認**（必須部分が完璧な場合のみ）

### 1.2 評価者が確認するポイント

- 設定が**正しく動作する**こと（単にファイルに書いてあるだけでは不十分）
- 各設定の**目的と仕組み**を説明できること
- **エラーなく**動作すること
- **要件に正確に**合致していること
- 「なぜ」を問われた時に**具体的かつ論理的に**答えられること

### 1.3 評価の心構え

評価は「暗記テスト」ではなく「理解度テスト」である。評価者は以下を見ている：

- **深い理解**: 表面的な回答ではなく、なぜその設定が必要か、内部でどう動作するかを説明できるか
- **応用力**: 想定外の質問に対しても、基礎知識から論理的に回答を導けるか
- **デモンストレーション能力**: 口頭の説明だけでなく、実際のコマンドで動作を見せられるか
- **トラブルシューティング力**: 何かが動かない場合に、原因を特定するプロセスを示せるか
- **関連知識**: Born2beRoot 以外の文脈（クラウド、コンテナ、実務）との関連を理解しているか

### 1.4 signature.txt の準備

```bash
# VirtualBox の場合（VM を必ず停止した状態で）
# macOS
shasum born2beroot.vdi

# Linux
sha1sum born2beroot.vdi

# 生成されたハッシュを signature.txt に保存
echo "ハッシュ値" > signature.txt
```

**重要**: signature.txt を生成した後に VM を起動すると、ディスクの内容が変わりハッシュが一致しなくなる。必ず **VM を停止した状態** でハッシュを生成し、生成後に再起動しないこと。評価当日にハッシュが一致しない場合は即座に **0点** となる可能性がある。

### 1.5 評価前日のチェックリスト

- [ ] signature.txt が正しいハッシュを含んでいる
- [ ] VM を起動して LUKS パスフレーズでブートできる
- [ ] ホスト名が `login42` 形式である
- [ ] ユーザーが sudo, user42 グループに所属している
- [ ] SSH が port 4242 でのみリスニングしている
- [ ] UFW が有効で port 4242 のみ許可している
- [ ] パスワードポリシーが設定されている
- [ ] sudo の設定が正しい（ログ、requiretty、secure_path）
- [ ] monitoring.sh が10分ごとに動作している
- [ ] AppArmor が有効である
- [ ] すべての質問に対する回答を練習した

---

## 2. 防御質問と回答（55問）

### カテゴリ A: 基礎概念 (Q1-Q12)

---

#### Q1: 仮想マシン (Virtual Machine) はどのように動作するか？

**短い版**:
仮想マシンは Hypervisor がハードウェアリソースを仮想的に分割して作った独立した計算環境。Born2beRoot では VirtualBox（Type 2 Hypervisor）を使用し、ホスト OS 上でゲスト OS（Debian）を動作させている。

**詳細版**:
仮想マシンは、Hypervisor と呼ばれるソフトウェアによって、物理ハードウェアのリソース（CPU、メモリ、ディスク、ネットワーク）を仮想的に分割・エミュレートして作られた独立したコンピューティング環境である。

Born2beRoot では VirtualBox（Type 2 Hypervisor）を使用している。Type 2 Hypervisor はホスト OS 上でアプリケーションとして動作し、その上にゲスト OS を動かす。

動作の流れ:
1. VirtualBox がホスト OS のリソースの一部を VM に割り当てる
2. VM 内でゲスト OS（Debian）が物理マシンのように起動する
3. ゲスト OS のハードウェアアクセスは VirtualBox が仲介し、実際のハードウェアに変換する
4. ハードウェア仮想化支援（Intel VT-x / AMD-V）により、多くの命令はネイティブに近い速度で実行される

Type 1 Hypervisor（ESXi、KVM など）との違いは、Type 1 はホスト OS なしで直接ハードウェア上で動作するため、オーバーヘッドが少なくデータセンターやクラウド環境で使用される。

仮想化の種類の整理:
- **完全仮想化 (Full Virtualization)**: ゲスト OS に変更を加えず、ハードウェア全体をエミュレート。VirtualBox、VMware がこの方式
- **準仮想化 (Paravirtualization)**: ゲスト OS のカーネルを変更して Hypervisor と直接通信。Xen が採用
- **コンテナ仮想化**: OS レベルの仮想化。Docker がこの方式。カーネルを共有するため軽量だが、異なる OS は動作させられない
- **ハードウェア仮想化支援**: Intel VT-x / AMD-V が CPU レベルで仮想化を支援。仮想マシンのゲスト OS がセンシティブ命令を実行する際のトラップを効率化

**デモンストレーション**:
```bash
# ゲスト OS 内から仮想化を確認
systemd-detect-virt
# → oracle（VirtualBox の場合）

# CPU の仮想化支援機能の確認
grep -cE "vmx|svm" /proc/cpuinfo

# VM のハードウェア情報
sudo dmidecode -t system | head -10
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「VM は物理マシンのシミュレーション」 | シミュレーションではなくエミュレーション。CPU 命令の多くは VT-x/AMD-V により直接ネイティブ実行される |
| 「VirtualBox は Type 1 Hypervisor」 | VirtualBox は Type 2（ホスト OS 上で動作）。Type 1 は ESXi, KVM, Hyper-V |
| 「VM はコンテナと同じ」 | VM はカーネルを含む完全な OS を動かす。コンテナはホストのカーネルを共有する |
| 「VM は必ず遅い」 | ハードウェア仮想化支援により CPU 処理はネイティブに近い速度で実行される。I/O は overhead がある |
| 「全ての VM は同じ OS しか動かせない」 | VM は各々独立したカーネルを持つため、異なる OS を同時に動作可能 |

---

#### Q2: なぜ Debian を選んだのか？（Rocky Linux との比較）

**短い版**:
安定性が高い、パッケージが豊富、UFW と AppArmor がデフォルトで利用可能、Debian/Ubuntu 系は情報量が圧倒的に多い。

**詳細版**:
Debian を選んだ理由:
1. **安定性**: Debian stable はリリース前に数年のテスト期間を経ており、サーバー運用に適している。フリーズからリリースまで通常6〜24ヶ月のテスト期間がある
2. **パッケージの豊富さ**: 約59,000以上のパッケージが利用可能。ほぼすべてのサーバーソフトウェアが公式リポジトリにある
3. **UFW が利用可能**: 直感的なファイアウォール設定が可能。`ufw allow 4242` のようなシンプルなコマンド
4. **AppArmor**: Debian ではデフォルトで有効。SELinux より学習曲線が緩やか（パスベース vs ラベルベース）
5. **情報量**: 1993年からの歴史を持ち、Debian/Ubuntu 系の情報が非常に多い。Stack Overflow、DigitalOcean、Arch Wiki（概念は共通）等
6. **Ubuntu の基盤**: Ubuntu は Debian ベースであり、学んだ知識がそのまま応用可能
7. **フリーソフトウェアの理念**: 完全にオープンソースで、商用企業に依存しない

Debian vs Rocky Linux の比較:

| 比較項目 | Debian | Rocky Linux |
|---------|--------|-------------|
| ベース | 独自 | RHEL クローン |
| パッケージ形式 | .deb (dpkg/apt) | .rpm (rpm/dnf) |
| ファイアウォール | UFW (iptables) | firewalld (nftables) |
| MAC | AppArmor | SELinux |
| リリースサイクル | ~2年 | RHEL に追従 |
| エンタープライズ | 限定的 | RHEL 互換 |
| 学習コスト | 低〜中 | 中〜高 |

Rocky Linux の利点: エンタープライズ環境（RHEL 互換）の知識が身につく。SELinux は細粒度の制御が可能。企業の求人で RHEL 系の需要がある。

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「Debian は Ubuntu と同じ」 | Ubuntu は Debian ベースだが独自のリリースサイクルと追加機能（snap 等）を持つ |
| 「Debian はセキュリティが弱い」 | Debian Security Team が迅速にパッチを提供。セキュリティ対応は十分 |
| 「Rocky Linux は難しいから避けた」 | 理由として不十分。UFW vs firewalld, AppArmor vs SELinux の具体的比較を述べるべき |
| 「Debian は古い」 | Debian は最新技術も追従している。stable は意図的に安定性重視でバージョンを固定している |

---

#### Q3: apt と aptitude の違いは何か？

**短い版**:
apt はコマンドラインのみで依存関係解決がシンプル。aptitude は対話型 TUI を持ち、依存関係の解決がより高度（複数の解決策を提案する）。

**詳細版**:
apt と aptitude はどちらも Debian のパッケージ管理ツール dpkg のフロントエンドだが、設計思想と機能に違いがある。

| 比較項目 | apt | aptitude |
|---------|-----|----------|
| インターフェース | コマンドラインのみ | コマンドライン + ncurses TUI |
| 依存関係解決 | 1つの解決策を提示 | 複数の候補を提案し選択させる |
| 推奨パッケージ | デフォルトでインストール | デフォルトでインストールしない |
| 未使用パッケージ | 手動で `apt autoremove` | 自動削除を提案 |
| デフォルト | Debian に含まれる | 追加インストールが必要 |
| パッケージ追跡 | 基本的 | 詳細（自動/手動の区別等） |
| なぜ必要か | `apt depends` | `aptitude why` |

**dpkg との関係**:
```
ユーザー → apt / aptitude → dpkg → ファイルシステム
               ↓
           リポジトリ（/etc/apt/sources.list）
```

dpkg は .deb ファイルを直接操作する低レベルツールで、依存関係は解決しない。apt/aptitude はリポジトリからのダウンロード、依存関係の解決を担い、最終的に dpkg を呼び出す。

**デモンストレーション**:
```bash
# apt でパッケージの依存関係を確認
apt depends vim

# aptitude の対話型インターフェース
sudo aptitude

# aptitude でパッケージが必要な理由を表示
aptitude why vim-common

# apt-get と apt の違い
# apt は apt-get + apt-cache の統合版で、プログレスバーなど UX が改善されている
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「apt-get と apt は同じ」 | apt は apt-get と apt-cache の機能を統合。プログレスバー等の UX 改善もある |
| 「aptitude は apt より高機能なので常に使うべき」 | 場面による。apt は軽量で高速、スクリプトでの使用に適している |
| 「dpkg は不要」 | dpkg は内部で常に使われている。apt/aptitude はフロントエンドに過ぎない |
| 「aptitude は Debian 以外でも使える」 | aptitude は Debian 系（Ubuntu 含む）専用 |

---

#### Q4: AppArmor (Application Armor) とは何か？

**短い版**:
AppArmor は Linux の MAC（強制アクセス制御 / Mandatory Access Control）セキュリティモジュール。各プログラムにプロファイルを定義し、アクセスできるリソースを制限する。従来の DAC（任意アクセス制御）を補完する。

**詳細版**:
AppArmor は Linux の MAC セキュリティモジュールで、Linux Security Modules (LSM) フレームワーク上で動作する。

**DAC の問題点と MAC の必要性**:
従来の DAC (Discretionary Access Control) では、ファイルの所有者がアクセス権を決定する。問題は、プログラムが脆弱性を突かれて乗っ取られた場合、そのプログラムのユーザー権限で任意の操作が可能になること。例えば、root で動作する sshd に脆弱性があれば、root 権限が完全に奪取される。

AppArmor（MAC）は、各プログラムに「プロファイル」を定義し、そのプログラムがアクセスできるファイルやリソースをシステム管理者が制限する。sshd が乗っ取られても、プロファイルで許可されていないファイルへのアクセスや操作は実行できない。

**AppArmor の特徴**:
- **パスベース**: ファイルパスでポリシーを定義する（直感的で学習しやすい）
- **プロファイルベース**: プログラムごとにプロファイルを作成
- **enforce / complain モード**: 本番用とテスト用のモードを切り替え可能

**AppArmor vs SELinux の比較**:

| 比較項目 | AppArmor | SELinux |
|---------|----------|---------|
| ポリシーの基準 | ファイルパス | セキュリティラベル |
| 学習曲線 | 緩やか | 急峻 |
| 柔軟性 | 中 | 高 |
| デフォルト採用 | Debian, Ubuntu, SUSE | RHEL, CentOS, Fedora |
| プロファイル作成 | 比較的容易 | 複雑 |
| 細粒度制御 | 中 | 高（ネットワーク、IPC レベルまで） |

**デモンストレーション**:
```bash
# AppArmor の状態確認
sudo aa-status
# → プロファイル数、enforce/complain モードのプロファイル一覧が表示される

# AppArmor サービスの状態確認
sudo systemctl status apparmor

# AppArmor のログ確認
sudo dmesg | grep apparmor

# 特定のプロファイルの内容確認（存在する場合）
ls /etc/apparmor.d/
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「AppArmor はファイアウォール」 | ファイアウォールはネットワークレベル。AppArmor はプロセスレベルのアクセス制御 |
| 「AppArmor は DAC を置き換える」 | DAC の上に追加される。DAC チェック後に AppArmor チェックが行われる |
| 「complain モードは意味がない」 | プロファイル開発やトラブルシューティングで不可欠 |
| 「SELinux の方が常に優れている」 | 用途による。AppArmor はシンプルさ、SELinux は細粒度が特徴 |
| 「AppArmor を無効にしても問題ない」 | MAC は多層防御の重要な一層。無効化はセキュリティリスク |

---

#### Q5: LVM (Logical Volume Manager) とは何か？

**短い版**:
LVM は物理ディスクを抽象化して柔軟なストレージ管理を提供する仕組み。PV（物理ボリューム）→ VG（ボリュームグループ）→ LV（論理ボリューム）の3層構造で、動的なサイズ変更やディスク統合が可能。

**詳細版**:
LVM (Logical Volume Manager) は、物理ディスクの境界を超えて柔軟にストレージを管理するための仕組みである。

**3層構造**:
```
物理ディスク → [PV] → [VG] → [LV] → ファイルシステム → マウントポイント
/dev/sda1         \      /       ├── root → ext4 → /
/dev/sdb           → VG ←       ├── home → ext4 → /home
/dev/sdc          /      \      ├── var  → ext4 → /var
                                ├── tmp  → ext4 → /tmp
                                └── swap → swap
```

1. **PV (Physical Volume)**: 物理ディスクまたはパーティションを LVM で使用可能にしたもの。LVM の最下層
2. **VG (Volume Group)**: 一つ以上の PV をまとめたストレージプール。ディスクの境界を超えて容量を統合する
3. **LV (Logical Volume)**: VG 内に作成される仮想パーティション。実際にファイルシステムを作成してマウントする対象

**LVM の利点**:
- パーティションサイズを**動的に変更**可能（従来は再フォーマットが必要）
- 複数の物理ディスクを**一つのボリュームグループに統合**可能
- **スナップショット**によるバックアップ（CoW 方式で効率的）
- **ダウンタイムなし**での拡張（ext4 はオンライン拡張可能）
- **シンプロビジョニング**: 実使用量分だけ物理ストレージを消費

**従来のパーティションの問題点**:
- 一度作成したパーティションのサイズ変更が困難
- ディスクをまたいでパーティションを作成できない
- パーティション数に制限がある（MBR: 最大4プライマリ）

**LVM の操作コマンド**:
```bash
# PV の作成
pvcreate /dev/sdb1

# VG の作成・拡張
vgcreate MyVG /dev/sdb1
vgextend MyVG /dev/sdc1

# LV の作成・拡張・縮小
lvcreate -L 10G -n mydata MyVG
lvextend -L +5G /dev/MyVG/mydata
lvreduce -L -2G /dev/MyVG/mydata  # ※先にファイルシステムを縮小すること

# ファイルシステムの拡張
resize2fs /dev/MyVG/mydata

# スナップショットの作成
lvcreate -L 1G -s -n snap_mydata /dev/MyVG/mydata
```

**デモンストレーション**:
```bash
lsblk             # ブロックデバイスの階層構造を木構造で表示
lsblk -f          # ファイルシステムタイプも表示
sudo lvdisplay    # LV の詳細
sudo vgdisplay    # VG の情報（空き容量含む）
sudo pvdisplay    # PV の情報
sudo vgs          # VG のサマリー
sudo lvs          # LV のサマリー
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「LVM は RAID と同じ」 | RAID は冗長性（ミラーリング/ストライピング）。LVM はボリュームの柔軟な管理。ただし LVM は RAID 機能（lvmraid）も持つ |
| 「LVM を使うとパフォーマンスが大幅に落ちる」 | linear マッピングではオーバーヘッドはほぼ無視できる |
| 「PV = パーティション」 | PV はパーティション上に作成されるが、ディスク全体を PV にすることも可能 |
| 「LV のサイズは縮小できない」 | ext4 は縮小可能（オフラインで）。ただし XFS は縮小不可 |
| 「LVM はバックアップツール」 | LVM はストレージ管理ツール。スナップショットはバックアップに使えるが、それは一機能に過ぎない |

---

#### Q6: パスワードポリシー (Password Policy) はどのように動作するか？

**短い版**:
2つの仕組みで構成。`/etc/login.defs` で有効期限（MAX 30日 / MIN 2日 / WARN 7日）を設定し、PAM の `pam_pwquality` で強度（最小10文字、大文字・小文字・数字必須、連続3文字制限等）をチェックする。

**詳細版**:
パスワードポリシーは2つの独立した仕組みで構成される。

**1. 有効期限ポリシー** (`/etc/login.defs`):
```
PASS_MAX_DAYS   30    # 30日ごとにパスワード変更必要
PASS_MIN_DAYS   2     # 変更後2日間は再変更不可（使い回し防止）
PASS_WARN_AGE   7     # 期限の7日前から警告表示
```

これは `/etc/shadow` ファイルに各ユーザーごとに記録される。

**重要**: login.defs の設定は**新規ユーザーにのみ**自動適用される。既存ユーザーには `chage` コマンドで個別に設定が必要:
```bash
sudo chage -M 30 -m 2 -W 7 existing_user
```

**2. 強度ポリシー** (PAM `pam_pwquality`):
```
minlen=10        # 最低10文字
ucredit=-1       # 大文字を最低1文字
dcredit=-1       # 数字を最低1文字
lcredit=-1       # 小文字を最低1文字
maxrepeat=3      # 同一文字の連続は最大3文字
reject_username  # ユーザー名を含むパスワードを拒否
difok=7          # 旧パスワードと7文字以上異なる
enforce_for_root # root にもポリシーを適用
```

**PAM がパスワード変更時に行う処理**:
1. ユーザーが `passwd` コマンドを実行
2. PAM が `/etc/pam.d/common-password` を読み込む
3. `pam_pwquality` モジュールが新しいパスワードを検証
4. 検証に合格すれば `pam_unix` がパスワードをハッシュ化して `/etc/shadow` に保存
5. 検証に失敗すれば、理由を表示してパスワード変更を拒否

**設定ファイルの場所**:
```
/etc/login.defs              → 有効期限の設定
/etc/pam.d/common-password   → pam_pwquality の設定行
/etc/security/pwquality.conf → pwquality の詳細設定（別途設定可能）
/etc/shadow                  → 各ユーザーのハッシュと有効期限の実際の値
```

**デモンストレーション**:
```bash
# 有効期限の確認
sudo chage -l kaztakam
# → Last password change, Password expires, Minimum, Maximum, Warning

# 強度ポリシーの確認
grep pam_pwquality /etc/pam.d/common-password

# テスト: 弱いパスワードで変更を試みる
passwd
# → "abc" を入力 → 拒否（短すぎる）
# → "short1A" を入力 → 拒否（10文字未満）
# → "aaaaaAAAA1" を入力 → 拒否（3文字以上連続）

# 新規ユーザーの自動適用確認
sudo adduser testpolicy
sudo chage -l testpolicy
# → Max: 30, Min: 2, Warning: 7 が自動設定されている
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「login.defs を変更すれば既存ユーザーにも適用される」 | 新規ユーザーにのみ適用。既存ユーザーには chage が必要 |
| 「パスワードは暗号化されている」 | ハッシュ化されている。暗号化は復号可能だがハッシュは一方向 |
| 「minlen=10 は正確に10文字」 | 「最低10文字」であり、10文字以上を意味する |
| 「ucredit=-1 は大文字を1文字だけ含むこと」 | 「最低1文字」であり、1文字以上を意味する |
| 「root はパスワードポリシーの対象外」 | enforce_for_root を設定すれば root にも適用される |

---

#### Q7: sudo はどのように動作するか？設定を見せてください。

**短い版**:
sudo は一般ユーザーが一時的に root 権限でコマンドを実行する仕組み。sudoers 設定で権限を確認し、パスワード認証後にコマンドを実行。全操作をログに記録する。

**詳細版**:
sudo (Superuser Do) の動作フロー:
1. ユーザーが `sudo command` を実行
2. `/etc/sudoers` および `/etc/sudoers.d/` の設定を読み込む
3. そのユーザーに対象コマンドの実行権限があるか検証
4. `requiretty`: TTY の存在を確認（Web Shell からの不正な sudo 実行を防止）
5. PAM を通じてパスワード入力を要求（`passwd_tries=3` で3回まで）
6. `secure_path`: sudo 実行時の PATH を安全なディレクトリのみに制限
7. パスワードが正しければ、指定された権限でコマンドを実行
8. `logfile`, `log_input`, `log_output`: 全操作をログに記録
9. タイムスタンプを記録（デフォルト15分間は再入力不要）

**Born2beRoot の sudo 設定の各項目解説**:

| 設定項目 | 値 | 目的 | 攻撃への対策 |
|---------|---|------|------------|
| `passwd_tries` | 3 | パスワード入力を3回間違えると終了 | ブルートフォース緩和 |
| `badpass_message` | カスタム | 間違えた時のメッセージ | UX 向上（攻撃対策ではない） |
| `logfile` | /var/log/sudo/sudo.log | 全操作をログ | 監査・フォレンジック |
| `log_input` | 有効 | キーストロークを記録 | 操作の完全な再現が可能 |
| `log_output` | 有効 | 出力を記録 | 操作結果の確認が可能 |
| `requiretty` | 有効 | TTY がない環境からの実行を拒否 | Web Shell、スクリプト攻撃 |
| `secure_path` | 安全なパスのみ | PATH を制限 | PATH 注入攻撃 |

**sudoers ファイルの構文**:
```
# 書式: ユーザー ホスト=(実行ユーザー:実行グループ) コマンド
kaztakam ALL=(ALL:ALL) ALL

# グループの場合（%でグループ名を指定）
%sudo ALL=(ALL:ALL) ALL
```

**デモンストレーション**:
```bash
# sudo 設定の表示
sudo cat /etc/sudoers.d/sudo_config

# sudo ログの確認
sudo cat /var/log/sudo/sudo.log

# カスタムエラーメッセージの確認
sudo ls
# → わざと間違ったパスワードを入力 → カスタムメッセージが表示

# 現在のユーザーの sudo 権限確認
sudo -l

# sudo グループの確認
getent group sudo
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「sudo は root になるコマンド」 | sudo は root になるのではなく、特定のコマンドを root 権限で実行する。su が「root になる」 |
| 「sudoers を nano で直接編集する」 | visudo を使用すべき。構文チェックにより設定ミスによるロックアウトを防ぐ |
| 「requiretty は不要」 | Web シェルやスクリプトからの不正な権限昇格を防ぐために重要 |
| 「sudo ログは /var/log/auth.log のみ」 | Born2beRoot では /var/log/sudo/sudo.log にも記録するよう設定している |
| 「sudo のタイムアウトがあるからパスワードは1回入力すれば永久に有効」 | デフォルトで15分。期限が切れたら再入力が必要 |

---

#### Q8: UFW (Uncomplicated Firewall) はどのように動作するか？

**短い版**:
UFW は iptables/Netfilter のフロントエンド。受信パケットをルールと照合し、一致すれば許可/拒否。Born2beRoot ではデフォルト deny + port 4242 のみ allow。

**詳細版**:
UFW の動作は3つの層で構成される:

```
UFW（ユーザーフレンドリーなコマンド）
  ↓ 設定を変換
iptables（ルール管理ツール）
  ↓ ルールを登録
Netfilter（Linux kernel のパケットフィルタリングフレームワーク）
  ↓ パケットをフィルタリング
ネットワークインターフェース
```

**パケット処理の流れ**:
1. 受信パケットがネットワークインターフェースに到達
2. Netfilter が INPUT チェーンでパケットを捕捉
3. UFW のルールに基づいて先頭から順番に評価
4. ルールに一致すれば ALLOW（許可）または DENY（拒否）
5. どのルールにも一致しない場合、デフォルトポリシーを適用

**Born2beRoot のルール設計**:
- デフォルト受信: **deny**（全て拒否）→ 最小攻撃面の原則
- デフォルト送信: **allow**（全て許可）→ DNS、NTP、パッケージ DL のため
- 許可ルール: **port 4242/tcp のみ**（SSH 用）

**iptables チェーンの概念**:
- **INPUT**: サーバー宛ての受信パケット（SSH 接続など）
- **OUTPUT**: サーバーからの送信パケット（DNS 問い合わせなど）
- **FORWARD**: サーバーを通過するパケット（ルーター動作時）

**デモンストレーション**:
```bash
# UFW の状態とルール
sudo ufw status verbose

# ルールの番号付き表示（削除時に使用）
sudo ufw status numbered

# ポートの追加と削除のデモ
sudo ufw allow 8080
sudo ufw status
sudo ufw delete allow 8080
sudo ufw status

# 特定 IP からのみ許可するルール
sudo ufw allow from 192.168.1.0/24 to any port 4242

# iptables レベルでの確認
sudo iptables -L -n -v | head -20
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「UFW はファイアウォールそのもの」 | UFW はフロントエンド。実際のフィルタリングは Netfilter が行う |
| 「UFW を無効にすると全通信が遮断」 | 逆。UFW を無効にするとフィルタリングが行われず全通信が許可される |
| 「port 22 も開いている」 | Born2beRoot では 22 は使用しない。4242 のみ |
| 「outgoing を deny にすべき」 | サーバーの正常動作（DNS、パッケージ更新等）に outgoing が必要。高セキュリティ環境では deny にすることもある |

---

#### Q9: SSH (Secure Shell) はどのように動作するか？

**短い版**:
SSH は暗号化されたリモートアクセスプロトコル。TCP 接続確立 → 鍵交換（DH/ECDH）で共有秘密を確立 → サーバー認証 → ユーザー認証 → 暗号化セッション。Born2beRoot では port 4242、root ログイン禁止。

**詳細版**:
SSH の接続確立プロセス:
1. **TCP 接続**: クライアントが port 4242 に TCP 3-way handshake (SYN → SYN-ACK → ACK) で接続
2. **プロトコルバージョン交換**: SSH-2.0 を使用
3. **アルゴリズムネゴシエーション**: 暗号化方式（AES-256-GCM 等）、MAC、鍵交換方式を合意
4. **鍵交換**: Diffie-Hellman / ECDH で共有秘密を確立（第三者が傍受しても計算不可能）
5. **サーバー認証**: ホスト鍵のフィンガープリントを確認（MITM 攻撃防止）
6. **ユーザー認証**: パスワードまたは公開鍵
7. **暗号化セッション**: 以降の全通信が暗号化される

**Born2beRoot の SSH 設定** (`/etc/ssh/sshd_config`):
```
Port 4242              # デフォルト 22 ではなく 4242
PermitRootLogin no     # root ログイン禁止
```

**パスワード認証 vs 公開鍵認証**:

| 比較項目 | パスワード認証 | 公開鍵認証 |
|---------|-------------|-----------|
| セキュリティ | ブルートフォース可能 | 秘密鍵なしでは不可能 |
| ネットワーク上 | パスワードが（暗号化されて）流れる | 秘密鍵は流れない |
| 管理 | パスワードの定期変更必要 | 鍵の管理が必要 |
| 推奨 | テスト・学習環境 | 本番環境 |

**デモンストレーション**:
```bash
# SSH 設定の確認
grep -E "^Port|^PermitRootLogin" /etc/ssh/sshd_config

# SSH サービスの状態
sudo systemctl status sshd

# ポートのリスニング確認
sudo ss -tunlp | grep ssh

# 接続テスト
ssh kaztakam@localhost -p 4242

# root 拒否の確認
ssh root@localhost -p 4242
# → Permission denied

# 詳細ログ付き接続（デバッグ用）
ssh -v kaztakam@localhost -p 4242
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「SSH は port 22 で動作する」 | Born2beRoot では port 4242 に変更 |
| 「ポート変更でセキュリティが大幅に向上」 | security through obscurity。nmap で発見可能 |
| 「SSH は暗号化されない」 | SSH は全通信を暗号化。暗号化されないのは Telnet |
| 「sshd_config を変更したら自動反映」 | `sudo systemctl restart sshd` が必要 |
| 「SSH の鍵交換でパスワードがネットワークを流れる」 | 鍵交換とユーザー認証は別のフェーズ。公開鍵認証なら秘密鍵は流れない |

---

#### Q10: monitoring.sh はどのように動作するか？停止方法は？

**短い版**:
monitoring.sh はシステム情報を収集し wall で全端末にブロードキャスト。cron で10分ごとに実行。停止方法: (1) crontab からエントリ削除 (2) cron サービス停止 (3) 実行権限を外す。

**詳細版**:
各情報の取得方法:

| 情報 | コマンド | 説明 |
|-----|---------|------|
| アーキテクチャ | `uname -a` | カーネル、CPU アーキテクチャ |
| 物理 CPU 数 | `grep "physical id" /proc/cpuinfo \| sort -u \| wc -l` | 物理プロセッサ数 |
| 仮想 CPU 数 | `grep -c "^processor" /proc/cpuinfo` | 論理プロセッサ数 |
| メモリ使用量 | `free --mega` | RAM 使用量/合計 |
| ディスク使用量 | `df -m` | ディスク使用量/合計 |
| CPU 使用率 | `top -bn1 \| grep "Cpu(s)"` | idle 率から使用率を計算 |
| 最終ブート | `who -b` | 最終起動時刻 |
| LVM 有無 | `lsblk \| grep lvm` | LVM 使用の有無 |
| TCP 接続 | `ss -t state established` | 確立済み TCP 接続数 |
| ログインユーザー | `who \| wc -l` | ログイン中のユーザー数 |
| IP / MAC | `hostname -I`, `ip link` | ネットワークアドレス |
| sudo コマンド数 | `journalctl _COMM=sudo \| grep COMMAND` | sudo 実行回数 |

**cron の設定**:
```bash
sudo crontab -l
# → */10 * * * * /usr/local/bin/monitoring.sh
```
`*/10` は「10で割り切れる分」→ 0, 10, 20, 30, 40, 50分に実行。

**スクリプトを変更せずに停止する3つの方法**:

方法1（最も推奨）: crontab からエントリを削除/コメントアウト
```bash
sudo crontab -e
# → */10 の行を削除、または先頭に # を追加
```

方法2: cron サービス自体を停止
```bash
sudo systemctl stop cron
# ※ 全 cron ジョブに影響する
```

方法3: 実行権限を外す
```bash
sudo chmod -x /usr/local/bin/monitoring.sh
# ※ スクリプトの内容は変更していない（パーミッションのみ変更）
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「スクリプトを rm で削除する」 | 「スクリプトの内容を変更せず」に反する |
| 「systemctl stop monitoring」 | monitoring.sh は systemd サービスではなく cron ジョブ |
| 「kill コマンドで止める」 | 一時的に止まるが次の cron 実行時にまた動く |
| 「sleep コマンドで10分待っている」 | cron が10分ごとにスクリプトを起動する。スクリプト内に sleep はない |

---

#### Q11: LUKS (Linux Unified Key Setup) 暗号化とは何か？

**短い版**:
LUKS はディスク暗号化の標準仕様。パスフレーズから鍵導出関数で暗号鍵を導出し、AES-256 等でパーティション全体を暗号化する。物理的なディスク盗難からデータを保護する。

**詳細版**:
LUKS は Linux でのディスク暗号化の標準仕様で、dm-crypt カーネルモジュールを使用してブロックデバイスレベルで暗号化を行う。

**動作の仕組み**:
```
パスフレーズ → PBKDF2/Argon2（鍵導出関数）→ マスター鍵 → AES-256-XTS → ディスクデータ
```

1. LUKS ヘッダーにメタデータを格納
2. マスター鍵がデータの暗号化/復号に使用される
3. パスフレーズから鍵導出関数でマスター鍵を導出（ブルートフォースに対する耐性を確保）
4. 最大8つの鍵スロットで複数のパスフレーズからアクセス可能

**保護するシナリオ**:
- ディスクの物理的な盗難
- 廃棄されたディスクからのデータ復旧
- コロケーション環境でのディスク交換時のデータ漏洩

**保護しないシナリオ**:
- OS 起動後のリモート攻撃（復号済みの状態）
- root 権限を取得した攻撃者（復号済みファイルシステムにアクセス可能）
- ネットワーク経由のデータ（SSH/TLS が担当）
- メモリ上のデータ（暗号鍵はメモリに展開されている）

**デモンストレーション**:
```bash
# LUKS 情報の確認
sudo cryptsetup luksDump /dev/sda5 | head -20

# 暗号化デバイスの確認
ls /dev/mapper/

# lsblk で暗号化の階層を確認
lsblk
# → sda5 → sda5_crypt (crypt) → LVMGroup-* (lvm)
```

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| 「LUKS は暗号化アルゴリズム」 | LUKS は仕様/フォーマット。実際の暗号化は AES 等が行う |
| 「パスフレーズ = 暗号鍵」 | パスフレーズから鍵導出関数でマスター鍵を導出する |
| 「LUKS で全てのデータが安全」 | OS 起動後はデータが復号された状態なのでリモート攻撃には無力 |
| 「LUKS パスフレーズを忘れてもリカバリ可能」 | パスフレーズを忘れるとデータにアクセス不可能（これが暗号化の目的） |

---

#### Q12: /etc/passwd と /etc/shadow の違いは何か？

**短い版**:
/etc/passwd はユーザー基本情報（全員が読める、パーミッション 644）。/etc/shadow はパスワードハッシュと有効期限（root のみ、パーミッション 640）。分離することでオフライン攻撃を防ぐ。

**詳細版**:
`/etc/passwd` のフィールド:
```
username:x:UID:GID:GECOS:home_dir:shell
kaztakam:x:1000:1000:kaztakam:/home/kaztakam:/bin/bash
```
- `x`: パスワードが /etc/shadow に格納されていることを示す
- パーミッション: 644（全ユーザーが読み取り可能）

`/etc/shadow` のフィールド:
```
username:$hash:last_change:min:max:warn:inactive:expire
kaztakam:$6$salt$hash:19500:2:30:7:::
```
- `$6$`: SHA-512 アルゴリズム使用（`$5$`=SHA-256, `$y$`=yescrypt）
- パーミッション: 640 または 000（root のみ読み取り可能）

**分離の歴史的経緯**:
かつては /etc/passwd にハッシュが含まれていたが、全ユーザーが読めるファイルにハッシュを置くとオフライン攻撃（辞書攻撃、レインボーテーブル攻撃）が容易になるため、/etc/shadow に分離された。

---

### カテゴリ B: 実演系質問 (Q13-Q22)

---

#### Q13: 新しいユーザーを作成し、グループに追加してください。

**短い版**: `adduser` でユーザー作成、`usermod -aG` でグループ追加、`chage -l` でポリシー確認。

**詳細版の実演手順**:
```bash
# 1. 新しいユーザーの作成
sudo adduser eval42
# → パスワードポリシーに従うパスワードを設定
#    例: Eval42Test! (10文字以上、大文字+小文字+数字)

# 2. グループへの追加（-aG で追加。-G のみだと上書きされる！）
sudo usermod -aG sudo eval42
sudo usermod -aG user42 eval42

# 3. 確認
id eval42
# → uid=1001(eval42) gid=1001(eval42) groups=1001(eval42),27(sudo),1002(user42)

groups eval42
# → eval42 : eval42 sudo user42

# 4. パスワードポリシーの自動適用確認
sudo chage -l eval42
# → Maximum number of days between password change: 30
# → Minimum number of days between password change: 2
# → Number of days of warning before password expires: 7

# 5. sudo 権限の確認
su - eval42
sudo whoami
# → root

# 6. SSH 接続テスト（別ターミナルで）
ssh eval42@localhost -p 4242
```

**adduser vs useradd の違い**:
- `adduser`: Debian の高レベルスクリプト。対話的にパスワード設定、ホームディレクトリ作成、login.defs のデフォルト適用を行う
- `useradd`: 低レベルコマンド。オプションなしではホームディレクトリも作られない

**よくある間違い回答と修正**:

| 間違い | 修正 |
|--------|------|
| `usermod -G sudo eval42` | `-G` だけだと既存グループが上書き。`-aG` で追加する |
| 「adduser は useradd のエイリアス」 | adduser は Perl スクリプト（Debian）で useradd を内部で呼び出しつつ追加設定を行う |

---

#### Q14: ホスト名を変更してください。

```bash
hostname                                      # 現在のホスト名確認
sudo hostnamectl set-hostname newhostname42   # 変更
sudo nano /etc/hosts                          # 127.0.1.1 の行を更新
hostname                                      # 変更確認

# 元に戻す:
sudo hostnamectl set-hostname kaztakam42
sudo nano /etc/hosts                          # 元のホスト名に戻す
```

**重要**: /etc/hosts を更新しないと名前解決で問題が起きる可能性がある。

---

#### Q15: UFW にポートを追加し、削除してください。

```bash
# 追加
sudo ufw allow 8080
sudo ufw status numbered

# 削除（番号指定またはルール指定）
sudo ufw delete allow 8080
# または
sudo ufw delete 2  # 番号で削除

sudo ufw status
```

---

#### Q16: パスワードポリシーが機能していることを証明してください。

```bash
# テスト1: 短すぎるパスワード
passwd  # → "abc123A" を入力 → 拒否（10文字未満）

# テスト2: 大文字なし
passwd  # → "abcdefghij1" を入力 → 拒否（大文字がない）

# テスト3: 数字なし
passwd  # → "AbcdefghijK" を入力 → 拒否（数字がない）

# テスト4: 連続文字
passwd  # → "AAAAbcdefg1" を入力 → 拒否（4文字連続）

# 有効期限の確認
sudo chage -l kaztakam
```

---

#### Q17: sudo のカスタムエラーメッセージを確認してください。

```bash
sudo ls
# → 間違ったパスワードを入力
# → カスタムの badpass_message が表示される

# 3回間違えると終了（passwd_tries=3）
```

---

#### Q18: sudo ログが記録されていることを確認してください。

```bash
# ログファイルの確認
sudo cat /var/log/sudo/sudo.log
# → 日時、ユーザー、コマンド等が記録されている

# ログディレクトリの確認
ls -la /var/log/sudo/

# 入出力ログの確認（log_input/log_output が有効な場合）
ls /var/log/sudo/00/00/
```

---

#### Q19: SSH で root ログインが禁止されていることを確認してください。

```bash
# 設定の確認
grep PermitRootLogin /etc/ssh/sshd_config
# → PermitRootLogin no

# 実際に接続を試みる（ホスト側から）
ssh root@localhost -p 4242
# → Permission denied
```

---

#### Q20: cron の設定を確認してください。

```bash
# root の crontab
sudo crontab -l
# → */10 * * * * /usr/local/bin/monitoring.sh

# crontab の書式説明
# 分(0-59) 時(0-23) 日(1-31) 月(1-12) 曜日(0-7) コマンド
# */10 は「10分ごと」= 0, 10, 20, 30, 40, 50分
```

---

#### Q21: AppArmor が有効であることを確認してください。

```bash
sudo aa-status
# → プロファイル数が表示される
# → enforce モードのプロファイルが一覧される

sudo systemctl status apparmor
# → active (exited) と表示される
```

---

#### Q22: パーティション構成を確認してください。

```bash
lsblk
# → LUKS + LVM の階層が表示される
# sda
# ├─sda1         /boot
# └─sda2
#   └─sda5_crypt
#     └─LVMGroup
#       ├─root     /
#       ├─swap     [SWAP]
#       ├─home     /home
#       └─...

sudo pvdisplay  # PV の情報
sudo vgdisplay  # VG の情報
sudo lvdisplay  # LV の詳細
```

---

### カテゴリ C: 深い理解を問う質問 (Q23-Q40)

---

#### Q23: SSH のポートを 22 から変更する効果と限界は？

**短い版**: 自動ボットの大半が port 22 をターゲットにするため、ノイズを減らせる。ただし security through obscurity であり、nmap でポートスキャンすれば発見可能。他の対策（公開鍵認証、Fail2ban 等）と組み合わせる必要がある。

**詳細版**:
**効果**: インターネット上のボットの約99%が port 22 をターゲットにブルートフォース攻撃を行う。ポート変更でこれらの無差別攻撃のノイズを大幅に減らせる。ログの可読性も向上する。

**限界**: ポートスキャンツール（nmap 等）で `nmap -sV -p 4242 target` を実行すれば、SSH サービスを容易に発見可能。したがって、標的型攻撃に対しては効果がない。

**組み合わせるべき対策**:
- 公開鍵認証（パスワード認証の無効化）
- Fail2ban（ブルートフォース検知と自動ブロック）
- MaxAuthTries の制限
- AllowUsers / AllowGroups による接続ユーザーの制限

---

#### Q24: requiretty は何を防ぐか？具体的なシナリオは？

**短い版**: Web Shell からの sudo 実行を防ぐ。

**詳細版**:
**攻撃シナリオ**:
1. Web アプリケーション（例: PHP）に脆弱性がある
2. 攻撃者が HTTP 経由でコマンド実行を行う（Remote Code Execution）
3. Web サーバープロセスは TTY を持たない
4. `requiretty` がなければ `sudo` を通じて権限昇格が可能
5. `requiretty` があれば TTY の不在を検知して sudo が拒否される

**注意**: `requiretty` は Ansible 等の自動化ツールとの互換性問題がある。本番環境では用途に応じて判断が必要。

---

#### Q25: secure_path は何を防ぐか？攻撃例は？

**短い版**: PATH 注入攻撃を防ぐ。

**詳細版**:
```bash
# 攻撃シナリオ
# 1. 攻撃者がユーザーのホームに悪意あるスクリプトを配置
echo '#!/bin/bash
/bin/bash -i' > /home/user/ls
chmod +x /home/user/ls

# 2. PATH を汚染（.bashrc 改ざんなど）
export PATH=/home/user:$PATH

# 3. sudo ls を実行すると...
# secure_path なし → /home/user/ls が root 権限で実行される！
# secure_path あり → /usr/bin/ls が実行される（安全）
```

---

#### Q26: LUKS が保護するもの・しないものは？

**保護する**: ディスクの物理的盗難、廃棄時のデータ漏洩、コールドブート時のデータ保護
**保護しない**: OS 起動後のリモート攻撃、root 権限取得後のアクセス、ネットワーク通信（SSH/TLS が担当）、メモリ上のデータ

---

#### Q27: なぜ /boot を暗号化しないのか？

GRUB がカーネルをロードするために /boot にアクセスする必要があるが、この時点では LUKS パスフレーズがまだ入力されていない。順序: GRUB → /boot からカーネルロード → カーネルが LUKS パスフレーズを要求 → 復号。/boot も暗号化されていたら、GRUB がカーネルを読み込めない。

---

#### Q28: DAC と MAC の根本的な違いは？

**DAC (Discretionary Access Control)**: リソースの所有者がアクセス権を決定する。「誰がファイルにアクセスできるか」。Linux の通常のパーミッション（rwx）。

**MAC (Mandatory Access Control)**: システム管理者がポリシーを定義する。「プログラムが何にアクセスできるか」。root でもポリシー違反は拒否される。AppArmor、SELinux が実装。

---

#### Q29: パスワードはどのように保存されるか？salt の役割は？

**ハッシュ化**: 一方向関数（SHA-512）でパスワードを固定長のハッシュ値に変換。復号不可能。

**salt の役割**: ランダムな値をパスワードに追加してからハッシュ化。同じパスワードでも異なるハッシュ値になる。レインボーテーブル（事前計算済み辞書）攻撃を無効化する。

```
/etc/shadow の形式: $6$randomsalt$longhashvalue
                     ^  ^            ^
                     |  |            └── ハッシュ値
                     |  └── salt
                     └── アルゴリズム（6=SHA-512）
```

---

#### Q30: sudo と su の違いは？なぜ sudo を使うのか？

| 比較項目 | su | sudo |
|---------|-----|------|
| 認証 | 切替先ユーザーのパスワード | 自分のパスワード |
| 権限範囲 | ユーザー全体に切替 | 個別コマンドのみ |
| ログ | 切替のみ記録 | 各コマンドを記録（監査性が高い） |
| 制御 | 全権限付与 | sudoers でコマンド単位で制御可能 |
| root パスワード | 共有が必要 | 不要（各自のパスワードで認証） |

Born2beRoot で sudo を使用する理由: **最小権限**（必要なコマンドだけ昇格）、**監査**（誰が何をしたかログに残る）、**制御**（コマンド制限可能）、**root パスワード不要**。

---

#### Q31: crontab の各フィールドの意味は？

```
分(0-59) 時(0-23) 日(1-31) 月(1-12) 曜日(0-7) コマンド

# Born2beRoot
*/10 * * * * /usr/local/bin/monitoring.sh
# → 0,10,20,30,40,50分に実行

# 例: 毎月15日の午前3時30分
30 3 15 * * /path/to/command

# 例: 毎週月曜の午前9時
0 9 * * 1 /path/to/command

# 例: 毎日午前0時
0 0 * * * /path/to/command
```

---

#### Q32: /proc ファイルシステムとは何か？

仮想ファイルシステムで、カーネルがランタイムに動的に生成する情報を提供する。ディスク上にファイルは存在しない。

重要な /proc エントリ:
- `/proc/cpuinfo`: CPU 情報 → monitoring.sh で使用
- `/proc/meminfo`: メモリ情報 → free コマンドが内部で参照
- `/proc/uptime`: 稼働時間
- `/proc/loadavg`: ロードアベレージ
- `/proc/net/`: ネットワーク統計
- `/proc/[PID]/`: 各プロセスの情報

---

#### Q33: systemd の役割と PID 1 の特殊性は？

systemd は Linux の init システムで PID 1 として動作する。

PID 1 の特殊性:
- カーネルから最初に起動されるプロセス
- 孤児プロセスの親となる（ゾンビプロセスの回収）
- SIGKILL で殺せない
- 全サービスのライフサイクルを管理

```bash
# サービス管理
sudo systemctl status sshd
sudo systemctl start/stop/restart sshd
sudo systemctl enable/disable sshd

# ログ管理
sudo journalctl -u sshd
sudo journalctl -f  # リアルタイム追跡
```

---

#### Q34: ブートプロセスの全体像は？

Born2beRoot の構成に即した流れ:
1. **BIOS/UEFI**: ハードウェア初期化、POST
2. **GRUB**: MBR → /boot → カーネルと initramfs をロード
3. **initramfs**: 一時ルートFS。LUKS 復号と LVM 認識に必要なモジュールを含む
4. **LUKS パスフレーズ**: ユーザーが入力、dm-crypt が復号
5. **LVM 認識**: VG/LV が認識される
6. **ルートFS マウント**: 実際の / にスイッチ
7. **systemd**: PID 1 として起動、各サービスを並列起動
8. **サービス起動**: sshd, ufw, cron, apparmor
9. **ログインプロンプト**: SSH 接続受付可能

---

#### Q35: /var と /tmp を別パーティションにする理由は？

| パーティション | 分離の理由 | セキュリティオプション |
|--------------|-----------|-------------------|
| `/var` | ログ肥大化が / に影響しない | - |
| `/tmp` | 一時ファイルの暴走防止 | noexec, nosuid, nodev |
| `/home` | ユーザーデータの暴走防止 | nosuid |
| `/var/log` | ログ専用で容量管理 | - |

`/tmp` に `noexec` を設定すると、一時ファイルとして配置された悪意あるプログラムの実行を防止できる。

---

#### Q36: wall コマンドの動作原理は？

wall (Write All) は全ログインユーザーの端末（/dev/pts/*）にメッセージをブロードキャストする。`mesg n` でメッセージ受信を無効にしているユーザーには表示されない。

---

#### Q37: dpkg と apt の関係は？`apt install vim` で何が起こるか？

```
ユーザー → apt → リポジトリ → .deb ダウンロード → dpkg → インストール
```

`apt install vim` の内部動作:
1. `/etc/apt/sources.list` のリポジトリからパッケージ情報を検索
2. vim の依存パッケージ（vim-common, vim-runtime 等）を特定
3. 依存パッケージも含めて .deb ファイルをダウンロード
4. dpkg を呼び出して各 .deb をインストール
5. postinst スクリプトを実行

---

#### Q38: PAM (Pluggable Authentication Modules) の仕組みは？

PAM は認証をモジュール化したフレームワーク。アプリケーション（login, ssh, sudo）と認証メカニズムを分離する。

4つのモジュールタイプ:
1. **auth**: ユーザー認証（パスワード確認）
2. **account**: アカウント有効性確認（期限切れ等）
3. **password**: パスワード変更処理（pam_pwquality がここで動作）
4. **session**: セッション管理（ログ記録等）

制御フラグ:
- **required**: 失敗しても次のモジュールを実行し、最終的に失敗
- **requisite**: 失敗したら即座に全体が失敗
- **sufficient**: 成功したら以降をスキップして成功
- **optional**: 結果が全体の成否に影響しない

---

#### Q39: visudo を使う理由は？直接編集ではだめか？

visudo は sudoers ファイル編集前に構文チェックを行う。直接編集で構文エラーがあると、sudo が使用不能になり、root 以外からシステム管理ができなくなる（**ロックアウト**）。visudo はエラーを検出すると保存を拒否して修正を促す。

```bash
# 正しい方法
sudo visudo
sudo visudo -f /etc/sudoers.d/sudo_config

# 危険な方法（非推奨）
sudo nano /etc/sudoers  # 構文チェックなし
```

---

#### Q40: Born2beRoot の「多層防御」の設計は？

| 層 | 実装 | 防御対象 |
|---|------|---------|
| 物理層 | LUKS 暗号化 | ディスク盗難 |
| ネットワーク層 | UFW（port 4242 のみ） | 不正な接続 |
| 認証層 | パスワードポリシー + SSH root 禁止 | ブルートフォース |
| 権限層 | sudo 制限 + ログ記録 | 権限昇格攻撃 |
| プロセス層 | AppArmor | 脆弱性の悪用 |
| 監視層 | monitoring.sh + sudo ログ | 異常検知 |

一つの層が突破されても次の層が防御する。例: UFW をバイパスして SSH に到達しても、強力なパスワードが必要。パスワードを推測しても、sudo 制限とログ記録がある。

---

### カテゴリ D: シナリオ問題 (Q41-Q55)

---

#### Q41: 朝出社したらサーバーに SSH 接続できない。原因調査手順は？

```bash
# 1. VirtualBox コンソールからローカルログイン
# 2. ネットワーク確認
ip addr show
ping 8.8.8.8

# 3. SSH サービス確認
sudo systemctl status sshd

# 4. ポート確認
sudo ss -tunlp | grep 4242

# 5. UFW 確認
sudo ufw status

# 6. SSH 設定確認
cat /etc/ssh/sshd_config

# 7. ログ確認
sudo journalctl -u sshd -n 50

# 8. ディスク容量確認（満杯だと sshd が起動できない場合がある）
df -h

# 9. DNS 確認
cat /etc/resolv.conf
```

---

#### Q42: monitoring.sh の TCP 接続が 0 → 15 に急増した。調査手順は？

```bash
# 1. 接続の詳細確認
sudo ss -tunp state established

# 2. 不明な接続元 IP やプロセスの特定
# 3. SSH ログの確認
sudo journalctl -u sshd -n 100

# 4. ログインユーザーの確認
who
last -n 20

# 5. 不正アクセスの疑いがある場合
sudo ufw deny from <suspicious_IP>
sudo kill <suspicious_PID>
passwd  # パスワード変更
```

---

#### Q43: /var/log パーティションが 95% に達した。対処方法は？

**即時対応**:
```bash
df -h /var/log
sudo du -sh /var/log/* | sort -hr | head -10
sudo journalctl --vacuum-time=7d
sudo logrotate -f /etc/logrotate.conf
```

**長期的対応**:
```bash
# LVM で LV を拡張（VG に空きがある場合）
sudo lvextend -L +2G /dev/LVMGroup/var--log
sudo resize2fs /dev/LVMGroup/var--log

# ログローテーション設定の見直し
sudo nano /etc/logrotate.d/rsyslog
```

---

#### Q44: root パスワードを忘れた場合のリカバリは？

1. VM を再起動し、GRUB メニューで `e` キー
2. `linux` 行の末尾に `init=/bin/bash` を追加
3. `Ctrl+X` でブート
4. `mount -o remount,rw /` でルートを書き込み可能にマウント
5. `passwd root` でパスワード変更
6. `exec /sbin/init` で再起動

**重要**: LUKS パスフレーズを忘れた場合はデータにアクセスする方法はない。

---

#### Q45: 新しいサービス（Web サーバー等）追加時のセキュリティチェックリストは？

1. そのサービスは本当に必要か？（最小攻撃面）
2. UFW で必要なポートのみ開放
3. サービスを非 root ユーザーで動作させる
4. 不要なモジュール/機能を無効化
5. AppArmor プロファイルを確認/作成
6. ログ設定を確認
7. 定期的なセキュリティアップデートの計画
8. テスト環境で検証してから本番に適用

---

#### Q46: この sshd_config の問題点を指摘せよ

```
Port 22
PermitRootLogin yes
PasswordAuthentication yes
MaxAuthTries 10
```

問題点:
1. **Port 22**: デフォルトポート。ボットの標的
2. **PermitRootLogin yes**: root 直接ログイン可能。パスワード漏洩で即全権限奪取
3. **MaxAuthTries 10**: 試行回数が多すぎる。ブルートフォースに弱い
4. 公開鍵認証が有効化されていない
5. AllowUsers/AllowGroups によるアクセス制限がない

---

#### Q47: LVM で /home を 2GB 拡張する手順は？

```bash
# VG の空き容量確認
sudo vgdisplay LVMGroup | grep "Free"

# LV を拡張
sudo lvextend -L +2G /dev/LVMGroup/home

# ファイルシステムを拡張
sudo resize2fs /dev/LVMGroup/home

# 確認
df -h /home
```

---

#### Q48: UFW で特定の IP サブネットからのみ SSH を許可するルールは？

```bash
# 現在のルールをリセット
sudo ufw reset

# 特定サブネットからのみ port 4242 を許可
sudo ufw allow from 192.168.1.0/24 to any port 4242

# デフォルトポリシー
sudo ufw default deny incoming
sudo ufw default allow outgoing

sudo ufw enable
sudo ufw status verbose
```

---

#### Q49: swap が暗号化されていない場合のリスクは？

メモリ上の機密データ（パスワード、暗号鍵、個人情報）がスワップ領域にページアウトされる可能性がある。ディスク盗難時にスワップ領域からこれらの機密データを復元可能。Born2beRoot では LVM 全体が LUKS で暗号化されているため、スワップも暗号化される。

---

#### Q50: CPU 使用率が突然 95% になった。何が考えられるか？

可能性:
1. 暗号通貨マイニングマルウェア
2. DoS 攻撃によるリソース消費
3. アプリケーションのバグ（無限ループ）
4. 正当な重い処理（アップデート、バックアップ）

調査:
```bash
top                         # リアルタイム CPU 消費
ps aux --sort=-%cpu | head  # CPU 消費上位プロセス
sudo journalctl -n 50      # 最近のログ
```

---

#### Q51: enforce_for_root の目的は？

pam_pwquality の `enforce_for_root` は、root にもパスワード強度ポリシーを適用する設定。デフォルトでは root はポリシーを無視してパスワードを設定できる（警告は出るが強制されない）。この設定により root のパスワードも10文字以上、大文字+小文字+数字等の要件を満たす必要がある。

---

#### Q52: cron と systemd timer の違いは？

| 比較項目 | cron | systemd timer |
|---------|------|---------------|
| 設定方法 | crontab (5フィールド) | .timer + .service ファイル |
| 精度 | 分単位 | 秒単位 |
| ブート後実行 | 不可 | OnBootSec で可能 |
| ログ | syslog | journalctl に統合 |
| 依存関係 | なし | systemd の依存関係管理 |

Born2beRoot では subject の要件に従い cron を使用。

---

#### Q53: Fail2ban の仕組みは？（Bonus 用）

Fail2ban はログファイル（auth.log 等）を監視し、失敗したログイン試行を検知して攻撃元 IP を自動的にブロックする。

```ini
[sshd]
enabled = true
port = 4242
maxretry = 3     # 3回失敗でブロック
bantime = 600    # 10分間ブロック
findtime = 600   # 10分以内の失敗をカウント
```

```bash
sudo fail2ban-client status sshd
```

---

#### Q54: WordPress の構成要素は？（Bonus 用）

```
クライアント → Lighttpd (port 80) → PHP-FPM → WordPress → MariaDB
```

- **Lighttpd**: 軽量 Web サーバー
- **PHP-FPM**: PHP スクリプトの実行
- **MariaDB**: MySQL 互換 DB
- **WordPress**: CMS アプリケーション

---

#### Q55: Born2beRoot で学んだことを実務にどう活かすか？

| Born2beRoot のスキル | 実務への発展 |
|--------------------|------------|
| 手動設定の経験 | Ansible/Terraform で自動化 |
| UFW | クラウドのセキュリティグループ |
| SSH 設定 | 公開鍵認証、bastion ホスト |
| AppArmor | Docker セキュリティプロファイル |
| LVM | クラウドのブロックストレージ |
| monitoring.sh | Prometheus/Grafana |
| パスワードポリシー | IDaaS (Auth0, Okta) |
| cron | systemd timer, Airflow |

---

## 3. 評価中のよくある失敗と対策

| # | 失敗事例 | 対策 | 重要度 |
|---|---------|------|-------|
| 1 | signature.txt のハッシュが一致しない | VM を停止してからハッシュ生成。生成後に VM を起動しない | 致命的 |
| 2 | root で SSH 接続できてしまう | `PermitRootLogin no` を確認し sshd を再起動 | 高 |
| 3 | UFW で他のポートが開いている | `sudo ufw status` で不要なルールを確認 | 高 |
| 4 | パスワードポリシーが既存ユーザーに適用されていない | `chage` で既存ユーザーにも設定 | 高 |
| 5 | sudo ログが出力されない | ディレクトリ /var/log/sudo/ の存在と sudoers 設定を確認 | 中 |
| 6 | monitoring.sh でエラーが出る | `bash -x monitoring.sh` でデバッグ | 中 |
| 7 | 仕組みを説明できない | 「なぜ」「どのように」を理解する | 高 |
| 8 | 新規ユーザーのポリシーが未適用 | login.defs の設定が正しいか確認 | 中 |
| 9 | ホスト名変更後に /etc/hosts を更新していない | 両方変更が必要 | 中 |
| 10 | AppArmor のプロファイル数が 0 | AppArmor の正しいインストール・有効化を確認 | 高 |
| 11 | UFW が enabled になっていない | `sudo ufw enable` を忘れない | 高 |
| 12 | SSH ポートフォワーディング未設定 | VirtualBox のネットワーク設定を確認 | 高 |
| 13 | crontab が一般ユーザーのもの | `sudo crontab -l` で root の crontab を確認 | 中 |
| 14 | usermod で `-aG` ではなく `-G` を使用 | -G だけだと既存グループ所属が上書きされる | 高 |
| 15 | monitoring.sh が wall ではなく echo を使用 | wall コマンドで全端末にブロードキャストする必要がある | 中 |

---

## 4. よくある間違い回答とその修正

| # | 間違い回答 | 正しい回答 |
|---|-----------|-----------|
| 1 | 「SSH のポート変更でセキュリティが大幅に向上する」 | security through obscurity。nmap で発見可能。他の対策との組み合わせが重要 |
| 2 | 「LVM はデータのバックアップ機能」 | LVM はストレージの抽象化・柔軟管理。スナップショットはあるがバックアップ代替ではない |
| 3 | 「AppArmor はファイアウォール」 | AppArmor は MAC（プロセスレベルのアクセス制御）。ファイアウォールはネットワークレベル |
| 4 | 「LUKS はネットワーク通信も暗号化する」 | LUKS はディスク上のデータのみ。通信の暗号化は SSH/TLS |
| 5 | 「cron を止めれば monitoring.sh が止まる」 | 全 cron ジョブに影響。crontab のエントリ削除が適切 |
| 6 | 「VM はコンテナと同じ」 | VM は完全な OS を動かす。コンテナはホストカーネルを共有 |
| 7 | 「パスワードは暗号化されている」 | ハッシュ化されている（一方向）。暗号化は復号可能 |
| 8 | 「login.defs の変更は全ユーザーに即適用」 | 新規ユーザーにのみ適用。既存ユーザーには chage が必要 |
| 9 | 「sudo は root になるコマンド」 | sudo は特定コマンドを root 権限で実行。su が「root になる」 |
| 10 | 「/proc はディスク上のファイル」 | /proc は仮想FS。カーネルが動的に生成する情報 |

---

## 5. 評価前の最終チェックスクリプト

```bash
#!/bin/bash
echo "========================================"
echo "  Born2beRoot 評価前チェック"
echo "========================================"

echo ""
echo "=== 1. OS ==="
cat /etc/os-release | head -2
uname -r

echo ""
echo "=== 2. GUI なし ==="
dpkg -l 2>/dev/null | grep -cE "xorg|wayland|x11" && echo "GUI found!" || echo "No GUI - OK"

echo ""
echo "=== 3. ホスト名 ==="
hostname

echo ""
echo "=== 4. ユーザー & グループ ==="
id $(whoami)
echo "sudo group: $(getent group sudo)"
echo "user42 group: $(getent group user42)"

echo ""
echo "=== 5. パーティション ==="
lsblk

echo ""
echo "=== 6. SSH ==="
grep -E "^Port|^PermitRootLogin" /etc/ssh/sshd_config
sudo ss -tunlp | grep ssh

echo ""
echo "=== 7. UFW ==="
sudo ufw status

echo ""
echo "=== 8. パスワードポリシー ==="
echo "-- login.defs --"
grep "^PASS_" /etc/login.defs
echo "-- pam_pwquality --"
grep pam_pwquality /etc/pam.d/common-password
echo "-- chage --"
sudo chage -l $(whoami) | head -6

echo ""
echo "=== 9. sudo 設定 ==="
sudo cat /etc/sudoers.d/sudo_config 2>/dev/null || echo "No custom sudoers file"

echo ""
echo "=== 10. AppArmor ==="
sudo aa-status 2>/dev/null | head -5

echo ""
echo "=== 11. cron ==="
sudo crontab -l

echo ""
echo "=== 12. monitoring.sh ==="
ls -la /usr/local/bin/monitoring.sh
sudo bash /usr/local/bin/monitoring.sh 2>/dev/null

echo ""
echo "=== 13. sudo ログ ==="
ls -la /var/log/sudo/ 2>/dev/null || echo "No sudo log directory"

echo ""
echo "========================================"
echo "  チェック完了"
echo "========================================"
```

---

## 6. 質問対策のための学習フレームワーク

### 6.1 WHAT-WHY-HOW-DEMO フレームワーク

各技術要素について4つの観点で準備する:
1. **WHAT（何か）**: 定義と基本概念
2. **WHY（なぜ必要か）**: 解決する問題
3. **HOW（どう動くか）**: 内部の動作メカニズム
4. **DEMO（見せて）**: 実際のコマンドによる動作確認

### 6.2 評価者がよく聞く「〇〇と△△の違い」

準備すべき比較:
- VM vs コンテナ
- apt vs aptitude
- su vs sudo
- DAC vs MAC
- AppArmor vs SELinux
- MBR vs GPT
- パスワード認証 vs 公開鍵認証
- cron vs systemd timer
- /etc/passwd vs /etc/shadow
- Type 1 vs Type 2 Hypervisor
- adduser vs useradd
- Debian vs Rocky Linux

### 6.3 シナリオ質問への回答フレームワーク

シナリオ質問には以下の順で回答する:
1. **状況の確認**: 何が起こっているかを正確に把握するコマンド
2. **原因の特定**: 考えられる原因を列挙し、一つずつ確認
3. **即時対応**: 被害を最小化する応急処置
4. **恒久対応**: 再発防止策

---

## 7. Bonus 部分の評価ポイント

Bonus は必須部分が**完璧**な場合のみ評価される。一つでも必須要件に問題があれば Bonus は0点。

### 7.1 パーティション構成（Bonus）

```
sda
├─sda1         /boot
└─sda2
  └─sda5_crypt (LUKS)
    └─LVMGroup
      ├─root     /
      ├─swap     [SWAP]
      ├─home     /home
      ├─var      /var
      ├─srv      /srv
      ├─tmp      /tmp
      └─var--log /var/log
```

### 7.2 WordPress 構成

```bash
sudo systemctl status lighttpd
sudo systemctl status mariadb
php -v
curl http://localhost
```

### 7.3 追加サービス（Fail2ban 等）

```bash
sudo systemctl status fail2ban
sudo fail2ban-client status
sudo fail2ban-client status sshd
```

---

## 8. 追加の防御質問（発展編）

以下は評価の必須質問ではないが、深い理解を示すために知っておくと有益な追加質問である。

---

### Q56: Netfilter のフックポイント (Hook Points) を説明してください。

**短い版**:
Netfilter には5つのフックポイントがある: PREROUTING, INPUT, FORWARD, OUTPUT, POSTROUTING。パケットの処理段階ごとにルールを適用できる。

**詳細版**:
```
           ┌──────────────────────────────────────┐
           │          Linux Kernel                  │
           │                                        │
  受信 ──→ │ PREROUTING → ルーティング判断           │
           │                  │                      │
           │          ┌──────┴──────┐                │
           │          ▼             ▼                │
           │       INPUT        FORWARD              │
           │          │             │                │
           │          ▼             │                │
           │    ローカルプロセス     │                │
           │          │             │                │
           │          ▼             │                │
           │       OUTPUT          │                │
           │          │             │                │
           │          └──────┬──────┘                │
           │                 ▼                       │
           │           POSTROUTING ──→ 送信          │
           └──────────────────────────────────────┘
```

- **PREROUTING**: パケット受信直後。DNAT（宛先 NAT）の処理
- **INPUT**: ローカルプロセス宛てのパケット。UFW の incoming ルール
- **FORWARD**: 他のホストに転送するパケット。ルーター動作時
- **OUTPUT**: ローカルプロセスから送信するパケット。UFW の outgoing ルール
- **POSTROUTING**: パケット送信直前。SNAT（送信元 NAT）の処理

UFW は主に INPUT と OUTPUT チェーンにルールを設定する。

---

### Q57: ファイルシステムのジャーナリング (Journaling) とは何か？

**短い版**:
メタデータの変更をジャーナル（ログ領域）に先行書き込みする仕組み。突然の電源断でもファイルシステムの整合性を保つ。

**詳細版**:
ext4 のジャーナリングの動作:
1. ファイルへの書き込み要求が発生
2. まずジャーナル領域にメタデータの変更を書き込む（トランザクション開始）
3. 実際のデータ/メタデータをディスクに書き込む
4. ジャーナルにトランザクション完了を記録

もしステップ3の途中で電源が断たれた場合:
- ジャーナルにトランザクション完了が記録されていない
- 次回起動時にジャーナルをリプレイして整合性を回復
- fsck（ファイルシステムチェック）の時間を大幅に短縮

ext4 のジャーナルモード:
- **journal**: データとメタデータの両方をジャーナリング（最も安全、最も遅い）
- **ordered**: メタデータのみジャーナリング、データは先に書き込み（デフォルト）
- **writeback**: メタデータのみジャーナリング、データの順序は保証しない（最速）

---

### Q58: inode (Index Node) とは何か？

**短い版**:
inode はファイルのメタデータ（所有者、パーミッション、サイズ、データブロックの位置等）を格納するデータ構造。ファイル名は inode に含まれない（ディレクトリエントリが名前と inode 番号を対応させる）。

**詳細版**:
```
ディレクトリエントリ: "myfile.txt" → inode #12345
                                        │
                                        ▼
                                    inode #12345
                                    ├── 所有者: kaztakam
                                    ├── パーミッション: 644
                                    ├── サイズ: 1024 bytes
                                    ├── タイムスタンプ: atime, mtime, ctime
                                    ├── リンク数: 1
                                    └── データブロックポインタ:
                                        ├── block 100
                                        ├── block 101
                                        └── block 102
```

**ハードリンクとシンボリックリンクの違い**:
- **ハードリンク**: 同じ inode を指す別の名前。`ln file1 file2`
- **シンボリックリンク**: 別の inode を持ち、パス文字列を格納。`ln -s file1 file2`

**inode の枯渇問題**:
ディスクの容量が残っていても、inode が不足するとファイルを作成できない。小さなファイルが大量にある場合に発生しうる。
```bash
# inode の使用状況確認
df -i
```

---

### Q59: TCP 3-way Handshake を説明してください。

**短い版**:
TCP 接続確立の3ステップ: クライアントが SYN → サーバーが SYN-ACK → クライアントが ACK。この後にデータ通信が始まる。

**詳細版**:
```
クライアント                          サーバー
    │                                    │
    │──── SYN (seq=x) ─────────────────→│  ① 接続要求
    │                                    │
    │←─── SYN-ACK (seq=y, ack=x+1) ────│  ② 要求受諾+応答
    │                                    │
    │──── ACK (ack=y+1) ───────────────→│  ③ 確認応答
    │                                    │
    │←──── データ通信開始 ──────────────→│
```

**SYN フラッド攻撃 (SYN Flood Attack)**:
攻撃者が大量の SYN パケットを送信し、SYN-ACK の応答を待つサーバーのリソースを消費する DDoS 攻撃の一種。サーバーは半開き接続（half-open connection）を大量に保持し、メモリを消費する。

対策:
- **SYN Cookies**: サーバーが SYN-ACK に特殊な値を埋め込み、ACK で検証
- **接続数の制限**: UFW/iptables でレート制限
- **Fail2ban**: 異常な接続パターンの検知とブロック

---

### Q60: DNS の名前解決プロセスを説明してください。

**短い版**:
クライアントが DNS リゾルバに問い合わせ → ルートサーバー → TLD サーバー → 権威サーバーの順で再帰的に名前解決を行う。結果はキャッシュされる。

**詳細版**:
`apt install vim` を実行した時の DNS 名前解決:
1. apt が `deb.debian.org` の IP アドレスを要求
2. `/etc/resolv.conf` に記載された DNS リゾルバに問い合わせ
3. リゾルバがキャッシュを確認。なければ再帰問い合わせ開始
4. ルートサーバー（.） → `.org` TLD サーバー → `debian.org` 権威サーバー
5. IP アドレスが返され、キャッシュに保存
6. apt がその IP にHTTP接続を行い、パッケージをダウンロード

```bash
# DNS の設定確認
cat /etc/resolv.conf

# 名前解決のテスト
nslookup deb.debian.org
dig deb.debian.org

# DNS キャッシュのクリア（systemd-resolved の場合）
sudo systemd-resolve --flush-caches
```

---

### Q61: VirtualBox のポートフォワーディングはどう動作するか？

**短い版**:
ホスト OS の特定のポート（例: 4242）への接続を、ゲスト OS の指定ポート（例: 4242）に転送する NAT の機能。

**詳細版**:
```
ホスト OS                    VirtualBox NAT              ゲスト OS
                             エンジン
localhost:4242  ──────→  ポートフォワーディング  ──────→  10.0.2.15:4242
                             (NAT テーブル)               (ゲストの SSH)
```

Born2beRoot では NAT モードを使用し、ポートフォワーディングで SSH 接続を可能にする:
- ホスト側: 127.0.0.1:4242
- ゲスト側: 10.0.2.15:4242（VirtualBox のデフォルト NAT アドレス）

VirtualBox のポートフォワーディング設定:
```
設定 → ネットワーク → アダプター1 → 高度 → ポートフォワーディング
名前: SSH
プロトコル: TCP
ホスト IP: 127.0.0.1 (または空欄)
ホストポート: 4242
ゲスト IP: 10.0.2.15 (または空欄)
ゲストポート: 4242
```

---

### Q62: logrotate の役割と設定を説明してください。

**短い版**:
ログファイルを定期的にローテーション（古いログを圧縮・アーカイブ・削除）して、ディスク容量の枯渇を防ぐ。

**詳細版**:
logrotate の設定例（`/etc/logrotate.d/rsyslog`）:
```
/var/log/syslog {
    rotate 7          # 7世代保持
    daily             # 毎日ローテーション
    missingok         # ファイルがなくてもエラーにしない
    notifempty        # 空ファイルはローテーションしない
    delaycompress     # 圧縮を1ローテーション遅延
    compress          # 古いログを gzip 圧縮
    postrotate        # ローテーション後に実行するコマンド
        /usr/lib/rsyslog/rsyslog-rotate
    endscript
}
```

ローテーション後のファイル:
```
/var/log/syslog          ← 現在のログ
/var/log/syslog.1        ← 1日前（未圧縮）
/var/log/syslog.2.gz     ← 2日前（圧縮）
/var/log/syslog.3.gz     ← 3日前（圧縮）
...
/var/log/syslog.7.gz     ← 7日前（圧縮、次回ローテーションで削除）
```

```bash
# 手動でローテーションを実行
sudo logrotate -f /etc/logrotate.conf

# ローテーションのテスト（実行はしない）
sudo logrotate -d /etc/logrotate.conf
```

---

### Q63: Diffie-Hellman 鍵交換の概要を説明してください。

**短い版**:
盗聴者が通信を傍受しても共有秘密を導出できない鍵交換プロトコル。両者が秘密値を生成し、公開値を交換して同じ共有秘密を計算する。

**詳細版**:
```
アリス（クライアント）              ボブ（サーバー）

公開パラメータ: 素数 p, 生成元 g（両者が知っている）

秘密値 a を生成                    秘密値 b を生成
A = g^a mod p を計算               B = g^b mod p を計算

        ──── A を送信 ────→
        ←──── B を送信 ────

共有秘密 = B^a mod p               共有秘密 = A^b mod p
         = (g^b)^a mod p                    = (g^a)^b mod p
         = g^(ab) mod p                     = g^(ab) mod p
         ↑ 同じ値！                         ↑ 同じ値！
```

盗聴者は A (= g^a mod p) と B (= g^b mod p) を見ても、離散対数問題の困難性により a と b を導出できない。したがって共有秘密 g^(ab) mod p を計算できない。

SSH ではこの共有秘密からセッション鍵を導出し、AES-256-GCM 等で通信を暗号化する。

---

### Q64: なぜパスワードに salt を付けるのか？レインボーテーブル攻撃とは？

**短い版**:
salt はランダムな値をパスワードに追加してハッシュを計算することで、同じパスワードでも異なるハッシュ値にする。レインボーテーブル（事前計算済みハッシュの辞書）攻撃を無効化する。

**詳細版**:
**レインボーテーブル攻撃**:
攻撃者が事前に大量のパスワードとそのハッシュ値の対応表を作成しておく。ハッシュ値を見つけたら、テーブルを検索して元のパスワードを特定する。

salt なしの場合:
```
"password123" → SHA-512 → "ef92b778..." (常に同じハッシュ)
→ レインボーテーブルに "ef92b778..." があれば即座にパスワード判明
```

salt ありの場合:
```
ユーザーA: salt="abc123" + "password123" → SHA-512 → "7f2a9d31..."
ユーザーB: salt="xyz789" + "password123" → SHA-512 → "3b8c4e55..."
→ 同じパスワードでも異なるハッシュ
→ レインボーテーブルは salt ごとに作り直す必要がある（事実上不可能）
```

Linux の /etc/shadow での表現:
```
$6$abc123$7f2a9d31...
 ^   ^       ^
 |   |       └── ハッシュ値
 |   └── salt（ユーザーごとにランダム）
 └── アルゴリズム（6=SHA-512）
```

---

### Q65: Born2beRoot で使用した全コマンドのカテゴリ別一覧

**ユーザー管理**:
```bash
adduser <username>           # ユーザー作成（対話型）
usermod -aG <group> <user>   # グループへの追加
id <username>                # ユーザー情報の表示
groups <username>            # 所属グループの表示
chage -l <username>          # パスワード有効期限の確認
passwd <username>            # パスワードの変更
getent group <groupname>     # グループ情報の表示
```

**SSH 関連**:
```bash
ssh <user>@<host> -p 4242    # SSH 接続
grep -E "^Port|^PermitRootLogin" /etc/ssh/sshd_config  # 設定確認
sudo systemctl status sshd   # サービス状態
sudo systemctl restart sshd  # サービス再起動
sudo ss -tunlp | grep ssh    # リスニングポート確認
```

**UFW 関連**:
```bash
sudo ufw status verbose      # 詳細なステータス
sudo ufw status numbered     # 番号付きルール一覧
sudo ufw allow <port>        # ポートの許可
sudo ufw delete allow <port> # ルールの削除
sudo ufw enable/disable      # UFW の有効化/無効化
```

**LVM 関連**:
```bash
lsblk                        # ブロックデバイスの階層表示
sudo pvdisplay               # PV の情報
sudo vgdisplay               # VG の情報
sudo lvdisplay               # LV の詳細
sudo vgs                     # VG のサマリー
sudo lvs                     # LV のサマリー
sudo lvextend -L +<size> <lv_path>  # LV の拡張
sudo resize2fs <lv_path>     # ファイルシステムの拡張
```

**システム情報**:
```bash
hostname                     # ホスト名
hostnamectl                  # 詳細なホスト名情報
uname -a                     # カーネル情報
cat /etc/os-release          # OS 情報
systemd-detect-virt          # 仮想化の種類
df -h                        # ディスク使用量
free -h                      # メモリ使用量
uptime                       # 稼働時間とロードアベレージ
```

**AppArmor 関連**:
```bash
sudo aa-status               # AppArmor の状態
sudo systemctl status apparmor  # サービス状態
```

**sudo 関連**:
```bash
sudo cat /etc/sudoers.d/sudo_config  # カスタム設定
sudo visudo -f /etc/sudoers.d/sudo_config  # 安全な編集
sudo -l                      # 現在のユーザーの sudo 権限
sudo cat /var/log/sudo/sudo.log  # sudo ログ
```

**パスワードポリシー関連**:
```bash
grep "^PASS_" /etc/login.defs                    # 有効期限設定
grep pam_pwquality /etc/pam.d/common-password    # 強度設定
sudo chage -l <username>                          # ユーザーの有効期限
sudo chage -M 30 -m 2 -W 7 <username>           # 有効期限の手動設定
```

**cron 関連**:
```bash
sudo crontab -l              # root の crontab 表示
sudo crontab -e              # root の crontab 編集
```

**ログ関連**:
```bash
sudo journalctl -u <service> # 特定サービスのログ
sudo journalctl -f           # リアルタイムログ
sudo journalctl -b           # 今回のブート以降
sudo journalctl --vacuum-time=7d  # 古いログの削除
```

**暗号化関連**:
```bash
sudo cryptsetup luksDump /dev/sda5  # LUKS 情報
ls /dev/mapper/                     # 暗号化デバイス一覧
```

---

## 9. 評価の時間配分の目安

評価は通常 30〜60 分程度で行われる。以下は典型的な時間配分:

| 時間 | 内容 | 目安 |
|------|------|------|
| 0-5分 | signature.txt の検証、VM 起動 | LUKS パスフレーズ入力含む |
| 5-15分 | 基本概念の質疑 (Q1-Q12) | VM、OS 選択、apt/aptitude、AppArmor、LVM |
| 15-25分 | 設定の確認と実演 (Q13-Q22) | ユーザー作成、ホスト名変更、UFW 操作 |
| 25-40分 | 深い理解の質疑 (Q23-Q40) | セキュリティ、暗号化、ブートプロセス |
| 40-50分 | シナリオ問題 (Q41-Q55) | トラブルシューティング、設計判断 |
| 50-60分 | Bonus 確認 | パーティション、WordPress、追加サービス |

**ヒント**:
- 質問に対して「わかりません」と正直に言うことは問題ない。推測で誤った回答をするよりも誠実
- デモンストレーションでコマンドが失敗した場合、パニックにならずに原因を調査するプロセスを見せる
- 評価者が追加質問をするのは理解を深めるためであり、間違いを指摘するためではないことが多い

---

## 10. 評価後のフォローアップ

### 10.1 評価で指摘された改善点の対応

評価者からフィードバックを受けた場合:
1. 指摘事項をメモする
2. 理解が不十分だった箇所を復習する
3. 実際にコマンドを実行して確認する
4. 必要に応じて設定を修正する

### 10.2 自己評価チェックリスト

評価後に以下を振り返る:
- [ ] 全ての基礎質問に自信を持って回答できたか
- [ ] デモンストレーションはスムーズに実行できたか
- [ ] 「なぜ」の質問に対して技術的な理由を説明できたか
- [ ] シナリオ問題に対して論理的な調査手順を示せたか
- [ ] Bonus 部分は問題なく動作したか

### 10.3 次のプロジェクトへの活用

Born2beRoot で学んだ知識は以下の 42 プロジェクトに直結する:
- **ft_server / Inception**: Docker コンテナでのサービス構築（ネットワーク、サービス管理）
- **NetPractice**: ネットワーク設計（TCP/IP、サブネット、ルーティング）
- **ft_transcendence**: Web アプリケーションのデプロイ（SSH、セキュリティ、サーバー設定）

---

## 11. 用語集 (Glossary)

Born2beRoot 評価で使われる主要な技術用語の簡潔な定義:

| 用語 | 英語 | 定義 |
|------|------|------|
| 仮想マシン | Virtual Machine (VM) | Hypervisor 上で動作する仮想的なコンピュータ環境 |
| ハイパーバイザー | Hypervisor | VM を作成・管理するソフトウェア。Type 1（ベアメタル）と Type 2（ホスト型）がある |
| パーティション | Partition | ディスクを論理的に分割した領域 |
| ファイルシステム | File System | ディスク上のデータの構造と管理方法。ext4、XFS など |
| マウント | Mount | ファイルシステムをディレクトリツリーに接続すること |
| デーモン | Daemon | バックグラウンドで動作するサービスプロセス |
| ポート | Port | ネットワーク通信でサービスを識別する番号（0-65535） |
| プロトコル | Protocol | 通信の規約。TCP/IP、SSH、HTTP など |
| 暗号化 | Encryption | データを解読不能な形式に変換すること |
| ハッシュ化 | Hashing | 一方向関数でデータを固定長の値に変換すること（復号不可能） |
| ソルト | Salt | ハッシュ計算にランダム値を追加してレインボーテーブル攻撃を防ぐ |
| MAC | Mandatory Access Control | システム管理者が定義するアクセス制御ポリシー |
| DAC | Discretionary Access Control | リソース所有者が設定するアクセス権限 |
| TTY | Teletypewriter | 端末/ターミナルデバイス |
| PID | Process ID | プロセスに割り当てられる一意の識別番号 |
| LUKS | Linux Unified Key Setup | Linux のディスク暗号化標準 |
| LVM | Logical Volume Manager | 論理的なストレージ管理の仕組み |
| UFW | Uncomplicated Firewall | iptables のフロントエンド |
| PAM | Pluggable Authentication Modules | 認証をモジュール化したフレームワーク |
| SSH | Secure Shell | 暗号化されたリモートアクセスプロトコル |
| GRUB | GRand Unified Bootloader | Linux のブートローダー |
| cron | cron | 定期タスクのスケジューラ |
| wall | Write All | 全端末にメッセージをブロードキャストするコマンド |
| NAT | Network Address Translation | プライベート IP とパブリック IP の変換 |
| DNS | Domain Name System | ドメイン名から IP アドレスへの名前解決 |
| AES | Advanced Encryption Standard | 対称暗号化アルゴリズム |
| RSA | Rivest-Shamir-Adleman | 非対称暗号化アルゴリズム |
| MBR | Master Boot Record | ディスクの先頭にあるブート情報 |
| GPT | GUID Partition Table | MBR の後継。大容量ディスクに対応 |
| inode | Index Node | ファイルのメタデータを格納するデータ構造 |
| systemd | systemd | Linux の init システム（PID 1） |
| initramfs | Initial RAM Filesystem | ブート時に使用される一時的なルートファイルシステム |
| PBKDF2 | Password-Based Key Derivation Function 2 | パスワードから暗号鍵を導出する関数 |

---

## 12. 参考コマンドクイックリファレンス

評価中に素早く参照するためのコマンド一覧:

```bash
# === 基本確認 ===
hostname && id $(whoami) && groups $(whoami)
cat /etc/os-release | head -2 && uname -r
systemd-detect-virt

# === セキュリティ確認 ===
sudo aa-status | head -5
grep -E "^Port|^PermitRootLogin" /etc/ssh/sshd_config
sudo ufw status verbose
grep "^PASS_" /etc/login.defs
grep pam_pwquality /etc/pam.d/common-password
sudo cat /etc/sudoers.d/sudo_config

# === ストレージ確認 ===
lsblk && df -h
sudo lvs && sudo vgs

# === 監視確認 ===
sudo crontab -l
sudo bash /usr/local/bin/monitoring.sh

# === ユーザー操作 ===
sudo adduser eval42
sudo usermod -aG sudo eval42 && sudo usermod -aG user42 eval42
id eval42 && sudo chage -l eval42

# === ホスト名変更 ===
sudo hostnamectl set-hostname newhostname42
# /etc/hosts も更新すること
```
