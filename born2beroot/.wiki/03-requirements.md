# 03 - 要件詳細 (Detailed Requirements)

Born2beRoot プロジェクトの全要件をチェックリスト形式で整理する。各要件について「何をするか」だけでなく、「なぜその要件があるのか」を具体的な攻撃シナリオとセットで解説する。さらに、設定ファイルの場所、構文、パラメータの意味、検証コマンドと期待される出力、よくある設定ミスとその対処法を網羅する。

---

## 1. 必須要件チェックリスト

### 1.1 VM 基本設定

- [ ] VirtualBox（または UTM）で VM を作成している
- [ ] 最新安定版の **Debian** または **Rocky Linux** をインストールしている
- [ ] グラフィカルインターフェースが一切インストールされていない（X.org, Wayland 等なし）
- [ ] VM 起動時に LUKS パスフレーズの入力が求められる

**なぜ GUI を排除するのか - 攻撃面の最小化**:

GUI 環境は数百のパッケージ（Xorg, Mesa, GTK, Qt, libX11 等）で構成される。各パッケージは潜在的な脆弱性のソースとなる。サーバーでは GUI は不要であり、SSH でのリモート管理で十分。不要なソフトウェアを入れないことが最も基本的なセキュリティ対策。

#### 検証コマンドと期待される出力

```bash
# OS の確認
cat /etc/os-release
# 期待される出力:
# PRETTY_NAME="Debian GNU/Linux 12 (bookworm)"
# NAME="Debian GNU/Linux"
# VERSION_ID="12"
# VERSION="12 (bookworm)"
# ID=debian
# ...

# GUI がインストールされていないことの確認
dpkg -l | grep -E "xorg|wayland|x11|xserver"
# 期待される出力: 何も表示されない

# より厳密な確認
systemctl get-default
# 期待される出力: multi-user.target
# NG の例: graphical.target ← GUI が設定されている

# デスクトップ環境の確認
dpkg -l | grep -E "gnome|kde|xfce|lxde|mate|cinnamon"
# 期待される出力: 何も表示されない

# カーネルバージョン
uname -r
# 出力例: 6.1.0-XX-amd64
```

#### よくある設定ミス

| ミス | 症状 | 対処法 |
|-----|------|--------|
| tasksel で GUI をインストール | graphical.target になる | `sudo apt purge xorg* && sudo systemctl set-default multi-user.target` |
| デスクトップ環境が混入 | ディスク容量が不足 | `sudo apt purge gnome* kde*` 等で削除 |

---

### 1.2 パーティション構成

#### 必須（最低要件）

- [ ] 最低 **2つ** の暗号化パーティションが LVM を使用している
- [ ] `/boot` パーティションが存在する（暗号化なし）

**なぜ暗号化パーティション + LVM が要件なのか**:

暗号化: ディスクの物理的盗難からデータを保護する。データセンターでは物理セキュリティが万全であっても、ディスクの廃棄時やオフサイトバックアップの輸送時にデータが漏洩するリスクがある。

LVM: 従来のパーティション管理では、一度設定したサイズの変更が困難。LVM により、運用中のサーバーでもストレージ構成を柔軟に変更できる。クラウド環境でのディスク拡張に相当する概念。

**なぜ /boot は暗号化しないのか**:

GRUB ブートローダーがカーネルと initramfs を読み込む必要があるため。GRUB 2 は LUKS1 をサポートするが、設定が複雑になり、パスフレーズの入力が2回必要になる場合がある（GRUB用とシステム用）。/boot を暗号化しない構成がシンプルで確実。

#### 検証コマンドと期待される出力

```bash
# パーティション構成の確認
lsblk
# 必須構成の期待される出力:
# NAME                    MAJ:MIN RM  SIZE RO TYPE  MOUNTPOINT
# sda                       8:0    0   20G  0 disk
# ├─sda1                    8:1    0  487M  0 part  /boot
# └─sda2                    8:2    0    1K  0 part
#   └─sda5                  8:5    0 19.5G  0 part
#     └─sda5_crypt        254:0    0 19.5G  0 crypt
#       ├─LVMGroup-root   254:1    0  9.8G  0 lvm   /
#       └─LVMGroup-swap   254:2    0  2.3G  0 lvm   [SWAP]

# ポイント:
# - TYPE=crypt があること → LUKS 暗号化が有効
# - TYPE=lvm があること → LVM が使用されている
# - /boot が暗号化の外にあること

# LVM の確認
sudo pvdisplay
# 期待: PV Name に /dev/mapper/sda5_crypt 等が表示される

sudo vgdisplay
# 期待: VG Name が LVMGroup（または設定した名前）

sudo lvdisplay
# 期待: 少なくとも root と swap の LV が表示される

# 暗号化の確認
cat /etc/crypttab
# 期待される出力例:
# sda5_crypt UUID=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx none luks

sudo cryptsetup status sda5_crypt
# 期待される出力:
# /dev/mapper/sda5_crypt is active and is in use.
#   type:    LUKS2
#   cipher:  aes-xts-plain64
#   keysize: 512 bits
#   ...

# マウントの確認
mount | grep -E "^/dev"
# 期待: LVMGroup-root が / にマウント
# 期待: sda1 が /boot にマウント

# スワップの確認
swapon --show
# 期待: /dev/dm-X が表示される
```

#### よくある設定ミス

| ミス | 症状 | 対処法 |
|-----|------|--------|
| /boot も暗号化してしまった | ブートできない | インストールやり直し |
| LVM を使わずにパーティション分割 | lsblk に lvm が表示されない | インストールやり直し |
| 暗号化パーティションが1つだけ | 最低2つ必要 | 要件を満たすパーティション構成で再インストール |
| VG 名が subject と異なる | 名前が違うだけなら通常問題ない | vgrename で変更可能 |

#### Bonus パーティション構成

- [ ] 以下のパーティションが存在する:

| LV 名 | マウントポイント | 目的 | なぜ分離するのか | 確認 |
|--------|----------------|------|----------------|------|
| root | `/` | OS の基本ファイル | 他が満杯でもOSが動作 | [ ] |
| swap | `[SWAP]` | 仮想メモリ | メモリ不足時の退避先 | [ ] |
| home | `/home` | ユーザーデータの隔離 | OS再インストールでもデータ保持 | [ ] |
| var | `/var` | 可変データ（ログ、キャッシュ）の隔離 | ログ肥大化からOSを保護 | [ ] |
| srv | `/srv` | サービスデータの隔離 | Webサーバー等のデータ管理 | [ ] |
| tmp | `/tmp` | 一時ファイルの隔離 | noexec設定で攻撃防止 | [ ] |
| var--log | `/var/log` | ログの /var からの隔離 | ログ爆発攻撃対策 | [ ] |

**具体的な攻撃シナリオ**:

**シナリオ1: ログ爆発攻撃 (Log Flooding Attack)**
```
攻撃の流れ:
1. 攻撃者が SSH で大量のログイン失敗を発生させる
   (1秒に100回の認証失敗 × 24時間 = 864万行のログ)
2. /var/log/auth.log が急激に肥大化
3. /var/log が / と同じパーティションの場合:
   → / パーティションが満杯
   → OS が新しいファイルを作成できない
   → サービスが停止する

対策: /var/log を別パーティションにする
  → ログが満杯になっても OS は動作を続ける
  → logrotate でログを自動ローテーション
```

**シナリオ2: /tmp を使った攻撃**
```
攻撃の流れ:
1. Web アプリケーションの脆弱性を突く
2. /tmp に悪意あるスクリプトをアップロード
3. そのスクリプトを実行して権限昇格

対策: /tmp を別パーティションにして noexec オプションを設定
  → /tmp 内のファイルは実行できない
  → 攻撃スクリプトの実行を防止
```

```bash
# Bonus パーティション構成の確認
lsblk
# 期待される出力:
# NAME                    TYPE  MOUNTPOINT
# sda                     disk
# ├─sda1                  part  /boot
# ├─sda2                  part
# │ └─sda5                part
# │   └─sda5_crypt        crypt
# │     ├─LVMGroup-root   lvm   /
# │     ├─LVMGroup-swap   lvm   [SWAP]
# │     ├─LVMGroup-home   lvm   /home
# │     ├─LVMGroup-var    lvm   /var
# │     ├─LVMGroup-srv    lvm   /srv
# │     ├─LVMGroup-tmp    lvm   /tmp
# │     └─LVMGroup-var--log lvm /var/log

# 全パーティションの容量確認
df -h
# 各パーティションが適切なサイズで表示されること

# マウントオプションの確認
mount | grep -E "tmp|var"
# /tmp に noexec が設定されているか確認
# 理想: /dev/mapper/LVMGroup-tmp on /tmp type ext4 (rw,nosuid,nodev,noexec,relatime)
```

---

### 1.3 ホスト名とユーザー

- [ ] hostname が `login42` の形式になっている（例: `kaztakam42`）
- [ ] root 以外に、login名のユーザーが存在する（例: `kaztakam`）
- [ ] そのユーザーが `user42` グループに所属している
- [ ] そのユーザーが `sudo` グループに所属している

**なぜ hostname を login42 形式にするのか**:
ネットワーク上でサーバーを識別するため。複数のサーバーを管理する環境では、hostname が管理者の識別に役立つ。ログに hostname が記録されるため、問題発生時のトラブルシューティングにも重要。

#### 検証コマンドと期待される出力

```bash
# ホスト名の確認
hostname
# 期待: kaztakam42

hostnamectl
# 期待:
#    Static hostname: kaztakam42
#          Icon name: computer-vm
#            Chassis: vm
#     Virtualization: oracle

# /etc/hostname の内容
cat /etc/hostname
# 期待: kaztakam42

# /etc/hosts の内容
cat /etc/hosts
# 期待:
# 127.0.0.1    localhost
# 127.0.1.1    kaztakam42

# ユーザーの確認
id kaztakam
# 期待: uid=1000(kaztakam) gid=1000(kaztakam) groups=1000(kaztakam),27(sudo),1001(user42)

# グループの確認
groups kaztakam
# 期待: kaztakam : kaztakam sudo user42

# user42 グループの確認
getent group user42
# 期待: user42:x:1001:kaztakam

# sudo グループの確認
getent group sudo
# 期待: sudo:x:27:kaztakam
```

#### ホスト名の変更方法

```bash
sudo hostnamectl set-hostname kaztakam42
sudo nano /etc/hosts
# 127.0.0.1    localhost
# 127.0.1.1    kaztakam42
```

#### よくある設定ミス

| ミス | 症状 | 対処法 |
|-----|------|--------|
| /etc/hosts を更新していない | sudo 実行時に名前解決で遅延 | /etc/hosts に 127.0.1.1 hostname を追加 |
| user42 グループを作り忘れ | getent group user42 が空 | `sudo groupadd user42 && sudo usermod -aG user42 username` |
| -a を忘れて usermod | sudo グループから外れた | root でログインして `usermod -aG sudo username` |

---

### 1.4 SSH 設定

- [ ] SSH サービスが **port 4242** で動作している
- [ ] SSH の root ログインが**無効化**されている
- [ ] SSH サービスが起動時に自動起動する

**なぜ root SSH ログインを禁止するのか - 攻撃シナリオ**:

root は Linux の最高権限アカウントであり、ユーザー名が全システムで共通（"root"）である。これにより:
1. 攻撃者はユーザー名を推測する必要がなくなる（root は確実に存在する）
2. root のパスワードが漏洩すると、システム全体が即座に侵害される
3. root ログインでは「誰が」ログインしたかの追跡が困難

root SSH を禁止し、一般ユーザー + sudo を使用することで:
1. 攻撃者はユーザー名とパスワードの両方を推測する必要がある
2. sudo ログにより「誰が何をしたか」が記録される
3. 一般ユーザーの権限では被害が限定される

**なぜ port 22 ではなく 4242 を使うのか**:

port 22 はインターネット上のボットネットによって常時スキャンされている。port を変更することで自動化された攻撃を回避できる。ただし、これは「隠蔽によるセキュリティ (Security through obscurity)」であり、本質的な防御ではない。nmap 等で全ポートをスキャンされれば発見される。他の対策（パスワードポリシー、ファイアウォール）と組み合わせることが重要。

#### 設定ファイル: `/etc/ssh/sshd_config`

