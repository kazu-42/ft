# 02 - 前提知識 (Prerequisites)

Born2beRoot プロジェクトに取り組む前に、以下の知識を確認しておくこと。各セクションは単なるコマンドリファレンスではなく、「なぜそのコマンドが必要なのか」「内部でどのように動作するのか」を解説する。

---

## 1. 基本的な Linux コマンドライン操作

### 1.1 ファイル・ディレクトリ操作

Born2beRoot ではすべての操作を CLI（コマンドラインインターフェース）で行う。GUI は使用しない。サーバー管理において CLI が標準である理由:

- **リモートアクセス**: SSH 経由では GUI は使えない（X11フォワーディングは可能だが重い）
- **自動化**: CLI コマンドはスクリプト化できる。GUI の操作は自動化が困難
- **リソース効率**: GUI は CPU、メモリ、ディスクを消費する。サーバーでは無駄
- **再現性**: CLI コマンドは文字として記録・共有できる

| コマンド | 説明 | 使用例 | Born2beRoot での用途 |
|---------|------|--------|---------------------|
| `ls` | ファイル一覧を表示 | `ls -la /etc/` | 設定ファイルの確認 |
| `cd` | ディレクトリ移動 | `cd /var/log/` | ログディレクトリへの移動 |
| `pwd` | 現在のディレクトリを表示 | `pwd` | 作業位置の確認 |
| `mkdir` | ディレクトリ作成 | `mkdir -p /var/log/sudo/` | sudo ログディレクトリの作成 |
| `cp` | ファイルコピー | `cp /etc/ssh/sshd_config /etc/ssh/sshd_config.bak` | 設定ファイルのバックアップ |
| `mv` | ファイル移動・名前変更 | `mv old_name new_name` | ファイルの整理 |
| `rm` | ファイル削除 | `rm -rf /tmp/test/` | 不要ファイルの削除 |
| `cat` | ファイル内容表示 | `cat /etc/hostname` | 設定値の確認 |
| `less` | ページ送りで表示 | `less /var/log/syslog` | 長いログファイルの閲覧 |
| `head` / `tail` | 先頭/末尾を表示 | `tail -f /var/log/auth.log` | リアルタイムログ監視 |
| `find` | ファイル検索 | `find / -name "*.conf"` | 設定ファイルの探索 |
| `grep` | テキスト検索 | `grep "Port" /etc/ssh/sshd_config` | 設定値の検索 |
| `chmod` | パーミッション変更 | `chmod 755 monitoring.sh` | スクリプトに実行権限付与 |
| `chown` | 所有者変更 | `chown root:root monitoring.sh` | ファイルの所有者設定 |
| `stat` | ファイルの詳細情報 | `stat /etc/shadow` | パーミッション・所有者の詳細確認 |
| `wc` | 行数・単語数カウント | `wc -l /etc/passwd` | ユーザー数の確認 |
| `sort` | ソート | `sort -u` | 重複排除 |
| `uniq` | 重複行の削除 | `sort | uniq -c` | 重複カウント |
| `diff` | ファイル差分表示 | `diff file1 file2` | 設定変更の確認 |
| `tee` | 出力を分岐 | `command | tee output.txt` | 出力の保存と表示 |
| `touch` | 空ファイル作成/タイムスタンプ更新 | `touch newfile` | ファイル作成 |
| `ln` | リンク作成 | `ln -s /path/to/file link` | シンボリックリンク作成 |
| `du` | ディスク使用量 | `du -sh /var/log/` | ディレクトリのサイズ確認 |
| `file` | ファイルタイプ判定 | `file /usr/bin/ls` | ファイルの種類確認 |
| `which` | コマンドのパス表示 | `which sshd` | コマンドの場所確認 |
| `whereis` | コマンド・ソース・マニュアルの場所 | `whereis sshd` | 関連ファイル検索 |
| `type` | コマンドの種類 | `type ls` | 内部/外部コマンドの判別 |

#### ls コマンドの詳細

```bash
ls -la /etc/ssh/
# 出力例:
# total 580
# drwxr-xr-x  4 root root   4096 Jan 15 10:00 .
# drwxr-xr-x 87 root root   4096 Jan 15 09:00 ..
# -rw-r--r--  1 root root 553122 Oct 10 12:00 moduli
# -rw-r--r--  1 root root   1580 Oct 10 12:00 ssh_config
# -rw-------  1 root root   3285 Jan 15 10:00 sshd_config    ← 注目
# -rw-------  1 root root    399 Jan 15 09:30 ssh_host_ed25519_key
# -rw-r--r--  1 root root     89 Jan 15 09:30 ssh_host_ed25519_key.pub

# 各フィールドの意味:
# -rw-------  ← パーミッション (owner=rw, group=none, others=none)
# 1          ← ハードリンク数
# root       ← 所有者
# root       ← グループ
# 3285       ← ファイルサイズ（バイト）
# Jan 15     ← 最終更新日
# sshd_config ← ファイル名

# ls の主要オプション:
# -l : 詳細表示（ロングフォーマット）
# -a : 隠しファイル（.で始まるファイル）も表示
# -h : サイズを人間が読みやすい形式で表示（1K, 2M, 3G）
# -t : 更新日時順にソート
# -r : ソート順を逆にする
# -R : 再帰的にサブディレクトリも表示
# -i : inode 番号を表示
# -d : ディレクトリ自体の情報を表示（中身ではなく）
# -S : ファイルサイズ順にソート
```

**重要**: `sshd_config` のパーミッションが `-rw-------` (600) であることに注目。SSH の設定ファイルは root のみが読み書きできるべきであり、他のユーザーが読めると攻撃者に設定情報を知られてしまう。

#### find コマンドの詳細

```bash
# 名前で検索
find / -name "sshd_config"
# → /etc/ssh/sshd_config

# ワイルドカードで検索
find /etc -name "*.conf" -type f
# → 全ての .conf ファイル

# サイズで検索
find /var/log -size +10M
# → 10MB 以上のファイル

# 更新日時で検索
find /var/log -mtime -1
# → 24時間以内に更新されたファイル

find /tmp -mmin +60
# → 60分以上前に更新されたファイル

# パーミッションで検索
find / -perm -4000 -type f
# → SUID ビットが設定されたファイル（セキュリティ監査用）

# 所有者で検索
find /home -user kaztakam -type f

# 検索結果に対してコマンド実行
find /tmp -name "*.tmp" -exec rm {} \;
# → 全ての .tmp ファイルを削除

# 複数条件の組み合わせ
find /etc -name "*.conf" -size +1k -mtime -7
# → 1KB以上かつ7日以内に更新された .conf ファイル
```

#### grep コマンドの詳細

```bash
# 基本的な検索
grep "Port" /etc/ssh/sshd_config
# → Port 4242

# 大文字小文字を無視
grep -i "port" /etc/ssh/sshd_config

# 行番号を表示
grep -n "Port" /etc/ssh/sshd_config
# → 13:Port 4242

# 一致しない行を表示（反転マッチ）
grep -v "^#" /etc/ssh/sshd_config
# → コメント行以外を表示

# 再帰的に検索
grep -r "4242" /etc/
# → /etc 以下の全ファイルから 4242 を検索

# 正規表現を使用
grep -E "^Port|^PermitRootLogin" /etc/ssh/sshd_config
# → Port と PermitRootLogin の行を抽出

# マッチしたファイル名のみ表示
grep -rl "password" /etc/pam.d/
# → password を含むファイルのリスト

# 前後の行も表示
grep -A 2 -B 1 "Port" /etc/ssh/sshd_config
# → マッチ行の前1行、後2行も表示

# 単語単位でマッチ
grep -w "root" /etc/passwd
# → "root" という単語のみマッチ（"rootkit" 等にはマッチしない）

# カウント
grep -c "physical id" /proc/cpuinfo
# → マッチした行数を出力
```

### 1.2 テキストエディタ

設定ファイルの編集には以下のエディタを使用する:

- **nano**: 初心者向けの簡単なエディタ
  ```bash
  nano /etc/ssh/sshd_config
  # Ctrl+O で保存、Ctrl+X で終了
  # Ctrl+W で検索、Ctrl+\ で置換
  # Ctrl+K で行の切り取り、Ctrl+U で貼り付け
  # Ctrl+G でヘルプ
  # Alt+N で行番号表示のトグル
  # Ctrl+_ で指定行にジャンプ
  ```

