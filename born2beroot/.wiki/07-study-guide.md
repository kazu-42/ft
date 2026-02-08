# 07 - 学習ガイド (Study Guide)

Born2beRoot の効率的な学習方法と、各トピックの要点を整理する。さらに実践的な演習問題（フォレンジック演習、LVM リサイズ演習、UFW 設計演習、設定 diff レビュー演習）を含む。

---

## 1. 推奨学習順序

以下の順序で学習を進めることを推奨する。各ステップは前のステップの知識に依存する。

```
Phase 1: 基礎知識（1-2日）
├── Step 1: 仮想化と VM の基礎概念
├── Step 2: Linux の基礎コマンド操作
└── Step 3: ファイルシステムとパーミッション

Phase 2: OS インストール（1日）
├── Step 4: Debian のインストール
├── Step 5: パーティション設定（LVM + LUKS）
└── Step 6: 初期設定（ユーザー、グループ）

Phase 3: セキュリティ設定（1-2日）
├── Step 7: SSH の設定
├── Step 8: UFW ファイアウォール
├── Step 9: パスワードポリシー
└── Step 10: sudo の設定

Phase 4: 監視スクリプト（1日）
├── Step 11: monitoring.sh の作成
├── Step 12: cron の設定
└── Step 13: テストと検証

Phase 5: Bonus（1-2日、オプション）
├── Step 14: WordPress (lighttpd + MariaDB + PHP)
└── Step 15: 追加サービス

Phase 6: 防御準備（1日）
├── Step 16: 全要件の動作確認
├── Step 17: 質問への回答準備
└── Step 18: signature.txt の生成と提出
```

### 学習の前提条件

本ガイドを効率的に進めるために、以下のスキルが必要である:

| スキル | 推定レベル | 準備方法 |
|--------|-----------|---------|
| ターミナル操作 | 基本的な cd, ls, cat ができる | 02-prerequisites.md を先に読む |
| テキストエディタ | nano または vim が使える | `vimtutor` を一通りやる |
| ネットワーク基礎 | IP アドレスとポートの概念がわかる | 02-prerequisites.md のネットワークセクション |
| 英語の技術文書 | man ページを読める | Google 翻訳を活用しながら慣れる |

---

## 2. 各トピックの要点

### Step 1: 仮想化と VM の基礎概念

**学ぶべきこと**:
- 仮想化とは何か、なぜ使うのか
- Type 1 vs Type 2 Hypervisor の違い
- VirtualBox の基本操作
- CPU の保護リング（Ring 0-3）と仮想化の関係

**キーワード**: Hypervisor, Guest OS, Host OS, NAT, Port Forwarding, VT-x/AMD-V

**確認項目**:
- [ ] 「仮想化とは何か」を自分の言葉で説明できる
- [ ] Type 1 と Type 2 の違いを例を挙げて説明できる
- [ ] VirtualBox で VM を作成できる
- [ ] NAT と Port Forwarding の関係を説明できる
- [ ] コンテナ（Docker）との違いを説明できる

**深堀り演習**:

```
問題 1: VirtualBox で VM を作成する際、「CPU の数」を指定する。
ホスト PC が 4 コア（論理 8 コア）の場合、VM に 8 コアすべてを
割り当てることは可能か？その場合何が起こるか？

考えるポイント:
- Type 2 Hypervisor はホスト OS 上で動作する
- ホスト OS 自体も CPU リソースを必要とする
- VM に全コアを割り当てるとホスト OS が応答不能になる可能性がある
- VirtualBox は警告を出すが設定自体は可能
- 推奨: ホストの論理コア数の半分以下を割り当てる
```

```
問題 2: Born2beRoot で VirtualBox (Type 2) を使う理由は何か？
KVM (Type 1) を使えばパフォーマンスが良いのになぜ使わないのか？

考えるポイント:
- 42 のマシンは共有環境であり、KVM のインストールが許可されていない
- VirtualBox はユーザー空間で動作するため、root 権限なしで使用可能
- Type 2 Hypervisor はデスクトップ用途に適している
- 教育目的では、GUI のある VirtualBox の方が学習しやすい
```

```
問題 3: 仮想マシンとコンテナ（Docker）の違いを図で説明せよ。

回答:
VM:
  ┌─ App ─┐ ┌─ App ─┐
  │ Guest │ │ Guest │
  │  OS   │ │  OS   │
  └───────┘ └───────┘
  ┌─── Hypervisor ───┐
  └──────────────────┘
  ┌──── Host OS ─────┐
  └──────────────────┘
  → 各 VM が独自の OS カーネルを持つ
  → オーバーヘッドが大きい
  → 完全な隔離性

コンテナ:
  ┌─ App ─┐ ┌─ App ─┐
  │ Libs  │ │ Libs  │
  └───────┘ └───────┘
  ┌─ Container Engine ┐
  └──────────────────┘
  ┌──── Host OS ─────┐
  └──────────────────┘
  → ホスト OS のカーネルを共有
  → オーバーヘッドが小さい
  → 隔離性は VM より低い

Born2beRoot は OS レベルの設定を学ぶため、コンテナではなく VM を使用する
```

---

### Step 2: Linux の基礎コマンド操作

**練習すべきコマンド**:
```bash
# ファイル操作
ls -la /etc/
cat /etc/hostname
nano /tmp/test.txt

# パイプとリダイレクト
ps aux | grep ssh
echo "test" > /tmp/output.txt       # 上書き
echo "append" >> /tmp/output.txt    # 追記
command 2>&1                         # stderr を stdout にリダイレクト
command > /dev/null 2>&1             # 全出力を破棄

# パーミッション
chmod 755 /tmp/test.sh
chown root:root /tmp/test.sh
stat /etc/passwd  # 詳細なファイル情報

# プロセス
ps aux
ps aux --forest  # プロセスツリー表示
systemctl status sshd
pstree  # プロセスの親子関係を表示
top -bn1  # 非対話的に CPU/メモリ情報を表示

# 検索
find / -name "sshd_config" 2>/dev/null
grep -r "Port" /etc/ssh/
which python3  # コマンドの場所を検索

# ネットワーク
ip addr show
ss -tunlp       # 開いているポートとプロセス
ping -c 3 8.8.8.8
```

**確認項目**:
- [ ] CLI でファイルの操作ができる
- [ ] テキストエディタで設定ファイルを編集できる
- [ ] パイプとリダイレクトを使える
- [ ] stderr と stdout の違いを理解している
- [ ] find と grep の基本的な使い方を知っている

**コマンド習熟度チェック**:

以下のタスクをコマンドだけで実行できるか確認する:

```bash
# Task 1: /etc/ssh/sshd_config から "Port" を含む行を表示
# ヒント: grep を使う

# Task 2: /var/log/ 以下で最も大きいファイル Top 5 を表示
# ヒント: du + sort + head のパイプ

# Task 3: 現在のメモリ使用率をパーセントで表示
# ヒント: free + awk

# Task 4: 現在ログインしているユーザーの一覧を表示
# ヒント: who または w

# Task 5: /etc/passwd からシェルが /bin/bash のユーザーだけ抽出
# ヒント: grep + awk

# Task 6: 現在の TCP ESTABLISHED 接続数を数える
# ヒント: ss + grep + wc

# Task 7: /etc 以下で過去24時間以内に変更されたファイルを表示
# ヒント: find + mtime

# Task 8: ディスク使用率を「使用量/総量GB (割合%)」形式で表示
# ヒント: df + awk
```

**回答例**:
```bash
# Task 1
grep "Port" /etc/ssh/sshd_config

# Task 2
sudo du -sh /var/log/* 2>/dev/null | sort -hr | head -5

# Task 3
free | awk '/^Mem:/ {printf("%.1f%%\n", $3/$2*100)}'

# Task 4
who

# Task 5
grep "/bin/bash$" /etc/passwd | awk -F: '{print $1}'

# Task 6
ss -t state established | tail -n +2 | wc -l

# Task 7
sudo find /etc -mtime -1 -type f 2>/dev/null

# Task 8
df -BG --total | awk '/total/ {printf("%s/%s (%s)\n", $3, $2, $5)}'
```

---

### Step 3: ファイルシステムとパーミッション

**学ぶべきこと**:
- FHS（Filesystem Hierarchy Standard）の主要ディレクトリ
- パーミッションの読み方と設定方法
- ユーザーとグループの概念
- inode の概念
- 特殊パーミッション（SUID, SGID, Sticky Bit）

**練習コマンド**:
```bash
# ディレクトリ構造の確認
ls /
ls /etc/ /var/ /home/ /boot/

# パーミッションの確認と変更
ls -la /etc/passwd /etc/shadow
stat /etc/passwd

# パーミッションの数値表記
# r=4, w=2, x=1
# 755 = rwxr-xr-x (owner: rwx, group: r-x, others: r-x)
# 644 = rw-r--r-- (owner: rw-, group: r--, others: r--)
# 600 = rw------- (owner: rw-, group: ---, others: ---)
# 440 = r--r----- (owner: r--, group: r--, others: ---)

# ユーザーとグループ
id
groups
cat /etc/passwd
cat /etc/group

# 特殊パーミッション
find / -perm -4000 -type f 2>/dev/null  # SUID ファイル一覧
find / -perm -2000 -type f 2>/dev/null  # SGID ファイル一覧
ls -la /tmp  # Sticky Bit の確認（/tmp は drwxrwxrwt）
```