```
# Born2beRoot 必須設定
Port 4242                      # デフォルト22から変更
PermitRootLogin no             # root SSH ログイン禁止

# 各パラメータの詳細解説:

# Port <number>
#   SSH が待ち受けるポート番号
#   デフォルト: 22
#   設定値: 4242（Born2beRoot 要件）
#   範囲: 1-65535（1024未満は root 権限必要）

# PermitRootLogin <yes|no|prohibit-password|forced-commands-only>
#   yes:                root ログインを許可
#   no:                 root ログインを完全に禁止（Born2beRoot 要件）
#   prohibit-password:  パスワード認証を禁止（公開鍵のみ許可）
#   forced-commands-only: 強制コマンドのみ許可

# その他の重要なパラメータ（Born2beRoot では任意）:
# PasswordAuthentication yes     パスワード認証の有効/無効
# PubkeyAuthentication yes       公開鍵認証の有効/無効
# MaxAuthTries 6                 認証試行回数の上限
# LoginGraceTime 120             認証タイムアウト（秒）
# AllowUsers user1 user2         許可するユーザーの限定
# Banner /etc/ssh/banner         ログイン前バナー
# ClientAliveInterval 300        クライアント生存確認間隔
# ClientAliveCountMax 3          生存確認の最大回数
# X11Forwarding no               X11転送の無効化
# UsePAM yes                     PAM の使用（パスワードポリシー適用に必要）
```

#### 検証コマンドと期待される出力

```bash
# ポート設定の確認
grep "^Port" /etc/ssh/sshd_config
# 期待: Port 4242

# root ログイン設定の確認
grep "^PermitRootLogin" /etc/ssh/sshd_config
# 期待: PermitRootLogin no

# sshd サービスの状態確認
sudo systemctl status sshd
# 期待:
#   Loaded: loaded (/lib/systemd/system/ssh.service; enabled; ...)
#   Active: active (running)
#   Main PID: XXXX (sshd)
# 注目: "enabled" = 自動起動有効、"active (running)" = 実行中

# ポート 4242 でリスニングしているか確認
sudo ss -tunlp | grep ssh
# 期待: tcp  LISTEN  0  128  0.0.0.0:4242  0.0.0.0:*  users:(("sshd",pid=XXX,fd=3))

# SSH 設定ファイルの構文チェック
sudo sshd -t
# 期待: 何も表示されない（エラーなし）

# root での SSH ログインが拒否されることの確認
ssh -p 4242 root@localhost
# 期待: Permission denied

# 一般ユーザーでの SSH ログインが成功することの確認
ssh -p 4242 kaztakam@localhost
# 期待: パスワード入力プロンプトが表示され、正しいパスワードでログイン成功
```

#### よくある設定ミス

| ミス | 症状 | 対処法 |
|-----|------|--------|
| Port の行がコメントアウト | デフォルトの22で動作 | `#Port` の `#` を削除 |
| PermitRootLogin がコメントアウト | デフォルトでroot許可の場合がある | 明示的に `PermitRootLogin no` を追記 |
| 設定変更後に再起動していない | 変更が反映されない | `sudo systemctl restart sshd` |
| sshd_config に構文エラー | サービスが起動しない | `sudo sshd -t` でチェック |
| UFW で 4242 を許可していない | 接続がタイムアウト | `sudo ufw allow 4242` |
| ポートフォワーディング未設定 | ホストからゲストに接続不可 | VirtualBox で設定 |

---

### 1.5 UFW ファイアウォール

- [ ] UFW がインストールされている
- [ ] UFW が有効（active）である
- [ ] port 4242 のみが許可されている
- [ ] UFW が起動時に自動有効化される

**なぜ port 4242 のみを開放するのか - 攻撃面の最小化**:

開放しているポートの数は、攻撃者にとっての「入口」の数に等しい。各ポートで動作するサービスには脆弱性が存在する可能性がある。1つのポートのみ開放することで:
- 攻撃対象がSSHサービスのみに限定される
- 仮に不要なサービスが誤って起動しても、ファイアウォールがアクセスをブロックする
- ネットワークスキャンで発見される情報が最小限になる

#### 設定手順

```bash
# 1. UFW のインストール
sudo apt install ufw

# 2. デフォルトポリシーの設定
sudo ufw default deny incoming      # 受信: 拒否（全て遮断）
sudo ufw default allow outgoing     # 送信: 許可（外部通信は許可）

# 3. port 4242 の許可
sudo ufw allow 4242
# 内部で生成される iptables ルール:
# -A ufw-user-input -p tcp --dport 4242 -j ACCEPT
# -A ufw-user-input -p udp --dport 4242 -j ACCEPT

# 4. UFW の有効化
sudo ufw enable
# 警告: "Command may disrupt existing ssh connections"
# → 既に SSH で接続中の場合、port 4242 の許可ルールが設定されていれば問題ない
```

#### 検証コマンドと期待される出力

```bash
# UFW の状態確認
sudo ufw status verbose
# 期待される出力:
# Status: active
# Logging: on (low)
# Default: deny (incoming), allow (outgoing), disabled (routed)
# New profiles: skip
#
# To                         Action      From
# --                         ------      ----
# 4242                       ALLOW IN    Anywhere
# 4242 (v6)                  ALLOW IN    Anywhere (v6)

# 番号付きルールの表示
sudo ufw status numbered
# 期待される出力:
#      To                         Action      From
#      --                         ------      ----
# [ 1] 4242                       ALLOW IN    Anywhere
# [ 2] 4242 (v6)                  ALLOW IN    Anywhere (v6)

# UFW が自動起動されるか確認
sudo systemctl is-enabled ufw
# 期待: enabled

cat /etc/ufw/ufw.conf | grep ENABLED
# 期待: ENABLED=yes

# port 4242 以外が閉じていることの確認
# （ゲスト VM 内で確認）
ss -tunlp
# sshd が 4242 で LISTEN していること
# 他のサービスが意図せずリスニングしていないこと
```

#### 不要なルールの削除

```bash
# ルール番号で削除
sudo ufw status numbered
sudo ufw delete 3    # 番号 3 のルールを削除

# ルール指定で削除
sudo ufw delete allow 80

# Bonus で port 80 を追加した場合:
sudo ufw allow 80          # lighttpd 用に追加
sudo ufw status numbered   # 確認
```

#### よくある設定ミス

| ミス | 症状 | 対処法 |
|-----|------|--------|
| ufw enable する前にルールを追加していない | SSH 接続が切れる | コンソールからログインして `ufw allow 4242 && ufw enable` |
| port 22 のルールが残っている | 不要なポートが開いている | `sudo ufw delete allow 22` |
| default が allow になっている | 全ポートが開いている | `sudo ufw default deny incoming` |
| IPv6 ルールが欠けている | IPv6 経由のアクセスがブロックされない | `sudo ufw allow 4242` は自動で v6 も追加 |

---

### 1.6 パスワードポリシー

#### 有効期限ポリシー

- [ ] パスワードの有効期限が **30日** に設定されている
- [ ] パスワード変更の最小間隔が **2日** に設定されている
- [ ] 有効期限の **7日前** に警告が表示される

**なぜ30日で期限切れにするのか**:
パスワードが漏洩した場合の被害期間を最大30日に限定する。パスワードの漏洩は発覚が遅れることが多いため、定期的な変更が被害を軽減する。NIST の最新ガイドライン（SP 800-63B）では定期的なパスワード変更は推奨されなくなっているが、Born2beRoot は教育目的でこのポリシーを採用している。

**なぜ最小2日間の変更禁止があるのか**:
ユーザーがパスワード変更を30回繰り返して元のパスワードに戻す「パスワード周回」を防止する。difok=7（旧パスワードと7文字以上異なる必要）と組み合わせることで、同じパスワードの再利用を困難にする。

#### 設定ファイル: `/etc/login.defs`

```
# パスワード有効期限の設定

PASS_MAX_DAYS   30
# パスワードの最大有効期間（日数）
# デフォルト: 99999（約273年 = 実質無期限）
# 設定値: 30（Born2beRoot 要件）
# 根拠: 漏洩時の被害期間を最大30日に制限

PASS_MIN_DAYS   2
# パスワード変更の最小間隔（日数）
# デフォルト: 0（制限なし）
# 設定値: 2（Born2beRoot 要件）
# 根拠: パスワード周回攻撃の防止

PASS_WARN_AGE   7
# 有効期限前の警告日数
# デフォルト: 7
# 設定値: 7（Born2beRoot 要件）
# 根拠: ユーザーにパスワード変更を促す猶予期間

# その他の関連設定:
# ENCRYPT_METHOD SHA512    パスワードハッシュアルゴリズム
# SHA_CRYPT_MIN_ROUNDS 5000  ハッシュの最小反復回数
# SHA_CRYPT_MAX_ROUNDS 500000  ハッシュの最大反復回数
```

#### 検証コマンドと期待される出力

```bash
# login.defs の設定確認
grep "^PASS_MAX_DAYS" /etc/login.defs
# 期待: PASS_MAX_DAYS   30

grep "^PASS_MIN_DAYS" /etc/login.defs
# 期待: PASS_MIN_DAYS   2

grep "^PASS_WARN_AGE" /etc/login.defs
# 期待: PASS_WARN_AGE   7

# ユーザーの実際のパスワードポリシー確認
sudo chage -l kaztakam
# 期待される出力:
# Last password change                    : Jan 15, 2024
# Password expires                        : Feb 14, 2024
# Password inactive                       : never
# Account expires                         : never
# Minimum number of days between password change : 2
# Maximum number of days between password change : 30
# Number of days of warning before password expires : 7

# root のポリシーも確認
sudo chage -l root
# 同様の出力が表示されること
```

#### 既存ユーザーへの適用

```bash
# login.defs の変更は新規ユーザーにのみ適用される
# 既存ユーザーには chage で個別に設定:

sudo chage -M 30 kaztakam     # 最大有効期限 30日
sudo chage -m 2 kaztakam      # 最小変更間隔 2日
sudo chage -W 7 kaztakam      # 警告 7日前

sudo chage -M 30 root
sudo chage -m 2 root
sudo chage -W 7 root
```

#### パスワード強度ポリシー

- [ ] `libpam-pwquality` がインストールされている
- [ ] 最小 **10文字**
- [ ] **大文字** を最低1文字含む
- [ ] **小文字** を最低1文字含む
- [ ] **数字** を最低1文字含む
- [ ] **3文字以上** の同一文字連続を禁止
- [ ] **ユーザー名** を含むことを禁止
- [ ] 旧パスワードと最低 **7文字** 異なる（root には適用されない場合あり）

#### 設定ファイル: `/etc/pam.d/common-password`

```
password    requisite    pam_pwquality.so retry=3 minlen=10 ucredit=-1 dcredit=-1 lcredit=-1 maxrepeat=3 reject_username difok=7 enforce_for_root
```

各パラメータの詳細解説:

| パラメータ | 値 | 意味 | なぜこの値なのか |
|-----------|---|------|----------------|
| `retry` | 3 | パスワード入力の再試行回数 | 3回でバランスよく、無限試行を防ぐ |
| `minlen` | 10 | 最小文字数 | 59.5 bits のエントロピーを確保（ブルートフォース耐性約26年） |
| `ucredit` | -1 | 大文字の最小必要数（負の値 = 最低N文字） | 文字種を増やしてエントロピーを向上 |
| `dcredit` | -1 | 数字の最小必要数 | 辞書攻撃への耐性向上 |
| `lcredit` | -1 | 小文字の最小必要数 | 基本的な文字種要件 |
| `maxrepeat` | 3 | 同一文字の連続の最大数 | "aaaa" のような弱いパターンを防止 |
| `reject_username` | (有効) | ユーザー名を含むパスワードを拒否 | 推測容易なパスワードの防止 |
| `difok` | 7 | 旧パスワードと異なる最小文字数 | パスワードの実質的な変更を強制 |
| `enforce_for_root` | (有効) | root にもポリシーを適用 | root も同じセキュリティ基準を適用 |

```
ucredit/dcredit/lcredit の値の解説:

正の値: その文字種1文字あたりのクレジット（ボーナス）
  ucredit=1  → 大文字1文字で minlen 要件が1緩和される
  例: minlen=10, ucredit=1 → 大文字1文字含めば実質9文字でOK

負の値: その文字種の最低必要数（Born2beRoot で使用）
  ucredit=-1 → 大文字が最低1文字必要
  ucredit=-2 → 大文字が最低2文字必要
```

#### パスワードの検証

```bash
# pam_pwquality がインストールされているか
dpkg -l | grep libpam-pwquality
# 期待: ii  libpam-pwquality ...

# 設定の確認
grep "pam_pwquality" /etc/pam.d/common-password
# 期待: password  requisite  pam_pwquality.so retry=3 minlen=10 ...

# パスワード変更テスト（以下のパスワードは拒否されるべき）
passwd
# "short"        → 10文字未満で拒否
# "alllowercase" → 大文字なしで拒否
# "ALLUPPERCASE" → 小文字なしで拒否
# "NoDigitsHere" → 数字なしで拒否
# "Aaaa1234567"  → 'a' が4連続で拒否（maxrepeat=3）
# "Kaztakam123"  → ユーザー名を含むため拒否
# 正しい例: "Str0ng!Pass" → 受理される
```