- **vim / vi**: 高機能なエディタ（学習コストが高いが、使いこなせると非常に効率的）
  ```bash
  vim /etc/ssh/sshd_config

  # モード:
  # Normal mode  : 移動、削除、コピー等（デフォルト）
  # Insert mode  : テキスト入力（i, a, o で遷移）
  # Visual mode  : 範囲選択（v で遷移）
  # Command mode : コマンド実行（: で遷移）

  # 基本操作:
  # i       → カーソル位置の前に挿入
  # a       → カーソル位置の後に挿入
  # o       → 次の行に挿入
  # Esc     → Normal mode に戻る
  # :w      → 保存
  # :q      → 終了
  # :wq     → 保存して終了
  # :q!     → 保存せず強制終了
  # /keyword → 前方検索、n で次、N で前
  # dd      → 行の削除
  # yy      → 行のコピー
  # p       → 貼り付け
  # u       → アンドゥ
  # Ctrl+r  → リドゥ
  # gg      → ファイル先頭に移動
  # G       → ファイル末尾に移動
  # :set number → 行番号表示
  # :%s/old/new/g → 全置換
  ```

Born2beRoot の設定では nano を使うことが多いが、vim の基本操作も知っておくと実務で役立つ。

### 1.3 パイプとリダイレクト

Unix 哲学の中核概念: 「一つのことをうまくやるプログラムを、パイプで組み合わせる」

#### パイプ (|)

```bash
# パイプの基本: コマンドの標準出力を次のコマンドの標準入力にする
cat /proc/cpuinfo | grep "physical id" | sort -u | wc -l
#  ↓ /proc/cpuinfo の内容を出力
#              ↓ "physical id" を含む行だけフィルタ
#                                  ↓ 重複を排除してソート
#                                             ↓ 行数をカウント

# パイプの内部動作:
# 1. カーネルがパイプ（名前なし）を作成（バッファサイズ: デフォルト 64KB）
# 2. 左のコマンドの stdout がパイプの書き込み端に接続
# 3. 右のコマンドの stdin がパイプの読み取り端に接続
# 4. 両コマンドは並列に実行される（パイプバッファが空/満で同期）
```

#### リダイレクト

```bash
# 標準出力のリダイレクト
echo "Port 4242" > /etc/ssh/sshd_config    # 上書き (truncate + write)
echo "Port 4242" >> /etc/ssh/sshd_config   # 追記 (append)

# 標準エラー出力のリダイレクト
command 2>/dev/null    # エラーを捨てる
command 2>&1           # エラーを標準出力にマージ
command > output.txt 2>&1  # 標準出力とエラーの両方をファイルに
command &> output.txt      # 上と同じ（bash 4+ の省略形）

# 標準入力のリダイレクト
command < input.txt    # ファイルから入力

# Here Document（複数行のテキストをコマンドに渡す）
cat << 'EOF' > /etc/sudoers.d/sudo_config
Defaults passwd_tries=3
Defaults badpass_message="Wrong password."
EOF

# Here String
grep "Port" <<< "Port 4242"

# ファイルディスクリプタの概念:
#   0 = stdin  (標準入力)
#   1 = stdout (標準出力)
#   2 = stderr (標準エラー出力)
```

#### 実践的なパイプの組み合わせ

```bash
# monitoring.sh で使用するパイプの例

# 物理CPU数の取得（3段階のパイプ）
grep "physical id" /proc/cpuinfo | sort -u | wc -l

# 処理の流れ:
# Step 1: grep が /proc/cpuinfo から "physical id" を含む行を抽出
#   physical id : 0
#   physical id : 0
#   physical id : 1
#   physical id : 1
#
# Step 2: sort -u が重複を排除してソート
#   physical id : 0
#   physical id : 1
#
# Step 3: wc -l が行数をカウント
#   2

# メモリ使用量の計算
free -m | awk '/^Mem:/ {printf("%d/%dMB (%.2f%%)\n", $3, $2, $3/$2*100)}'
# 処理の流れ:
# Step 1: free -m がメモリ情報をMB単位で出力
#               total        used        free      ...
# Mem:           460         143          98        ...
# Swap:          952           0         952
#
# Step 2: awk が Mem: で始まる行を処理
#   $2 = 460 (total), $3 = 143 (used)
#   printf で "143/460MB (31.09%)" と出力

# ディスク使用量の計算
df -BG --total | awk '/^total/ {print $3"/"$2" ("$5")"}'

# sudo コマンドの実行回数
journalctl _COMM=sudo | grep COMMAND | wc -l

# TCP 接続数
ss -t state established | tail -n +2 | wc -l
# tail -n +2 でヘッダー行をスキップ
```

### 1.4 プロセス管理

#### プロセスのライフサイクル

Linux のプロセスは `fork()` と `exec()` システムコールで作成される:

```
PID 1 (systemd)
├── PID 100 (sshd)            ← systemd が fork+exec で起動
│   └── PID 200 (sshd: user)  ← SSH 接続時に fork
│       └── PID 201 (bash)    ← ログイン時に exec
│           └── PID 300 (ls)  ← コマンド実行時に fork+exec
├── PID 101 (cron)
│   └── PID 400 (monitoring.sh) ← 10分ごとに fork+exec
└── PID 102 (ufw)

fork() の動作:
  親プロセス                      子プロセス
  ┌──────────┐                   ┌──────────┐
  │ PID: 100 │ ── fork() ──→    │ PID: 200 │
  │ PPID: 1  │                   │ PPID: 100│
  │ メモリ:  │                   │ メモリ:  │
  │  (コピー)│                   │  (コピー)│
  └──────────┘                   └──────────┘

exec() の動作:
  子プロセス
  ┌──────────┐                   ┌──────────┐
  │ PID: 200 │ ── exec() ──→    │ PID: 200 │
  │ 旧プログラム│                 │ 新プログラム│
  │ (bash)   │                   │ (ls)     │
  └──────────┘                   └──────────┘
  ※ PID は変わらない。プログラムの内容が置き換わる
```

#### プロセス管理コマンド

```bash
# 実行中のプロセスを表示
ps aux
# a: 全ユーザーのプロセス
# u: ユーザー名と詳細情報を表示
# x: 端末に関連付けられていないプロセスも表示

# 出力例:
# USER  PID %CPU %MEM    VSZ   RSS TTY STAT START  TIME COMMAND
# root    1  0.0  0.5  16824  2400 ?   Ss   10:00  0:01 /sbin/init
# root  100  0.0  0.1  12345  2048 ?   Ss   10:00  0:00 /usr/sbin/sshd -D

# 各フィールドの意味:
# USER:  プロセスの所有者
# PID:   プロセスID
# %CPU:  CPU使用率
# %MEM:  メモリ使用率
# VSZ:   仮想メモリサイズ (KB)
# RSS:   物理メモリ使用量 (KB)
# TTY:   制御端末 (?=端末なし)
# STAT:  プロセス状態
#   S: スリープ（割り込み可能）
#   R: 実行中/実行可能
#   D: スリープ（割り込み不可、I/O待ち）
#   Z: ゾンビ（終了したが親が wait していない）
#   T: 停止
#   s: セッションリーダー
#   +: フォアグラウンド

ps aux | grep sshd
# 出力例: root  100  0.0  0.1  12345  2048 ?  Ss  10:00  0:00 /usr/sbin/sshd -D

# プロセスのツリー表示
pstree -p

# リアルタイムのプロセス監視
top
# top の読み方:
#   %us: ユーザー空間の CPU 使用率
#   %sy: カーネル空間の CPU 使用率
#   %id: アイドル率
#   %wa: I/O 待ち率
#   Tasks: XX total, X running, XX sleeping, X stopped, X zombie

# プロセスの終了
kill <PID>        # SIGTERM (15): 正常終了を要求
kill -9 <PID>     # SIGKILL (9): 強制終了
kill -HUP <PID>   # SIGHUP (1): 設定の再読み込み
```

#### シグナルの種類

| シグナル | 番号 | 動作 | 用途 | キーボード |
|---------|------|------|------|-----------|
| SIGTERM | 15 | 正常終了 | プロセスの停止（デフォルト） | - |
| SIGKILL | 9 | 強制終了 | 応答しないプロセスの強制停止 | - |
| SIGHUP | 1 | 設定再読込 | デーモンの設定リロード | - |
| SIGINT | 2 | 割り込み | プログラムの中断 | Ctrl+C |
| SIGTSTP | 20 | 一時停止 | プログラムの一時停止 | Ctrl+Z |
| SIGCONT | 18 | 再開 | 停止プロセスの再開 | fg / bg |
| SIGUSR1 | 10 | ユーザー定義 | アプリケーション固有 | - |
| SIGQUIT | 3 | コアダンプ付き終了 | デバッグ用 | Ctrl+\ |
| SIGPIPE | 13 | パイプ切断 | 読み手がいないパイプへの書き込み | - |
| SIGALRM | 14 | アラーム | タイマーによるシグナル | - |
| SIGCHLD | 17 | 子プロセス終了 | 親への通知 | - |