**パーミッションの読み方トレーニング**:

```
問題 1: 以下の ls -la 出力を解釈せよ
-rw-r----- 1 root shadow 1234 Jan 15 10:00 /etc/shadow

回答:
- ファイルタイプ: - (通常ファイル)
- Owner (root): rw- (読み書き可能)
- Group (shadow): r-- (読み取りのみ)
- Others: --- (アクセス不可)
- 数値表記: 640
- 意味: root は読み書き可能、shadow グループのメンバーは読み取り可能、
  その他はアクセス不可
- セキュリティ上の意味: パスワードハッシュは root と shadow グループの
  プログラムのみがアクセスできる
```

```
問題 2: 以下のパーミッションを解釈せよ
-rwsr-xr-x 1 root root 54096 Jan 15 10:00 /usr/bin/passwd

回答:
- ファイルタイプ: - (通常ファイル)
- Owner (root): rws → SUID ビットが設定されている
- Group (root): r-x (読み取り+実行)
- Others: r-x (読み取り+実行)
- 数値表記: 4755
- SUID の意味: 一般ユーザーが passwd を実行すると、
  プロセスは root 権限で動作する
- なぜ必要か: /etc/shadow の書き込みには root 権限が必要だが、
  一般ユーザーも自分のパスワードを変更できるようにするため
```

```
問題 3: /tmp ディレクトリのパーミッションを解釈せよ
drwxrwxrwt 10 root root 4096 Jan 15 10:00 /tmp

回答:
- ファイルタイプ: d (ディレクトリ)
- パーミッション: rwxrwxrwt → 全ユーザーが読み書き可能
- 末尾の t: Sticky Bit が設定されている
- Sticky Bit の意味: /tmp 内のファイルは、そのファイルの所有者のみが
  削除・名前変更できる（他のユーザーは削除不可）
- なぜ必要か: /tmp は全ユーザーが書き込みできる共有ディレクトリだが、
  他のユーザーのファイルを削除されないようにするため
```

---

### Step 4-5: Debian インストールとパーティション設定

**学ぶべきこと**:
- Debian インストーラの操作
- パーティションテーブルの概念（MBR vs GPT）
- LVM の3層構造（PV → VG → LV）
- LUKS 暗号化の設定
- マウントポイントの意味

**確認項目**:
- [ ] `lsblk` の出力を読み取れる
- [ ] LVM の PV, VG, LV の関係を説明できる
- [ ] 暗号化パーティションが正しく設定されている
- [ ] 各パーティションの役割を説明できる
- [ ] /etc/fstab の内容を理解できる

**検証コマンド**:
```bash
lsblk
sudo pvdisplay
sudo vgdisplay
sudo lvdisplay
cat /etc/fstab
cat /etc/crypttab
sudo pvs  # PV の要約表示
sudo vgs  # VG の要約表示
sudo lvs  # LV の要約表示
```

**lsblk の出力読み方演習**:

```
以下の lsblk 出力を解釈せよ:

NAME                    MAJ:MIN RM  SIZE RO TYPE  MOUNTPOINT
sda                       8:0    0   30G  0 disk
├─sda1                    8:1    0  500M  0 part  /boot
├─sda2                    8:2    0    1K  0 part
└─sda5                    8:5    0 29.5G  0 part
  └─sda5_crypt          254:0    0 29.5G  0 crypt
    ├─LVMGroup-root     254:1    0   10G  0 lvm   /
    ├─LVMGroup-swap     254:2    0  2.3G  0 lvm   [SWAP]
    ├─LVMGroup-home     254:3    0    5G  0 lvm   /home
    ├─LVMGroup-var      254:4    0    3G  0 lvm   /var
    ├─LVMGroup-srv      254:5    0    3G  0 lvm   /srv
    ├─LVMGroup-tmp      254:6    0    3G  0 lvm   /tmp
    └─LVMGroup-var--log 254:7    0    4G  0 lvm   /var/log

解答:
1. sda: 30GB の物理ディスク
2. sda1 (500MB): /boot パーティション（暗号化されていない、ブートローダーが必要）
3. sda2 (1K): 拡張パーティションのヘッダ（MBR の制限: プライマリは4つまで）
4. sda5 (29.5GB): LUKS 暗号化パーティション（論理パーティション）
5. sda5_crypt: LUKS で復号された仮想デバイス
6. LVMGroup-*: LVM の論理ボリューム（7つの LV）
7. 階層: 物理ディスク → パーティション → LUKS暗号化 → LVM → ファイルシステム

なぜ /boot は暗号化されないか:
  → ブートローダー (GRUB) が暗号化されたパーティションを読むためには、
    最初に復号化の仕組みを読み込む必要がある
  → /boot にはカーネルとブートローダーの設定が格納される
  → GRUB が暗号化パーティションのパスフレーズ入力を促すために、
    /boot は暗号化せずにアクセス可能にする必要がある
```

**LVM の概念図**:

```
LVM の3層構造:

Layer 1: Physical Volume (PV) - 物理ボリューム
  /dev/sda5_crypt → PV として初期化
  (他のディスクも PV として追加可能)

Layer 2: Volume Group (VG) - ボリュームグループ
  LVMGroup ← PV を束ねた「プール」
  (複数の PV を1つの VG にまとめられる)

Layer 3: Logical Volume (LV) - 論理ボリューム
  root, swap, home, var, srv, tmp, var-log
  (VG のプールから必要なサイズを切り出す)

     ┌── /dev/sda5_crypt ──┐
     │    Physical Volume   │
     └──────────┬───────────┘
                │
     ┌──────────▼───────────┐
     │    LVMGroup (VG)     │
     │    Total: 29.5GB     │
     └──┬──┬──┬──┬──┬──┬──┬┘
        │  │  │  │  │  │  │
  ┌─────┘  │  │  │  │  │  └─────┐
  ▼        ▼  ▼  ▼  ▼  ▼        ▼
 root   swap home var srv tmp  var-log
 10G    2.3G 5G  3G  3G  3G    4G

LVM の利点:
  - パーティションのリサイズが容易（後から拡張/縮小可能）
  - 複数の物理ディスクをまたぐボリュームが作成可能
  - スナップショット機能（バックアップに便利）
```

---

### Step 6: ユーザーとグループの設定

**実行すべき手順**:
```bash
# user42 グループの作成
sudo groupadd user42

# ユーザーをグループに追加
sudo usermod -aG sudo kaztakam
sudo usermod -aG user42 kaztakam

# 確認
id kaztakam
groups kaztakam
```

**確認項目**:
- [ ] `id kaztakam` で sudo と user42 が表示される
- [ ] ホスト名が `kaztakam42` になっている
- [ ] `/etc/hostname` と `/etc/hosts` の両方が変更されている

**ホスト名変更の手順と検証**:
```bash
# ホスト名の変更
sudo hostnamectl set-hostname kaztakam42

# /etc/hosts の更新
sudo nano /etc/hosts
# 127.0.0.1    localhost
# 127.0.1.1    kaztakam42

# 検証（再ログイン後）
hostname
hostnamectl
cat /etc/hostname
```

**注意: よくあるミス**
```
問題: hostnamectl でホスト名を変更したが、/etc/hosts を更新していない
症状: sudo 実行時に「unable to resolve host」警告が出る
対策: /etc/hosts の 127.0.1.1 行も新しいホスト名に変更する

問題: usermod の -a オプションを忘れた
症状: usermod -G sudo user → 既存のグループメンバーシップが全て削除される
対策: 必ず -aG を使用する（-a = append）
```

**評価で聞かれるユーザー追加の手順**:

```bash
# 評価時に新しいユーザーを作成する手順
# 1. ユーザーの作成
sudo adduser evaluator
# → パスワードの入力を求められる（ポリシーに準拠したものを入力）

# 2. user42 グループに追加
sudo usermod -aG user42 evaluator

# 3. 確認
id evaluator
# → uid=1001(evaluator) gid=1001(evaluator) groups=1001(evaluator),1002(user42)

# 4. パスワードポリシーの確認
sudo chage -l evaluator
# → Maximum number of days between password change: 30
# → Minimum number of days between password change: 2
# → Number of days of warning before password expires: 7
```

---

### Step 7: SSH の設定

**実行すべき手順**:
```bash
# 1. sshd_config の編集
sudo nano /etc/ssh/sshd_config
# Port 4242
# PermitRootLogin no

# 2. サービスの再起動
sudo systemctl restart sshd

# 3. 確認
sudo ss -tunlp | grep ssh
```

**検証方法**:
```bash
# ポートの確認
sudo ss -tunlp | grep 4242

# root ログインの拒否を確認（別ターミナルから）
ssh root@localhost -p 4242
# → Permission denied

# 一般ユーザーでのログイン確認
ssh kaztakam@localhost -p 4242
# → 成功するはず
```

**SSH 設定の深堀り**:

```
重要: sshd_config の編集で注意すべき点

1. コメントアウトされた設定はデフォルト値を示す
   #Port 22      ← デフォルトで port 22 が使用される
   Port 4242     ← コメントを外して値を変更する

2. PermitRootLogin のオプション:
   - yes: root ログイン許可（危険）
   - no: root ログイン完全禁止（Born2beRoot の要件）
   - prohibit-password: パスワード認証禁止、公開鍵認証のみ許可
   - forced-commands-only: 特定コマンドのみ許可

3. 設定変更後は必ずサービスを再起動する
   sudo systemctl restart sshd
   ※ 再起動しないと新しい設定が反映されない

4. 設定を間違えた場合のリカバリ:
   - SSH 接続が切れても、コンソール（VirtualBox のウィンドウ）からログインできる
   - 必ず設定変更前にコンソールアクセスが可能なことを確認する
```

**VirtualBox の Port Forwarding 設定**:
```
VirtualBox でホストからゲストに SSH するには Port Forwarding が必要:

設定手順:
1. VM を選択 → 設定 → ネットワーク → アダプター 1
2. 「高度」を展開 → 「ポートフォワーディング」
3. 以下のルールを追加:
   名前: SSH
   プロトコル: TCP
   ホスト IP: 127.0.0.1
   ホストポート: 4242
   ゲスト IP: (空欄)
   ゲストポート: 4242

接続コマンド（ホスト OS から）:
ssh kaztakam@127.0.0.1 -p 4242

仕組み:
  ホスト(127.0.0.1:4242) → NAT変換 → ゲスト(10.0.2.15:4242)
```

---

### Step 8: UFW ファイアウォール

**実行すべき手順**:
```bash
# 1. インストール
sudo apt install ufw

# 2. デフォルトポリシーの設定
sudo ufw default deny incoming
sudo ufw default allow outgoing

# 3. SSH ポートの許可
sudo ufw allow 4242

# 4. 有効化
sudo ufw enable

# 5. 確認
sudo ufw status verbose
```

**検証方法**:
```bash
# ルールの確認
sudo ufw status numbered
# → 4242 のみが ALLOW であること

# UFW が active であること
sudo ufw status
# → Status: active

# iptables のルールも確認（UFW の内部実装を理解する）
sudo iptables -L -n -v
```

**UFW の操作早見表**:

| 操作 | コマンド |
|------|---------|
| ルール一覧（番号付き） | `sudo ufw status numbered` |
| ポート追加 | `sudo ufw allow 80` |
| 特定 IP から特定ポート | `sudo ufw allow from 192.168.1.0/24 to any port 4242` |
| ルール削除（番号指定） | `sudo ufw delete 2` |
| ルール削除（ルール指定） | `sudo ufw delete allow 80` |
| リセット | `sudo ufw reset` |
| ログ有効化 | `sudo ufw logging on` |
| ログ確認 | `sudo cat /var/log/ufw.log` |
| 有効化 | `sudo ufw enable` |
| 無効化 | `sudo ufw disable` |

---

### Step 9: パスワードポリシー

**実行すべき手順**:
```bash
# 1. login.defs の編集
sudo nano /etc/login.defs
# PASS_MAX_DAYS   30
# PASS_MIN_DAYS   2
# PASS_WARN_AGE   7

# 2. pam_pwquality のインストール
sudo apt install libpam-pwquality

# 3. PAM 設定の編集
sudo nano /etc/pam.d/common-password
# password requisite pam_pwquality.so retry=3 minlen=10 ucredit=-1 dcredit=-1 lcredit=-1 maxrepeat=3 reject_username difok=7 enforce_for_root

# 4. 既存ユーザーへの適用
sudo chage -M 30 -m 2 -W 7 kaztakam
sudo chage -M 30 -m 2 -W 7 root
```

**検証方法**:
```bash
# 既存ユーザーのポリシー確認
sudo chage -l kaztakam

# パスワード強度のテスト
passwd
# → 弱いパスワードを入力して拒否されることを確認
```

**pam_pwquality の各パラメータ詳細**:

| パラメータ | 値 | 意味 | セキュリティ上の理由 |
|-----------|-----|------|---------------------|
| retry | 3 | パスワード入力の再試行回数 | ブルートフォースの速度低下 |
| minlen | 10 | パスワードの最低文字数 | エントロピーの確保 |
| ucredit | -1 | 大文字を最低1文字必須 | 文字空間の拡大 |
| dcredit | -1 | 数字を最低1文字必須 | 文字空間の拡大 |
| lcredit | -1 | 小文字を最低1文字必須 | 辞書攻撃への耐性 |
| maxrepeat | 3 | 同じ文字の連続を3文字以下に制限 | 単純パターンの防止 |
| reject_username | - | ユーザー名を含むパスワードを拒否 | 推測容易なパスワードの防止 |
| difok | 7 | 旧パスワードと7文字以上異なること | パスワードの使い回し防止 |
| enforce_for_root | - | root にもポリシーを適用 | 特権ユーザーも例外としない |

**パスワード変更テスト手順**:
```bash
# テスト 1: 短すぎるパスワード
passwd
New password: Ab1234567  # 9文字 → 拒否される（minlen=10）

# テスト 2: 大文字なし
passwd
New password: abcd123456  # 大文字なし → 拒否される（ucredit=-1）

# テスト 3: 数字なし
passwd
New password: Abcdefghij  # 数字なし → 拒否される（dcredit=-1）

# テスト 4: 同じ文字の連続
passwd
New password: Aaaa123456  # 'a' が4連続 → 拒否される（maxrepeat=3）

# テスト 5: ユーザー名を含む
passwd
New password: Kaztakam12  # ユーザー名を含む → 拒否される（reject_username）

# テスト 6: 有効なパスワード
passwd
New password: Xb9mK2pL7w  # 全条件を満たす → 受理される
```

---

### Step 10: sudo の設定

**実行すべき手順**:
```bash
# 1. ログディレクトリの作成
sudo mkdir -p /var/log/sudo

# 2. sudo 設定ファイルの作成
sudo visudo -f /etc/sudoers.d/sudo_config
# 以下を記入:
# Defaults    passwd_tries=3
# Defaults    badpass_message="Wrong password. Access denied."
# Defaults    logfile="/var/log/sudo/sudo.log"
# Defaults    log_input
# Defaults    log_output
# Defaults    iolog_dir="/var/log/sudo"
# Defaults    requiretty
# Defaults    secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin"
```

**検証方法**:
```bash
# カスタムエラーメッセージの確認
sudo ls
# → 間違ったパスワードを入力
# → "Wrong password. Access denied." と表示される

# ログの確認
sudo cat /var/log/sudo/sudo.log
# → 実行したコマンドが記録されている

# 3回間違えると sudo が終了することの確認
```

**各設定の攻撃シナリオとの対応**:

```
設定: requiretty
攻撃シナリオ: Web Shell からの sudo 実行
  1. 攻撃者が Web アプリの脆弱性を悪用して Web Shell を設置
  2. Web Shell から sudo コマンドを実行しようとする
  3. Web Shell には TTY がないため sudo が拒否される
  4. 結果: 権限昇格が阻止される

設定: secure_path
攻撃シナリオ: PATH 注入攻撃
  1. 攻撃者が /tmp/ls という悪意あるスクリプトを配置
  2. ユーザーの PATH に /tmp を追加（.bashrc 改竄）
  3. ユーザーが sudo ls を実行
  4. secure_path があるため /tmp/ls ではなく /bin/ls が実行される
  5. 結果: 悪意あるコードの root 実行が阻止される

設定: log_input + log_output
セキュリティ効果: フォレンジック（事後調査）
  - sudo で実行されたすべてのコマンドの入出力が記録される
  - セキュリティインシデント発生時に「何が行われたか」を正確に追跡可能
  - 「否認防止」（Non-Repudiation）: ユーザーが「やっていない」と主張できない
```

---

### Step 11-12: monitoring.sh と cron

**検証方法**:
```bash
# スクリプトの手動実行
sudo bash /usr/local/bin/monitoring.sh
# → wall メッセージが表示される

# cron の確認
sudo crontab -l
# → */10 * * * * /usr/local/bin/monitoring.sh

# スクリプトのデバッグ
sudo bash -x /usr/local/bin/monitoring.sh
# → 各コマンドの実行過程が表示される
```

**monitoring.sh の各指標の意味と重要性**:

| 指標 | 取得コマンド | 監視する理由 |
|------|-------------|-------------|
| Architecture | `uname -a` | OS とカーネルバージョンの確認 |
| Physical CPUs | `grep "physical id" /proc/cpuinfo \| sort -u \| wc -l` | リソースの基準値 |
| Virtual CPUs | `grep -c "^processor" /proc/cpuinfo` | 論理プロセッサ数 |
| Memory Usage | `free -m` + awk | メモリリーク検出 |
| Disk Usage | `df` + awk | ディスク枯渇の予防 |
| CPU Load | `mpstat` or `/proc/loadavg` | 異常プロセスの検出 |
| Last Boot | `who -b` | 不正な再起動の検出 |
| LVM | `lsblk` | ストレージ構成の変更検出 |
| TCP Connections | `ss -t` | 不正な接続の検出 |
| Users Logged | `who` | 不正ログインの検出 |
| Network | `hostname -I` + `ip link` | ネットワーク変更検出 |
| Sudo Commands | `journalctl _COMM=sudo` | 権限昇格の監視 |