#### よくある設定ミス

| ミス | 症状 | 対処法 |
|-----|------|--------|
| libpam-pwquality 未インストール | パスワード制限が効かない | `sudo apt install libpam-pwquality` |
| common-password の行が間違った位置 | ポリシーが適用されない | pam_pwquality.so の行を pam_unix.so の前に配置 |
| enforce_for_root がない | root のパスワードにポリシーが適用されない | パラメータを追加 |
| ucredit=1（正の値）にしてしまった | 大文字が必須にならない | -1（負の値）に修正 |

---

### 1.7 sudo 設定

- [ ] sudo がインストールされている
- [ ] パスワード試行回数が **3回** に制限されている
- [ ] 間違ったパスワード時の**カスタムメッセージ**が設定されている
- [ ] sudo の使用が `/var/log/sudo/` に**ログ記録**されている（入力・出力とも）
- [ ] **TTY モード** が有効化されている
- [ ] sudo の **PATH** が制限されている

#### 設定ファイル: `/etc/sudoers.d/sudo_config`

```
Defaults    passwd_tries=3
Defaults    badpass_message="Wrong password. Access denied."
Defaults    logfile="/var/log/sudo/sudo.log"
Defaults    log_input
Defaults    log_output
Defaults    iolog_dir="/var/log/sudo"
Defaults    requiretty
Defaults    secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin"
```

各設定の詳細と防御する攻撃:

| 設定 | 防ぐ攻撃 | 詳細説明 |
|------|---------|---------|
| `passwd_tries=3` | ブルートフォース | sudo パスワード推測を3回に制限。3回失敗後は `sudo` コマンドが終了 |
| `badpass_message` | 情報漏洩 | デフォルトメッセージ("Sorry, try again")からカスタムに変更。攻撃者にsudoのバージョン等の情報を与えない |
| `requiretty` | Web Shell 攻撃 | TTY（端末）がないと sudo を実行できない。CGI や cron 経由の権限昇格を防止 |
| `secure_path` | PATH 注入 | sudo 実行時に使用する PATH を固定。悪意あるコマンドの優先実行を防止 |
| `log_input` | フォレンジック | sudo セッション中のキー入力を記録。不正操作の証拠を保全 |
| `log_output` | フォレンジック | sudo セッション中のコマンド出力を記録。操作結果の証拠を保全 |
| `logfile` | 監査 | 全 sudo コマンドをファイルに記録。who/when/what の追跡 |

#### 前提条件

```bash
# ログディレクトリの作成
sudo mkdir -p /var/log/sudo
```

#### 検証コマンドと期待される出力

```bash
# sudo がインストールされているか
dpkg -l | grep sudo
# 期待: ii  sudo ...

# 設定ファイルの存在確認
ls -la /etc/sudoers.d/sudo_config
# 期待: -r--r----- 1 root root ... /etc/sudoers.d/sudo_config

# 設定内容の確認
sudo cat /etc/sudoers.d/sudo_config
# 期待: 上記の全設定が表示される

# sudoers の構文チェック
sudo visudo -c
# 期待: /etc/sudoers: parsed OK
#        /etc/sudoers.d/sudo_config: parsed OK

# パスワード試行回数のテスト
sudo ls
# 3回間違ったパスワードを入力
# 期待: 3回目の失敗後に "3 incorrect password attempts" と表示

# カスタムメッセージの確認
sudo ls
# 間違ったパスワードを入力
# 期待: "Wrong password. Access denied." と表示

# TTY の確認
sudo grep "requiretty" /etc/sudoers.d/sudo_config
# 期待: Defaults    requiretty

# PATH の確認
sudo env | grep PATH
# 期待: PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin

# ログの確認
ls -la /var/log/sudo/
# 期待: sudo.log ファイルと I/O ログディレクトリが存在

cat /var/log/sudo/sudo.log
# 期待: sudo コマンドの使用履歴が記録されている
# 例: Jan 15 10:00:00 : kaztakam : TTY=pts/0 ; PWD=/home/kaztakam ;
#     USER=root ; COMMAND=/usr/bin/apt update

# I/O ログの確認
ls /var/log/sudo/00/00/
# 期待: 01, 02, ... のディレクトリが存在
#       各ディレクトリ内に log, timing, ttyin, ttyout 等のファイル
```

#### よくある設定ミス

| ミス | 症状 | 対処法 |
|-----|------|--------|
| /var/log/sudo ディレクトリ未作成 | ログが記録されない | `sudo mkdir -p /var/log/sudo` |
| visudo を使わずに直接編集 | 構文エラーで sudo が使えなくなる | `pkexec visudo` でリカバリ |
| sudoers.d のファイル名にドット | 読み込まれない | ファイル名にドットを含めない |
| パーミッションが不適切 | sudo 実行時にエラー | `chmod 440 /etc/sudoers.d/sudo_config` |

---

### 1.8 monitoring.sh

- [ ] `/usr/local/bin/monitoring.sh` にスクリプトが配置されている
- [ ] スクリプトが実行可能（`chmod +x`）である
- [ ] cron で **10分ごと** に実行される
- [ ] `wall` コマンドで全端末にブロードキャストされる
- [ ] エラーが表示されない

#### 表示する情報チェックリスト

| # | 情報 | 取得方法 | なぜ監視するのか | 確認 |
|---|------|---------|----------------|------|
| 1 | OS アーキテクチャとカーネルバージョン | `uname -a` | 脆弱なカーネルの使用を検知 | [ ] |
| 2 | 物理 CPU 数 | `/proc/cpuinfo` | リソースの把握 | [ ] |
| 3 | 仮想 CPU (vCPU) 数 | `/proc/cpuinfo` | リソースの把握 | [ ] |
| 4 | RAM 使用量/合計 | `free` | メモリリーク検知 | [ ] |
| 5 | ディスク使用量/合計 | `df` | 容量不足の早期発見 | [ ] |
| 6 | CPU 使用率 | `top -bn1` | 不正プロセスの検知 | [ ] |
| 7 | 最終再起動日時 | `who -b` | 不正な再起動の検知 | [ ] |
| 8 | LVM がアクティブかどうか | `lsblk` | ストレージ構成の確認 | [ ] |
| 9 | アクティブな TCP 接続数 | `ss` | 不正接続の検知 | [ ] |
| 10 | ログインユーザー数 | `who` | 不正ログインの検知 | [ ] |
| 11 | IPv4 アドレスと MAC アドレス | `hostname -I`, `ip link` | ネットワーク設定の確認 | [ ] |
| 12 | sudo コマンドの実行回数 | `journalctl` | 権限昇格の追跡 | [ ] |

#### 期待される出力形式

```
	#Architecture: Linux kaztakam42 6.1.0-XX-amd64 #1 SMP ... x86_64 GNU/Linux
	#CPU physical: 1
	#vCPU: 1
	#Memory Usage: 143/460MB (31.09%)
	#Disk Usage: 1009/4.2Gb (26%)
	#CPU load: 6.7%
	#Last boot: 2024-01-15 10:24
	#LVM use: yes
	#Connections TCP: 1 ESTABLISHED
	#User log: 1
	#Network: IP 10.0.2.15 (08:00:27:xx:xx:xx)
	#Sudo: 42 cmd
```

#### cron 設定

```bash
sudo crontab -e
# 追加する行:
*/10 * * * * /usr/local/bin/monitoring.sh
```

#### 検証コマンドと期待される出力

```bash
# スクリプトの存在と権限確認
ls -la /usr/local/bin/monitoring.sh
# 期待: -rwxr-xr-x 1 root root XXXX ... /usr/local/bin/monitoring.sh

# 手動実行テスト
sudo /usr/local/bin/monitoring.sh
# 期待: 上記の形式で情報が wall で表示される

# cron 設定の確認
sudo crontab -l
# 期待: */10 * * * * /usr/local/bin/monitoring.sh

# cron ログの確認
grep CRON /var/log/syslog | tail -5
# 期待: 10分ごとの実行ログが表示される

# エラーの確認
sudo /usr/local/bin/monitoring.sh 2>&1 | head
# 期待: エラーメッセージが表示されない
```

#### monitoring.sh の停止方法（防御評価時）

```bash
# 方法1: cron ジョブの一時的な無効化
sudo crontab -e
# ジョブの行をコメントアウト: #*/10 * * * * /usr/local/bin/monitoring.sh

# 方法2: cron サービスの停止
sudo systemctl stop cron
# ※ 防御評価でサービスの停止を見せる場合に使用
```

---

### 1.9 提出物

- [ ] Git リポジトリのルートに `signature.txt` が存在する
- [ ] `signature.txt` に VM の仮想ディスクの SHA1 ハッシュが記載されている

**重要**: signature 生成後に VM を起動すると、ディスクファイルが変更されてハッシュが一致しなくなる。必ず VM を停止した状態で生成し、生成後は VM を起動しないこと。

```bash
# VM を停止した状態でホストOSで実行
# Linux の場合:
sha1sum "/path/to/VirtualBox VMs/Born2beRoot/Born2beRoot.vdi" > signature.txt

# macOS の場合:
shasum "/path/to/VirtualBox VMs/Born2beRoot/Born2beRoot.vdi" > signature.txt

# Windows の場合:
certutil -hashfile "C:\path\to\Born2beRoot.vdi" SHA1 > signature.txt
```

---

## 2. Bonus 要件チェックリスト

**注意**: Bonus は必須部分が完璧（PERFECT）な場合のみ評価される。必須部分に1つでも不備があれば、Bonus は評価されない。

### 2.1 パーティション構成

- [ ] 上記 1.2 の Bonus パーティション構成を満たしている
- [ ] 各パーティションのサイズが適切である
- [ ] lsblk の出力が subject のスクリーンショットと一致する

検証:
```bash
lsblk
# 7つの LV が表示されること（root, swap, home, var, srv, tmp, var--log）
```

### 2.2 WordPress サイト

- [ ] **lighttpd** がインストールされ、動作している
- [ ] **MariaDB** がインストールされ、動作している
- [ ] **PHP** がインストールされている
- [ ] WordPress が正常に動作している
- [ ] lighttpd が port 80 でリスニングしている
- [ ] UFW で port 80 が追加されている

**なぜ lighttpd なのか**:
subject で NGINX と Apache2 が Bonus の Web サーバーとして禁止されているため。lighttpd は軽量で設定がシンプルな Web サーバーであり、WordPress を動作させる最低限の機能を持つ。

#### 検証コマンド

```bash
# lighttpd の確認
sudo systemctl status lighttpd
# 期待: active (running)

# MariaDB の確認
sudo systemctl status mariadb
# 期待: active (running)

# PHP の確認
php -v
# 期待: PHP 8.x.x が表示される

# ポートの確認
sudo ss -tunlp | grep lighttpd
# 期待: 0.0.0.0:80 で LISTEN

# UFW ルールの確認
sudo ufw status
# 期待: port 80 が ALLOW として表示される

# WordPress の動作確認
# ブラウザから http://localhost:80 にアクセス
# または curl でテスト:
curl -I http://localhost:80
# 期待: HTTP/1.1 200 OK
```

#### WordPress の設定ファイル

```bash
# wp-config.php の場所
/var/www/html/wp-config.php

# MariaDB の WordPress データベース確認
sudo mariadb -u root -p
# MariaDB [(none)]> SHOW DATABASES;
# 期待: wordpress データベースが表示される

# MariaDB [(none)]> USE wordpress;
# MariaDB [wordpress]> SHOW TABLES;
# 期待: wp_* テーブルが表示される
```

### 2.3 追加サービス

- [ ] NGINX および Apache2 **以外** のサービスを選択している
- [ ] そのサービスの選択理由を説明できる
- [ ] サービスが正常に動作している

推奨サービス例と選択理由:

#### Fail2ban（推奨度: 高）

Born2beRoot のセキュリティ設計と最も一貫性があるサービス。