```bash
# シグナルの一覧表示
kill -l
# 出力: 1) SIGHUP  2) SIGINT  3) SIGQUIT  4) SIGILL  ...

# ジョブ管理
command &          # バックグラウンドで実行
jobs               # ジョブ一覧
fg %1              # ジョブ1をフォアグラウンドに
bg %1              # ジョブ1をバックグラウンドに
```

---

## 2. ネットワークの基礎

### 2.1 TCP/IP の4層モデル

Born2beRoot ではネットワーク設定（SSH、UFW、NAT）を行うため、TCP/IP の基礎理解が不可欠である。

```
┌─────────────────────────────────────────┐
│  Layer 4: アプリケーション層             │
│  (HTTP, SSH, DNS, SMTP, FTP)            │
│  → SSHプロトコル（Born2beRoot）         │
├─────────────────────────────────────────┤
│  Layer 3: トランスポート層               │
│  (TCP, UDP)                              │
│  → TCP ポート 4242（Born2beRoot）       │
├─────────────────────────────────────────┤
│  Layer 2: インターネット層               │
│  (IP, ICMP, ARP)                         │
│  → IP アドレス 10.0.2.15（NAT）        │
├─────────────────────────────────────────┤
│  Layer 1: ネットワークインターフェース層 │
│  (Ethernet, Wi-Fi)                       │
│  → MAC アドレス（monitoring.sh で取得） │
└─────────────────────────────────────────┘
```

### 2.2 IP アドレスの詳細

**IP アドレス**はネットワーク上のデバイスを識別する番号である。

- **IPv4**: 32ビット、例: `192.168.1.100`（ドット区切り10進表記）
- **IPv6**: 128ビット、例: `2001:0db8::1`（コロン区切り16進表記）
- **サブネットマスク**: ネットワーク部とホスト部を区別
- **ゲートウェイ**: ネットワーク外部への出口となるルーター

#### サブネットマスクの計算

```
サブネットマスク計算の実践:

例: 192.168.1.100/24 の場合

1. /24 を2進数に変換:
   11111111.11111111.11111111.00000000
   = 255.255.255.0

2. ネットワークアドレスの計算（IP AND サブネットマスク）:
   192.168.1.100:  11000000.10101000.00000001.01100100
   255.255.255.0:  11111111.11111111.11111111.00000000
   ─────────── AND ────────────────────────────────
   192.168.1.0:    11000000.10101000.00000001.00000000

3. ブロードキャストアドレス（ホスト部を全て1）:
   192.168.1.255:  11000000.10101000.00000001.11111111

4. 使用可能なホストアドレス:
   192.168.1.1 ～ 192.168.1.254
   ホスト数: 2^8 - 2 = 254
   (ネットワークアドレスとブロードキャストアドレスを除く)

よく使う CIDR とサブネットマスク:
  /8   = 255.0.0.0       → 16,777,214 ホスト
  /16  = 255.255.0.0     → 65,534 ホスト
  /24  = 255.255.255.0   → 254 ホスト
  /25  = 255.255.255.128 → 126 ホスト
  /26  = 255.255.255.192 → 62 ホスト
  /27  = 255.255.255.224 → 30 ホスト
  /28  = 255.255.255.240 → 14 ホスト
  /29  = 255.255.255.248 → 6 ホスト
  /30  = 255.255.255.252 → 2 ホスト（ポイントツーポイント）
  /32  = 255.255.255.255 → 1 ホスト（ホストルート）

計算式:
  ホスト数 = 2^(32 - CIDR) - 2
  サブネット数 = 2^(CIDR - 元のCIDR)
```

プライベート IP アドレスの範囲（RFC 1918）:
```
10.0.0.0/8        (10.0.0.0 - 10.255.255.255)     Class A: 約1677万アドレス
172.16.0.0/12     (172.16.0.0 - 172.31.255.255)    Class B: 約104万アドレス
192.168.0.0/16    (192.168.0.0 - 192.168.255.255)  Class C: 約65000アドレス
```

VirtualBox NAT モードでは、ゲスト VM に `10.0.2.15` が割り当てられる（デフォルト）。

#### サブネット計算の練習問題

```
問題1: 172.16.10.0/22 のネットワーク情報を求めよ

解答:
  サブネットマスク: 255.255.252.0
  (11111111.11111111.11111100.00000000)

  ネットワークアドレス:
  172.16.10.0:    10101100.00010000.00001010.00000000
  255.255.252.0:  11111111.11111111.11111100.00000000
  AND結果:        10101100.00010000.00001000.00000000
  → 172.16.8.0

  ブロードキャスト: 172.16.11.255
  ホスト範囲: 172.16.8.1 ～ 172.16.11.254
  ホスト数: 2^10 - 2 = 1022
```

### 2.3 ポート番号

**ポート番号**は、一つの IP アドレス上で複数のサービスを識別するための番号（0-65535）である。

よく使われるポート (Well-known Ports):

| ポート | サービス | プロトコル | 説明 |
|--------|---------|-----------|------|
| 20/21 | FTP | TCP | ファイル転送 |
| 22 | SSH | TCP | リモートログイン（暗号化） |
| 23 | Telnet | TCP | リモートログイン（非暗号化、廃止推奨） |
| 25 | SMTP | TCP | メール送信 |
| 53 | DNS | TCP/UDP | 名前解決 |
| 67/68 | DHCP | UDP | IP アドレスの自動割り当て |
| 80 | HTTP | TCP | Web サーバー |
| 110 | POP3 | TCP | メール受信 |
| 123 | NTP | UDP | 時刻同期 |
| 143 | IMAP | TCP | メール受信 |
| 443 | HTTPS | TCP | 暗号化 Web サーバー |
| 3306 | MySQL | TCP | データベース |
| 5432 | PostgreSQL | TCP | データベース |
| 6379 | Redis | TCP | キャッシュ |

ポート番号の分類:
- **Well-known ports** (0-1023): 標準サービス用、root 権限が必要
- **Registered ports** (1024-49151): 登録されたサービス用
- **Dynamic/Ephemeral ports** (49152-65535): 一時的な接続用（クライアント側）

Born2beRoot では port 4242 を使用する（Registered ports の範囲）。

### 2.4 TCP と UDP

#### TCP (Transmission Control Protocol)

**コネクション指向**のプロトコル。信頼性の高いデータ転送を保証する。

**3-Way Handshake** (接続確立):
```
Client                    Server
  │── SYN ──────────────→│    (1) 接続要求
  │←── SYN-ACK ─────────│    (2) 接続要求確認 + 応答
  │── ACK ──────────────→│    (3) 応答確認
  │                       │
  │←── データ通信 ──────→│    暗号化されたSSHセッション
  │                       │
  │── FIN ──────────────→│    (1) 切断要求
  │←── FIN-ACK ─────────│    (2) 切断確認
  │── ACK ──────────────→│    (3) 最終確認
```

SSH は TCP を使用する（信頼性が必要なため）。

#### UDP (User Datagram Protocol)

**コネクションレス**のプロトコル。高速だが信頼性の保証がない。

```
Client                    Server
  │── データ ──────────→│    送りっぱなし
  │── データ ──────────→│    応答確認なし
  │── データ ──────────→│    パケットロストの可能性あり
```

DNS, DHCP, NTP, ストリーミング等で使用。

### 2.5 DNS (Domain Name System) の基礎

```bash
# DNS クエリの実行
nslookup deb.debian.org
# 出力:
# Server:     10.0.2.3
# Address:    10.0.2.3#53
# Non-authoritative answer:
# Name:       deb.debian.org
# Address:    199.232.182.132

# dig コマンド（より詳細）
dig deb.debian.org A
# QUESTION SECTION: deb.debian.org. IN A
# ANSWER SECTION: deb.debian.org. 300 IN A 199.232.182.132

# レコードタイプ:
# A:     IPv4 アドレス
# AAAA:  IPv6 アドレス
# CNAME: 別名（エイリアス）
# MX:    メールサーバー
# NS:    ネームサーバー
# PTR:   逆引き（IP → ドメイン名）
# TXT:   テキスト情報

# DNS 設定ファイル
cat /etc/resolv.conf
# nameserver 10.0.2.3

# ローカルの名前解決
cat /etc/hosts
# 127.0.0.1    localhost
# 127.0.1.1    kaztakam42

# 名前解決の優先順位
cat /etc/nsswitch.conf | grep hosts
# hosts: files dns
# → まず /etc/hosts を参照、次に DNS に問い合わせ
```

### 2.6 NAT の仕組み (VirtualBox のポートフォワーディング)

VirtualBox NAT モードの内部動作を詳細に解説する:

```
ホスト OS                          VirtualBox NAT Engine              ゲスト VM
(192.168.1.100)                    (NAT テーブル管理)                 (10.0.2.15)

[ホストからゲストへのSSH接続の流れ]

(1) ssh localhost -p 4242
    │
    ▼
(2) localhost:4242 へ TCP 接続
    │
    ▼
(3) VirtualBox がポートフォワーディングルールに一致するか確認
    ルール: Host 127.0.0.1:4242 → Guest 10.0.2.15:4242
    │
    ▼
(4) VirtualBox が新しい TCP 接続を 10.0.2.15:4242 に作成
    │
    ▼
(5) ゲスト VM の sshd (port 4242) が接続を受け入れ

[ゲストから外部への通信の流れ]

(1) ゲスト VM が apt update を実行
    10.0.2.15:50000 → deb.debian.org:443
    │
    ▼
(2) VirtualBox NAT Engine が Source NAT を実行
    変換: 10.0.2.15:50000 → 192.168.1.100:60000
    NAT テーブルにマッピングを記録
    │
    ▼
(3) ホスト OS のネットワークスタックから外部へ送信
    192.168.1.100:60000 → deb.debian.org:443
    │
    ▼
(4) レスポンスが返ってくる
    deb.debian.org:443 → 192.168.1.100:60000
    │
    ▼
(5) VirtualBox が NAT テーブルを参照して逆変換
    192.168.1.100:60000 → 10.0.2.15:50000
    │
    ▼
(6) ゲスト VM にレスポンスを配送
```

### 2.7 ルーティングの基礎

```bash
# ルーティングテーブルの確認
ip route show
# 出力例:
# default via 10.0.2.2 dev enp0s3
# 10.0.2.0/24 dev enp0s3 proto kernel scope link src 10.0.2.15

# ルーティングの読み方:
# default via 10.0.2.2 dev enp0s3
#   → 宛先不明のパケットは 10.0.2.2 (ゲートウェイ) に転送
#
# 10.0.2.0/24 dev enp0s3 proto kernel scope link src 10.0.2.15
#   → 10.0.2.0/24 宛は enp0s3 から直接送信

# 経路追跡
traceroute 8.8.8.8
# 1  10.0.2.2   (VirtualBox NAT ゲートウェイ)
# 2  192.168.1.1 (ホストのルーター)
# ...

# ネットワークインターフェースの確認
ip addr show
# 出力例:
# 1: lo: <LOOPBACK,UP> ...
#     inet 127.0.0.1/8 scope host lo
# 2: enp0s3: <BROADCAST,MULTICAST,UP> ...
#     link/ether 08:00:27:xx:xx:xx brd ff:ff:ff:ff:ff:ff
#     inet 10.0.2.15/24 brd 10.0.2.255 scope global dynamic enp0s3
```

### 2.8 ネットワーク関連コマンド

```bash
# IP アドレスの確認
ip addr show
hostname -I

# ネットワーク接続の確認
ss -tunlp          # リスニングポートの一覧
# -t: TCP
# -u: UDP
# -n: 数値表示（名前解決しない）
# -l: LISTEN 状態のみ
# -p: プロセス情報を表示

ss -t state established  # 確立済み TCP 接続

# DNS 解決
nslookup example.com
dig example.com

# 接続テスト
ping 8.8.8.8            # ICMP による到達性確認
ping -c 4 8.8.8.8       # 4回だけ送信
```

---

## 3. ユーザー・グループ・パーミッション

### 3.1 ユーザー管理

Linux はマルチユーザー OS であり、各ユーザーは固有の **UID (User ID)** を持つ。

```bash
# ユーザーの作成
sudo adduser username
# adduser は対話的にパスワード、フルネーム等を設定する（Debian 推奨）
# useradd はより低レベルで、オプションを手動で指定する必要がある

# adduser と useradd の違い:
# adduser:
#   - Debian 固有の高レベルスクリプト
#   - 対話的にパスワード等を設定
#   - ホームディレクトリを自動作成
#   - /etc/skel のファイルをコピー
#   - Born2beRoot ではこちらを使用

# useradd:
#   - 低レベルコマンド（全ディストリビューション共通）
#   - オプションなしではホームディレクトリを作成しない
#   - useradd -m -s /bin/bash username のように指定が必要

# ユーザーの削除
sudo deluser username
sudo deluser --remove-home username  # ホームディレクトリも削除

# ユーザー情報の確認
id username
# 出力例: uid=1000(kaztakam) gid=1000(kaztakam) groups=1000(kaztakam),27(sudo),1001(user42)

# パスワードの変更
passwd           # 自分のパスワード
sudo passwd username  # 他のユーザーのパスワード

# ユーザーの一覧
cat /etc/passwd | cut -d: -f1

# ログイン中のユーザー
who
w          # より詳細
users      # ユーザー名のみ
```

重要なファイル:

#### /etc/passwd の構造
```
kaztakam:x:1000:1000:Kaztakam,,,:/home/kaztakam:/bin/bash
│        │ │    │    │           │               │
│        │ │    │    │           │               └── ログインシェル
│        │ │    │    │           └── ホームディレクトリ
│        │ │    │    └── GECOS フィールド（フルネーム等）
│        │ │    └── プライマリ GID
│        │ └── UID
│        └── パスワードフィールド（x = shadow に格納）
└── ユーザー名

特殊な UID:
  0:       root（スーパーユーザー）
  1-999:   システムユーザー（デーモン等）
  1000+:   一般ユーザー
  65534:   nobody（権限なしユーザー）
```

#### /etc/shadow の構造
```
kaztakam:$6$salt$hash:19600:2:30:7:::
│        │             │    │ │  │
│        │             │    │ │  └── パスワード期限警告日数 (WARN_AGE)
│        │             │    │ └── パスワード最大有効日数 (MAX_DAYS)
│        │             │    └── パスワード最小変更間隔 (MIN_DAYS)
│        │             └── 最終パスワード変更日（1970/1/1 からの日数）
│        └── パスワードハッシュ（$6$ = SHA-512, $salt$ = ソルト, $hash$ = ハッシュ値）
└── ユーザー名

ハッシュアルゴリズムの識別子:
  $1$:  MD5（非推奨、脆弱）
  $5$:  SHA-256
  $6$:  SHA-512（Debian のデフォルト、推奨）
  $y$:  yescrypt（最新のディストリビューションで採用）
```

### 3.2 グループ管理

```bash
# グループの作成
sudo groupadd user42

# ユーザーをグループに追加
sudo usermod -aG sudo username      # sudo グループに追加
sudo usermod -aG user42 username    # user42 グループに追加
# -a: append（既存のグループを保持したまま追加）
# -G: supplementary groups の指定
# 注意: -a を忘れると、指定したグループ以外から削除される!

# ★★★ -a を忘れた場合の影響 ★★★
# usermod -G user42 username  ← -a がない!
# → username は user42 グループのみになり、
#   sudo グループから外れる → sudo が使えなくなる!
# → リカバリには root でログインする必要がある

# ユーザーのグループを確認
groups username
id username

# グループの一覧
cat /etc/group
getent group user42
# 出力: user42:x:1001:kaztakam

# /etc/group の構造:
# group_name:password:GID:member_list
# user42:x:1001:kaztakam,user2
```

Born2beRoot で必要なグループ設定:
- ユーザーを `sudo` グループに追加（sudo 権限の付与）
- ユーザーを `user42` グループに追加（プロジェクト要件）

### 3.3 ファイルパーミッション

```
-rwxr-xr-x  1  root  root  1234  Jan  1 00:00  monitoring.sh
│└┬┘└┬┘└┬┘     └┬┘   └┬┘
│ │   │  │       │     │
│ │   │  │       │     └── グループ所有者
│ │   │  │       └──────── ファイル所有者
│ │   │  └──────────────── Others のパーミッション (r-x = 5)
│ │   └─────────────────── Group のパーミッション (r-x = 5)
│ └─────────────────────── Owner のパーミッション (rwx = 7)
└───────────────────────── ファイルタイプ
```

#### パーミッションの8進数表記の詳細

```
パーミッションビットの構造:

   r   w   x
   4   2   1    ← 各ビットの数値

組み合わせ:
   rwx = 4+2+1 = 7  (読み取り + 書き込み + 実行)
   rw- = 4+2+0 = 6  (読み取り + 書き込み)
   r-x = 4+0+1 = 5  (読み取り + 実行)
   r-- = 4+0+0 = 4  (読み取りのみ)
   -wx = 0+2+1 = 3  (書き込み + 実行)
   -w- = 0+2+0 = 2  (書き込みのみ)
   --x = 0+0+1 = 1  (実行のみ)
   --- = 0+0+0 = 0  (なし)

3桁の8進数: Owner | Group | Others
  755 = rwxr-xr-x  (Owner=全権限, Group/Others=読み取り+実行)
  644 = rw-r--r--  (Owner=読み書き, Group/Others=読み取りのみ)
  600 = rw-------  (Owner=読み書き, 他はアクセス不可)
  440 = r--r-----  (Owner/Group=読み取りのみ, Others=不可)
  777 = rwxrwxrwx  (全ユーザーに全権限 - 通常は危険!)
  000 = ---------  (誰もアクセスできない)

Born2beRoot で重要なパーミッション:
  chmod 755 /usr/local/bin/monitoring.sh  # 実行可能スクリプト
  chmod 600 /etc/shadow                    # パスワードファイル
  chmod 440 /etc/sudoers                   # sudoers ファイル
  chmod 700 /root                          # root ホームディレクトリ
  chmod 644 /etc/ssh/sshd_config           # SSH設定（rootが読み書き、他は読み取り）
```