**cron の設定で注意すべき点**:

```bash
# crontab の編集（root として）
sudo crontab -e

# 10分ごとに実行
*/10 * * * * /usr/local/bin/monitoring.sh

# 注意点:
# 1. 評価時に cron を停止する方法を聞かれることがある
#    方法 A: crontab -e で行をコメントアウト
#    方法 B: sudo systemctl stop cron
#    方法 C: sudo chmod -x /usr/local/bin/monitoring.sh

# 2. */10 は「0, 10, 20, 30, 40, 50分」に実行される
#    「サーバー起動後10分ごと」ではない

# 3. sleep を使って「サーバー起動後30秒後から」開始する方法:
#    */10 * * * * sleep 30 && /usr/local/bin/monitoring.sh

# 4. cron の書式:
#    分 時 日 月 曜日 コマンド
#    *  *  *  *  *     = 毎分実行
#    */10 * * * *      = 10分ごと
#    0 */2 * * *       = 2時間ごとの0分
#    30 3 * * 1        = 毎週月曜 3:30
```

---

## 3. よくあるミスと対策

| # | よくあるミス | 症状 | 対策 |
|---|------------|------|------|
| 1 | sshd_config 編集後に再起動を忘れる | 設定が反映されない | `sudo systemctl restart sshd` を必ず実行 |
| 2 | UFW を有効化する前に SSH ポートを許可し忘れる | SSH 接続が切断される | `ufw allow 4242` してから `ufw enable` |
| 3 | /etc/login.defs の変更が既存ユーザーに適用されない | chage -l で古い値が表示 | `chage` コマンドで既存ユーザーにも適用 |
| 4 | sudoers ファイルの構文エラー | sudo が全く使えなくなる | 必ず `visudo` を使って編集する |
| 5 | monitoring.sh のコマンドが動かない | wall にエラーが含まれる | `bash -x` でデバッグ |
| 6 | cron に設定したが動かない | 定期実行されない | `crontab -l` で確認、パスが正しいか確認 |
| 7 | hostname を変更したが /etc/hosts を更新していない | sudo で警告が出る | 両方を変更する |
| 8 | signature.txt 生成後に VM を起動 | ハッシュ値が変わる | 生成後は VM を起動しない |
| 9 | パスワードを忘れる | ログインできなくなる | root のパスワードを安全に記録しておく |
| 10 | Bonus に手を出して必須が未完成 | 必須部分で減点 | 必須を完璧にしてから Bonus |
| 11 | usermod で -a を忘れて -G だけ | 既存グループから外れる | 必ず -aG を使用する |
| 12 | パーティション要件が不足 | 評価時に不合格 | lsblk で Bonus の構成と一致するか確認 |
| 13 | AppArmor の状態を確認していない | aa-status でエラー | `sudo aa-status` で enforce を確認 |
| 14 | 評価用ユーザーの作成手順を暗記していない | 評価時にパニック | adduser + usermod -aG の手順を暗記 |

---

## 4. 各要件の検証コマンド集

以下のコマンドを順番に実行して、全要件を確認できる:

```bash
echo "===== 1. OS の確認 ====="
cat /etc/os-release | head -2
uname -r

echo "===== 2. GUI がないことの確認 ====="
dpkg -l | grep -cE "xorg|wayland|x11" && echo "GUI found!" || echo "No GUI - OK"

echo "===== 3. ホスト名 ====="
hostname

echo "===== 4. ユーザーとグループ ====="
id kaztakam
getent group user42

echo "===== 5. パーティション ====="
lsblk

echo "===== 6. SSH ====="
grep "^Port" /etc/ssh/sshd_config
grep "^PermitRootLogin" /etc/ssh/sshd_config
sudo ss -tunlp | grep ssh

echo "===== 7. UFW ====="
sudo ufw status

echo "===== 8. パスワードポリシー（有効期限） ====="
grep "^PASS_" /etc/login.defs

echo "===== 9. パスワードポリシー（強度） ====="
grep pam_pwquality /etc/pam.d/common-password

echo "===== 10. sudo 設定 ====="
sudo cat /etc/sudoers.d/sudo_config

echo "===== 11. sudo ログ ====="
ls -la /var/log/sudo/

echo "===== 12. AppArmor ====="
sudo aa-status | head -3

echo "===== 13. cron ====="
sudo crontab -l

echo "===== 14. monitoring.sh ====="
ls -la /usr/local/bin/monitoring.sh
```

---

## 5. 実践演習

### 演習 1: フォレンジック演習 - 不正アクセスの調査

**シナリオ**: あなたは Born2beRoot サーバーの管理者である。ある朝出勤すると、監視システムから「TCP ESTABLISHED 接続数が異常に多い」というアラートが来ていた。このサーバーが侵害された可能性がある。原因を調査せよ。

**調査手順**:

```bash
# Step 1: 現在の状況把握
echo "=== 現在の接続状況 ==="
sudo ss -tunp state established

echo "=== ログインユーザー ==="
who
w

echo "=== 最近のログイン履歴 ==="
last -20

echo "=== 認証ログ ==="
sudo journalctl -u sshd --since "24 hours ago" | tail -50
```

```bash
# Step 2: 不正なプロセスの確認
echo "=== 異常なプロセス ==="
ps aux --sort=-%cpu | head -20
ps aux --sort=-%mem | head -20

echo "=== 不審なネットワーク接続 ==="
sudo ss -tunp | grep -v "sshd\|4242"

echo "=== 最近変更されたファイル ==="
find /etc -mtime -1 -type f 2>/dev/null
find /usr/local/bin -mtime -1 -type f 2>/dev/null
```

```bash
# Step 3: ログの詳細確認
echo "=== sudo ログの確認 ==="
sudo cat /var/log/sudo/sudo.log | tail -30

echo "=== 失敗したログイン試行 ==="
sudo journalctl -u sshd | grep -i "failed\|invalid" | tail -20

echo "=== crontab の確認（不正なジョブ） ==="
sudo crontab -l
for user in $(cut -f1 -d: /etc/passwd); do
  crontab_content=$(sudo crontab -u $user -l 2>/dev/null)
  if [ -n "$crontab_content" ]; then
    echo "User: $user"
    echo "$crontab_content"
  fi
done
```

```bash
# Step 4: ファイルの整合性確認
echo "=== SUID ファイルの確認 ==="
find / -perm -4000 -type f 2>/dev/null

echo "=== /tmp の不審なファイル ==="
ls -la /tmp/ /var/tmp/

echo "=== 設定ファイルの変更確認 ==="
stat /etc/ssh/sshd_config
stat /etc/sudoers
stat /etc/pam.d/common-password
```

**調査レポートのテンプレート**:

```
=== インシデント調査報告 ===
日時: YYYY/MM/DD HH:MM
調査者: [名前]

1. アラート概要:
   - [何が検知されたか]

2. 影響範囲:
   - [影響を受けたユーザー/サービス]

3. 調査結果:
   - [発見した不審な活動]
   - [侵入経路の推定]

4. 対応措置:
   - [実施した対応]

5. 再発防止策:
   - [今後の対策]
```

---

### 演習 2: LVM リサイズ演習

**シナリオ**: /var/log パーティションの使用率が 90% に達した。LVM を使って容量を拡張する必要がある。

**事前確認**:
```bash
# 現在のストレージ状況を確認
echo "=== ディスク使用率 ==="
df -h

echo "=== VG の空き容量 ==="
sudo vgs

echo "=== LV の詳細 ==="
sudo lvs
```

**ケース A: VG に空きがある場合**:
```bash
# 手順 1: LV を拡張（+2GB）
sudo lvextend -L +2G /dev/LVMGroup/var--log

# 手順 2: ファイルシステムを拡張
# ext4 の場合（オンラインで拡張可能）:
sudo resize2fs /dev/LVMGroup/var--log

# 手順 3: 確認
df -h /var/log
sudo lvs
```

**ケース B: VG に空きがない場合（新しいディスクを追加）**:
```bash
# 手順 1: 新しいディスクの確認
lsblk
# → /dev/sdb が新しいディスクとして認識されている

# 手順 2: PV の作成
sudo pvcreate /dev/sdb

# 手順 3: 既存の VG に PV を追加
sudo vgextend LVMGroup /dev/sdb

# 手順 4: VG の空き容量を確認
sudo vgs

# 手順 5: LV を拡張
sudo lvextend -L +2G /dev/LVMGroup/var--log

# 手順 6: ファイルシステムを拡張
sudo resize2fs /dev/LVMGroup/var--log

# 手順 7: 確認
df -h /var/log
sudo pvs  # 新しい PV が VG に含まれていることを確認
```

**LV の縮小（注意: 危険な操作）**:
```bash
# 警告: LV の縮小はデータ損失の危険がある。必ずバックアップを取ること。

# 手順 1: バックアップ
sudo tar -czf /root/home_backup.tar.gz /home

# 手順 2: アンマウント（/ や /var/log など使用中の FS は縮小不可）
sudo umount /home

# 手順 3: ファイルシステムのチェック
sudo e2fsck -f /dev/LVMGroup/home

# 手順 4: ファイルシステムの縮小（先にファイルシステムを縮小）
sudo resize2fs /dev/LVMGroup/home 3G

# 手順 5: LV の縮小
sudo lvreduce -L 3G /dev/LVMGroup/home

# 手順 6: 再マウント
sudo mount /dev/LVMGroup/home /home
```