```bash
# インストール
sudo apt install fail2ban

# 設定ファイル
sudo cp /etc/fail2ban/jail.conf /etc/fail2ban/jail.local
sudo nano /etc/fail2ban/jail.local

# SSH の保護設定:
# [sshd]
# enabled = true
# port = 4242
# filter = sshd
# logpath = /var/log/auth.log
# maxretry = 3
# bantime = 600
# findtime = 600

# 検証
sudo systemctl status fail2ban
sudo fail2ban-client status
sudo fail2ban-client status sshd
# 期待:
# Status for the jail: sshd
# |- Filter
# |  |- Currently failed: 0
# |  |- Total failed:     0
# |  `- File list:        /var/log/auth.log
# `- Actions
#    |- Currently banned: 0
#    |- Total banned:     0
#    `- Banned IP list:
```

選択理由の説明例:
「Fail2ban は SSH のブルートフォース攻撃を自動的に検知してブロックするサービスです。Born2beRoot ではパスワードポリシーと UFW でセキュリティを強化していますが、Fail2ban を追加することで、繰り返しログインに失敗した IP アドレスを自動的にファイアウォールでブロックし、多層防御をさらに強化できます。」

#### その他の候補

| サービス | 説明 | 選択理由 |
|---------|------|---------|
| **Postfix** | メールサーバー | cron のメール通知に使用可能 |
| **Redis** | インメモリキャッシュ | WordPress の高速化に使用可能 |
| **Cockpit** | Web ベースサーバー管理 | port 9090 で管理画面提供 |
| **Prometheus + Node Exporter** | 監視システム | monitoring.sh の発展形 |

---

## 3. 要件の仕様一覧表

| カテゴリ | 項目 | 仕様値 | 設定ファイル | 検証コマンド |
|---------|------|--------|-------------|-------------|
| SSH | ポート | 4242 | `/etc/ssh/sshd_config` | `grep "^Port" /etc/ssh/sshd_config` |
| SSH | root ログイン | 禁止 | `/etc/ssh/sshd_config` | `grep "^PermitRootLogin" /etc/ssh/sshd_config` |
| SSH | 自動起動 | enabled | systemd | `systemctl is-enabled sshd` |
| UFW | 状態 | active | ufw | `sudo ufw status` |
| UFW | 許可ポート | 4242 のみ | ufw | `sudo ufw status numbered` |
| UFW | 自動起動 | enabled | `/etc/ufw/ufw.conf` | `systemctl is-enabled ufw` |
| ホスト名 | 形式 | login42 | `/etc/hostname`, `/etc/hosts` | `hostname` |
| グループ | 必須所属 | sudo, user42 | `/etc/group` | `groups username` |
| パスワード | 有効期限 | 30 日 | `/etc/login.defs` | `chage -l username` |
| パスワード | 変更最小間隔 | 2 日 | `/etc/login.defs` | `chage -l username` |
| パスワード | 警告 | 7 日前 | `/etc/login.defs` | `chage -l username` |
| パスワード | 最小文字数 | 10 文字 | `/etc/pam.d/common-password` | パスワード変更テスト |
| パスワード | 大文字 | 1 文字以上 | `/etc/pam.d/common-password` | パスワード変更テスト |
| パスワード | 小文字 | 1 文字以上 | `/etc/pam.d/common-password` | パスワード変更テスト |
| パスワード | 数字 | 1 文字以上 | `/etc/pam.d/common-password` | パスワード変更テスト |
| パスワード | 連続同一文字 | 3 文字以下 | `/etc/pam.d/common-password` | パスワード変更テスト |
| パスワード | ユーザー名含有 | 禁止 | `/etc/pam.d/common-password` | パスワード変更テスト |
| パスワード | 旧PWとの差分 | 7 文字以上 | `/etc/pam.d/common-password` | パスワード変更テスト |
| sudo | 試行回数 | 3 回 | `/etc/sudoers.d/sudo_config` | sudo で3回失敗テスト |
| sudo | エラーメッセージ | カスタム | `/etc/sudoers.d/sudo_config` | sudo で失敗時のメッセージ確認 |
| sudo | ログ | /var/log/sudo/ | `/etc/sudoers.d/sudo_config` | `ls /var/log/sudo/` |
| sudo | TTY モード | 必須 | `/etc/sudoers.d/sudo_config` | `grep requiretty` |
| sudo | PATH 制限 | 指定のパス | `/etc/sudoers.d/sudo_config` | `sudo env \| grep PATH` |
| cron | monitoring.sh 間隔 | 10 分 | `crontab -e` | `sudo crontab -l` |
| cron | 出力先 | wall（全端末） | monitoring.sh 内 | 手動実行テスト |
| AppArmor | 状態 | enabled, running | systemd | `sudo aa-status` |

---

## 4. 要件の相互依存関係

各要件は独立ではなく、相互に関連している:

```
パーティション構成
├── LUKS 暗号化 ← ブート時のパスフレーズ入力が必要
│   └── dm-crypt カーネルモジュール
│       └── AES-256-XTS 暗号化
├── LVM ← PV/VG/LV の3層構造
│   └── /boot は暗号化外（GRUB がアクセスするため）
└── 各マウントポイントの分離 ← セキュリティと運用性
    └── /var/log, /tmp の分離（Bonus）

セキュリティスタック
├── UFW (ネットワーク層) ← port 4242 のみ許可
│   └── iptables/Netfilter (カーネル)
│       └── filter テーブルの INPUT チェーン
├── SSH (認証層) ← port 4242, root 禁止
│   └── パスワードポリシー ← PAM (pam_pwquality)
│       ├── /etc/login.defs ← 有効期限ポリシー
│       └── /etc/pam.d/common-password ← 強度ポリシー
├── sudo (権限管理層) ← 制限設定, ログ記録
│   ├── requiretty ← Web Shell 対策
│   ├── secure_path ← PATH 注入対策
│   └── log_input/output ← フォレンジック
└── AppArmor (MAC層) ← プロセスレベルのアクセス制御
    └── enforce モードのプロファイル

監視
├── monitoring.sh ← シェルスクリプト
│   ├── /proc/cpuinfo ← CPU 情報
│   ├── free ← メモリ情報
│   ├── df ← ディスク情報
│   ├── ss ← ネットワーク情報
│   └── journalctl ← sudo コマンド数
│       └── cron ← 10分間隔で実行
│           └── wall ← 全端末にブロードキャスト
└── sudo ログ ← /var/log/sudo/
    └── 入力・出力の完全記録
```

---

## 5. 防御評価の準備

防御評価で確認される可能性がある操作:

### 5.1 新しいユーザーの作成

```bash
# 評価者が新しいユーザーの作成を求める場合
sudo adduser newuser
# パスワードポリシーに準拠したパスワードを設定

# グループに追加
sudo usermod -aG user42 newuser
sudo usermod -aG sudo newuser

# パスワードポリシーの適用確認
sudo chage -l newuser
# MAX_DAYS=30, MIN_DAYS=2, WARN_AGE=7 であること
```

### 5.2 ホスト名の変更

```bash
# 評価者がホスト名の変更を求める場合
sudo hostnamectl set-hostname newname42
sudo nano /etc/hosts
# 127.0.1.1    newname42

# 確認
hostname
# → newname42

# 元に戻す
sudo hostnamectl set-hostname kaztakam42
sudo nano /etc/hosts
# 127.0.1.1    kaztakam42
```

### 5.3 UFW ルールの操作

```bash
# 新しいポートを開放
sudo ufw allow 8080
sudo ufw status numbered

# ルールの削除
sudo ufw delete allow 8080
sudo ufw status numbered
```

### 5.4 monitoring.sh の制御

```bash
# cron ジョブの停止
sudo crontab -e
# */10 の行をコメントアウト

# cron サービス自体の停止
sudo systemctl stop cron

# 再開
sudo systemctl start cron
sudo crontab -e
# コメントを解除
```

### 5.5 よく聞かれる質問への回答準備

- パーティションの構成理由を説明できるか
- LVM の利点を説明できるか
- LUKS 暗号化の仕組みを説明できるか
- AppArmor と SELinux の違いを説明できるか
- apt と aptitude の違いを説明できるか
- SSH の仕組みと設定の理由を説明できるか
- UFW のルール追加・削除ができるか
- パスワードポリシーの各値の根拠を説明できるか
- sudo の各設定が防ぐ攻撃を説明できるか
- monitoring.sh の各コマンドの意味を説明できるか
- cron の設定を変更できるか

---

## 6. AppArmor の詳細要件

### 6.1 AppArmor の基本確認

- [ ] AppArmor がインストールされている
- [ ] AppArmor が起動時に有効化されている
- [ ] 少なくとも1つのプロファイルが enforce モードで動作している

**なぜ AppArmor が必要なのか - MAC（強制アクセス制御）の意義**:

Linux の標準的なアクセス制御（DAC: Discretionary Access Control）では、ファイルの所有者がアクセス権を自由に設定できる。しかし、プロセスが乗っ取られた場合、そのプロセスの権限で任意の操作が可能になる。

AppArmor は MAC（Mandatory Access Control: 強制アクセス制御）を提供する。プロファイルで定義されたアクセス以外は、たとえ root 権限であっても拒否される。これにより、プロセスが乗っ取られた場合の被害を限定できる。

```
DAC（通常のLinuxアクセス制御）の問題:

攻撃シナリオ:
1. Web サーバー (www-data ユーザー) に脆弱性
2. 攻撃者が www-data 権限でシェルを取得
3. www-data が読めるファイルは全てアクセス可能
   → /etc/passwd, /tmp 内のファイル, 他のユーザーのpublic読み取り可能ファイル
4. さらに権限昇格の足がかりを探索