#### chmod コマンドの使い方

```bash
# 数値モード
chmod 755 monitoring.sh

# シンボリックモード
chmod u+x monitoring.sh        # Owner に実行権限を追加
chmod g-w file                 # Group から書き込み権限を削除
chmod o= file                  # Others のパーミッションをクリア
chmod a+r file                 # 全員に読み取り権限を追加 (a=all)
chmod u=rwx,g=rx,o=rx file     # 755 と同じ

# 再帰的に変更
chmod -R 755 /var/www/          # ディレクトリ以下を再帰的に変更

# 参照ファイルと同じパーミッション
chmod --reference=file1 file2
```

#### 特殊パーミッション

| ビット | 名前 | 8進数 | 効果 |
|--------|------|-------|------|
| SUID | Set User ID | 4000 | 実行時に所有者の権限で実行される |
| SGID | Set Group ID | 2000 | 実行時にグループの権限で実行される |
| Sticky Bit | スティッキービット | 1000 | ディレクトリ内のファイルは所有者のみ削除可能 |

```bash
# SUID の例: passwd コマンド
ls -la /usr/bin/passwd
# -rwsr-xr-x 1 root root 59640 Jan 1 00:00 /usr/bin/passwd
#    ^
#    s = SUID ビットが設定されている
# 一般ユーザーが passwd を実行しても、root 権限で /etc/shadow を更新できる

# Sticky Bit の例: /tmp
ls -ld /tmp
# drwxrwxrwt 10 root root 4096 Jan 1 00:00 /tmp
#          ^
#          t = Sticky Bit が設定されている
# 全ユーザーが /tmp に書き込み可能だが、他人のファイルは削除できない

# SUID ビットが設定されたファイルの一覧（セキュリティ監査用）
find / -perm -4000 -type f 2>/dev/null
# 出力例:
# /usr/bin/passwd
# /usr/bin/sudo
# /usr/bin/su
# /usr/bin/chsh
# /usr/bin/mount
# /usr/bin/umount

# 特殊パーミッションの設定
chmod 4755 file    # SUID + 755
chmod 2755 dir     # SGID + 755
chmod 1777 dir     # Sticky Bit + 777
```

#### umask

umask はファイル作成時のデフォルトパーミッションを決定する:

```bash
# 現在の umask を確認
umask
# 出力: 0022

# umask の計算:
# ファイル: 666 - 022 = 644 (rw-r--r--)
# ディレクトリ: 777 - 022 = 755 (rwxr-xr-x)

# umask の変更
umask 027
# ファイル: 666 - 027 = 640 (rw-r-----)
# ディレクトリ: 777 - 027 = 750 (rwxr-x---)
```

---

## 4. シェルスクリプトの基礎

### 4.1 基本構文

```bash
#!/bin/bash
# Shebang (#!): このスクリプトを実行するインタプリタを指定
# /bin/bash: Bourne Again Shell
# /bin/sh:   POSIX 互換シェル（より移植性が高い）

# 変数（= の前後にスペースを入れない）
NAME="Born2beRoot"
echo "Project: ${NAME}"

# コマンド置換（コマンドの出力を変数に格納）
KERNEL=$(uname -r)
echo "Kernel: ${KERNEL}"

# バッククォート形式（古い書き方、非推奨）
KERNEL=`uname -r`

# 算術演算
COUNT=$((5 + 3))          # → 8
COUNT=$((COUNT + 1))      # → 9
let "COUNT = COUNT + 1"   # → 10

# 文字列操作
STR="Hello World"
echo ${#STR}              # 文字列長: 11
echo ${STR:0:5}           # 部分文字列: Hello
echo ${STR/World/Linux}   # 置換: Hello Linux
echo ${STR,,}             # 小文字化: hello world
echo ${STR^^}             # 大文字化: HELLO WORLD

# 条件分岐
if [ -f /etc/debian_version ]; then
    echo "This is Debian"
elif [ -f /etc/redhat-release ]; then
    echo "This is Red Hat based"
else
    echo "Unknown distribution"
fi

# 条件テスト演算子
# ファイルテスト:
# -f FILE: 通常ファイルが存在する
# -d DIR:  ディレクトリが存在する
# -e PATH: パスが存在する（ファイル・ディレクトリどちらでも）
# -r FILE: 読み取り可能
# -w FILE: 書き込み可能
# -x FILE: 実行可能
# -s FILE: ファイルサイズが0でない
# -L FILE: シンボリックリンク

# 文字列テスト:
# -z STRING: 文字列が空
# -n STRING: 文字列が空でない
# STRING1 = STRING2: 文字列が等しい（[ ]内で使用）
# STRING1 == STRING2: 文字列が等しい（[[ ]]内で使用）
# STRING1 != STRING2: 文字列が異なる

# 数値テスト:
# NUM1 -eq NUM2: 等しい (equal)
# NUM1 -ne NUM2: 等しくない (not equal)
# NUM1 -gt NUM2: より大きい (greater than)
# NUM1 -ge NUM2: 以上 (greater or equal)
# NUM1 -lt NUM2: より小さい (less than)
# NUM1 -le NUM2: 以下 (less or equal)

# 論理演算子:
# [ cond1 ] && [ cond2 ]   : AND
# [ cond1 ] || [ cond2 ]   : OR
# ! [ cond ]               : NOT

# ループ
for i in 1 2 3 4 5; do
    echo "Number: ${i}"
done

# C 言語スタイルの for ループ
for ((i=0; i<5; i++)); do
    echo "Index: ${i}"
done

# ファイル一覧のループ
for file in /etc/*.conf; do
    echo "Config: ${file}"
done

# while ループ
count=0
while [ $count -lt 5 ]; do
    echo "Count: ${count}"
    count=$((count + 1))
done

# case 文
case "$1" in
    start)
        echo "Starting..."
        ;;
    stop)
        echo "Stopping..."
        ;;
    restart)
        echo "Restarting..."
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        ;;
esac

# 関数
get_ram_usage() {
    free -m | awk '/^Mem:/ {printf("%d/%dMB (%.2f%%)", $3, $2, $3/$2*100)}'
}
# 関数の呼び出し
RAM=$(get_ram_usage)
echo "RAM: ${RAM}"

# 終了コード
# $? : 直前のコマンドの終了コード（0=成功, 非0=失敗）
ls /etc/hostname
echo $?    # → 0（成功）
ls /nonexistent
echo $?    # → 2（失敗）

# 特殊変数
# $0 : スクリプト名
# $1, $2, ... : 引数
# $# : 引数の数
# $@ : 全引数（個別に展開）
# $* : 全引数（1つの文字列として展開）
# $$ : 現在のプロセスID
# $! : 最後のバックグラウンドプロセスのPID
```

### 4.2 sed コマンドの詳細

**sed (Stream Editor)** はテキストの変換・置換を行うコマンド:

```bash
# 基本的な置換（最初のマッチのみ）
sed 's/old/new/' file

# 全てのマッチを置換（g フラグ）
sed 's/old/new/g' file

# ファイルを直接編集（-i オプション）
sed -i 's/Port 22/Port 4242/' /etc/ssh/sshd_config

# バックアップを作成して編集
sed -i.bak 's/Port 22/Port 4242/' /etc/ssh/sshd_config
# → sshd_config.bak が作成される

# 特定の行のみ置換
sed '5s/old/new/' file       # 5行目のみ
sed '1,10s/old/new/g' file   # 1-10行目

# 行の削除
sed '/^#/d' file             # コメント行を削除
sed '/^$/d' file             # 空行を削除

# 行の挿入
sed '3i\New line' file       # 3行目の前に挿入
sed '3a\New line' file       # 3行目の後に挿入

# 特定のパターンを含む行を表示
sed -n '/Port/p' /etc/ssh/sshd_config
# → grep "Port" /etc/ssh/sshd_config と同じ

# 区切り文字の変更（/ を含む文字列の置換に便利）
sed 's|/usr/bin|/usr/local/bin|g' file
```

### 4.3 awk の詳細