**重要な注意事項**:
```
- LV の拡張は安全（オンラインで可能、データ損失なし）
- LV の縮小は危険（ファイルシステムを先に縮小する必要がある）
- 縮小の順序を間違えるとデータが失われる
- 順序: 拡張時 → LV拡張 → FS拡張
        縮小時 → FS縮小 → LV縮小
- / (root) パーティションの縮小は LiveCD からの作業が必要
```

---

### 演習 3: UFW ルール設計演習

**シナリオ**: Born2beRoot サーバーに以下の要件が追加された。適切な UFW ルールを設計せよ。

**要件 1: 基本構成（Born2beRoot 必須部分）**:
```bash
# SSH のみ許可
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow 4242
```

**要件 2: Bonus - WordPress 構成の追加**:
```bash
# SSH + HTTP (lighttpd) を許可
sudo ufw allow 4242
sudo ufw allow 80
```

**要件 3: 社内ネットワーク限定の管理サーバー**:
```bash
sudo ufw default deny incoming
sudo ufw default deny outgoing  # 送信も制限

# SSH: 社内からのみ
sudo ufw allow from 192.168.1.0/24 to any port 4242 proto tcp

# HTTP: 社内からのみ
sudo ufw allow from 192.168.1.0/24 to any port 80 proto tcp

# DNS 解決を許可
sudo ufw allow out 53

# NTP を許可（時刻同期）
sudo ufw allow out 123/udp

# パッケージ更新を許可
sudo ufw allow out 80/tcp
sudo ufw allow out 443/tcp
```

**要件 4: DMZ（非武装地帯）を模擬した構成**:
```bash
# Web サービス: どこからでもアクセス可能
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp

# SSH: 管理ネットワークからのみ
sudo ufw allow from 10.0.0.0/8 to any port 4242 proto tcp
```

**UFW ルール設計のチェックリスト**:
```
□ デフォルトポリシーは deny incoming になっているか
□ 必要最小限のポートだけが開放されているか
□ 可能であればソース IP を制限しているか
□ 使わなくなったルールを削除したか
□ ルールの順序は正しいか
□ IPv6 のルールも考慮したか
□ ufw status で設定を確認したか
```

---

### 演習 4: 設定ファイルの diff レビュー演習

**目的**: 設定ファイルの変更内容を読み取り、セキュリティへの影響を評価する能力を養う。

**問題 1: sshd_config の diff**:
```diff
--- /etc/ssh/sshd_config.orig
+++ /etc/ssh/sshd_config
@@ -13,7 +13,7 @@
 #Port 22
+Port 4242

-#PermitRootLogin prohibit-password
+PermitRootLogin no

-#PasswordAuthentication yes
+PasswordAuthentication yes

+MaxAuthTries 3
+LoginGraceTime 60
```

**分析**:
```
変更 1: Port 22 → 4242
- セキュリティ効果: 自動スキャンボットの回避
- 本質的なセキュリティ向上: なし（ポートスキャンで発見可能）
- 評価: 低リスク改善

変更 2: PermitRootLogin prohibit-password → no
- セキュリティ効果: root への直接 SSH ログインを完全禁止
- prohibit-password は公開鍵認証での root ログインを許可していた
- no はすべての方法での root ログインを拒否
- 評価: セキュリティ向上

変更 3: PasswordAuthentication yes（コメント解除）
- パスワード認証を明示的に有効化
- 注意: 公開鍵認証のみに制限する方がセキュリティは高い
- Born2beRoot では評価のためパスワード認証が必要
- 評価: 要件に適合するが、実運用ではリスク

変更 4: MaxAuthTries 3
- 3回の認証失敗で接続を切断
- 評価: ブルートフォース対策として有効

変更 5: LoginGraceTime 60
- 認証に60秒の制限時間を設定
- 評価: DoS 攻撃対策
```

**問題 2: login.defs の diff**:
```diff
--- /etc/login.defs.orig
+++ /etc/login.defs
-PASS_MAX_DAYS   99999
+PASS_MAX_DAYS   30
-PASS_MIN_DAYS   0
+PASS_MIN_DAYS   2
-PASS_WARN_AGE   7
+PASS_WARN_AGE   7
```

**分析**:
```
PASS_MAX_DAYS: 99999 → 30
- パスワードの最大有効期間を30日に制限
- パスワードが漏洩しても30日後には無効になる
- 注意: 短すぎると「Password1, Password2, ...」のパターン変更を誘発

PASS_MIN_DAYS: 0 → 2
- パスワード変更後2日間は再変更不可
- パスワードローテーション攻撃を防止:
  新パスワード → すぐ旧パスワードに戻す → ポリシーを実質無効化

重要な注意点:
- login.defs の変更は新規ユーザーにのみ適用される
- 既存ユーザーには chage コマンドで個別に適用する必要がある
  sudo chage -M 30 -m 2 -W 7 username
```

---

### 演習 5: トラブルシューティング演習

**問題 1: SSH で接続できない**

```
症状: ssh kaztakam@127.0.0.1 -p 4242 が失敗する
エラー: Connection refused

調査フローチャート:

1. sshd が動作しているか？
   └─ sudo systemctl status sshd
       ├─ active → 次のステップへ
       └─ inactive → sudo systemctl start sshd

2. ポートが正しいか？
   └─ sudo ss -tunlp | grep ssh
       ├─ :4242 → 次のステップへ
       └─ :22 → /etc/ssh/sshd_config を確認

3. UFW が許可しているか？
   └─ sudo ufw status
       ├─ 4242 ALLOW → 次のステップへ
       └─ 4242 がない → sudo ufw allow 4242

4. VirtualBox の Port Forwarding は正しいか？
   └─ VirtualBox の設定を確認
       ├─ ホスト 4242 → ゲスト 4242 → 次のステップへ
       └─ 設定なし → Port Forwarding を追加

5. ログを確認
   └─ sudo journalctl -u sshd -n 50
```

**問題 2: sudo が使えない**

```
症状: sudo ls → "kaztakam is not in the sudoers file"

調査フローチャート:

1. sudo グループに所属しているか？
   └─ id kaztakam
       ├─ groups に sudo が含まれる → 次のステップへ
       └─ sudo が含まれない → root で usermod -aG sudo kaztakam

2. sudoers ファイルに構文エラーがないか？
   └─ root でログインして visudo -c
       ├─ OK → 次のステップへ
       └─ エラー → root で visudo を実行して修正

3. sudoers.d/ のファイルに問題がないか？
   └─ root で ls -la /etc/sudoers.d/
       └─ 各ファイルを visudo -f で確認
```

**問題 3: monitoring.sh が wall メッセージを表示しない**

```
症状: cron で設定しているが wall メッセージが表示されない

調査フローチャート:

1. cron ジョブが設定されているか？
   └─ sudo crontab -l
       ├─ エントリがある → 次のステップへ
       └─ エントリがない → sudo crontab -e で追加

2. スクリプトのパスは正しいか？
   └─ ls -la /usr/local/bin/monitoring.sh
       ├─ 存在する → 次のステップへ
       └─ 存在しない → パスを確認

3. 実行権限があるか？
   └─ stat /usr/local/bin/monitoring.sh
       ├─ -rwxr-xr-x → 次のステップへ
       └─ 実行権限なし → sudo chmod +x /usr/local/bin/monitoring.sh

4. 手動実行でエラーがないか？
   └─ sudo bash -x /usr/local/bin/monitoring.sh
       ├─ 正常終了 → cron のログを確認
       └─ エラー → エラーの原因を修正

5. cron サービスが動作しているか？
   └─ sudo systemctl status cron
       ├─ active → sudo grep CRON /var/log/syslog
       └─ inactive → sudo systemctl start cron
```

**問題 4: パスワード変更が拒否される**

```
症状: passwd で新パスワードが全て拒否される

調査フローチャート:

1. エラーメッセージを確認
   └─ "too short" → minlen=10 に満たない
   └─ "not enough uppercase" → ucredit=-1（大文字が不足）
   └─ "not enough digits" → dcredit=-1（数字が不足）
   └─ "too many same characters" → maxrepeat=3（連続文字制限）
   └─ "based on username" → reject_username（ユーザー名を含む）
   └─ "not different enough" → difok=7（旧パスワードと類似）

2. 全条件を満たすパスワードを入力:
   - 10文字以上
   - 大文字1つ以上
   - 小文字1つ以上
   - 数字1つ以上
   - 同じ文字は3つまで連続可能
   - ユーザー名を含まない
   - 旧パスワードと7文字以上異なる

3. 例: Xb9mK2pL7w
```

**問題 5: UFW を有効化した後 SSH が切断された**