MAC（AppArmor）での防御:
1. Web サーバーに AppArmor プロファイルを適用
2. プロファイルで許可されたファイル/ディレクトリのみアクセス可能
   → /var/www/html/** (read)
   → /var/log/lighttpd/** (write)
   → /tmp/lighttpd/** (read, write)
3. それ以外のアクセスは全てブロック
   → /etc/shadow, /home/*, /root/* 等へのアクセスは不可
4. 権限昇格の試行も制限される
```

#### AppArmor vs SELinux の比較

| 特徴 | AppArmor | SELinux |
|------|----------|---------|
| 採用ディストリ | Debian, Ubuntu, SUSE | RHEL, CentOS, Fedora, Rocky |
| アクセス制御方式 | パスベース (pathname-based) | ラベルベース (label-based) |
| 学習コスト | 低い（パスで直感的に理解可能） | 高い（ラベルの概念を理解する必要） |
| 設定の粒度 | 中程度 | 非常に細かい |
| プロファイル形式 | テキストファイル（可読性が高い） | バイナリポリシー（semodule で管理） |
| 新規ファイル対応 | パスが一致すれば自動適用 | ラベルの継承ルールが必要 |
| デフォルト動作 | 未定義のアクセスは許可 | 未定義のアクセスは拒否 |
| Born2beRoot | Debian を選択した場合に使用 | Rocky Linux を選択した場合に使用 |

#### 検証コマンドと期待される出力

```bash
# AppArmor の状態確認
sudo aa-status
# 期待される出力（例）:
# apparmor module is loaded.
# XX profiles are loaded.
# XX profiles are in enforce mode.
#    /usr/bin/man
#    /usr/sbin/ntpd
#    man_filter
#    man_groff
#    ...
# XX profiles are in complain mode.
# XX processes have profiles defined.
# XX processes are in enforce mode.
# XX processes are in complain mode.
# XX processes are unconfined but have a profile defined.

# AppArmor がカーネルパラメータで有効か確認
cat /sys/module/apparmor/parameters/enabled
# 期待: Y

# AppArmor のカーネルモジュール確認
lsmod | grep apparmor
# 期待: apparmor  XXX  X

# ブートパラメータの確認
cat /proc/cmdline | grep apparmor
# 期待: apparmor=1 security=apparmor

# AppArmor のサービス状態
sudo systemctl status apparmor
# 期待:
#   Loaded: loaded (/lib/systemd/system/apparmor.service; enabled; ...)
#   Active: active (exited)

# 特定プロファイルの内容確認（例: usr.sbin.sshd）
cat /etc/apparmor.d/usr.sbin.sshd
# プロファイルの内容が表示される
```

#### AppArmor プロファイルの構造

```
# プロファイルの基本構造
# ファイル: /etc/apparmor.d/usr.sbin.example

#include <tunables/global>

/usr/sbin/example {
  # ライブラリの読み込み
  #include <abstractions/base>
  #include <abstractions/nameservice>

  # ファイルアクセス権
  /etc/example.conf          r,      # 設定ファイルの読み取り
  /var/log/example.log        w,      # ログへの書き込み
  /var/run/example.pid        rw,     # PID ファイルの読み書き
  /usr/lib/example/**         r,      # ライブラリの読み取り
  /tmp/example-*              rw,     # 一時ファイルの読み書き

  # ネットワークアクセス
  network inet stream,                # TCP 接続を許可
  network inet dgram,                 # UDP を許可

  # Capability（特権操作）
  capability net_bind_service,        # 特権ポートのバインド
  capability setuid,                  # UID の変更
  capability setgid,                  # GID の変更

  # アクセス権の記号:
  # r  = 読み取り (read)
  # w  = 書き込み (write)
  # a  = 追記 (append)
  # k  = ファイルロック (lock)
  # l  = リンク (link)
  # m  = メモリマップ実行 (mmap exec)
  # x  = 実行 (execute)
  # ix = inherit execute（子プロセスに同じプロファイルを適用）
  # cx = child execute（子プロファイルを適用）
  # px = profile execute（指定プロファイルを適用）
  # ux = unconfined execute（制限なしで実行 - 非推奨）
}
```

#### AppArmor の操作コマンド

```bash
# プロファイルの一覧
ls /etc/apparmor.d/

# プロファイルを enforce モードに設定
sudo aa-enforce /etc/apparmor.d/usr.sbin.example
# enforce: ポリシー違反をブロックしてログに記録

# プロファイルを complain モードに設定
sudo aa-complain /etc/apparmor.d/usr.sbin.example
# complain: ポリシー違反をログに記録するがブロックしない

# プロファイルの無効化
sudo aa-disable /etc/apparmor.d/usr.sbin.example

# プロファイルの再読み込み
sudo apparmor_parser -r /etc/apparmor.d/usr.sbin.example

# 全プロファイルの再読み込み
sudo systemctl reload apparmor

# AppArmor のログ確認
sudo dmesg | grep apparmor
# または
sudo journalctl -k | grep apparmor
```

#### よくある設定ミス

| ミス | 症状 | 対処法 |
|-----|------|--------|
| AppArmor が無効化されている | aa-status が "module is not loaded" | GRUB で apparmor=1 を設定 |
| 全プロファイルが complain モード | enforce が 0 | aa-enforce で enforce に変更 |
| GRUB パラメータが未設定 | 再起動後に AppArmor が無効 | /etc/default/grub を編集 |
| apparmor-utils 未インストール | aa-enforce 等のコマンドがない | `sudo apt install apparmor-utils` |

---

## 7. monitoring.sh の詳細要件

### 7.1 スクリプトの各コマンド詳細解説

monitoring.sh は Born2beRoot のシステム監視スクリプトである。各行の取得方法と意味を詳細に解説する。

#### Architecture（アーキテクチャ）

```bash
arc=$(uname -a)
# uname -a の出力解説:
# Linux        : カーネル名
# kaztakam42   : ホスト名
# 6.1.0-XX     : カーネルバージョン
# #1 SMP       : カーネルビルド情報 (SMP = Symmetric Multi-Processing)
# PREEMPT_DYNAMIC: プリエンプションモデル
# Debian 6.x.x : ディストリビューション情報
# x86_64       : アーキテクチャ（64bit）
# GNU/Linux    : OS タイプ

# なぜこの情報を監視するのか:
# - カーネルバージョンから既知の脆弱性をチェック可能
# - アーキテクチャの確認（不正な環境変更の検知）
# - SMP の有無でマルチプロセッサ対応を確認
```

#### Physical CPU / vCPU（物理CPU数 / 仮想CPU数）

```bash
# 物理 CPU 数
pcpu=$(grep "physical id" /proc/cpuinfo | sort -u | wc -l)

# /proc/cpuinfo の構造:
# processor    : 0         ← 論理プロセッサの番号
# model name   : Intel...  ← CPU モデル名
# physical id  : 0         ← 物理 CPU の ID
# core id      : 0         ← 物理コア内の ID
# cpu cores    : 4         ← 物理 CPU あたりのコア数

# sort -u で重複を除去し、ユニークな physical id の数をカウント
# VirtualBox では通常 physical id は 0 の1つだけ

# 仮想 CPU (vCPU) 数
vcpu=$(grep -c "^processor" /proc/cpuinfo)

# processor 行の数 = 論理プロセッサ数
# 物理CPU数 × コア数 × (ハイパースレッディング有効なら 2) = vCPU数
# VirtualBox の設定で割り当てた CPU 数が表示される
```

#### Memory Usage（メモリ使用量）

```bash
# メモリ情報の取得
total_mem=$(free -m | awk '/Mem:/ {print $2}')
used_mem=$(free -m | awk '/Mem:/ {print $3}')
pct_mem=$(free -m | awk '/Mem:/ {printf("%.2f"), $3/$2*100}')

# free -m の出力:
#               total        used        free      shared  buff/cache   available
# Mem:           460         143          15          12         301         283
# Swap:         2296           0        2296

# 各列の意味:
# total     : 物理メモリの合計 (MB)
# used      : 使用中のメモリ (バッファ/キャッシュ含む)
# free      : 完全に未使用のメモリ
# shared    : 共有メモリ (tmpfs 等)
# buff/cache: バッファとキャッシュに使用中（必要に応じて解放可能）
# available : 新しいプロセスに割り当て可能なメモリ（実質的な空き）

# awk でパース:
# /Mem:/    : "Mem:" を含む行を選択
# {print $2}: 2番目のフィールド（total）を出力
# {print $3}: 3番目のフィールド（used）を出力
# printf    : パーセンテージをフォーマット出力
```

#### Disk Usage（ディスク使用量）

```bash
# ディスク情報の取得
total_disk=$(df -BG --total | awk '/^total/ {print $2}' | tr -d 'G')
used_disk=$(df -BM --total | awk '/^total/ {print $3}' | tr -d 'M')
pct_disk=$(df --total | awk '/^total/ {print $5}')

# df -BG: ブロックサイズを GB 単位で表示
# df -BM: ブロックサイズを MB 単位で表示
# --total: 全ファイルシステムの合計行を追加

# df の出力:
# Filesystem               1K-blocks    Used Available Use% Mounted on
# /dev/mapper/LVMGroup-root  9742960 1008404   8220804  11% /
# /dev/sda1                   471712  121116    325672  28% /boot
# /dev/mapper/LVMGroup-home  4834080    20476   4555988   1% /home
# ...
# total                     19234728 1200000  17200000  7%  -

# tr -d 'G': 'G' 文字を削除（数値のみ抽出）
```

#### CPU Load（CPU使用率）

```bash
# CPU 使用率の取得
cpu_load=$(top -bn1 | grep '^%Cpu' | awk '{printf("%.1f%%"), $2 + $4}')

# top -bn1 の解説:
# -b: バッチモード（対話的でない出力）
# -n1: 1回だけ測定

# %Cpu(s) の行:
# %Cpu(s):  6.2 us,  1.5 sy,  0.0 ni, 92.0 id,  0.0 wa,  0.3 hi,  0.0 si,  0.0 st
#           $2       $4       $6       $8       $10      $12      $14      $16

# 各フィールド:
# us (user):     ユーザープロセスの CPU 使用率
# sy (system):   カーネルプロセスの CPU 使用率
# ni (nice):     nice 値が変更されたプロセス
# id (idle):     アイドル（未使用）
# wa (iowait):   I/O 待ち
# hi (hardware): ハードウェア割り込み
# si (software): ソフトウェア割り込み
# st (steal):    仮想化環境で他VMに奪われた時間

# us + sy = ユーザー + システムの合計使用率を表示
```

#### Last Boot（最終起動日時）

```bash
# 最終起動日時
last_boot=$(who -b | awk '{print $3 " " $4}')

# who -b の出力:
#          system boot  2024-01-15 10:24

# awk でフィールド3（日付）とフィールド4（時刻）を抽出
# 出力: 2024-01-15 10:24
```

#### LVM Active（LVM状態）

```bash
# LVM がアクティブか確認
lvm_use=$(if [ $(lsblk | grep -c "lvm") -gt 0 ]; then echo yes; else echo no; fi)

# lsblk の TYPE カラムに "lvm" が含まれるか確認
# grep -c: マッチした行数をカウント
# -gt 0: 0 より大きければ LVM が使用されている
```

#### TCP Connections（TCP接続数）

```bash
# TCP 接続数
tcp_conn=$(ss -t state established | grep -c "ESTAB")

# ss -t: TCP ソケットのみ表示
# state established: ESTABLISHED 状態のみ
# grep -c: 行数をカウント

# 代替方法:
# tcp_conn=$(ss -s | grep "estab" | awk '{print $4}' | tr -d ',')
# ss -s: サマリー表示
```

#### User Log（ログインユーザー数）

```bash
# ログインユーザー数
user_log=$(who | wc -l)

# who の出力:
# kaztakam pts/0        2024-01-15 10:30 (10.0.2.2)

# wc -l: 行数をカウント（= ログインセッション数）
# 同じユーザーが複数セッションを持つ場合、セッション数がカウントされる
```

#### Network（ネットワーク情報）

```bash
# IP アドレス
ip_addr=$(hostname -I | awk '{print $1}')

# hostname -I: 全 IP アドレスを表示（ループバック除く）
# awk '{print $1}': 最初の IP アドレスのみ取得

# MAC アドレス
mac_addr=$(ip link show | grep "link/ether" | awk '{print $2}' | head -1)

# ip link show: ネットワークインターフェースの情報
# link/ether: Ethernet MAC アドレスの行
# head -1: 最初のインターフェースのみ
```

#### Sudo Commands（sudo実行回数）

```bash
# sudo コマンドの実行回数
sudo_count=$(journalctl _COMM=sudo 2>/dev/null | grep -c "COMMAND")

# journalctl _COMM=sudo: sudo コマンドの systemd ジャーナルログ
# COMMAND: 実際にコマンドが実行されたログエントリ
# 2>/dev/null: エラー出力を破棄

# 代替方法（auth.log を使用）:
# sudo_count=$(grep -c "COMMAND" /var/log/auth.log 2>/dev/null || echo 0)
```

### 7.2 完全な monitoring.sh スクリプト例

```bash
#!/bin/bash

# ========================================
# monitoring.sh - Born2beRoot System Monitor
# ========================================
# 10分ごとに cron で実行され、
# wall コマンドで全端末にシステム情報をブロードキャストする

# --- データ収集 ---

# アーキテクチャとカーネルバージョン
arc=$(uname -a)

# 物理 CPU 数
pcpu=$(grep "physical id" /proc/cpuinfo | sort -u | wc -l)

# 仮想 CPU 数
vcpu=$(grep -c "^processor" /proc/cpuinfo)

# メモリ使用量
total_mem=$(free -m | awk '/Mem:/ {print $2}')
used_mem=$(free -m | awk '/Mem:/ {print $3}')
pct_mem=$(free -m | awk '/Mem:/ {printf("%.2f"), $3/$2*100}')

# ディスク使用量
total_disk=$(df -BG --total | awk '/^total/ {print $2}' | tr -d 'G')
used_disk=$(df -BM --total | awk '/^total/ {print $3}' | tr -d 'M')
pct_disk=$(df --total | awk '/^total/ {print $5}')

# CPU 使用率
cpu_load=$(top -bn1 | grep '^%Cpu' | awk '{printf("%.1f%%"), $2 + $4}')

# 最終起動日時
last_boot=$(who -b | awk '{print $3 " " $4}')

# LVM 使用状態
lvm_use=$(if [ $(lsblk | grep -c "lvm") -gt 0 ]; then echo yes; else echo no; fi)

# TCP 接続数
tcp_conn=$(ss -t state established 2>/dev/null | grep -c "ESTAB")

# ログインユーザー数
user_log=$(who | wc -l)

# ネットワーク情報
ip_addr=$(hostname -I | awk '{print $1}')
mac_addr=$(ip link show | grep "link/ether" | awk '{print $2}' | head -1)

# sudo コマンド実行回数
sudo_count=$(journalctl _COMM=sudo 2>/dev/null | grep -c "COMMAND")

# --- ブロードキャスト ---

wall "	#Architecture: ${arc}
	#CPU physical: ${pcpu}
	#vCPU: ${vcpu}
	#Memory Usage: ${used_mem}/${total_mem}MB (${pct_mem}%)
	#Disk Usage: ${used_disk}/${total_disk}Gb (${pct_disk})
	#CPU load: ${cpu_load}
	#Last boot: ${last_boot}
	#LVM use: ${lvm_use}
	#Connections TCP: ${tcp_conn} ESTABLISHED
	#User log: ${user_log}
	#Network: IP ${ip_addr} (${mac_addr})
	#Sudo: ${sudo_count} cmd"
```

### 7.3 cron 設定の詳細

```bash
# cron の基本形式:
# ┌────────────── 分 (0 - 59)
# │ ┌──────────── 時 (0 - 23)
# │ │ ┌────────── 日 (1 - 31)
# │ │ │ ┌──────── 月 (1 - 12)
# │ │ │ │ ┌────── 曜日 (0 - 7, 0と7 = 日曜)
# │ │ │ │ │
# * * * * * コマンド

# Born2beRoot の設定:
*/10 * * * * /usr/local/bin/monitoring.sh

# */10 = 10分ごと（0, 10, 20, 30, 40, 50分）
# *    = 毎時
# *    = 毎日
# *    = 毎月
# *    = 毎曜日

# その他の cron 表現例:
# 0 */2 * * *     = 2時間ごと（毎時0分）
# 30 3 * * *      = 毎日 3:30
# 0 0 * * 0       = 毎週日曜 0:00
# 0 0 1 * *       = 毎月1日 0:00
# */5 * * * *     = 5分ごと
# 0 9-17 * * 1-5  = 平日 9:00-17:00 の毎時0分