`awk` は monitoring.sh で頻繁に使用するテキスト処理ツールである:

```bash
# フィールドの抽出（デフォルトはスペース/タブ区切り）
echo "hello world" | awk '{print $2}'    # → world
# $0: 行全体, $1: 第1フィールド, $2: 第2フィールド, ...
# NF: フィールド数, NR: 行番号

# 特定の行をフィルタ（正規表現パターン）
free -m | awk '/^Mem:/ {print $2}'       # → Mem: で始まる行の第2フィールド

# printf で書式指定
free | awk '/^Mem:/ {printf("%.2f%%\n", $3/$2*100)}'  # → メモリ使用率

# フィールドセパレータの変更
cat /etc/passwd | awk -F: '{print $1, $3}'  # → ユーザー名とUID

# 複数の条件
awk '/pattern1/ && /pattern2/ {print $0}' file

# 組み込み変数
awk '{print NR": "$0}' file       # 行番号付きで全行表示
awk 'END {print NR}' file         # 全行数を表示
awk '{sum += $1} END {print sum}' file  # 第1フィールドの合計

# BEGIN と END ブロック
awk 'BEGIN {print "--- Start ---"}
     {print $0}
     END {print "--- End ---"}' file

# 条件分岐
awk '{if ($3 > 1000) print $1, "is a regular user"}' /etc/passwd

# monitoring.sh で使用する具体例:

# メモリ使用量
free -m | awk '/^Mem:/ {printf("%d/%dMB (%.2f%%)\n", $3, $2, $3/$2*100)}'
# 出力: 143/460MB (31.09%)

# ディスク使用量
df -BG --total | awk '/^total/ {printf("%s/%s (%s)\n", $3, $2, $5)}'
# 出力: 1G/5G (26%)

# CPU 使用率
top -bn1 | awk '/^%Cpu/ {printf("%.1f%%\n", 100.0 - $8)}'
# 出力: 6.7%

# 最終起動時刻
who -b | awk '{print $3" "$4}'
# 出力: 2024-01-15 10:24
```

### 4.4 monitoring.sh で使用するツール

| コマンド | 用途 | 具体的な使い方 |
|---------|------|--------------|
| `uname -a` | カーネル/アーキテクチャ情報 | `ARCH=$(uname -a)` |
| `grep` / `wc` | テキスト検索とカウント | `grep "physical id" /proc/cpuinfo \| wc -l` |
| `free` | メモリ使用状況 | `free -m \| awk '/^Mem:/ {print $3}'` |
| `df` | ディスク使用状況 | `df -BG --total \| awk '/^total/ {print $2}'` |
| `top -bn1` | CPU 使用率（バッチモード） | `top -bn1 \| grep "^%Cpu"` |
| `who -b` | 最終ブート時刻 | `who -b \| awk '{print $3" "$4}'` |
| `lsblk` | ブロックデバイス一覧 | LVM 確認 |
| `ss` | ソケット統計（TCP 接続） | `ss -t state established \| wc -l` |
| `hostname -I` | IP アドレス | `hostname -I \| awk '{print $1}'` |
| `ip link` | ネットワークインターフェース | MAC アドレス取得 |
| `awk` | テキスト処理 | 各種データの抽出・計算 |
| `wall` | 全端末へのメッセージ送信 | 最終出力 |
| `journalctl` | systemd ジャーナル | sudo コマンド数の取得 |

---

## 5. サービスと systemd

### 5.1 systemd とは何か

**systemd** は Linux の init システム（PID 1 のプロセス）で、システムの起動、サービスの管理、ログの管理を行う。従来の SysV init を置き換え、並列起動や依存関係管理を提供する。

### 5.2 systemctl 全サブコマンド解説

```bash
# ===== サービスの基本操作 =====

# 状態確認
sudo systemctl status sshd
# 出力の読み方:
#   Loaded: loaded (/lib/systemd/system/ssh.service; enabled)
#     ↑ unit ファイルの場所                    ↑ 自動起動が有効
#   Active: active (running) since Mon 2024-01-15 10:00:00 UTC; 2h ago
#     ↑ 現在の状態
#   Main PID: 100 (sshd)
#   CGroup: /system.slice/ssh.service
#           └─100 /usr/sbin/sshd -D

# 起動・停止・再起動
sudo systemctl start sshd       # 起動
sudo systemctl stop sshd        # 停止
sudo systemctl restart sshd     # 停止してから起動
sudo systemctl reload sshd      # 設定の再読み込み（サービスを停止しない）
sudo systemctl try-restart sshd # 実行中の場合のみ再起動
sudo systemctl reload-or-restart sshd  # reload 可能なら reload、不可なら restart

# 自動起動設定
sudo systemctl enable sshd      # ブート時に自動起動
sudo systemctl disable sshd     # 自動起動を無効化
sudo systemctl enable --now sshd # 有効化 + 即時起動
sudo systemctl is-enabled sshd  # 自動起動の設定確認
# 出力: enabled / disabled / static / masked

# マスク（完全な無効化）
sudo systemctl mask sshd        # サービスを完全に無効化（手動起動も不可）
sudo systemctl unmask sshd      # マスク解除

# ===== 状態確認 =====
systemctl is-active sshd        # アクティブかどうか
systemctl is-failed sshd        # 失敗しているかどうか
systemctl show sshd             # 全プロパティ表示
systemctl show sshd -p MainPID  # 特定プロパティ

# ===== ユニット一覧 =====
systemctl list-units                         # 全ユニット
systemctl list-units --type=service          # サービスのみ
systemctl list-units --type=service --state=running  # 実行中のサービス
systemctl list-units --state=failed          # 失敗したユニット
systemctl list-unit-files                    # 全ユニットファイル
systemctl list-unit-files --type=service     # サービスファイル
systemctl list-dependencies sshd             # 依存関係ツリー
systemctl list-dependencies --reverse sshd   # 逆依存関係

# ===== ユニットファイル操作 =====
systemctl cat sshd               # ユニットファイルの内容表示
systemctl edit sshd              # ドロップインファイルの編集
systemctl edit --full sshd       # ユニットファイル全体の編集
systemctl daemon-reload          # ユニットファイル変更の反映

# ===== システム全体 =====
systemctl get-default            # デフォルトターゲット
sudo systemctl set-default multi-user.target  # デフォルト設定

# ===== 電源管理 =====
sudo systemctl reboot            # 再起動
sudo systemctl poweroff          # シャットダウン
sudo systemctl halt              # 停止

# ===== ログ =====
journalctl -u sshd               # sshd のログ
journalctl -u sshd -f            # リアルタイム監視
journalctl -u sshd --since today # 今日のログ
journalctl -u sshd -p err        # エラーのみ
journalctl -b                    # 現在のブートのログ
journalctl --disk-usage          # ジャーナルのディスク使用量
```

### 5.3 Born2beRoot で管理するサービス

| サービス | 説明 | 期待される状態 | 確認コマンド |
|---------|------|-------------|-------------|
| `sshd` (ssh) | SSH サーバー | enabled, running | `systemctl status sshd` |
| `ufw` | ファイアウォール | enabled, active | `systemctl status ufw` |
| `cron` | タスクスケジューラ | enabled, running | `systemctl status cron` |
| `apparmor` | セキュリティモジュール | enabled, running | `systemctl status apparmor` |

---

## 6. 実践演習

### 6.1 ファイル操作の演習

```bash
# 演習1: ログファイルの分析
# auth.log から SSH ログイン失敗の回数を数える
grep "Failed password" /var/log/auth.log | wc -l

# 演習2: ログファイルから攻撃元 IP を抽出
grep "Failed password" /var/log/auth.log | awk '{print $(NF-3)}' | sort | uniq -c | sort -rn

# 演習3: 大きなファイルの検索
find / -type f -size +10M 2>/dev/null | sort

# 演習4: 設定ファイルのバックアップ
for conf in /etc/ssh/sshd_config /etc/pam.d/common-password /etc/login.defs; do
    cp "$conf" "${conf}.backup.$(date +%Y%m%d)"
done
```

### 6.2 ネットワーク診断の演習

```bash
# 演習1: ネットワーク接続の確認
ip addr show                     # IP アドレス確認
ip route show                    # ルーティング確認
ss -tunlp                        # リスニングポート確認
ping -c 3 8.8.8.8               # 外部接続確認

# 演習2: SSH 接続のトラブルシューティング
# Step 1: sshd が起動しているか
systemctl status sshd

# Step 2: port 4242 でリスニングしているか
ss -tunlp | grep 4242

# Step 3: UFW が port 4242 を許可しているか
sudo ufw status

# Step 4: 接続テスト
ssh -v -p 4242 user@localhost
# -v: verbose モード（接続の各ステップを表示）
```