```
症状: ufw enable 後に SSH 接続が切断、再接続もできない

原因: ufw allow 4242 をする前に ufw enable した

対応方法:
1. VirtualBox のコンソール（GUI ウィンドウ）からログイン
   → コンソールは UFW の影響を受けない（ネットワークを経由しない）

2. UFW にルールを追加
   sudo ufw allow 4242

3. SSH 接続を再試行
   ssh kaztakam@127.0.0.1 -p 4242

予防策:
  → ufw allow 4242 を先に実行してから ufw enable する
  → ufw enable 前にスナップショットを取る
```

---

### 演習 6: サービス追加チェックリスト演習

**シナリオ**: Born2beRoot サーバーに新しいサービスを追加する必要がある。追加前に確認すべきセキュリティチェックリストを作成せよ。

**汎用チェックリスト**:

```
=== サービス追加前チェックリスト ===

1. サービスの必要性
   □ なぜこのサービスが必要か？
   □ 既存のサービスで代替できないか？
   □ 攻撃面がどの程度広がるか？

2. ネットワーク
   □ サービスはどのポートを使用するか？
   □ UFW にルールを追加する必要があるか？
   □ ローカルのみか、外部に公開するか？

3. 認証とアクセス制御
   □ デフォルト認証情報を変更したか？
   □ 不要なアカウントを無効化したか？

4. 権限
   □ サービスは専用ユーザーで動作するか（root 以外）？
   □ ファイルパーミッションは適切か？

5. ログ
   □ サービスのログは有効か？
   □ ログローテーションは設定されているか？

6. 更新
   □ サービスのバージョンは最新か？
   □ 既知の脆弱性はないか？

7. バックアップ
   □ 設定変更前にスナップショットを取得したか？

8. テスト
   □ サービスが正常に動作するか？
   □ 他のサービスに影響がないか？
```

---

## 6. 学習リソース

### 公式ドキュメント

- Debian Wiki: https://wiki.debian.org/
- LVM: https://wiki.debian.org/LVM
- UFW: https://wiki.debian.org/Uncomplicated%20Firewall%20(ufw)
- AppArmor: https://wiki.debian.org/AppArmor
- OpenSSH: https://www.openssh.com/manual.html
- Linux kernel documentation: https://www.kernel.org/doc/html/latest/

### man ページ（VM 内で確認可能）

```bash
man sshd_config    # SSH サーバーの設定オプション
man sudoers        # sudo の設定書式
man ufw            # UFW の使い方
man crontab        # cron の書式
man chage          # パスワード有効期限の管理
man pam_pwquality  # パスワード品質チェックモジュール
man wall           # 全ユーザーへのメッセージ送信
man lvm            # LVM の概要
man cryptsetup     # LUKS 暗号化の管理
man fstab          # ファイルシステムのマウント設定
man login.defs     # ログインのデフォルト設定
man usermod        # ユーザーアカウントの変更
man adduser        # ユーザーの追加
man systemctl      # systemd サービスの管理
man journalctl     # systemd ジャーナルログの閲覧
```

### 推奨書籍・資料

| カテゴリ | 資料名 | 対象レベル |
|---------|--------|----------|
| Linux 基礎 | "The Linux Command Line" (William Shotts) | 初級 |
| Linux 管理 | "UNIX and Linux System Administration Handbook" | 中級 |
| セキュリティ | CIS Benchmark for Debian Linux | 中級 |
| ネットワーク | "TCP/IP Illustrated" (W. Richard Stevens) | 中級-上級 |
| 暗号化 | "Serious Cryptography" (Jean-Philippe Aumasson) | 上級 |
| IaC | Terraform Up & Running (Yevgeniy Brikman) | 中級 |

---

## 7. 学習のヒント

### 効率的な学習法

1. **手を動かす**: ドキュメントを読むだけでなく、実際に VM で試す。「読む:作業 = 3:7」の比率を目指す
2. **壊す勇気を持つ**: VM はスナップショットで復元できる。恐れずに実験する。「壊して直す」経験が最も学びが深い
3. **エラーメッセージを読む**: エラーは学習の最大の機会。エラーメッセージをそのまま検索すると解決策が見つかることが多い
4. **一つずつ進める**: 複数の設定を同時に変更しない。一つ変更したら動作確認する。問題が発生した時に原因を特定しやすい
5. **「なぜ」を問う**: 設定方法だけでなく、その設定がなぜ必要かを理解する。評価では「なぜ」を必ず聞かれる
6. **スナップショットを活用**: 重要な変更の前にスナップショットを取る。失敗しても30秒で復元できる
7. **メモを取る**: 実行したコマンドと結果を記録する。後から見直せるようにする

### スナップショット戦略

```
推奨スナップショットポイント:
1. OS インストール直後（パーティション設定完了後）
2. ユーザー・グループ設定完了後
3. SSH + UFW 設定完了後
4. パスワードポリシー + sudo 設定完了後
5. monitoring.sh + cron 設定完了後
6. 全必須要件完了後（Bonus 着手前）
7. Bonus 完了後（signature.txt 生成前）

命名規則:
  snap_01_os_installed
  snap_02_users_done
  snap_03_ssh_ufw_done
  snap_04_password_sudo_done
  snap_05_monitoring_done
  snap_06_mandatory_complete
  snap_07_bonus_complete
```

### 評価前の最終チェック

```bash
# 全要件を一括確認するスクリプト
#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

check() {
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}[OK]${NC} $1"
    else
        echo -e "${RED}[NG]${NC} $1"
    fi
}

# OS
test "$(cat /etc/os-release | grep -c "Debian")" -gt 0
check "Debian がインストールされている"

# GUI
test "$(dpkg -l 2>/dev/null | grep -cE 'xorg|wayland|x11')" -eq 0
check "GUI がインストールされていない"

# Hostname
test "$(hostname)" = "kaztakam42"
check "ホスト名が kaztakam42"

# Users
id kaztakam | grep -q "sudo" 2>/dev/null
check "kaztakam が sudo グループに所属"

id kaztakam | grep -q "user42" 2>/dev/null
check "kaztakam が user42 グループに所属"

# SSH
grep -q "^Port 4242" /etc/ssh/sshd_config
check "SSH ポートが 4242"

grep -q "^PermitRootLogin no" /etc/ssh/sshd_config
check "Root ログインが禁止"

# UFW
sudo ufw status | grep -q "Status: active"
check "UFW が有効"

# Password Policy
grep -q "^PASS_MAX_DAYS.*30" /etc/login.defs
check "パスワード最大有効期間が 30 日"

grep -q "^PASS_MIN_DAYS.*2" /etc/login.defs
check "パスワード最小有効期間が 2 日"

grep -q "pam_pwquality" /etc/pam.d/common-password
check "pam_pwquality が設定されている"

# sudo
test -f /etc/sudoers.d/sudo_config
check "sudo 設定ファイルが存在"

test -d /var/log/sudo
check "sudo ログディレクトリが存在"

# AppArmor
sudo aa-status 2>/dev/null | grep -q "enforce"
check "AppArmor が enforce モード"

# Cron
sudo crontab -l 2>/dev/null | grep -q "monitoring.sh"
check "cron に monitoring.sh が設定されている"

# monitoring.sh
test -x /usr/local/bin/monitoring.sh
check "monitoring.sh に実行権限がある"

echo ""
echo "=== チェック完了 ==="
```

---

## 8. 学習タイムライン（推奨スケジュール）

### 1週間プラン（必須部分のみ）

| 日 | 内容 | 目標 |
|----|------|------|
| Day 1 | 背景知識の学習、VirtualBox セットアップ | 01-background.md を読了 |
| Day 2 | Debian インストール、パーティション設定 | lsblk で正しい構成を確認 |
| Day 3 | SSH + UFW の設定 | ホスト OS から SSH 接続成功 |
| Day 4 | パスワードポリシー + sudo の設定 | 全セキュリティ設定完了 |
| Day 5 | monitoring.sh の作成 + cron 設定 | wall メッセージが10分ごとに表示 |
| Day 6 | 全体テスト + 評価準備 | 検証スクリプトが全て OK |
| Day 7 | 評価リハーサル + signature.txt 生成 | 04-evaluation.md の全質問に回答可能 |

### 2週間プラン（Bonus 含む）

| 日 | 内容 | 目標 |
|----|------|------|
| Day 1-5 | 上記 1 週間プランと同じ | 必須部分完了 |
| Day 6 | スナップショット取得、lighttpd インストール | lighttpd が動作 |
| Day 7 | MariaDB + PHP のインストール | データベースが動作 |
| Day 8 | WordPress のインストールと設定 | WordPress が表示される |
| Day 9 | 追加サービス（Fail2ban 等）の設定 | 追加サービスが動作 |
| Day 10 | Bonus 全体のテスト | 全 Bonus 要件を確認 |
| Day 11-12 | 評価準備（深い理解） | 「なぜ」の質問に全て回答可能 |
| Day 13 | 評価リハーサル（ペアで練習） | 時間内に全デモを完了 |
| Day 14 | signature.txt 生成、最終確認 | 提出準備完了 |

### 評価で頻出の「なぜ」質問と回答ガイド