# cron のログ確認
grep CRON /var/log/syslog
# 出力例:
# Jan 15 10:00:01 kaztakam42 CRON[1234]: (root) CMD (/usr/local/bin/monitoring.sh)
# Jan 15 10:10:01 kaztakam42 CRON[1250]: (root) CMD (/usr/local/bin/monitoring.sh)

# cron の注意点:
# - cron は root の crontab に設定する（sudo crontab -e）
# - monitoring.sh には実行権限が必要（chmod +x）
# - スクリプト内のコマンドはフルパスが推奨される
# - 環境変数は cron 内で限定的（PATH が異なる場合がある）
```

### 7.4 wall コマンドの詳細

```bash
# wall (Write All) - 全端末にメッセージをブロードキャスト
# 基本構文: wall [message]

# wall の動作:
# 1. /dev/pts/* および /dev/tty* の全端末デバイスにメッセージを送信
# 2. mesg n で受信拒否されている端末にはroot以外は送信不可
# 3. cron から実行される場合、root 権限で動作するため全端末に送信可能

# メッセージの受信設定:
mesg y    # メッセージ受信を許可
mesg n    # メッセージ受信を拒否

# 現在の設定確認:
mesg
# 出力: is y  または  is n

# wall の出力例:
# Broadcast message from root@kaztakam42 (somewhere) (Mon Jan 15 10:00:01 2024):
#
# 	#Architecture: Linux kaztakam42 ...
# 	#CPU physical: 1
# 	...
```

---

## 8. sshd_config パラメータ完全リファレンス

Born2beRoot の要件に関連する `/etc/ssh/sshd_config` の全パラメータを網羅的に解説する。

### 8.1 必須パラメータ

```
# === Born2beRoot 必須設定 ===

Port 4242
# デフォルト: 22
# SSH が待ち受けるポート番号
# 1-65535 の範囲で設定可能
# 1024未満のポートは特権ポート（root 権限必要）
# 複数のポートを指定可能: Port 4242 と Port 4243 を別行で記述
# 設定変更後は必ず: sudo systemctl restart sshd

PermitRootLogin no
# デフォルト: prohibit-password（Debian 12）
# 選択肢:
#   yes                  - root ログインを完全に許可
#   no                   - root ログインを完全に拒否（Born2beRoot 要件）
#   prohibit-password    - パスワード認証を禁止（公開鍵のみ許可）
#   forced-commands-only - 強制コマンドが設定された鍵のみ許可
```

### 8.2 認証関連パラメータ

```
# === 認証設定 ===

PasswordAuthentication yes
# デフォルト: yes
# パスワード認証の有効/無効
# Born2beRoot ではパスワード認証を使用
# 本番環境では公開鍵認証のみを推奨（no に設定）

PubkeyAuthentication yes
# デフォルト: yes
# 公開鍵認証の有効/無効
# 有効にしておくことを推奨

AuthorizedKeysFile .ssh/authorized_keys
# 公開鍵の格納ファイル
# ユーザーのホームディレクトリからの相対パス

MaxAuthTries 6
# デフォルト: 6
# 1接続あたりの認証試行回数
# 半分を超えると失敗がログに記録される
# 3-6 が推奨値

LoginGraceTime 120
# デフォルト: 120（秒）
# 認証を完了するまでの制限時間
# この時間内にログインしないと接続切断
# 0 = 無制限（非推奨）

MaxSessions 10
# デフォルト: 10
# 1接続上の最大セッション数（多重化時）

MaxStartups 10:30:100
# デフォルト: 10:30:100
# 未認証接続の制限
# 10: 未認証接続が10を超えると拒否開始
# 30: 30%の確率で新規接続を拒否
# 100: 未認証接続が100に達すると全て拒否
```

### 8.3 セキュリティ強化パラメータ

```
# === セキュリティ強化 ===

PermitEmptyPasswords no
# デフォルト: no
# 空パスワードでのログインを禁止
# 必ず no に設定

X11Forwarding no
# デフォルト: no（Debian）/ yes（一部ディストリ）
# X11 (GUI) のフォワーディング
# サーバーでは不要なため no を推奨

AllowTcpForwarding no
# デフォルト: yes
# TCP フォワーディングの許可/禁止
# 不要なら no に設定（トンネリング攻撃の防止）

GatewayPorts no
# デフォルト: no
# リモートポートフォワーディングのバインドアドレス
# no = localhost のみ

StrictModes yes
# デフォルト: yes
# ユーザーのホームディレクトリと .ssh の
# パーミッションをチェック
# 不適切なパーミッション（例: 777）の場合は拒否

UsePAM yes
# デフォルト: yes（Debian）
# PAM (Pluggable Authentication Modules) の使用
# パスワードポリシー（pam_pwquality）の適用に必要
# 必ず yes に設定

# === ログ設定 ===

SyslogFacility AUTH
# デフォルト: AUTH
# syslog のファシリティ
# AUTH: 認証関連メッセージ

LogLevel INFO
# デフォルト: INFO
# ログの詳細度
# QUIET, FATAL, ERROR, INFO, VERBOSE, DEBUG, DEBUG1-3
# セキュリティ監査には VERBOSE が推奨

# === 接続維持設定 ===

ClientAliveInterval 300
# デフォルト: 0（無効）
# クライアントの生存確認間隔（秒）
# 300 = 5分ごとに確認

ClientAliveCountMax 3
# デフォルト: 3
# 生存確認の最大回数
# 応答がなければ切断
# 300 × 3 = 15分で切断

# === バナー設定 ===

Banner /etc/ssh/banner
# デフォルト: none
# ログイン前に表示するバナーファイル
# セキュリティ警告を表示するのに使用
# 例: "Unauthorized access is prohibited"

PrintMotd no
# デフォルト: yes
# MOTD (Message Of The Day) の表示
# /etc/motd の内容をログイン後に表示
```

### 8.4 SSH の認証フロー

```
SSH 接続の認証フロー:

クライアント                           サーバー
    │                                    │
    │──── TCP 接続 (port 4242) ──────→  │
    │                                    │
    │←── サーバー版文字列 ──────────    │
    │     "SSH-2.0-OpenSSH_9.2p1"       │
    │                                    │
    │──── クライアント版文字列 ──────→  │
    │     "SSH-2.0-OpenSSH_9.5p1"       │
    │                                    │
    │←→ 鍵交換（Key Exchange）─────→   │
    │    Diffie-Hellman / ECDH           │
    │    → セッション鍵の合意           │
    │                                    │
    │←── サーバーホスト鍵 ─────────    │
    │    初回接続時: known_hosts に保存   │
    │    2回目以降: known_hosts と照合    │
    │                                    │
    │    ※ ここから暗号化通信 ※         │
    │                                    │
    │──── ユーザー認証要求 ──────────→ │
    │     "ssh-userauth"                 │
    │                                    │
    │←── 認証方式の提示 ──────────     │
    │     publickey, password            │
    │                                    │
    │──── パスワード認証 ────────────→ │
    │     (暗号化済みパスワード)          │
    │                                    │
    │     サーバー側の認証処理:          │
    │     1. /etc/shadow のハッシュと照合 │
    │     2. PAM モジュールの実行        │
    │        (pam_pwquality 等)          │
    │     3. AppArmor のチェック         │
    │                                    │
    │←── 認証結果 ─────────────────    │
    │     成功: シェルセッション開始     │
    │     失敗: "Permission denied"      │
    │                                    │

SSH 鍵交換で使用されるアルゴリズム:
┌─────────────────────────────────────────────┐
│  鍵交換:     curve25519-sha256 (推奨)       │
│              ecdh-sha2-nistp256              │
│              diffie-hellman-group16-sha512   │
├─────────────────────────────────────────────┤
│  ホスト鍵:   ssh-ed25519 (推奨)             │
│              ecdsa-sha2-nistp256             │
│              rsa-sha2-512                    │
├─────────────────────────────────────────────┤
│  暗号化:     chacha20-poly1305 (推奨)       │
│              aes256-gcm@openssh.com         │
│              aes256-ctr                      │
├─────────────────────────────────────────────┤
│  MAC:        hmac-sha2-256-etm (推奨)       │
│              hmac-sha2-512-etm              │
└─────────────────────────────────────────────┘
```

---

## 9. sudoers 設定の詳細リファレンス

### 9.1 sudoers ファイルの構文

```
# sudoers の基本構文:
# ユーザー ホスト = (実行ユーザー:実行グループ) コマンド

# 例:
kaztakam ALL=(ALL:ALL) ALL
# kaztakam  : sudo を実行できるユーザー
# ALL       : 任意のホストから（通常 ALL）
# (ALL:ALL) : 任意のユーザー/グループとして
# ALL       : 任意のコマンドを実行可能

# グループ指定:
%sudo ALL=(ALL:ALL) ALL
# %sudo: sudo グループの全メンバー

# 特定コマンドのみ許可:
backup ALL=(root) /usr/bin/rsync, /usr/bin/tar
# backup ユーザーは root として rsync と tar のみ実行可能