### 6.3 パーミッションの演習

```bash
# 演習1: パーミッションの計算
# 以下のパーミッションを8進数で表現せよ:
# rwxr-x---  → 750
# rw-r--r--  → 644
# rwx------  → 700
# r--r-----  → 440

# 演習2: SUID ファイルの監査
find / -perm -4000 -type f 2>/dev/null

# 演習3: 適切なパーミッションの設定
chmod 755 /usr/local/bin/monitoring.sh   # 実行スクリプト
chmod 600 /etc/shadow                     # パスワードファイル
chmod 440 /etc/sudoers                    # sudoers
chmod 700 /root                           # root ホームディレクトリ
```

---

## 確認チェックリスト

以下の項目がすべて理解できているか確認すること:

### コマンド操作
- [ ] `ls`, `cd`, `cp`, `mv`, `rm`, `chmod`, `chown` が使える
- [ ] テキストエディタ（nano または vim）で設定ファイルを編集できる
- [ ] パイプ (`|`) とリダイレクト (`>`, `>>`, `2>`) を理解している
- [ ] `find` と `grep` でファイルやテキストを検索できる
- [ ] `sed` で基本的なテキスト置換ができる
- [ ] 特殊パーミッション（SUID, SGID, Sticky Bit）を理解している
- [ ] umask の計算ができる

### ネットワーク
- [ ] IP アドレス、サブネットマスク、ゲートウェイの概念を理解している
- [ ] サブネットマスクの計算（CIDR 表記との変換）ができる
- [ ] ポート番号の役割と Well-known ports を知っている
- [ ] TCP と UDP の違いを説明できる
- [ ] TCP 3-way handshake の流れを説明できる
- [ ] NAT の基本的な仕組みを理解している
- [ ] DNS の名前解決プロセスを説明できる
- [ ] ルーティングテーブルの読み方を理解している
- [ ] `ss` コマンドでネットワーク接続を確認できる

### ユーザー・パーミッション
- [ ] ユーザーの作成、グループへの追加ができる
- [ ] `/etc/passwd`, `/etc/shadow`, `/etc/group` の構造を理解している
- [ ] ファイルパーミッション（rwx, 755, SUID, Sticky Bit）を理解している
- [ ] `usermod -aG` の `-a` フラグの重要性を知っている
- [ ] パーミッションの8進数表記を即座に変換できる

### シェルスクリプト
- [ ] シェルスクリプトの基本構文（変数、条件分岐、ループ、コマンド置換）を理解している
- [ ] `awk` で基本的なテキスト処理ができる
- [ ] `sed` で基本的なテキスト置換ができる
- [ ] パイプを使った複数コマンドの組み合わせができる
- [ ] monitoring.sh の各行が何をしているか説明できる

### プロセス管理
- [ ] `fork()` と `exec()` の動作を理解している
- [ ] シグナル（SIGTERM, SIGKILL, SIGHUP）の役割を知っている
- [ ] `ps aux` の出力を読み取れる
- [ ] プロセスの状態（S, R, D, Z, T）を理解している

### サービス管理
- [ ] `systemctl` の主要サブコマンド（start, stop, enable, status, etc.）を使える
- [ ] `journalctl` でログを確認できる
- [ ] unit ファイルの基本構造（[Unit], [Service], [Install]）を理解している
- [ ] サービスの依存関係を確認できる

---

## 7. 追加の実践演習と知識

### 7.1 正規表現 (Regular Expression) の基礎

grep, sed, awk で使用する正規表現の基本:

```
基本的な正規表現:

  .       任意の1文字
  *       直前の文字の0回以上の繰り返し
  ^       行頭
  $       行末
  []      文字クラス（いずれか1文字）
  [^]     否定文字クラス
  \       エスケープ
  \b      単語境界

拡張正規表現 (grep -E):
  +       直前の文字の1回以上の繰り返し
  ?       直前の文字の0回または1回
  |       OR（選択）
  ()      グループ化
  {n}     n回の繰り返し
  {n,m}   n回以上m回以下の繰り返し

実践例:

# 空行以外の行を表示
grep -v "^$" /etc/ssh/sshd_config

# コメント行以外を表示
grep -v "^#" /etc/ssh/sshd_config

# コメントと空行以外を表示
grep -Ev "^#|^$" /etc/ssh/sshd_config

# IP アドレスのパターン
grep -E "[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}" file

# Port で始まる行
grep "^Port" /etc/ssh/sshd_config

# root を含む行（単語単位）
grep -w "root" /etc/passwd

# 大文字小文字無視で検索
grep -i "error" /var/log/syslog
```

### 7.2 ディスク管理コマンド

```bash
# ディスク使用量の確認
df -h
# 出力例:
# Filesystem                  Size  Used Avail Use% Mounted on
# /dev/mapper/LVMGroup-root   9.8G  2.5G  6.8G  27% /
# /dev/sda1                   461M   84M  349M  20% /boot
# /dev/mapper/LVMGroup-home   4.8G  200M  4.4G   5% /home

# inode 使用量
df -i

# ディレクトリのサイズ
du -sh /var/log/
du -sh /var/log/* | sort -rh | head -10

# ブロックデバイス一覧（monitoring.sh で LVM 確認に使用）
lsblk
# 出力例:
# NAME                    MAJ:MIN RM  SIZE RO TYPE  MOUNTPOINT
# sda                       8:0    0   20G  0 disk
# ├─sda1                    8:1    0  487M  0 part  /boot
# └─sda2                    8:2    0    1K  0 part
#   └─sda5                  8:5    0 19.5G  0 part
#     └─sda5_crypt        254:0    0 19.5G  0 crypt
#       ├─LVMGroup-root   254:1    0  9.8G  0 lvm   /
#       ├─LVMGroup-swap   254:2    0  2.3G  0 lvm   [SWAP]
#       └─LVMGroup-home   254:3    0  4.8G  0 lvm   /home

# LVM の確認コマンド
sudo pvs       # Physical Volume の簡易表示
sudo vgs       # Volume Group の簡易表示
sudo lvs       # Logical Volume の簡易表示

# マウント情報の確認
mount | column -t
cat /etc/fstab

# /etc/fstab の構造:
# device        mount_point  type  options        dump  pass
# /dev/mapper/LVMGroup-root  /  ext4  errors=remount-ro  0  1
# /dev/mapper/LVMGroup-home  /home  ext4  defaults  0  2
# /dev/mapper/LVMGroup-swap  none   swap  sw        0  0
# /dev/sda1                  /boot  ext4  defaults  0  2

# 各フィールド:
# device:      デバイスファイルまたは UUID
# mount_point: マウントポイント
# type:        ファイルシステムの種類
# options:     マウントオプション
#   defaults:  rw,suid,dev,exec,auto,nouser,async
#   noexec:    実行禁止（/tmp に設定推奨）
#   nosuid:    SUID 無効（セキュリティ強化）
#   nodev:     デバイスファイル無効
# dump:        バックアップフラグ（0=無効）
# pass:        fsck の実行順序（0=無効, 1=ルート, 2=その他）
```

### 7.3 システム情報の取得

```bash
# OS 情報
cat /etc/os-release
# 出力例:
# PRETTY_NAME="Debian GNU/Linux 12 (bookworm)"
# NAME="Debian GNU/Linux"
# VERSION_ID="12"
# VERSION="12 (bookworm)"
# ID=debian

# カーネル情報
uname -a
# 出力: Linux kaztakam42 5.10.0-XX-amd64 #1 SMP Debian ... x86_64 GNU/Linux

uname -r    # カーネルバージョンのみ
uname -m    # アーキテクチャのみ (x86_64)
uname -n    # ホスト名のみ

# ホスト名の確認・変更
hostname
hostnamectl
# 出力:
#    Static hostname: kaztakam42
#          Icon name: computer-vm
#            Chassis: vm
#         Machine ID: xxxx
#            Boot ID: yyyy
#     Virtualization: oracle
#   Operating System: Debian GNU/Linux 12 (bookworm)
#             Kernel: Linux 5.10.0-XX-amd64
#       Architecture: x86-64

# ホスト名の変更
sudo hostnamectl set-hostname kaztakam42
# /etc/hosts も更新する必要がある:
sudo nano /etc/hosts
# 127.0.1.1    kaztakam42

# 稼働時間
uptime
# 出力: 10:30:00 up 2:30, 1 user, load average: 0.00, 0.01, 0.05

# 最終起動時刻
who -b
# 出力:          system boot  2024-01-15 08:00

# メモリ情報
free -h
# 出力:
#               total        used        free      shared  buff/cache   available
# Mem:          460Mi       143Mi        98Mi       5.0Mi       218Mi       300Mi
# Swap:         952Mi          0B       952Mi

# CPU 情報
lscpu
# 出力:
# Architecture:          x86_64
# CPU(s):                1
# Thread(s) per core:    1
# Core(s) per socket:    1
# Socket(s):             1
# Model name:            Intel(R) Core(TM) i7-8550U CPU @ 1.80GHz
```