| 質問 | 回答のポイント |
|------|--------------|
| なぜ Debian を選んだか？ | 安定性、セキュリティアップデートの長期サポート、42 のコミュニティ情報の多さ |
| apt と aptitude の違いは？ | aptitude はより高度な依存関係解決機能を持つ。GUIモード対応。apt は推奨 |
| LVM とは何か？ | 物理ディスクを抽象化し、柔軟なパーティション管理を可能にする仕組み |
| なぜ SSH のポートを変更するのか？ | 自動スキャンボットの回避。本質的なセキュリティ向上ではないが、ノイズ削減 |
| なぜ root ログインを禁止するのか？ | 最小権限の原則。監査証跡の確保。パスワード漏洩時のリスク軽減 |
| AppArmor とは何か？ | MAC (Mandatory Access Control) の実装。プロセスのファイルアクセスを制限 |
| UFW とは何か？ | iptables/nftables のフロントエンド。ファイアウォールルールを簡単に管理 |
| パスワードポリシーはどこで設定するか？ | 有効期限: /etc/login.defs、強度: /etc/pam.d/common-password (pam_pwquality) |
| monitoring.sh を停止する方法は？ | crontab -e で行を削除/コメントアウト、または systemctl stop cron |
| LUKS とは何か？ | Linux Unified Key Setup。ディスク暗号化の標準。AES-256-XTS で保存データを暗号化 |
| cron と systemd timer の違いは？ | cron は伝統的なスケジューラ、systemd timer はログ統合・依存関係管理が可能 |
| sudo と su の違いは？ | sudo は特定コマンドを別ユーザーとして実行、su はユーザー切り替え |

---

## 9. 応用演習: セキュリティ検証

### 9.1 演習: パスワードクラッキングのシミュレーション

**目的**: パスワードポリシーの重要性を体験的に理解する

```bash
# 注意: この演習は自分の Born2beRoot 環境でのみ実施すること

# Step 1: パスワードハッシュの形式を確認
sudo cat /etc/shadow | grep kaztakam
# 出力例: kaztakam:$6$salt$hash:19700:2:30:7:::
# $6$ = SHA-512, salt = ソルト値, hash = ハッシュ値

# Step 2: ハッシュのアルゴリズムを確認
# $1$ = MD5 (非推奨)
# $5$ = SHA-256
# $6$ = SHA-512 (Debian デフォルト)
# $y$ = yescrypt (新しいシステム)

# Step 3: パスワード強度の確認
# 弱いパスワードでテスト（設定前に試す）
echo "password123" | pwscore
# 出力: 0 (非常に弱い)

echo "MyStr0ngP@ss!" | pwscore
# 出力: 100 (強い)

# Step 4: パスワードポリシーのテスト
# 各ルールが正しく動作するか確認
passwd  # パスワード変更を試みる

# 以下のパスワードが全て拒否されることを確認:
# "short"        → 10文字未満
# "alllowercase1" → 大文字なし（ucredit=-1 違反）
# "ALLUPPERCASE" → 数字なし（dcredit=-1 違反）
# "aaaaaBBBB1"   → 連続文字（maxrepeat=3 違反）
# "kaztakam42A1" → ユーザー名を含む（reject_username 違反）
```

### 9.2 演習: SSH セキュリティの検証

```bash
# Step 1: SSH の設定を確認
sudo sshd -T | grep -E "port|permitrootlogin|maxauthtries|passwordauthentication"
# port 4242
# permitrootlogin no
# maxauthtries 3
# passwordauthentication yes

# Step 2: root ログインの拒否を確認
ssh root@localhost -p 4242
# → Permission denied（パスワードが正しくても拒否される）

# Step 3: 存在しないユーザーでの接続を試行
ssh nonexistent@localhost -p 4242
# → 3回失敗後に切断される（MaxAuthTries）

# Step 4: SSH の使用している暗号アルゴリズムを確認
ssh -vv kaztakam@localhost -p 4242 2>&1 | grep "kex:"
# → 鍵交換アルゴリズムを確認
ssh -vv kaztakam@localhost -p 4242 2>&1 | grep "cipher:"
# → 暗号化アルゴリズムを確認

# Step 5: ホスト鍵のフィンガープリントを確認
ssh-keygen -lf /etc/ssh/ssh_host_ed25519_key.pub
# → サーバーの Ed25519 ホスト鍵のフィンガープリント
```

### 9.3 演習: UFW ルールの検証

```bash
# Step 1: 現在のルールを確認
sudo ufw status verbose
# → Default: deny (incoming), allow (outgoing), deny (routed)
# → 4242/tcp ALLOW IN Anywhere

# Step 2: iptables レベルでの確認
sudo iptables -L -n -v | head -30
# → UFW の背後にある実際の iptables ルールを確認

# Step 3: 特定ポートへの接続テスト（別のターミナルから）
# ポート 80 への接続（ブロックされるはず）
nc -zv localhost 80 2>&1
# → Connection refused（UFW がブロック）

# ポート 4242 への接続（許可されるはず）
nc -zv localhost 4242 2>&1
# → Connection to localhost 4242 port [tcp/*] succeeded!

# Step 4: UFW ログの確認
sudo cat /var/log/ufw.log | tail -20
# → ブロックされたパケットのログを確認
```

### 9.4 演習: sudo ログの分析

```bash
# Step 1: sudo で何かコマンドを実行
sudo whoami
sudo cat /etc/shadow | head -1
sudo ls /root

# Step 2: sudo ログを確認
sudo cat /var/log/sudo/sudo.log
# → 上記のコマンドが全て記録されていることを確認
# → TTY, PWD, USER, COMMAND が記録される

# Step 3: 不正なパスワードで sudo を試行
sudo -k  # sudo のキャッシュをクリア
sudo ls  # 間違ったパスワードを3回入力
# → "3 incorrect password attempts" がログに記録される

# Step 4: requiretty の動作確認
# スクリプトから sudo を実行してみる
echo '#!/bin/bash
sudo whoami' > /tmp/test_tty.sh
chmod +x /tmp/test_tty.sh
/tmp/test_tty.sh
# → requiretty が有効なら "sudo: a terminal is required" エラー
```

---

## 10. 応用演習: システム管理

### 10.1 演習: サービス管理の理解

```bash
# systemd の基本操作

# サービスの状態確認
systemctl status sshd
systemctl status ufw
systemctl status cron
systemctl status apparmor

# 全ての有効なサービスを一覧
systemctl list-units --type=service --state=running

# サービスの起動順序を確認
systemd-analyze blame | head -20
# → 各サービスの起動時間を表示

# サービスの依存関係を確認
systemctl list-dependencies sshd
# → sshd が依存するサービスのツリー表示

# サービスのログを確認
journalctl -u sshd --since "1 hour ago"
# → 直近1時間の sshd のログ

# 質問: 以下のそれぞれについて、何のサービスか説明できるか？
# - sshd.service
# - cron.service
# - ufw.service
# - apparmor.service
# - systemd-logind.service
# - systemd-journald.service
```

### 10.2 演習: ディスク管理の深掘り

```bash
# LVM の構造を完全に理解する

# Physical Volume (PV) の確認
sudo pvdisplay
# → PV Name, VG Name, PV Size, PE Size, Total PE, Free PE

# Volume Group (VG) の確認
sudo vgdisplay
# → VG Name, VG Size, PE Size, Total PE, Alloc PE, Free PE

# Logical Volume (LV) の確認
sudo lvdisplay
# → LV Name, VG Name, LV Size, # of segments

# 全体構造の視覚的な表示
sudo lsblk -f
# → NAME, FSTYPE, LABEL, UUID, MOUNTPOINT

# ディスク使用量の確認
df -hT
# → Filesystem, Type, Size, Used, Avail, Use%, Mounted on

# inode の使用量を確認
df -i
# → inode の枯渇もディスク容量枯渇と同様に問題を引き起こす

# LUKS 暗号化の確認
sudo cryptsetup status sda5_crypt
# → type, cipher, keysize, key location, device

# 質問への回答練習:
# Q: PV, VG, LV の関係を図で説明できるか？
# Q: なぜ LVM を使うのか？ 普通のパーティションとの違いは？
# Q: LUKS 暗号化はどの層で動作するか？
```

### 10.3 演習: ネットワーク診断

```bash
# ネットワーク設定の確認
ip addr show
# → インターフェース名、IP アドレス、MAC アドレス

# ルーティングテーブルの確認
ip route show
# → デフォルトゲートウェイ、サブネット

# DNS 設定の確認
cat /etc/resolv.conf
# → ネームサーバー

# 待ち受けポートの確認
sudo ss -tlnp
# → LISTEN 状態のポートと、そのプロセス
# → 4242 が sshd で待ち受けていることを確認

# アクティブな接続の確認
ss -tn
# → ESTABLISHED 状態の TCP 接続

# ネットワーク統計
ss -s
# → TCP, UDP, RAW の統計情報

# ARP テーブル（同一ネットワーク上の機器）
ip neigh show
```

---

## 11. 評価対策: よくある失敗と対処法

### 11.1 評価前の最終チェック

```bash
# 評価直前に確認すべき項目（チェックリスト）

# 1. VM が正常に起動するか
# → LUKS パスフレーズの入力 → ログイン画面が表示される

# 2. ホスト名が正しいか
hostname
# → <login>42 (例: kaztakam42)

# 3. SSH 接続ができるか（ホスト OS から）
# → VirtualBox のポートフォワーディングが設定済みか確認
# → ホスト OS のターミナルから:
ssh kaztakam@localhost -p 4242

# 4. signature.txt が最新か
# → VM を起動後にハッシュを取得しないこと！
# → 提出する signature.txt と .vdi のハッシュが一致するか確認
```