# パスワードなし:
nagios ALL=(root) NOPASSWD: /usr/lib/nagios/plugins/*
# 監視スクリプトをパスワードなしで実行
```

### 9.2 Defaults の全オプション解説

```
# Born2beRoot で設定する Defaults:

Defaults    passwd_tries=3
# パスワードの試行回数
# デフォルト: 3
# 0 = 無制限（非推奨）
# 失敗後は「X incorrect password attempts」と表示

Defaults    badpass_message="Wrong password. Access denied."
# パスワード失敗時のメッセージ
# デフォルト: "Sorry, try again."
# カスタマイズすることで、sudo のバージョン情報等を隠す

Defaults    logfile="/var/log/sudo/sudo.log"
# sudo コマンドのログファイル
# デフォルト: syslog に出力
# 専用ファイルに出力することで、sudo の使用履歴を集中管理
# ログ形式:
# Jan 15 10:00:00 : kaztakam : TTY=pts/0 ; PWD=/home/kaztakam ;
#     USER=root ; COMMAND=/usr/bin/apt update

Defaults    log_input
# sudo セッションのキー入力を記録
# iolog_dir で指定したディレクトリに保存
# フォレンジック（事後分析）に重要
# 記録される内容: ユーザーが入力した全キーストローク

Defaults    log_output
# sudo セッションのコマンド出力を記録
# iolog_dir で指定したディレクトリに保存
# 記録される内容: コマンドの標準出力と標準エラー出力

Defaults    iolog_dir="/var/log/sudo"
# I/O ログの保存ディレクトリ
# デフォルト: /var/log/sudo-io
# ディレクトリ構造:
# /var/log/sudo/
# ├── sudo.log          ← コマンドログ
# └── 00/
#     └── 00/
#         ├── 01/       ← セッション 1
#         │   ├── log       ← セッション情報
#         │   ├── timing    ← タイミング情報
#         │   ├── ttyin     ← キー入力
#         │   └── ttyout    ← コマンド出力
#         ├── 02/       ← セッション 2
#         └── ...

Defaults    requiretty
# TTY（端末）からの実行を必須にする
# TTY がない環境（cgi-bin, cron, スクリプトの中など）から
# sudo を実行できなくなる
#
# 防ぐ攻撃:
# 1. Web Shell 攻撃: CGI 経由の sudo 実行を防止
# 2. バックドア: デーモンプロセスからの sudo 実行を防止
#
# 注意: cron で sudo を使用する場合は除外設定が必要
# Defaults:cronuser !requiretty

Defaults    secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin"
# sudo 実行時に使用する PATH
# ユーザーの PATH ではなく、この固定 PATH が使用される
#
# 防ぐ攻撃（PATH 注入攻撃）:
# 1. 攻撃者が /tmp/ls に悪意あるスクリプトを配置
# 2. PATH=/tmp:$PATH に変更
# 3. sudo ls を実行 → /tmp/ls が root 権限で実行される
#
# secure_path により、sudo は常に信頼されたディレクトリの
# コマンドのみを実行する
```

### 9.3 sudoers のトラブルシューティング

```bash
# sudoers の構文チェック
sudo visudo -c
# 期待: /etc/sudoers: parsed OK
#        /etc/sudoers.d/sudo_config: parsed OK

# sudoers の編集は必ず visudo を使用する
# visudo は保存前に構文チェックを行う
# 直接編集で構文エラーがあると sudo が使えなくなる

# sudo が使えなくなった場合のリカバリ:
# 方法1: root でログイン
su -
visudo

# 方法2: pkexec（PolicyKit）
pkexec visudo

# 方法3: シングルユーザーモード
# GRUB メニューで "e" を押してカーネルパラメータに "single" を追加
# root シェルが起動する

# sudo のデバッグ
sudo -l
# 現在のユーザーで実行可能なコマンドを表示

sudo -l -U kaztakam
# 指定ユーザーで実行可能なコマンドを表示

# sudo のバージョン確認
sudo -V
# Sudo version X.X.X
# Sudoers policy plugin version X.X.X
```

---

## 10. パスワードポリシーの詳細理論

### 10.1 パスワードエントロピー計算

パスワードの強度は**エントロピー（情報量のビット数）**で測定される。エントロピーが高いほど、ブルートフォース攻撃に対する耐性が高い。

```
エントロピーの計算式:
  H = L × log₂(R)

  H: エントロピー（ビット）
  L: パスワードの長さ
  R: 使用可能な文字の種類数

文字種ごとの文字数:
  小文字 (a-z):     26文字
  大文字 (A-Z):     26文字
  数字 (0-9):       10文字
  記号 (!@#$%...):  ~33文字
  合計:             ~95文字

Born2beRoot のパスワード要件でのエントロピー:
  最低条件: 小文字 + 大文字 + 数字 = 62文字種
  10文字のパスワード: H = 10 × log₂(62) = 10 × 5.95 = 59.5 bits

ブルートフォース攻撃の所要時間（毎秒10億回試行の場合）:
  40 bits: 2⁴⁰ / 10⁹ ≈ 1,100 秒 ≈ 18分
  50 bits: 2⁵⁰ / 10⁹ ≈ 1,125,900 秒 ≈ 13日
  59.5 bits: 2⁵⁹·⁵ / 10⁹ ≈ 849,346,560 秒 ≈ 26.9年 ★ Born2beRoot
  60 bits: 2⁶⁰ / 10⁹ ≈ 1,152,921,505 秒 ≈ 36.5年
  80 bits: 2⁸⁰ / 10⁹ ≈ 3.8 × 10¹⁶ 秒 ≈ 12億年

推奨エントロピー:
  一般的なシステム: 50+ bits
  重要なシステム:   60+ bits
  暗号鍵:          128+ bits
```

### 10.2 パスワードハッシュの仕組み

```
パスワード保存の流れ:

ユーザーが入力                    /etc/shadow に保存
"Str0ng!Pass"
      │
      ▼
┌─────────────────────────┐
│  ハッシュ関数            │
│  SHA-512 (5000 rounds)   │
│                          │
│  入力: パスワード + salt  │
│  出力: 86文字のハッシュ   │
└─────────────────────────┘
      │
      ▼
$6$salt$hash_value

/etc/shadow の形式:
kaztakam:$6$rounds=5000$xxxxxx$yyyyyy...:19738:2:30:7:::

フィールド:
  $6       : ハッシュアルゴリズム ($6 = SHA-512)
  $rounds= : ハッシュの反復回数（ストレッチング）
  $salt    : ランダムなソルト値（レインボーテーブル対策）
  $hash    : 最終ハッシュ値
  19738    : 最終変更日（1970/1/1 からの日数）
  2        : 最小変更間隔（日数）= PASS_MIN_DAYS
  30       : 最大有効期限（日数）= PASS_MAX_DAYS
  7        : 警告日数 = PASS_WARN_AGE

ハッシュアルゴリズムの ID:
  $1  : MD5（非推奨、レガシー）
  $5  : SHA-256
  $6  : SHA-512（Debian デフォルト、推奨）
  $y$ : yescrypt（最新の Debian で使用される場合がある）
```

### 10.3 PAM (Pluggable Authentication Modules) の仕組み

```
PAM の処理フロー（パスワード変更時）:

ユーザー: passwd コマンド実行
      │
      ▼
┌─────────────────────────────────────────────┐
│  /etc/pam.d/common-password                  │
│                                              │
│  1. pam_pwquality.so (requisite)             │
│     → パスワード強度チェック                  │
│     → 失敗: 即座に拒否（requisite）          │
│     │                                        │
│     │  チェック項目:                          │
│     │  □ 10文字以上 (minlen=10)             │
│     │  □ 大文字1文字以上 (ucredit=-1)       │
│     │  □ 小文字1文字以上 (lcredit=-1)       │
│     │  □ 数字1文字以上 (dcredit=-1)         │
│     │  □ 3文字連続なし (maxrepeat=3)        │
│     │  □ ユーザー名なし (reject_username)    │
│     │  □ 旧PWと7文字差 (difok=7)            │
│     │                                        │
│  2. pam_unix.so (sufficient)                 │
│     → パスワードのハッシュ化と保存            │
│     → SHA-512 でハッシュ化                   │
│     → /etc/shadow に書き込み                 │
│     → 成功: 処理完了                         │
│                                              │
│  PAM のコントロールフラグ:                    │
│  required:    失敗しても残りのモジュールを実行 │
│  requisite:   失敗したら即座にエラーを返す    │
│  sufficient:  成功したら残りのモジュールをスキップ│
│  optional:    結果を無視（他に結果がない場合のみ使用）│
└─────────────────────────────────────────────┘
```

---

## 11. ネットワーク設定の詳細

### 11.1 VirtualBox ポートフォワーディング設定

Born2beRoot で SSH 接続するためのポートフォワーディング設定:

```
VirtualBox のポートフォワーディング設定:

設定パス: VM設定 → ネットワーク → アダプター1 → 高度 → ポートフォワーディング

ルール:
  名前       プロトコル  ホストIP     ホストポート  ゲストIP     ゲストポート
  SSH        TCP        127.0.0.1    4242          10.0.2.15    4242

接続コマンド（ホスト OS から）:
  ssh kaztakam@localhost -p 4242
  または
  ssh kaztakam@127.0.0.1 -p 4242
```

#### パケットの流れ

```
ホスト OS                          ゲスト VM
┌─────────────────────┐          ┌─────────────────────┐
│  ssh client          │          │  sshd (port 4242)    │
│  接続先:             │          │                      │
│  localhost:4242      │          │  Listen: 0.0.0.0:4242│
│       │              │          │       ▲              │
│       ▼              │          │       │              │
│  TCP 接続            │          │  TCP 接続            │
│  dst: 127.0.0.1:4242│          │  dst: 10.0.2.15:4242 │
│       │              │          │       ▲              │
│       ▼              │          │       │              │
│  VirtualBox          │          │  仮想NIC             │
│  NAT Engine          │  ────→  │  (enp0s3)            │
│  ポートフォワーディング│          │  IP: 10.0.2.15       │
│  127.0.0.1:4242 →   │          │                      │
│  10.0.2.15:4242      │          │                      │
└─────────────────────┘          └─────────────────────┘
```

### 11.2 ネットワークインターフェースの確認

```bash
# ゲスト VM 内でのネットワーク確認

# IP アドレスの確認
ip addr show
# 出力例:
# 1: lo: <LOOPBACK,UP> ...
#     link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
#     inet 127.0.0.1/8 scope host lo
#
# 2: enp0s3: <BROADCAST,MULTICAST,UP> ...
#     link/ether 08:00:27:xx:xx:xx brd ff:ff:ff:ff:ff:ff
#     inet 10.0.2.15/24 brd 10.0.2.255 scope global dynamic enp0s3

# ルーティングテーブル
ip route show
# 出力例:
# default via 10.0.2.2 dev enp0s3
# 10.0.2.0/24 dev enp0s3 proto kernel scope link src 10.0.2.15

# DNS 設定
cat /etc/resolv.conf
# 出力例:
# nameserver 10.0.2.3

# ネットワーク接続のテスト
ping -c 3 8.8.8.8
# → インターネット接続の確認

ping -c 3 google.com
# → DNS 解決の確認
```

---

## 12. LUKS/dm-crypt 暗号化の詳細要件

### 12.1 暗号化の検証

```bash
# LUKS ヘッダー情報の確認
sudo cryptsetup luksDump /dev/sda5
# 出力例:
# LUKS header information
# Version:        2
# Epoch:          3
# Metadata area:  16384 [bytes]
# Keyslots area:  16744448 [bytes]
# UUID:           xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
#
# Data segments:
#   0: crypt
#     offset:  16777216 [bytes]
#     length:  (whole device)
#     cipher:  aes-xts-plain64
#     sector:  512 [bytes]
#
# Keyslots:
#   0: luks2
#     Key:        512 bits
#     Priority:   normal
#     Cipher:     aes-xts-plain64
#     Cipher key: 512 bits
#     PBKDF:      argon2id
#       Time cost: 4
#       Memory:    1048576
#       Threads:   4
#     AF stripes:  4000
#     AF hash:     sha256
#     Area offset: 32768 [bytes]
#     Area length: 258048 [bytes]
#     Digest ID:   0

# 各フィールドの解説:
# Version: 2          → LUKS2 形式（LUKS1 より高機能）
# cipher: aes-xts-plain64 → 暗号化方式
#   aes   : Advanced Encryption Standard（暗号アルゴリズム）
#   xts   : XEX-based Tweaked-codebook mode with ciphertext Stealing
#           ディスク暗号化に最適化されたモード
#   plain64: IV (Initialization Vector) の生成方式
# Key: 512 bits       → AES-256（XTSモードでは鍵が2倍: 256×2=512）
# PBKDF: argon2id     → パスフレーズから鍵を導出するアルゴリズム
#   Argon2id: メモリハード関数（GPU攻撃に強い）
#   Memory: 1048576   → 1GB のメモリを使用（ブルートフォース対策）
```

### 12.2 暗号化パスフレーズの管理

```bash
# キースロットの確認
sudo cryptsetup luksDump /dev/sda5 | grep "Keyslot"
# LUKS2 は最大 32 個のキースロットを持つ
# 各スロットに異なるパスフレーズを設定可能

# パスフレーズの追加
sudo cryptsetup luksAddKey /dev/sda5
# 既存のパスフレーズを入力後、新しいパスフレーズを設定

# パスフレーズの削除
sudo cryptsetup luksRemoveKey /dev/sda5
# 削除するパスフレーズを入力

# 注意: 全てのパスフレーズを削除すると、
# データに永久にアクセスできなくなる

# パスフレーズの変更
sudo cryptsetup luksChangeKey /dev/sda5
# 古いパスフレーズを入力後、新しいパスフレーズを設定
```

---

## 13. LVM 管理の詳細要件

### 13.1 LVM の階層構造確認

```bash
# PV (Physical Volume) の確認
sudo pvdisplay
# 出力例:
#   --- Physical volume ---
#   PV Name               /dev/mapper/sda5_crypt
#   VG Name               LVMGroup
#   PV Size               19.52 GiB / not usable 0
#   Allocatable           yes
#   PE Size               4.00 MiB
#   Total PE              4997
#   Free PE               0
#   Allocated PE          4997
#   PV UUID               xxxxxx-xxxx-xxxx-xxxx-xxxx-xxxx-xxxxxx

# PE Size (Physical Extent): LVM の最小割り当て単位
# デフォルト: 4 MiB
# Total PE × PE Size = 合計容量

# VG (Volume Group) の確認
sudo vgdisplay
# 出力例:
#   --- Volume group ---
#   VG Name               LVMGroup
#   System ID
#   Format                lvm2
#   VG Size               19.52 GiB
#   PE Size               4.00 MiB
#   Total PE              4997
#   Alloc PE / Size       4997 / 19.52 GiB
#   Free  PE / Size       0 / 0

# LV (Logical Volume) の確認
sudo lvdisplay
# 各 LV について:
#   --- Logical volume ---
#   LV Path               /dev/LVMGroup/root
#   LV Name               root
#   VG Name               LVMGroup
#   LV Size               9.80 GiB
#   Current LE            2509
#   Segments              1

# LE (Logical Extent) と PE の関係:
# PE (Physical Extent) = 物理ディスク上の最小単位
# LE (Logical Extent) = 論理ボリューム上の最小単位
# 1 LE = 1 PE（リニアマッピングの場合）
```

### 13.2 LVM の操作（防御評価用）

```bash
# LV のサイズ拡張
sudo lvextend -L +1G /dev/LVMGroup/home
sudo resize2fs /dev/LVMGroup/home
# -L +1G: 1GB 追加
# resize2fs: ファイルシステムを拡張

# LV のサイズ縮小（データバックアップ後に実行）
sudo e2fsck -f /dev/LVMGroup/home
sudo resize2fs /dev/LVMGroup/home 3G
sudo lvreduce -L 3G /dev/LVMGroup/home
# 縮小は危険: 先にファイルシステムを縮小してから LV を縮小

# 新しい LV の作成
sudo lvcreate -L 500M -n newlv LVMGroup
sudo mkfs.ext4 /dev/LVMGroup/newlv
sudo mount /dev/LVMGroup/newlv /mnt/newlv

# LV の削除
sudo umount /mnt/newlv
sudo lvremove /dev/LVMGroup/newlv

# VG の空き容量確認
sudo vgdisplay | grep "Free"
# Free  PE / Size が 0 以上であれば新しい LV を作成可能
```

---

## 14. 防御評価の詳細 Q&A 準備

### 14.1 一般的な質問と回答例

#### Q: 仮想マシンとは何ですか？

A: 仮想マシン (VM) は、物理コンピュータのソフトウェア的な表現です。ハイパーバイザー（VirtualBox 等）が物理ハードウェアのリソース（CPU、メモリ、ディスク、ネットワーク）を抽象化し、複数の独立した仮想環境を作成します。各 VM は独自の OS を持ち、互いに分離されているため、一つの VM の障害が他に影響しません。Born2beRoot では、VirtualBox（Type 2 Hypervisor）を使用してホスト OS 上で Debian Linux を動作させています。

#### Q: Debian と Rocky Linux の違いは何ですか？なぜ Debian を選びましたか？

A: Debian はコミュニティ主導の独立したディストリビューションで、安定性と広大なパッケージリポジトリが特徴です。Rocky Linux は Red Hat Enterprise Linux (RHEL) のクローンで、エンタープライズ向けに設計されています。

主な違い:
- パッケージ管理: Debian は apt/dpkg、Rocky は dnf/rpm
- MAC: Debian は AppArmor、Rocky は SELinux
- リリースサイクル: Debian は約2年ごと、Rocky は RHEL に追随

Debian を選んだ理由: 42 のカリキュラムが初心者向けに Debian を推奨しており、apt の操作が直感的で、AppArmor の設定が SELinux より簡単なため。

#### Q: aptitude と apt の違いは何ですか？

A: apt と aptitude はどちらも Debian のパッケージ管理フロントエンドですが、重要な違いがあります。

```
apt:
  - コマンドラインインターフェースのみ
  - シンプルで高速
  - 依存関係の自動解決は基本的
  - Debian 8 以降の推奨ツール

aptitude:
  - コマンドラインと TUI（テキストUI）の両方
  - より高度な依存関係解決アルゴリズム
  - パッケージの「自動インストール」フラグを管理
  - 依存関係の競合時に複数の解決案を提示
  - 孤立パッケージの管理が容易

具体例:
  パッケージ A が B と C に依存し、B と C が競合する場合:
  - apt: エラーを表示して停止
  - aptitude: 複数の解決案を提示（B を削除、C をダウングレード等）
```

#### Q: AppArmor とは何ですか？

A: AppArmor (Application Armor) は Linux の強制アクセス制御 (MAC) システムです。通常の Linux パーミッション (DAC) に加えて、プロセスごとにアクセス可能なファイル、ネットワーク、ケーパビリティを制限するプロファイルを定義します。

```
DAC (通常の Linux パーミッション):
  - ファイル所有者がアクセス権を設定
  - root は全てにアクセス可能
  - プロセスが乗っ取られると、そのユーザーの全権限が奪われる

MAC (AppArmor):
  - システム管理者がプロファイルで制限を定義
  - root でもプロファイルの制限を超えられない
  - プロセスが乗っ取られても、プロファイルで許可された範囲のみアクセス可能
```

#### Q: UFW のルールを追加・削除してください

A:
```bash
# ルールの追加（例: port 8080）
sudo ufw allow 8080
sudo ufw status numbered

# ルールの削除
sudo ufw delete allow 8080
# または番号指定で削除
sudo ufw status numbered
sudo ufw delete 3

# 確認
sudo ufw status verbose
```

#### Q: 新しいユーザーを作成してパスワードポリシーが適用されていることを確認してください

A:
```bash
# ユーザーの作成
sudo adduser evaluator
# パスワードポリシーに準拠したパスワードを設定
# （10文字以上、大文字、小文字、数字含む、3文字連続なし）

# グループへの追加
sudo usermod -aG user42 evaluator
sudo usermod -aG sudo evaluator

# パスワードポリシーの確認
sudo chage -l evaluator
# Maximum: 30
# Minimum: 2
# Warning: 7

# パスワード強度テスト（弱いパスワードは拒否される）
sudo passwd evaluator
# "weak" → 拒否（短すぎる）
# "allonecase1" → 拒否（大文字なし）
# "Good1Pass!" → 受理
```

#### Q: cron のスケジュールを変更してください

A:
```bash
# cron ジョブの確認
sudo crontab -l
# */10 * * * * /usr/local/bin/monitoring.sh