### 7.4 パッケージ管理の詳細

```bash
# パッケージの検索
apt search openssh
apt list --installed | grep ssh

# パッケージの詳細情報
apt show openssh-server
# 出力:
# Package: openssh-server
# Version: 1:9.2p1-2+deb12u1
# Priority: optional
# Section: net
# Maintainer: Debian OpenSSH Maintainers
# Depends: ...
# Description: secure shell (SSH) server

# パッケージが提供するファイルの一覧
dpkg -L openssh-server
# 出力:
# /etc/ssh/sshd_config
# /usr/sbin/sshd
# /lib/systemd/system/ssh.service
# ...

# ファイルが属するパッケージを調べる
dpkg -S /usr/sbin/sshd
# 出力: openssh-server: /usr/sbin/sshd

# インストール済みパッケージの一覧
dpkg -l
dpkg -l | grep -i ssh

# パッケージのインストール状態
dpkg -l openssh-server
# 出力:
# Desired=Unknown/Install/Remove/Purge/Hold
# | Status=Not/Inst/Conf-files/Unpacked/halF-conf/Half-inst/trig-aWait/Trig-pend
# |/ Err?=(none)/Reinst-required (Status,Err: uppercase=bad)
# ||/ Name           Version      Architecture Description
# +++-==============-============-============-============================
# ii  openssh-server 1:9.2p1-2    amd64        secure shell (SSH) server
#  ↑↑
#  ii = Installed, OK

# パッケージのアップデート手順
sudo apt update            # パッケージリストの更新
sudo apt list --upgradable # アップグレード可能なパッケージの確認
sudo apt upgrade           # パッケージのアップグレード
sudo apt autoremove        # 不要パッケージの削除
sudo apt clean             # ダウンロードキャッシュの削除
```

### 7.5 ログファイルの管理

```bash
# 主要なログファイル
/var/log/syslog      # システム全般のログ
/var/log/auth.log    # 認証関連（SSH ログイン試行等）
/var/log/kern.log    # カーネルメッセージ
/var/log/dpkg.log    # パッケージ管理
/var/log/boot.log    # ブートメッセージ
/var/log/cron.log    # cron ジョブ（設定による）
/var/log/ufw.log     # UFW ファイアウォール

# リアルタイムログ監視
tail -f /var/log/auth.log

# ログの検索
grep "Failed password" /var/log/auth.log
grep "Accepted password" /var/log/auth.log

# ログローテーション
cat /etc/logrotate.conf
ls /etc/logrotate.d/

# journalctl（systemd ジャーナル）
journalctl -u ssh --since "1 hour ago"
journalctl -p err --since today
journalctl --disk-usage
```

### 7.6 トラブルシューティングの手順

Born2beRoot で問題が発生した場合の診断手順:

```
問題: SSH で接続できない
─────────────────────────────
Step 1: sshd が起動しているか？
  $ sudo systemctl status sshd
  → Active: inactive → sudo systemctl start sshd

Step 2: port 4242 で LISTEN しているか？
  $ ss -tunlp | grep 4242
  → 表示なし → /etc/ssh/sshd_config の Port 設定を確認

Step 3: UFW が port 4242 を許可しているか？
  $ sudo ufw status
  → 4242 が表示されない → sudo ufw allow 4242

Step 4: ポートフォワーディングが設定されているか？
  VirtualBox の設定 → ネットワーク → ポートフォワーディング

Step 5: SSH の設定にエラーがないか？
  $ sudo sshd -t
  → エラーメッセージ → sshd_config を修正

Step 6: ログを確認
  $ journalctl -u sshd -n 20
  $ cat /var/log/auth.log | tail -20

問題: sudo が使えない
─────────────────────────────
Step 1: ユーザーが sudo グループに所属しているか？
  $ groups username
  → sudo が含まれていない → root でログインして usermod -aG sudo username

Step 2: sudoers ファイルに構文エラーがないか？
  $ sudo visudo -c
  → エラー → visudo で修正

Step 3: sudo ログを確認
  $ cat /var/log/sudo/sudo.log

問題: パスワードが設定できない（ポリシー違反）
─────────────────────────────
pam_pwquality のエラーメッセージを確認:
  "is too short"      → 10文字以上必要
  "not enough uppercase" → 大文字が必要
  "not enough digits"  → 数字が必要
  "contains the user name" → ユーザー名を含まない
  "too many same characters" → 連続同一文字3文字以下
```

### 7.7 セキュリティの基本概念

```
多層防御 (Defense in Depth):

Born2beRoot で実装する多層防御:

┌──────────────────────────────────────────┐
│  Layer 6: 監視 (Monitoring)              │
│  monitoring.sh + cron                    │
│  不正な活動の早期発見                     │
├──────────────────────────────────────────┤
│  Layer 5: ログと監査 (Logging/Auditing)  │
│  sudo log, auth.log, journald           │
│  事後追跡（フォレンジック）              │
├──────────────────────────────────────────┤
│  Layer 4: アクセス制御 (Access Control)  │
│  AppArmor (MAC)                          │
│  プロセスレベルの制限                    │
├──────────────────────────────────────────┤
│  Layer 3: 権限管理 (Authorization)       │
│  sudo, file permissions                  │
│  最小権限の原則                          │
├──────────────────────────────────────────┤
│  Layer 2: 認証 (Authentication)          │
│  パスワードポリシー (PAM)                │
│  SSH ログイン制限                        │
├──────────────────────────────────────────┤
│  Layer 1: ネットワーク (Network)         │
│  UFW ファイアウォール                    │
│  port 4242 のみ許可                     │
├──────────────────────────────────────────┤
│  Layer 0: 暗号化 (Encryption)            │
│  LUKS ディスク暗号化                     │
│  物理アクセスからの保護                  │
└──────────────────────────────────────────┘

各層が独立して機能するため、
1つの層が突破されても他の層が防御を提供する。
```

### 7.8 よく使うキーボードショートカット

ターミナル操作で効率を上げるためのショートカット:

```
Bash のキーボードショートカット:

カーソル移動:
  Ctrl+A    行頭に移動
  Ctrl+E    行末に移動
  Ctrl+B    1文字後退（←キーと同じ）
  Ctrl+F    1文字前進（→キーと同じ）
  Alt+B     1単語後退
  Alt+F     1単語前進

編集:
  Ctrl+U    カーソルから行頭まで削除
  Ctrl+K    カーソルから行末まで削除
  Ctrl+W    カーソルの前の1単語を削除
  Ctrl+Y    最後に削除した内容を貼り付け
  Ctrl+L    画面クリア（clear コマンドと同等）

履歴:
  Ctrl+R    履歴の逆方向検索
  Ctrl+P    前のコマンド（↑キーと同じ）
  Ctrl+N    次のコマンド（↓キーと同じ）
  !!        直前のコマンドを再実行
  !n        履歴番号 n のコマンドを再実行
  !$        直前のコマンドの最後の引数

プロセス制御:
  Ctrl+C    現在のコマンドを中断 (SIGINT)
  Ctrl+Z    現在のコマンドを一時停止 (SIGTSTP)
  Ctrl+D    EOF を送信（シェルの終了）
  Ctrl+S    端末出力の一時停止
  Ctrl+Q    端末出力の再開
```

### 7.9 正規表現の基礎

monitoring.sh や日常的なテキスト処理で使用する正規表現:

```
基本正規表現 (BRE) - grep で使用:

メタ文字:
  .         任意の1文字
  *         直前の文字の0回以上の繰り返し
  ^         行頭
  $         行末
  []        文字クラス（括弧内の任意の1文字）
  [^]       否定文字クラス（括弧内以外の1文字）
  \         エスケープ（特殊文字をリテラルとして扱う）

拡張正規表現 (ERE) - grep -E / egrep で使用:

  +         直前の文字の1回以上の繰り返し
  ?         直前の文字の0回または1回
  |         OR（選択）
  ()        グループ化
  {n}       直前の文字のちょうど n 回
  {n,m}     直前の文字の n 回以上 m 回以下

実用例:
  grep "^Port" /etc/ssh/sshd_config
  # → "Port" で始まる行（Born2beRoot: SSH ポートの確認）

  grep -E "^(PASS_MAX|PASS_MIN|PASS_WARN)" /etc/login.defs
  # → パスワードポリシー関連の行を抽出

  grep -c "COMMAND" /var/log/auth.log
  # → sudo コマンドの実行回数をカウント

  grep "Failed password" /var/log/auth.log
  # → SSH ログイン失敗の記録を検索
```