### 11.2 評価中によくある質問への対策

```
Q: 「新しいユーザーを作成して」
A:
  sudo adduser evaluator
  # → パスワード入力（ポリシーを満たすもの）
  sudo usermod -aG user42 evaluator
  # → user42 グループに追加
  id evaluator
  # → uid, gid, groups を確認

Q: 「新しい group を作成して」
A:
  sudo groupadd evaluating
  sudo usermod -aG evaluating evaluator
  id evaluator
  # → evaluating グループが追加されていることを確認

Q: 「UFW のルールを追加して」
A:
  sudo ufw allow 8080/tcp
  sudo ufw status
  # → 8080/tcp ALLOW IN が表示される
  # 評価後に削除:
  sudo ufw delete allow 8080/tcp

Q: 「monitoring.sh を一時的に停止して」
A:
  sudo crontab -e
  # → monitoring.sh の行をコメントアウト（# を先頭に追加）
  # 再開する場合は # を削除

Q: 「パスワードを変更して」
A:
  passwd
  # → 現在のパスワードを入力
  # → 新しいパスワードを入力（ポリシーを満たすもの）
  # → ポリシー違反のパスワードを試して、拒否されることも見せる

Q: 「パーティション構成を見せて」
A:
  lsblk
  # → 暗号化されたパーティションとLVMの構成が表示される
```

### 11.3 よくあるトラブルと復旧手順

| トラブル | 症状 | 復旧手順 |
|---------|------|---------|
| VM が起動しない | VirtualBox がエラー表示 | Settings → System → Enable PAE/NX をチェック |
| LUKS パスフレーズが通らない | 起動時にパスフレーズ拒否 | キーボードレイアウトの確認（US配列に注意）|
| SSH 接続できない | Connection refused | `sudo systemctl status sshd`、ポートフォワーディング確認 |
| sudo が使えない | user is not in sudoers | `su -` で root → `usermod -aG sudo kaztakam` |
| UFW でロックアウト | SSH が繋がらない | VM のコンソールから直接ログイン → `sudo ufw allow 4242` |
| monitoring.sh がエラー | wall が表示されない | `bash -x /usr/local/bin/monitoring.sh` でデバッグ |
| cron が動作しない | 10分経ってもメッセージなし | `sudo systemctl status cron`、`sudo crontab -l` で確認 |
| signature.txt が不一致 | 提出後にハッシュ不一致 | VM を起動せずに `shasum *.vdi` で再生成 |
| ホスト名が違う | hostname が <login>42 でない | `sudo hostnamectl set-hostname <login>42` |
| AppArmor が無効 | aa-status で loaded だが enforce なし | `sudo aa-enforce /etc/apparmor.d/*` |

---

## 12. 発展学習: Born2beRoot の先へ

### 12.1 次のステップ

Born2beRoot で学んだスキルを発展させるための学習パス:

```
Born2beRoot の学習        →    次に学ぶべきこと

1. Linux 基礎             →    Linux Foundation Certified (LFCS)
   (ファイルシステム,            Shell scripting
    プロセス管理,                 systemd の深い理解
    ユーザー管理)                 カーネルパラメータの理解

2. ネットワーク            →    iptables/nftables の直接操作
   (SSH, UFW)                    VPN (WireGuard, OpenVPN)
                                  ネットワーク名前空間

3. セキュリティ            →    SELinux (AppArmor の次)
   (AppArmor, LUKS,              Fail2ban
    パスワードポリシー)           auditd (監査ログ)
                                  CIS Benchmark 完全準拠

4. 監視                    →    Prometheus + Grafana
   (monitoring.sh)                Node Exporter
                                  Alertmanager

5. 自動化                  →    Ansible (構成管理)
   (手動設定)                     Terraform (IaC)
                                  Docker / Kubernetes
```

### 12.2 関連する 42 プロジェクトとの繋がり

| Born2beRoot のスキル | 関連する 42 プロジェクト |
|---------------------|----------------------|
| パーティション管理、ファイルシステム | ft_linux |
| ネットワーク設定 | NetPractice |
| サービス管理、デーモン | Inception (Docker) |
| セキュリティ設定 | ft_shield |
| シェルスクリプト | minishell |
| システム管理 | cloud-1 |

### 12.3 実務で役立つ追加スキル

```
Born2beRoot → 実務への橋渡し:

1. コンテナ技術:
   Born2beRoot: VM の手動管理
   → Docker: コンテナによる軽量な仮想化
   → Kubernetes: コンテナオーケストレーション
   → 42 の Inception プロジェクトで学べる

2. Infrastructure as Code:
   Born2beRoot: 手動設定
   → Ansible: 構成管理の自動化
   → Terraform: インフラのコード化
   → 05-solution-terraform.md で詳細を学べる

3. 継続的インテグレーション:
   Born2beRoot: 手動テスト
   → GitHub Actions: テストの自動化
   → Jenkins: CI/CD パイプライン

4. クラウドサービス:
   Born2beRoot: VirtualBox (ローカル VM)
   → AWS EC2: クラウド上の VM
   → GCP Compute Engine: Google のクラウド
   → Azure VM: Microsoft のクラウド
```

---

## 13. クイックリファレンス

### 13.1 必須コマンド一覧

```bash
# ============================
# システム情報
# ============================
hostname                    # ホスト名表示
hostnamectl                 # ホスト名の詳細情報
uname -a                   # カーネル情報
cat /etc/os-release         # OS バージョン

# ============================
# ユーザー・グループ管理
# ============================
id <user>                   # ユーザーの UID, GID, グループ
groups <user>               # ユーザーの所属グループ
adduser <user>              # ユーザー追加（対話式）
usermod -aG <group> <user>  # グループに追加
groupadd <group>            # グループ作成
getent group <group>        # グループのメンバー一覧
passwd <user>               # パスワード変更
chage -l <user>             # パスワードポリシーの確認

# ============================
# SSH
# ============================
sudo systemctl status sshd  # SSH サービスの状態
sudo ss -tlnp | grep sshd  # SSH の待ち受けポート確認
sudo sshd -T                # SSH の全設定を表示

# ============================
# UFW
# ============================
sudo ufw status verbose     # ルールの詳細表示
sudo ufw allow <port>/tcp   # ポート開放
sudo ufw delete allow <port>/tcp  # ルール削除
sudo ufw enable             # UFW 有効化
sudo ufw disable            # UFW 無効化（注意）

# ============================
# LVM
# ============================
lsblk                       # ブロックデバイスのツリー表示
sudo pvdisplay              # Physical Volume の情報
sudo vgdisplay              # Volume Group の情報
sudo lvdisplay              # Logical Volume の情報
df -hT                      # マウントポイントと使用量

# ============================
# LUKS
# ============================
sudo cryptsetup status <name>    # 暗号化の状態
sudo cryptsetup luksDump /dev/sda5  # LUKS ヘッダーの情報

# ============================
# AppArmor
# ============================
sudo aa-status              # AppArmor の状態
sudo aa-enforce <profile>   # プロファイルを enforce モードに
sudo aa-complain <profile>  # プロファイルを complain モードに

# ============================
# sudo
# ============================
sudo visudo                 # sudoers ファイルの安全な編集
sudo cat /var/log/sudo/sudo.log  # sudo ログの確認

# ============================
# cron
# ============================
sudo crontab -l             # root の cron ジョブ一覧
sudo crontab -e             # root の cron ジョブ編集
sudo systemctl status cron  # cron サービスの状態

# ============================
# パスワードポリシー
# ============================
grep PASS /etc/login.defs   # 有効期限の設定
cat /etc/pam.d/common-password  # 複雑さの設定
pwscore                     # パスワード強度の確認（入力式）
```

### 13.2 設定ファイルの場所

| 設定内容 | ファイルパス |
|---------|------------|
| SSH 設定 | /etc/ssh/sshd_config |
| UFW ルール | /etc/ufw/ |
| パスワード有効期限 | /etc/login.defs |
| パスワード強度 | /etc/pam.d/common-password, /etc/security/pwquality.conf |
| sudo 設定 | /etc/sudoers.d/sudo_config |
| sudo ログ | /var/log/sudo/sudo.log |
| ホスト名 | /etc/hostname, /etc/hosts |
| ユーザー情報 | /etc/passwd |
| パスワードハッシュ | /etc/shadow |
| グループ情報 | /etc/group |
| AppArmor プロファイル | /etc/apparmor.d/ |
| cron ジョブ | /var/spool/cron/crontabs/ |
| monitoring.sh | /usr/local/bin/monitoring.sh |
| fstab（マウント設定） | /etc/fstab |
| ネットワーク設定 | /etc/network/interfaces |
| SSH ホスト鍵 | /etc/ssh/ssh_host_*_key |
| SSH 認証ログ | /var/log/auth.log |
| カーネルログ | /var/log/kern.log |
| UFW ログ | /var/log/ufw.log |