# 1分ごとに変更（テスト用）
sudo crontab -e
# */1 * * * * /usr/local/bin/monitoring.sh

# monitoring.sh を停止
sudo crontab -e
# ジョブの行をコメントアウト
# #*/10 * * * * /usr/local/bin/monitoring.sh

# 元に戻す
sudo crontab -e
# */10 * * * * /usr/local/bin/monitoring.sh
```

#### Q: パーティション構成を説明してください

A:
```bash
lsblk
# パーティション構成の説明:
# /boot (sda1): GRUB が読み込むため暗号化の外に配置
#   → GRUB は LUKS2 を読めないため、/boot は暗号化しない
#
# LUKS 暗号化パーティション (sda5):
#   → ディスクの物理的盗難からデータを保護
#   → AES-256-XTS で暗号化
#
# LVM (LVMGroup):
#   → 暗号化パーティション内に構築
#   → 柔軟なストレージ管理（サイズ変更、スナップショット）
#
# 各 LV の分離理由:
#   root(/):    OS の基本ファイル
#   swap:       メモリ不足時の退避先
#   home:       ユーザーデータ（OS再インストールでもデータ保持）
#   var:        ログ、キャッシュ（可変データの隔離）
#   srv:        サービスデータ
#   tmp:        一時ファイル（noexec で攻撃スクリプト実行防止）
#   var-log:    ログ（ログ爆発攻撃からの保護）
```

#### Q: sudo のログはどこにありますか？

A:
```bash
# sudo コマンドログ
cat /var/log/sudo/sudo.log
# 全ての sudo コマンドの使用履歴

# I/O ログ（入力と出力の記録）
ls /var/log/sudo/00/00/
# 各セッションのディレクトリ

# 特定セッションの再生
sudo sudoreplay /var/log/sudo/00/00/01
# セッションの入出力を再生
```

### 14.2 防御評価のフロー

```
防御評価の一般的な流れ:

1. VM の起動と LUKS パスフレーズの入力
   → 暗号化が機能していることを確認

2. ログインとホスト名の確認
   → hostname が login42 形式であること

3. ユーザーとグループの確認
   → id username で sudo, user42 グループ所属を確認

4. パスワードポリシーの確認
   → chage -l username
   → /etc/pam.d/common-password の確認
   → 弱いパスワードでの変更テスト

5. SSH の確認
   → port 4242 で動作
   → root ログイン禁止
   → ホストから SSH 接続テスト

6. UFW の確認
   → ufw status: port 4242 のみ許可
   → ルールの追加と削除のデモ

7. sudo の確認
   → 設定ファイルの内容
   → ログの確認
   → パスワード3回失敗のテスト

8. monitoring.sh の確認
   → 手動実行
   → 各情報の取得方法を説明
   → cron の設定確認

9. 操作テスト
   → 新しいユーザーの作成
   → ホスト名の変更
   → パーティション構成の説明

10. Bonus（該当する場合）
    → WordPress の動作確認
    → 追加サービスの説明

所要時間の目安: 30-60分
```

---

## 15. セキュリティ設計の全体像

Born2beRoot で構築するセキュリティは、多層防御 (Defense in Depth) の考え方に基づいている。各層が独立して機能し、一つの層が突破されても他の層が防御を提供する。

```
┌────────────────────────────────────────────────────────────┐
│                    攻撃者の視点                              │
│                                                             │
│  Step 1: ネットワークスキャン                               │
│    → UFW が port 4242 のみ許可                             │
│    → 他のポートは全てブロック                               │
│    → 攻撃対象: SSH サービスのみ                            │
│                                                             │
│  Step 2: SSH ブルートフォース                               │
│    → root ログイン禁止（ユーザー名推測が必要）             │
│    → ポート変更（自動スキャナーの回避）                     │
│    → パスワードポリシー（強力なパスワード）                 │
│    → Fail2ban（Bonus: IPアドレスのブロック）                │
│                                                             │
│  Step 3: ログイン成功した場合                               │
│    → 一般ユーザー権限のみ（root ではない）                 │
│    → AppArmor によるプロセス制限                           │
│    → ファイルパーミッションによる制限                       │
│                                                             │
│  Step 4: 権限昇格の試行                                    │
│    → sudo に3回試行制限                                    │
│    → requiretty で非対話的な sudo を防止                   │
│    → secure_path で PATH 注入を防止                        │
│    → 全操作がログに記録される                               │
│                                                             │
│  Step 5: データの盗取の試行                                │
│    → LUKS 暗号化（ディスクの物理的盗取に対応）             │
│    → パーティション分離（被害範囲の限定）                   │
│    → /tmp に noexec（スクリプト実行防止）                   │
│                                                             │
│  Step 6: 痕跡の消去の試行                                  │
│    → /var/log が別パーティション（ログ削除が困難）         │
│    → sudo の入出力ログ（全操作が記録済み）                 │
│    → monitoring.sh による定期監視                           │
│                                                             │
│  結論: 多層防御により、各段階で攻撃が困難になる             │
└────────────────────────────────────────────────────────────┘
```

---

## 16. トラブルシューティングガイド

### 16.1 起動関連の問題

```
問題: GRUB が表示されない
─────────────────────
原因: ブートローダーが正しくインストールされていない
対処:
  1. Debian インストールメディアで起動
  2. レスキューモードを選択
  3. grub-install /dev/sda
  4. update-grub

問題: LUKS パスフレーズが通らない
─────────────────────
原因: キーボードレイアウトの問題（US配列で入力されている可能性）
対処:
  1. パスフレーズに記号を使用している場合、US配列での入力を試行
  2. CapsLock がオンになっていないか確認
  3. 数字はテンキーではなくメインキーボードで入力

問題: 起動後にネットワークが繋がらない
─────────────────────
原因: DHCP クライアントが動作していない
対処:
  1. ip addr show でインターフェース名を確認
  2. sudo dhclient enp0s3 で DHCP 要求
  3. /etc/network/interfaces の設定確認
```

### 16.2 SSH 関連の問題

```
問題: ホストから SSH 接続できない
─────────────────────
チェックリスト:
  □ VirtualBox のポートフォワーディング設定
    VM設定 → ネットワーク → ポートフォワーディング
    ホスト: 127.0.0.1:4242 → ゲスト: 10.0.2.15:4242

  □ sshd が port 4242 で動作しているか
    sudo ss -tunlp | grep 4242

  □ UFW で port 4242 が許可されているか
    sudo ufw status

  □ sshd サービスが起動しているか
    sudo systemctl status sshd

  □ sshd_config に構文エラーがないか
    sudo sshd -t

問題: "WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED"
─────────────────────
原因: VM を再作成した場合、SSH ホスト鍵が変更される
対処:
  ssh-keygen -R "[localhost]:4242"
  # known_hosts から古い鍵を削除
```

### 16.3 sudo 関連の問題

```
問題: "username is not in the sudoers file"
─────────────────────
対処:
  1. root でログイン: su -
  2. usermod -aG sudo username
  3. exit
  4. 再ログイン（グループの変更を反映）

問題: sudo 設定ファイルの構文エラー
─────────────────────
対処:
  1. pkexec visudo でエラーを修正
  2. または root でログインして visudo
  3. /etc/sudoers.d/ 内の問題ファイルを修正

問題: "sudo: no tty present and no askpass program specified"
─────────────────────
原因: requiretty が設定されているが、TTY がない環境で sudo を実行
対処:
  - cron から sudo を使う場合: NOPASSWD オプションを使用
  - または特定ユーザーに requiretty の除外を設定:
    Defaults:cronuser !requiretty
```

### 16.4 パスワード関連の問題

```
問題: パスワードが設定できない（ポリシー違反）
─────────────────────
pam_pwquality のエラーメッセージ対応表:

| エラーメッセージ | 原因 | 対処 |
|-----------------|------|------|
| "is too short" | 10文字未満 | 10文字以上にする |
| "not enough uppercase letters" | 大文字なし | 大文字を1文字以上追加 |
| "not enough lowercase letters" | 小文字なし | 小文字を1文字以上追加 |
| "not enough digits" | 数字なし | 数字を1文字以上追加 |
| "contains too many same characters consecutively" | 3文字超の連続 | 同一文字の連続を3文字以下に |
| "contains the user name" | ユーザー名含有 | ユーザー名を含めない |
| "not enough characters differ from old" | 旧PWとの差が7未満 | 7文字以上変更する |

問題: chage -l で有効期限が 99999 のまま
─────────────────────
原因: login.defs の変更は既存ユーザーに適用されない
対処:
  sudo chage -M 30 -m 2 -W 7 username
```
