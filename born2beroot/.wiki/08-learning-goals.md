# 08 - 学習目標 (Learning Goals)

Born2beRoot プロジェクトを通じて習得すべき知識とスキルを整理する。単なるチェックリストではなく、各スキルが実務でどのように活きるか、キャリアパスとの関連性、そして段階的な成長の道筋を示す。

---

## 1. システム管理の基礎

### 1.1 OS のインストールと初期設定

**目標**: オペレーティングシステムを手動でインストールし、本番運用に向けた初期設定を行えるようになる

習得すべきスキル:
- Debian/Rocky Linux のインストールプロセスの理解
- インストーラの各選択肢（ロケール、パーティション、パッケージ）の意味の理解
- インストール直後に必要な初期設定の判断力
- ネットワーク設定の基礎
- タイムゾーン、ロケール、キーボードレイアウトの設定
- ミラーサイトの選択とリポジトリの概念

なぜ重要か:
- クラウド環境でもイメージのカスタマイズには OS レベルの知識が必要
- トラブルシューティング時に OS の基本構造を理解していることが前提となる
- 自動化する際にも、何を自動化しているかの理解が不可欠
- コンテナのベースイメージ選定にも OS の知識が必要

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
Debian インストール              → AMI/VM イメージのカスタマイズ
パーティション設定               → EBS ボリュームの設計
初期設定の手順化                 → cloud-init / user-data スクリプト
インストーラの選択肢の理解       → Packer でのイメージビルド
ミラーサイトの選択               → プライベートリポジトリ (Artifactory)
ロケール設定                    → コンテナのロケール問題の理解
```

**段階的スキルアップ**:
```
Level 1: Debian をインストールできる
  → インストーラの指示に従って最小構成で Debian をインストールする
  → 必要なパッケージグループの選択ができる
  → ネットワーク設定（DHCP/static）を行える

Level 2: パーティション設計の理由を説明できる
  → 各パーティション（/, /boot, /home, /var, /tmp）の分離理由を説明できる
  → パーティションサイズの決定基準を理解している
  → swap 領域のサイジング根拠を説明できる

Level 3: 設定手順を再現可能な手順書として文書化できる
  → 第三者が同じ結果を得られる手順書を作成できる
  → スクリーンショットと設定値の記録ができる
  → 設定変更の理由を文書化できる

Level 4: Packer/cloud-init で自動インストールスクリプトを書ける
  → preseed.cfg / kickstart を使った無人インストール
  → cloud-init の設定ファイルを作成できる
  → Packer テンプレートでカスタムイメージを作成できる

Level 5: カスタム ISO/AMI を作成して組織で共有できる
  → 組織の標準イメージを定義・管理できる
  → CIS Benchmark に準拠したベースイメージを作成できる
  → イメージのライフサイクル管理（作成、テスト、公開、廃止）を設計できる
```

**インストール時の重要な判断ポイント**:

| 判断ポイント | 選択肢 | 推奨と理由 |
|------------|--------|-----------|
| ロケール | ja_JP.UTF-8 / en_US.UTF-8 | en_US.UTF-8（エラーメッセージの検索容易性） |
| パッケージ | Desktop / Server / Minimal | Minimal（Born2beRoot は GUI 不要） |
| ブートローダー | GRUB / systemd-boot | GRUB（Born2beRoot の標準） |
| ファイルシステム | ext4 / XFS / btrfs | ext4（安定性、広いサポート） |
| init システム | systemd / SysVinit | systemd（Debian 8+ のデフォルト） |
| カーネル | linux / linux-lts | 通常 linux（VirtualBox 互換性） |

**Debian と Rocky Linux の比較詳細**:

| 項目 | Debian | Rocky Linux |
|------|--------|-------------|
| ベース | 独立したディストリビューション | RHEL クローン |
| パッケージ形式 | .deb (dpkg/apt) | .rpm (rpm/dnf) |
| パッケージ数 | ~59,000+ | ~8,000+ (AppStream含む) |
| デフォルト MAC | AppArmor | SELinux |
| ファイアウォール | UFW (iptables) | firewalld (nftables) |
| サポート期間 | ~3年 (stable) | ~10年 (RHEL と同期) |
| リリースモデル | 安定性重視 | エンタープライズ互換 |
| 主な用途 | Web サーバー、開発 | エンタープライズ、金融 |
| 企業サポート | コミュニティのみ | 商用サポート (RHEL) |
| 設定ファイル配置 | /etc/default/ | /etc/sysconfig/ |

### 1.2 ユーザーとグループの管理

**目標**: Linux のユーザー・グループモデルを理解し、適切なアクセス制御を設計できるようになる

習得すべきスキル:
- ユーザーの作成・削除・変更
- グループの作成と管理
- パーミッションモデル（DAC）の理解
- `/etc/passwd`, `/etc/shadow`, `/etc/group` の構造理解
- SUID, SGID, Sticky Bit の理解
- umask の概念と設定
- ACL (Access Control List) の基礎

なぜ重要か:
- マルチユーザー環境での権限管理はセキュリティの基礎
- CI/CD パイプラインやコンテナでもユーザー権限の理解は必要
- 権限に関するバグの多くは、この基礎の理解不足が原因
- 不正アクセスの多くはユーザー管理の不備に起因する

**/etc/passwd の構造詳細**:
```
kaztakam:x:1000:1000:Kaztakam:/home/kaztakam:/bin/bash
│        │ │    │    │         │              │
│        │ │    │    │         │              └── ログインシェル
│        │ │    │    │         └── ホームディレクトリ
│        │ │    │    └── GECOS フィールド（フルネーム等）
│        │ │    └── プライマリ GID
│        │ └── UID
│        └── パスワードは /etc/shadow に格納（x は shadow 使用を示す）
└── ユーザー名
```

**/etc/shadow の構造詳細**:
```
kaztakam:$6$rounds=5000$salt$hash:19384:2:30:7:::
│        │                       │     │ │  │
│        │                       │     │ │  └── パスワード有効期限警告日数
│        │                       │     │ └── 最大パスワード日数 (PASS_MAX_DAYS)
│        │                       │     └── 最小パスワード日数 (PASS_MIN_DAYS)
│        │                       └── 最終変更日（1970/1/1 からの日数）
│        └── ハッシュ（$6$=SHA-512, salt, ハッシュ値）
└── ユーザー名
```

**特殊パーミッションの詳細**:

| パーミッション | 数値 | 効果 | 具体例 |
|-------------|------|------|--------|
| SUID | 4000 | 実行時にファイル所有者の権限で動作 | `/usr/bin/passwd` (root 所有だが一般ユーザーが実行) |
| SGID | 2000 | 実行時にグループ権限で動作 / ディレクトリ内の新規ファイルが親のグループを継承 | `/usr/bin/wall` (tty グループ) |
| Sticky Bit | 1000 | ディレクトリ内のファイルは所有者のみ削除可能 | `/tmp` (他ユーザーのファイルを削除できない) |

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
ユーザー作成・管理               → LDAP/Active Directory のユーザー管理
グループによるアクセス制御       → IAM ロール/ポリシーの設計
/etc/passwd の構造理解           → LDIF フォーマットの理解
SUID/SGID の理解                → コンテナのセキュリティコンテキスト
umask の設定                    → Docker のファイル権限問題の解決
ACL の基礎                     → POSIX ACL / NFSv4 ACL
```

**段階的スキルアップ**:
```
Level 1: adduser/usermod でユーザーを管理できる
  → ユーザーの作成、パスワード設定、グループ追加ができる
  → id コマンドでユーザー情報を確認できる
  → /etc/passwd, /etc/group を読み取れる

Level 2: パーミッションの数値表記を即座に読み書きできる
  → 755, 644, 600 等の意味を瞬時に判断できる
  → ls -la の出力を読み解ける
  → chmod, chown を適切に使い分けられる

Level 3: 組織の要件に基づいてグループ設計ができる
  → 開発チーム、運用チーム等のグループ構成を設計できる
  → 共有ディレクトリの権限設計ができる
  → SGID + umask を使った共同作業環境を構築できる

Level 4: LDAP/SSO と連携したユーザー管理を構築できる
  → PAM + LDAP でリモート認証を設定できる
  → nsswitch.conf を理解している
  → Kerberos チケット認証を設定できる

Level 5: RBAC (Role-Based Access Control) を設計・実装できる
  → 組織全体のアクセス制御ポリシーを設計できる
  → 監査とコンプライアンス要件を満たすシステムを構築できる
  → IdP (Identity Provider) の設計と運用ができる
```

### 1.3 サービスの管理

**目標**: systemd を使ってサービスの起動・停止・自動起動を管理できるようになる

習得すべきスキル:
- `systemctl` の基本操作（start, stop, restart, enable, disable, status）
- サービスの状態確認とトラブルシューティング
- `journalctl` によるログの確認
- サービスの依存関係の理解
- ユニットファイルの構造理解
- タイマーユニット（cron の代替）の理解
- socket activation の概念

なぜ重要か:
- サーバー上で動作するすべてのアプリケーションはサービスとして管理される
- 障害対応の第一歩は「サービスの状態確認」
- Docker や Kubernetes でも、コンテナ内のプロセス管理の概念は同じ
- サービスの起動順序の誤りはシステム障害の一般的な原因

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
systemctl での管理               → Kubernetes の Pod 管理
サービスの依存関係               → マイクロサービスのヘルスチェック
journalctl でのログ確認          → CloudWatch Logs / ELK Stack
サービスの自動起動設定           → supervisord / init container
ユニットファイルの理解           → Kubernetes manifest の理解
タイマーユニット                → Kubernetes CronJob
```

**段階的スキルアップ**:
```
Level 1: systemctl で基本操作ができる
  → start, stop, restart, status を使い分けられる
  → enable, disable で自動起動を制御できる
  → is-active, is-enabled で状態を確認できる

Level 2: journalctl で問題を特定できる
  → -u でサービス指定、-f でリアルタイム監視ができる
  → -b でブート以降のログを確認できる
  → --since / --until で時間範囲指定ができる
  → -p でログレベルフィルタリングができる

Level 3: カスタムユニットファイルを作成できる
  → [Unit], [Service], [Install] セクションを記述できる
  → After/Requires/Wants で依存関係を定義できる
  → Restart ポリシーを適切に設定できる

Level 4: サービスの依存関係を設計できる
  → 複数サービスの起動順序を設計できる
  → socket activation を使った遅延起動を実装できる
  → systemd-analyze で起動パフォーマンスを分析できる

Level 5: 複雑なサービスのオーケストレーションを構築できる
  → systemd-nspawn / machined でコンテナ管理ができる
  → networkd / resolved / timesyncd を設計できる
  → 大規模サービスの依存関係グラフを設計できる
```

**systemd の操作早見表**:

| 操作 | コマンド | 用途 |
|------|---------|------|
| サービス開始 | `systemctl start sshd` | 即座に起動 |
| サービス停止 | `systemctl stop sshd` | 即座に停止 |
| サービス再起動 | `systemctl restart sshd` | 設定変更後の反映 |
| 設定リロード | `systemctl reload sshd` | 接続を切断せずに設定反映 |
| 自動起動有効 | `systemctl enable sshd` | 次回ブート時に自動起動 |
| 自動起動無効 | `systemctl disable sshd` | 自動起動を停止 |
| 状態確認 | `systemctl status sshd` | 動作状態の詳細表示 |
| 全サービス一覧 | `systemctl list-units --type=service` | 動作中のサービス一覧 |
| 起動失敗一覧 | `systemctl --failed` | 失敗したサービスの確認 |
| ログ確認 | `journalctl -u sshd` | 特定サービスのログ |
| リアルタイムログ | `journalctl -u sshd -f` | ログのリアルタイム監視 |
| ブート以降のログ | `journalctl -b -u sshd` | 今回のブート以降のログ |
| ブート分析 | `systemd-analyze blame` | 各サービスの起動時間 |
| 依存関係表示 | `systemctl list-dependencies sshd` | 依存関係ツリー |

**ユニットファイルの構造例**:

```ini
# /etc/systemd/system/monitoring.service
[Unit]
Description=Born2beRoot Monitoring Script
After=network.target          # ネットワーク起動後に実行
Wants=network-online.target   # ネットワーク接続を希望

[Service]
Type=oneshot                  # 実行完了後にプロセスが終了するタイプ
ExecStart=/usr/local/bin/monitoring.sh
StandardOutput=journal        # 標準出力を journalctl に送る

[Install]
WantedBy=multi-user.target    # multi-user.target で起動
```

### 1.4 ネットワークの基礎

**目標**: Linux のネットワーク設定を理解し、基本的なネットワークトラブルシューティングを行えるようになる

習得すべきスキル:
- IP アドレスの確認と設定（ip コマンド）
- DNS の基礎（/etc/resolv.conf, /etc/hosts）
- ルーティングの基礎（ip route）
- ネットワーク接続の確認（ping, ss, curl）
- VirtualBox のネットワークモード（NAT, ブリッジ, ホストオンリー）

なぜ重要か:
- サーバー間通信の問題診断にはネットワークの知識が不可欠
- クラウド環境でも VPC, サブネット, ルーティングの設計が必要
- SSH 接続の問題の多くはネットワーク設定に起因する

**VirtualBox のネットワークモード比較**:

| モード | VM→外部 | 外部→VM | VM間 | ホスト→VM | Born2beRoot での使用 |
|--------|---------|---------|------|----------|-------------------|
| NAT | ○ | × (ポートフォワード可) | × | × (ポートフォワード可) | 基本構成 |
| ブリッジ | ○ | ○ | ○ | ○ | IP 直接割当 |
| ホストオンリー | × | × | ○ | ○ | ホストとの通信のみ |
| 内部ネットワーク | × | × | ○ | × | VM 間のみ |

**ネットワーク診断フローチャート**:

```
SSH 接続できない
  │
  ├── ping が通らない
  │     ├── VM が起動しているか → VirtualBox で確認
  │     ├── IP アドレスは正しいか → ip addr show
  │     ├── ネットワークモードは正しいか → VirtualBox 設定
  │     └── ポートフォワーディングは設定されているか
  │
  ├── ping は通るが SSH 接続できない
  │     ├── sshd が起動しているか → systemctl status sshd
  │     ├── ポートは正しいか → ss -tunlp | grep sshd
  │     ├── UFW でポートが許可されているか → ufw status
  │     └── sshd_config の設定は正しいか → 設定ファイル確認
  │
  └── SSH 接続できるが認証失敗
        ├── パスワードは正しいか
        ├── ユーザーは存在するか → id username
        ├── PermitRootLogin の設定 → root でログインしようとしていないか
        └── AllowUsers/DenyUsers の制限 → sshd_config 確認
```

**段階的スキルアップ**:
```
Level 1: ip addr, ip route の出力を読み取れる
Level 2: ネットワーク接続問題の基本的な切り分けができる
Level 3: VPN, VLAN の概念を理解している
Level 4: AWS VPC / サブネット / ルーティングテーブルを設計できる
Level 5: SDN (Software Defined Networking) を設計・運用できる
```

---

## 2. セキュリティハードニングの概念

### 2.1 ネットワークセキュリティ

**目標**: ファイアウォールの概念と設定を理解し、必要最小限のネットワークアクセスを設計できるようになる

習得すべきスキル:
- TCP/IP の基礎（IP アドレス、ポート番号、プロトコル）
- ファイアウォールの動作原理（Netfilter, iptables, UFW）
- ルールの設計（デフォルト deny + 明示的な allow）
- ポートスキャンの概念と防御
- NAT (Network Address Translation) の仕組み
- TCP 3-way handshake の理解
- 各プロトコル（TCP, UDP, ICMP）の特性

なぜ重要か:
- ネットワークはサーバーへの主要な攻撃経路
- クラウド環境でもセキュリティグループ/ネットワークポリシーの設計が必要
- マイクロサービスアーキテクチャでは、サービス間の通信制御が重要
- ゼロトラストネットワークの基礎知識として不可欠

**Netfilter / iptables / UFW の階層関係**:

```
UFW (Uncomplicated Firewall)     ← ユーザーフレンドリーなインターフェース
  │                                 Born2beRoot で操作する層
  ▼
iptables                          ← ルール管理ツール
  │                                 UFW が内部で生成するルール
  ▼
Netfilter                         ← Linux カーネルのパケットフィルタリング
  │                                 実際にパケットを処理する層
  ▼
nftables                          ← iptables の後継（新しいカーネル）
                                    将来的な移行先
```

**iptables のチェーン詳細**:

```
パケット到着
  │
  ▼
[PREROUTING]  ← NAT (DNAT) のルール
  │
  ├── 自分宛？ ──Yes──→ [INPUT]  ← 受信パケットのフィルタリング
  │                       │          Born2beRoot の UFW ルールが主にここに適用
  │                       ▼
  │                     ローカルプロセス
  │                       │
  │                     [OUTPUT] ← 送信パケットのフィルタリング
  │                       │
  │                       ▼
  └── 転送？ ──Yes──→ [FORWARD] ← 転送パケットのフィルタリング
                          │         ルーターとして動作する場合に使用
                          ▼
                     [POSTROUTING] ← NAT (SNAT/MASQUERADE) のルール
                          │
                          ▼
                     パケット送出
```

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
UFW のルール設計                 → AWS Security Group / Azure NSG
デフォルト deny ポリシー         → ゼロトラストネットワーク設計
ポートの開閉管理                 → Kubernetes NetworkPolicy
NAT の理解                      → VPC NAT Gateway の設計
iptables の概念                  → Calico / Cilium (CNI) の理解
TCP 3-way handshake              → SYN flood 攻撃の理解と防御
ステートフルファイアウォール     → AWS SG (stateful) vs NACL (stateless)
```

**段階的スキルアップ**:
```
Level 1: UFW で基本的なルールを設定できる
  → ufw allow/deny でポートを制御できる
  → ufw status で現在のルールを確認できる
  → デフォルトポリシーの設定ができる

Level 2: iptables のチェーン（INPUT/OUTPUT/FORWARD）を理解している
  → iptables -L で現在のルールを読み取れる
  → UFW が生成するルールの意味を理解できる
  → ステートフル vs ステートレスの違いを説明できる

Level 3: ネットワーク設計図を描いてファイアウォールルールを導出できる
  → 要件からセキュリティグループのルールを設計できる
  → サブネット間のトラフィックフローを図示できる
  → NAT/PAT の設計ができる

Level 4: クラウドのセキュリティグループを要件に基づいて設計できる
  → AWS/Azure/GCP のネットワークセキュリティを設計できる
  → VPC ピアリング / Transit Gateway を理解している
  → PrivateLink / VPC Endpoint を設計できる

Level 5: サービスメッシュ/ゼロトラストアーキテクチャを設計できる
  → Istio / Linkerd のセキュリティポリシーを設計できる
  → mTLS (mutual TLS) を実装できる
  → BeyondCorp モデルを理解し適用できる
```

### 2.2 認証とアクセス制御

**目標**: 認証の仕組みを理解し、強固な認証ポリシーを設計・実装できるようになる

習得すべきスキル:
- パスワード認証の仕組み（ハッシュ、salt、ストレッチング）
- パスワードポリシーの設計原則
- SSH の認証方式（パスワード vs 公開鍵）
- PAM (Pluggable Authentication Modules) の概念
- sudo の設定と監査
- 多要素認証 (MFA) の概念
- OAuth / OIDC の基礎概念

なぜ重要か:
- 弱い認証は最も一般的な攻撃ベクター
- 多要素認証 (MFA) の理解にも基礎が必要
- 実務では LDAP, SAML, OAuth などの認証基盤と連携する
- パスワードスプレー、クレデンシャルスタッフィング等の攻撃を理解するために必要

**パスワード認証の深い理解**:

```
平文パスワード
    ↓
salt（ランダム値）を付加
    ↓
ハッシュ関数（SHA-512）
    ↓
ストレッチング（複数回ハッシュ: 5000 rounds デフォルト）
    ↓
$6$salt$hash 形式で /etc/shadow に保存

認証時:
入力パスワード + 保存された salt → 同じハッシュ計算 → 保存ハッシュと比較
```

**ハッシュアルゴリズムの比較**:

| アルゴリズム | /etc/shadow の識別子 | 強度 | 用途 |
|------------|---------------------|------|------|
| MD5 | $1$ | 非推奨（脆弱） | レガシーシステム |
| SHA-256 | $5$ | 中 | 一般用途 |
| SHA-512 | $6$ | 高 | Debian デフォルト |
| bcrypt | $2b$ | 高 | Web アプリケーション |
| Argon2 | $argon2id$ | 最高 | 最新の推奨 |
| yescrypt | $y$ | 高 | 新しい Linux ディストリビューション |

**PAM の詳細アーキテクチャ**:

```
アプリケーション（login, sshd, sudo）
    │
    ▼
PAM ライブラリ (/lib/security/)
    │
    ├── auth（認証）
    │     ├── pam_unix.so      → パスワード検証
    │     ├── pam_deny.so      → 常に拒否
    │     └── pam_permit.so    → 常に許可
    │
    ├── account（アカウント管理）
    │     ├── pam_unix.so      → アカウント有効期限チェック
    │     └── pam_nologin.so   → /etc/nologin 存在時にログイン拒否
    │
    ├── password（パスワード変更）
    │     ├── pam_pwquality.so → パスワード強度検証 ← Born2beRoot
    │     └── pam_unix.so      → パスワードの実際の変更
    │
    └── session（セッション管理）
          ├── pam_unix.so      → ログの記録
          ├── pam_limits.so    → リソース制限
          └── pam_env.so       → 環境変数の設定
```

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
パスワードハッシュの理解         → bcrypt/Argon2 の選定判断
PAM の設定                      → LDAP/SAML 連携の理解
SSH 認証                        → Certificate-based 認証
sudo の監査ログ                 → SIEM (Splunk, ELK) との連携
パスワードポリシー設計           → IdP (Okta, Azure AD) のポリシー設計
MFA の概念理解                  → TOTP/FIDO2 の実装
```

**段階的スキルアップ**:
```
Level 1: パスワードポリシーを設定できる
  → login.defs の PASS_MAX_DAYS 等を設定できる
  → pam_pwquality の各パラメータを設定できる
  → chage でユーザーごとのポリシーを確認・設定できる

Level 2: SSH 公開鍵認証を設定できる
  → ssh-keygen で鍵ペアを生成できる
  → authorized_keys ファイルを正しく設定できる
  → sshd_config でパスワード認証を無効化できる

Level 3: PAM モジュールを組み合わせて認証フローを構築できる
  → PAM の制御フラグ（required, requisite, sufficient, optional）を理解している
  → カスタム PAM 設定を作成できる
  → PAM デバッグの方法を知っている

Level 4: LDAP/SAML を使った SSO を構築できる
  → OpenLDAP / FreeIPA を設定できる
  → SAML IdP / SP の連携を設定できる
  → OIDC プロバイダを理解している

Level 5: ゼロトラスト認証アーキテクチャを設計できる
  → 継続的認証の仕組みを設計できる
  → デバイス認証 + ユーザー認証の組み合わせを実装できる
  → コンディショナルアクセスポリシーを設計できる
```

### 2.3 暗号化

**目標**: ディスク暗号化の概念と実装を理解し、保存データの保護を実現できるようになる

習得すべきスキル:
- 暗号化の基礎概念（対称暗号、鍵導出関数）
- LUKS の設定と管理
- AES-256-XTS の基本的な理解
- 暗号化が保護するシナリオと保護しないシナリオの理解
- 暗号化のパフォーマンスへの影響の理解
- 鍵管理のベストプラクティス

なぜ重要か:
- データ保護規制（GDPR, HIPAA 等）が保存データの暗号化を要求
- クラウド環境でもディスク暗号化は標準的なプラクティス
- TLS/SSL の理解にも暗号化の基礎知識が必要
- 暗号化の限界を理解しないと、誤ったセキュリティ設計を行う

**LUKS の内部構造**:

```
┌──────────────────────────────────────────────┐
│              LUKS ヘッダー                      │
│  ├── マジックナンバー ("LUKS\xba\xbe")         │
│  ├── バージョン (1 or 2)                       │
│  ├── 暗号アルゴリズム (aes-xts-plain64)        │
│  ├── ハッシュ関数 (sha256)                     │
│  ├── ペイロードオフセット                       │
│  ├── マスターキーの長さ (512 bits)              │
│  ├── マスターキーの salt                       │
│  ├── PBKDF2 イテレーション回数                 │
│  └── キースロット (0-7)                        │
│       ├── スロット 0: パスフレーズ 1 の暗号化鍵 │
│       ├── スロット 1: パスフレーズ 2 の暗号化鍵 │
│       └── スロット 2-7: 未使用                 │
├──────────────────────────────────────────────┤
│              暗号化データ領域                    │
│  AES-256-XTS で暗号化されたデータ              │
└──────────────────────────────────────────────┘
```

**暗号化の種類と Born2beRoot での使用箇所**:

| 暗号化の種類 | Born2beRoot での使用 | 保護対象 | 鍵の管理場所 |
|-------------|---------------------|---------|-------------|
| ディスク暗号化 (LUKS) | パーティション全体 | 保存データ (Data at Rest) | LUKS ヘッダー |
| 通信暗号化 (SSH) | リモートアクセス | 通信データ (Data in Transit) | ~/.ssh/ |
| ハッシュ (SHA-512) | パスワード保存 | 認証情報 | /etc/shadow |
| 鍵導出 (PBKDF2) | LUKS パスフレーズ | 暗号鍵 | LUKS ヘッダー |

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
LUKS ディスク暗号化              → AWS EBS 暗号化 / Azure Disk Encryption
AES-256 の概念                  → KMS (Key Management Service) の理解
鍵管理の基礎                    → HashiCorp Vault / AWS KMS
暗号化の限界の理解               → セキュリティアーキテクチャの設計
TLS/SSH の暗号化                → Let's Encrypt / Certificate Manager
PBKDF2 の理解                   → パスワードストレッチングの設計
```

**段階的スキルアップ**:
```
Level 1: LUKS でディスク暗号化を設定できる
Level 2: 暗号化が何を保護し、何を保護しないかを説明できる
Level 3: TLS 証明書を取得・設定できる
Level 4: KMS を使った鍵管理を設計できる
Level 5: エンドツーエンド暗号化アーキテクチャを設計できる
```

### 2.4 Mandatory Access Control

**目標**: AppArmor/SELinux の概念を理解し、プロセスレベルのアクセス制御の必要性を認識できるようになる

習得すべきスキル:
- DAC vs MAC の違い
- AppArmor のプロファイル概念
- enforce vs complain モード
- セキュリティモジュールの状態確認
- LSM (Linux Security Modules) フレームワークの概念
- プロファイルの作成と管理

なぜ重要か:
- コンテナのセキュリティ（seccomp, AppArmor プロファイル）に直結
- ゼロトラストセキュリティの一要素
- コンプライアンス要件で MAC の有効化が求められることがある
- 特権エスカレーション攻撃の最終防御線

**セキュリティモデルの階層**:

```
Layer 4: AppArmor (MAC)     ← Born2beRoot で学ぶ
  プロセスごとのアクセス制御。DAC を突破されても防御する最後の砦

Layer 3: sudo (権限昇格制御)  ← Born2beRoot で学ぶ
  権限昇格の制御と監査

Layer 2: DAC (パーミッション)  ← Born2beRoot で学ぶ
  ファイルの所有者が設定するアクセス権限

Layer 1: 認証
  パスワード/鍵によるユーザーの確認

Layer 0: ネットワーク (UFW)   ← Born2beRoot で学ぶ
  通信レベルでのアクセス制御
```

**AppArmor と SELinux の詳細比較**:

| 項目 | AppArmor | SELinux |
|------|----------|---------|
| アプローチ | パスベース（ファイルパスで制御） | ラベルベース（セキュリティコンテキストで制御） |
| 学習コスト | 低い（プロファイルが直感的） | 高い（ポリシー言語が複雑） |
| 柔軟性 | 中（パスに依存） | 高い（ラベルは柔軟） |
| デフォルト | Debian, Ubuntu, SUSE | RHEL, CentOS, Fedora |
| プロファイル作成 | aa-genprof で自動生成可能 | audit2allow で補助 |
| 移植性 | 低い（パス依存のため環境で変わる） | 高い（ラベルは環境に依存しない） |
| 管理ツール | aa-status, aa-enforce, aa-complain | semanage, setsebool, getenforce |
| Kubernetes | Docker/containerd でサポート | Pod Security Admission でサポート |

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
AppArmor プロファイル            → Docker の seccomp プロファイル
enforce/complain モード          → Kubernetes の PodSecurityPolicy
DAC vs MAC の概念               → AWS IAM の Boundary Policy
プロセスレベルの制御             → eBPF ベースのセキュリティ (Falco)
プロファイルの作成               → カスタムセキュリティポリシーの設計
LSM フレームワーク               → Linux セキュリティ全般の理解
```

**段階的スキルアップ**:
```
Level 1: AppArmor の状態を確認できる
Level 2: enforce と complain の違いを説明できる
Level 3: 新しいプロファイルを作成できる
Level 4: コンテナの seccomp/AppArmor プロファイルを設計できる
Level 5: 包括的な MAC ポリシーを組織全体に展開できる
```

---

## 3. Linux 内部の理解

### 3.1 ストレージ管理

**目標**: Linux のストレージスタックを理解し、柔軟なストレージ構成を設計できるようになる

習得すべきスキル:
- パーティションテーブル（MBR, GPT）
- ファイルシステム（ext4, XFS 等）
- LVM の3層構造と操作（拡張・縮小・スナップショット）
- マウントポイントと `/etc/fstab`
- `df`, `lsblk`, `fdisk`, `lvdisplay` の使用
- inode, superblock, block の概念
- RAID の概念と種類

なぜ重要か:
- ストレージの容量不足はサーバー障害の最も一般的な原因の一つ
- クラウドでもディスクの追加・拡張は日常的な作業
- データベースやログの配置にストレージ設計の知識が必要
- パフォーマンスチューニングにファイルシステムの知識が必要

**ストレージスタック全体像**:

```
┌─────────────────────────────┐
│        アプリケーション        │
│    (ファイルの読み書き)        │
├─────────────────────────────┤
│        VFS (Virtual FS)       │  ← カーネルのファイルシステム抽象化
├─────────────────────────────┤
│    ext4 / XFS / btrfs         │  ← ファイルシステム
├─────────────────────────────┤
│    LVM (Logical Volume)       │  ← 論理ボリューム管理  ← Born2beRoot
├─────────────────────────────┤
│    dm-crypt (LUKS)            │  ← ディスク暗号化      ← Born2beRoot
├─────────────────────────────┤
│    パーティション (MBR/GPT)   │  ← ディスク分割        ← Born2beRoot
├─────────────────────────────┤
│    ブロックデバイス (/dev/sda) │  ← 物理/仮想ディスク
└─────────────────────────────┘
```

**LVM の操作一覧**:

| 操作 | PV | VG | LV |
|------|----|----|-----|
| 作成 | pvcreate | vgcreate | lvcreate |
| 表示 | pvdisplay | vgdisplay | lvdisplay |
| 拡張 | - | vgextend | lvextend |
| 縮小 | - | vgreduce | lvreduce |
| 削除 | pvremove | vgremove | lvremove |
| スキャン | pvscan | vgscan | lvscan |
| 一覧 | pvs | vgs | lvs |

**ファイルシステムの比較**:

| 項目 | ext4 | XFS | btrfs | ZFS |
|------|------|-----|-------|-----|
| 最大ファイルサイズ | 16 TB | 8 EB | 16 EB | 16 EB |
| 最大FS サイズ | 1 EB | 8 EB | 16 EB | 256 ZB |
| スナップショット | × (LVM で代替) | × (LVM で代替) | ○ (ネイティブ) | ○ (ネイティブ) |
| 圧縮 | × | × | ○ | ○ |
| チェックサム | × (メタデータのみ) | × (メタデータのみ) | ○ | ○ |
| 縮小 | ○ | × | ○ | × |
| Debian デフォルト | ○ | - | - | - |
| 用途 | 汎用 | 大規模ファイル | 柔軟な管理 | データ完全性重視 |

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
LVM の操作                      → AWS EBS の拡張/スナップショット
パーティション設計               → データベースサーバーのディスク設計
/etc/fstab                      → クラウドの自動マウント設定
ext4 の理解                     → ファイルシステムの選定（ext4 vs XFS vs ZFS）
ディスク容量監視                 → Prometheus + node_exporter
inode の理解                    → 「ディスク空きがあるのに書けない」問題の診断
```

**段階的スキルアップ**:
```
Level 1: lsblk で構成を読み取れる
Level 2: LVM の PV/VG/LV を操作できる
Level 3: パーティション設計を要件から導出できる
Level 4: RAID + LVM の構成を設計できる
Level 5: 分散ストレージ（Ceph, GlusterFS）を設計・運用できる
```

### 3.2 ブートプロセス

**目標**: Linux の起動プロセスを理解し、起動に関するトラブルに対処できるようになる

習得すべきスキル:
- BIOS/UEFI → GRUB → Kernel → systemd の流れ
- GRUB の基本設定
- LUKS パスフレーズによるブート時の暗号化解除
- 起動に失敗した場合のリカバリ方法
- initramfs の役割
- カーネルパラメータの設定

なぜ重要か:
- 起動障害の診断にはブートプロセスの理解が不可欠
- カーネルパラメータの調整にも知識が必要
- VM やコンテナでもブートの概念は（簡略化されて）存在する
- GRUB の設定ミスはシステムをブート不能にする

**Born2beRoot のブートシーケンス詳細**:

```
電源投入
  │
  ▼
BIOS/UEFI
  │ POST (Power-On Self-Test)
  │ ハードウェアの初期化
  │ ブートデバイスの検索
  ▼
GRUB (MBR から読み込み)
  │ /boot パーティションにアクセス（暗号化されていない）
  │ vmlinuz (カーネル) と initrd (初期 RAM ディスク) をロード
  │ カーネルパラメータの設定
  ▼
Linux Kernel
  │ ハードウェアの検出とドライバのロード
  │ initramfs の展開
  ▼
initramfs
  │ LUKS パスフレーズの要求 ← ここでユーザー入力
  │ dm-crypt でパーティションを復号
  │ LVM ボリュームグループの認識
  │ / のマウント
  ▼
systemd (PID 1)
  │ default.target への依存関係を解決
  │ 各サービスを並列起動:
  │   ├── apparmor.service
  │   ├── sshd.service
  │   ├── ufw.service
  │   ├── cron.service
  │   └── multi-user.target
  ▼
Login Prompt / SSH Ready
```

**BIOS vs UEFI の詳細比較**:

| 項目 | BIOS | UEFI |
|------|------|------|
| 正式名称 | Basic Input/Output System | Unified Extensible Firmware Interface |
| パーティションテーブル | MBR | GPT (MBR も可) |
| ブートモード | 16-bit リアルモード | 32/64-bit |
| セキュアブート | × | ○ |
| 最大ディスクサイズ | 2 TB | 9.4 ZB |
| GUI | × | ○（マウス操作可能） |
| Born2beRoot | VirtualBox デフォルト | VirtualBox で UEFI 有効化が必要 |

**段階的スキルアップ**:
```
Level 1: ブートプロセスの各段階を説明できる
Level 2: GRUB でリカバリモードに入れる
Level 3: initramfs の仕組みを理解している
Level 4: カスタムカーネルパラメータを設定できる
Level 5: PXE ブート/ネットワークブートを構築できる
```

### 3.3 プロセスとサービス

**目標**: Linux のプロセスモデルを理解し、デーモンの管理ができるようになる

習得すべきスキル:
- プロセス、デーモン、スレッドの概念
- PID, PPID の関係
- シグナル（SIGTERM, SIGKILL, SIGHUP, SIGINT）
- systemd のユニットファイル
- cron によるタスクスケジューリング
- プロセスのライフサイクル（fork → exec → wait → exit）
- /proc/[pid]/ によるプロセス情報の取得

なぜ重要か:
- サーバー上のすべてのソフトウェアはプロセスとして動作する
- パフォーマンス問題の診断にプロセスの理解が必要
- コンテナ内の PID 1 問題などにも関連する
- ゾンビプロセスやオーファンプロセスの理解が障害対応に必要

**プロセスのライフサイクル**:

```
親プロセス
  │
  │ fork()  ← プロセスの複製
  ├────────────┐
  │            ▼
  │         子プロセス（親のコピー）
  │            │
  │            │ exec()  ← 新しいプログラムに置換
  │            ▼
  │         新しいプログラム
  │            │
  │            │ exit()  ← 終了
  │            ▼
  │         ゾンビプロセス（終了コードを保持）
  │            │
  │ wait()  ← 終了コードの回収
  ├────────────┘
  │
  ▼
親プロセス（続行）
```

**プロセスの状態遷移**:

```
                    fork()
                      │
                      ▼
              ┌──── Created ────┐
              │                 │
              ▼                 │
    ┌──── Ready ◄───────────┐  │
    │       │               │  │
    │  scheduler             │  │
    │       ▼               │  │
    │    Running ───────► Blocked (I/O wait)
    │       │
    │       ├── exit() ──► Zombie ──► Terminated
    │       │                          (wait() で回収)
    │       └── signal ──► Stopped
    │                       │
    └───────────────────────┘
              SIGCONT
```

**シグナルの種類と用途**:

| シグナル | 番号 | 動作 | 用途 | 捕捉可能 |
|---------|------|------|------|---------|
| SIGTERM | 15 | 正常終了要求 | `kill PID` のデフォルト | ○ |
| SIGKILL | 9 | 強制終了（無視不可） | プロセスが応答しない場合 | × |
| SIGHUP | 1 | 設定リロード | `systemctl reload` と同等 | ○ |
| SIGINT | 2 | 割り込み | Ctrl+C | ○ |
| SIGTSTP | 20 | 一時停止 | Ctrl+Z | ○ |
| SIGCONT | 18 | 再開 | `fg` / `bg` | ○ |
| SIGCHLD | 17 | 子プロセス終了通知 | wait() のトリガー | ○ |
| SIGSEGV | 11 | セグメンテーション違反 | メモリ不正アクセス | ○ |
| SIGPIPE | 13 | パイプ切断 | 読み手のないパイプへの書き込み | ○ |

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
プロセス管理                     → コンテナ内のプロセス管理
PID 1 (systemd)                 → Docker の CMD / ENTRYPOINT
シグナル処理                     → Graceful shutdown の実装
cron ジョブ                     → Kubernetes CronJob / AWS EventBridge
デーモンの概念                   → マイクロサービスの設計
ゾンビプロセス                   → コンテナの PID 1 問題（tini/dumb-init）
```

**段階的スキルアップ**:
```
Level 1: ps/top でプロセスの状態を確認できる
Level 2: シグナルを使ってプロセスを制御できる
Level 3: cron ジョブを設計・デバッグできる
Level 4: systemd のカスタムユニットファイルを作成できる
Level 5: プロセス間通信 (IPC) を設計できる
```

---

## 4. シェルスクリプティング

### 4.1 Bash スクリプトの作成

**目標**: 実用的な Bash スクリプトを作成し、システム管理タスクを自動化できるようになる

習得すべきスキル:
- Bash の基本構文（変数、条件分岐、ループ、関数）
- コマンド置換 `$(command)`
- パイプとリダイレクト
- `awk`, `sed`, `grep` によるテキスト処理
- `/proc` ファイルシステムからの情報取得
- エラーハンドリング
- 終了コードの理解
- 正規表現の基礎

monitoring.sh で使用するテクニック:
```bash
# コマンド置換
ARCH=$(uname -a)

# /proc からの情報取得
PCPU=$(grep "physical id" /proc/cpuinfo | sort -u | wc -l)

# awk による計算
RAM_PERCENT=$(free | awk '/^Mem:/ {printf("%.2f"), $3/$2*100}')

# 条件分岐
LVM=$(if [ "$(lsblk | grep -c "lvm")" -gt 0 ]; then echo yes; else echo no; fi)

# パイプチェーン
SUDO_LOG=$(journalctl _COMM=sudo | grep -c "COMMAND")

# 複数コマンドの出力結合
TCP=$(ss -t state established | tail -n +2 | wc -l)
```

なぜ重要か:
- シェルスクリプトは Linux 管理の基本ツール
- 自動化の第一歩（Ansible や Terraform の基礎）
- CI/CD パイプラインでもシェルスクリプトは頻繁に使用される
- 日常的な運用タスクの効率化に不可欠

**Bash スクリプトのベストプラクティス**:

```bash
#!/bin/bash
# 1. 安全なスクリプトの開始
set -euo pipefail  # エラー時停止、未定義変数禁止、パイプエラー検出
IFS=$'\n\t'         # Internal Field Separator を安全に設定

# 2. 変数は常にダブルクォート
echo "${VARIABLE}"  # ○ 正しい
echo $VARIABLE      # × 危険（スペースやグロブ展開の問題）

# 3. コマンドの存在確認
if ! command -v ufw &> /dev/null; then
    echo "Error: ufw is not installed" >&2
    exit 1
fi

# 4. 一時ファイルの安全な作成とクリーンアップ
TMPFILE=$(mktemp)
trap 'rm -f "${TMPFILE}"' EXIT

# 5. ログ出力の関数化
log() { echo "[$(date '+%Y-%m-%d %H:%M:%S')] $*" >&2; }
log "Script started"
```

**スクリプト品質チェックリスト**:

```
□ shebang (#!/bin/bash) が先頭にあるか
□ set -euo pipefail を使用しているか
□ 変数はダブルクォートで囲んでいるか（"$VAR"）
□ コマンドの存在確認をしているか（command -v）
□ エラー時の処理を定義しているか
□ 一時ファイルを使う場合、trap でクリーンアップしているか
□ 可読性のためにコメントを入れているか
□ shellcheck で静的解析を通しているか
□ 関数を使って構造化しているか
□ 終了コードを適切に返しているか
```

**実務との接点**:
```
Born2beRoot で学ぶこと          → 実務での応用
──────────────────────────────────────────────────
monitoring.sh の作成             → Prometheus exporter の理解
/proc からのデータ取得           → node_exporter のメトリクス理解
awk によるテキスト処理           → ログ解析スクリプト
wall によるブロードキャスト      → Slack/PagerDuty アラート連携
cron でのスケジューリング        → GitHub Actions の cron trigger
set -euo pipefail               → CI/CD パイプラインの品質
```

**段階的スキルアップ**:
```
Level 1: 基本的な変数とコマンド置換が使える
Level 2: monitoring.sh レベルのスクリプトを書ける
Level 3: エラーハンドリングと引数処理を含むスクリプトを書ける
Level 4: 複雑な自動化スクリプト（バックアップ、デプロイ等）を書ける
Level 5: Python/Go に置き換えるべきタイミングを判断できる
```

---

## 5. インフラストラクチャ管理マインドセット

### 5.1 ドキュメント化の重要性

**目標**: 実施した設定と変更を適切に記録し、再現可能な状態を維持する習慣を身につける

Born2beRoot で培うべき習慣:
- 実行したコマンドとその結果を記録する
- 変更前の状態をスナップショットで保存する
- 設定の意図（なぜその設定にしたか）を記録する
- 手順書を他者が再現できる粒度で書く

**ドキュメント化の実践**:

```
悪い例:
  「SSH を設定した」

良い例:
  2024/01/15 14:30 - SSH 設定変更
  変更前: Port 22, PermitRootLogin yes
  変更後: Port 4242, PermitRootLogin no
  理由: Born2beRoot の要件に従い、ポートスキャン対策と root 直接ログイン防止
  確認: ss -tunlp | grep 4242 で待ち受けを確認
  テスト: 別ターミナルから ssh kaztakam@localhost -p 4242 で接続成功
```

**Runbook テンプレート**:

```markdown
## インシデント: [カテゴリ]

### 概要
- 症状: [何が起きているか]
- 影響: [誰/何に影響があるか]
- 緊急度: [高/中/低]

### 診断手順
1. [コマンド]: [期待される出力]
2. [コマンド]: [期待される出力]

### 復旧手順
1. [手順]
2. [手順]

### 確認手順
1. [復旧の確認方法]

### エスカレーション
- [判断基準]: [連絡先]
```

**段階的スキルアップ**:
```
Level 1: 実行したコマンドを記録できる
Level 2: 変更理由と確認手順を含めた手順書を書ける
Level 3: Runbook（運用手順書）を作成できる
Level 4: ADR (Architecture Decision Records) を書ける
Level 5: 組織のナレッジ管理体制を構築できる
```

### 5.2 Infrastructure as Code (IaC) マインド

**目標**: 手動設定の限界を理解し、コードによるインフラ管理の利点を認識する

Born2beRoot の経験から学ぶこと:
- 手動設定は再現性が低い（毎回同じ結果になる保証がない）
- 設定の変更履歴を追跡することが難しい
- 台数が増えると手動管理は破綻する
- コードで定義すれば、レビュー・テスト・バージョン管理が可能

**手動 vs IaC の比較**:

| 観点 | 手動 (Born2beRoot) | IaC (Terraform/Ansible) |
|------|-------------------|------------------------|
| 再現性 | 低い（手順を忘れる、順番を間違える） | 高い（コードが手順そのもの） |
| スケーラビリティ | 1台ずつ設定（10台で10倍の時間） | 100台でもコマンド1回 |
| 変更管理 | ノートに記録（漏れが発生） | Git で全変更を追跡 |
| レビュー | 事前レビューが困難 | Pull Request でレビュー可能 |
| テスト | 本番で直接確認 | staging 環境で事前テスト |
| ロールバック | 記憶に頼る（スナップショットがあれば復元可能） | `git revert` + `terraform apply` |
| ドキュメント | 別途作成が必要 | コードがドキュメント |
| コスト | 時間コスト高（反復作業） | 初期学習コスト高、長期的に低い |

**Born2beRoot → IaC への発展ロードマップ**:

```
Stage 1: Born2beRoot（手動設定を経験）
  → 「この作業、自動化したい」という動機を得る

Stage 2: シェルスクリプト化
  → Born2beRoot の設定手順をスクリプトにまとめる
  → 再現性は向上するが、冪等性（何度実行しても同じ結果）がない

Stage 3: Ansible
  → 設定管理を宣言的に定義する
  → Born2beRoot の全設定を Ansible playbook で表現する
  → 冪等性が保証される

Stage 4: Terraform + Ansible
  → インフラ（VM/ネットワーク）は Terraform で
  → OS 設定は Ansible で
  → 完全に再現可能なインフラが実現

Stage 5: GitOps
  → Git リポジトリをインフラの「Single Source of Truth」に
  → Pull Request = インフラ変更の承認プロセス
  → ArgoCD / Flux でクラスタ管理
```

**Born2beRoot を Ansible で再現する例**:

```yaml
# born2beroot.yml - Ansible Playbook
---
- name: Born2beRoot Configuration
  hosts: debian_server
  become: yes

  tasks:
    - name: Install required packages
      apt:
        name: [sudo, ufw, openssh-server, libpam-pwquality, apparmor]
        state: present

    - name: Configure SSH port
      lineinfile:
        path: /etc/ssh/sshd_config
        regexp: '^#?Port'
        line: 'Port 4242'
      notify: restart sshd

    - name: Disable root SSH login
      lineinfile:
        path: /etc/ssh/sshd_config
        regexp: '^#?PermitRootLogin'
        line: 'PermitRootLogin no'
      notify: restart sshd

    - name: Configure UFW default deny
      ufw:
        state: enabled
        policy: deny
        direction: incoming

    - name: Allow SSH port 4242
      ufw:
        rule: allow
        port: '4242'
        proto: tcp

    - name: Configure password policy
      lineinfile:
        path: /etc/login.defs
        regexp: "{{ item.regexp }}"
        line: "{{ item.line }}"
      loop:
        - { regexp: '^PASS_MAX_DAYS', line: 'PASS_MAX_DAYS   30' }
        - { regexp: '^PASS_MIN_DAYS', line: 'PASS_MIN_DAYS   2' }
        - { regexp: '^PASS_WARN_AGE', line: 'PASS_WARN_AGE   7' }
```

**段階的スキルアップ**:
```
Level 1: 手動設定の問題点を説明できる
Level 2: 設定手順をシェルスクリプトで自動化できる
Level 3: Ansible playbook で設定を管理できる
Level 4: Terraform + Ansible でインフラ全体を管理できる
Level 5: GitOps ワークフローを設計・運用できる
```

### 5.3 セキュリティファースト

**目標**: セキュリティを後付けではなく、設計段階から組み込む思考を身につける

Born2beRoot で身につくセキュリティ思考:
- デフォルトで拒否し、明示的に許可する（UFW の設計）
- 必要最小限の権限のみ付与する（sudo、root 禁止）
- 監査可能な状態を維持する（sudo ログ、monitoring.sh）
- 多層防御を構築する（暗号化 + ファイアウォール + 認証 + MAC）

**セキュリティ設計原則の Born2beRoot マッピング**:

| 原則 | Born2beRoot での実装 | 実務での適用例 |
|------|---------------------|--------------|
| 最小権限 | sudo（必要なコマンドのみ root 実行） | IAM ポリシーの最小化 |
| 多層防御 | LUKS + UFW + SSH + sudo + AppArmor | WAF + SG + IAM + 暗号化 |
| デフォルト拒否 | UFW default deny | Security Group のデフォルト |
| 監査可能性 | sudo ログ、monitoring.sh | CloudTrail, VPC Flow Logs |
| 最小攻撃面 | GUI なし、ポート制限 | 不要なサービスの無効化 |
| 責務の分離 | 一般ユーザーと root の分離 | 本番/開発環境の分離 |
| フェイルセキュア | UFW deny がデフォルト | 認証失敗時はアクセス拒否 |
| 経済性 | 簡潔な設定で最大の効果 | リスクに見合った投資 |

**CIS Benchmark との対応**:

| CIS Benchmark 項目 | Born2beRoot での対応 |
|-------------------|---------------------|
| 1.1.1 Filesystem Configuration | LVM パーティション分離 |
| 1.4.1 Ensure GRUB password is set | （要追加設定） |
| 3.4.1 Ensure firewall package is installed | UFW インストール・設定済み |
| 4.2.1 Ensure SSH Protocol is set to 2 | SSH 設定で対応 |
| 5.3.1 Ensure password creation requirements | pam_pwquality で設定 |
| 5.4.1 Ensure password expiration is 365 days or less | login.defs で 30 日に設定 |
| 6.1.2 Ensure permissions on /etc/shadow | Debian デフォルトで対応 |

**段階的スキルアップ**:
```
Level 1: Born2beRoot の各設定のセキュリティ目的を説明できる
Level 2: 新しいサービス追加時のセキュリティチェックリストを作成できる
Level 3: CIS Benchmark に基づいたサーバーハードニングを実施できる
Level 4: 脅威モデリング（STRIDE/DREAD）を実施できる
Level 5: 組織のセキュリティポリシーを策定・運用できる
```

---

## 6. 学習目標の到達度チェック

プロジェクト完了後に、以下の質問に答えられるか確認する:

### 基礎レベル（必須: 全項目クリアすること）

- [ ] 仮想マシンと物理マシンの違いを説明できる
- [ ] Type 1 と Type 2 ハイパーバイザーの違いを説明できる
- [ ] Debian と Rocky Linux の主な違いを3つ以上挙げられる
- [ ] LVM の3層構造（PV, VG, LV）を説明できる
- [ ] SSH で接続する手順を説明できる
- [ ] ファイアウォールの基本的な動作を説明できる
- [ ] パスワードがどのように保存されているか説明できる
- [ ] AppArmor が何であるか説明できる
- [ ] cron の書式を記述できる
- [ ] monitoring.sh の各コマンドの役割を説明できる
- [ ] sudo と su の違いを説明できる
- [ ] /etc/passwd と /etc/shadow の違いを説明できる

### 中級レベル（目標: 80% 以上クリア）

- [ ] パスワードポリシーの各パラメータの意味と目的を説明できる
- [ ] sudo の各設定項目のセキュリティ上の意味を説明できる
- [ ] monitoring.sh の各コマンドが内部で何をしているか説明できる
- [ ] AppArmor と SELinux の違いを説明できる
- [ ] LUKS が保護する脅威と保護しない脅威を区別できる
- [ ] SSH のポート変更がセキュリティに与える効果と限界を説明できる
- [ ] LVM でディスクを拡張する手順を記述できる
- [ ] /proc ファイルシステムの役割を説明できる
- [ ] NAT と Port Forwarding の関係を説明できる
- [ ] ブートプロセスの各段階を説明できる
- [ ] PAM の役割と設定方法を説明できる
- [ ] systemd のユニットファイルの構造を説明できる
- [ ] iptables のチェーン（INPUT/OUTPUT/FORWARD）の役割を説明できる
- [ ] パスワードハッシュの salt とストレッチングの目的を説明できる

### 応用レベル（目標: 50% 以上クリア）

- [ ] Born2beRoot の設計が「多層防御」をどのように実現しているか説明できる
- [ ] 手動セットアップと IaC の利点・欠点を比較できる
- [ ] 新しいサービスを追加する際に、セキュリティ上の考慮点を列挙できる
- [ ] パスワード認証と公開鍵認証の違いを暗号学的な観点から説明できる
- [ ] ストレージの設計（パーティション分割）の理由を説明できる
- [ ] iptables のチェーン（INPUT/OUTPUT/FORWARD）の関係を説明できる
- [ ] requiretty と secure_path の攻撃防御シナリオを具体的に説明できる
- [ ] パスワードのエントロピーを計算できる
- [ ] CVE を引用してセキュリティ設定の必要性を説明できる
- [ ] Terraform/Ansible で Born2beRoot の設定を再現する方法を説明できる
- [ ] CIS Benchmark の項目と Born2beRoot の設定を対応させられる
- [ ] プロセスのライフサイクル（fork-exec-wait-exit）を説明できる

### エキスパートレベル（挑戦: 合格に必須ではないが理解を深める）

- [ ] AES-256-XTS の動作原理を概説できる
- [ ] Linux カーネルのスケジューラ (CFS) の基本原理を説明できる
- [ ] Netfilter のフックポイント (PREROUTING/INPUT/FORWARD/OUTPUT/POSTROUTING) を説明できる
- [ ] ext4 ファイルシステムの inode と superblock の関係を説明できる
- [ ] SSH のキー交換プロトコル（Diffie-Hellman）の原理を説明できる
- [ ] PBKDF2 / bcrypt / Argon2 の違いと選定基準を説明できる
- [ ] ゼロトラストセキュリティモデルを Born2beRoot の文脈で説明できる
- [ ] コンテナ (Docker) と VM の分離レベルの違いを技術的に説明できる
- [ ] Linux Namespace と cgroup の役割を説明できる
- [ ] ASLR (Address Space Layout Randomization) の仕組みを説明できる

---

## 7. 資格試験との対応

Born2beRoot で学んだ知識は、以下の資格試験の出題範囲と重なる:

### 7.1 LPIC-1 (Linux Professional Institute Certification Level 1)

| LPIC-1 出題範囲 | Born2beRoot での学習 | カバー率 |
|----------------|---------------------|---------|
| 101.1 ハードウェア設定 | VirtualBox ハードウェア設定 | 30% |
| 101.2 システムのブート | BIOS→GRUB→Kernel→systemd | 80% |
| 102.1 ディスクレイアウト | LVM, パーティション設計 | 90% |
| 102.3 共有ライブラリ管理 | （直接は扱わない） | 10% |
| 103.1 コマンドライン操作 | シェルスクリプト、パイプ | 60% |
| 104.1 パーティションとファイルシステム | LVM, ext4, fstab | 90% |
| 105.1 シェルスクリプト | monitoring.sh | 50% |
| 107.1 ユーザーアカウント管理 | adduser, usermod, /etc/passwd | 90% |
| 108.1 時間管理 | cron, NTP | 40% |
| 109.1 ネットワーク基礎 | TCP/IP, NAT, UFW | 60% |
| 110.1 セキュリティ管理 | SSH, sudo, AppArmor, 暗号化 | 80% |

### 7.2 CompTIA Security+

| Security+ ドメイン | Born2beRoot での学習 | 関連トピック |
|-------------------|---------------------|-------------|
| 1.0 脅威、攻撃、脆弱性 | パスワード攻撃、ポートスキャン | ブルートフォース、辞書攻撃 |
| 2.0 技術とツール | SSH, UFW, AppArmor | ファイアウォール、IDS/IPS |
| 3.0 アーキテクチャと設計 | 多層防御、最小権限 | Defense in Depth |
| 4.0 ID とアクセス管理 | PAM, sudo, /etc/shadow | 認証、認可、アカウンタビリティ |
| 5.0 リスク管理 | セキュリティポリシー設計 | CIS Benchmark |
| 6.0 暗号化と PKI | LUKS, SSH, ハッシュ | 対称暗号、公開鍵暗号 |

### 7.3 CISSP (Certified Information Systems Security Professional)

| CISSP ドメイン | Born2beRoot との関連 |
|---------------|---------------------|
| Domain 1: セキュリティとリスク管理 | セキュリティ原則（最小権限、多層防御） |
| Domain 2: 資産セキュリティ | データ分類、暗号化 (LUKS) |
| Domain 3: セキュリティアーキテクチャ | 多層防御モデル、MAC/DAC |
| Domain 4: 通信とネットワークセキュリティ | SSH, UFW, TCP/IP |
| Domain 5: ID とアクセス管理 | 認証 (PAM)、認可 (sudo)、アカウンタビリティ (ログ) |
| Domain 7: セキュリティ運用 | 監視 (monitoring.sh)、ログ管理 |
| Domain 8: ソフトウェア開発セキュリティ | セキュアコーディング (shellcheck) |

---

## 8. キャリアパスと Born2beRoot の関連

Born2beRoot で学んだスキルは、以下のキャリアパスの基礎となる:

### 8.1 SRE (Site Reliability Engineer)

```
Born2beRoot での経験:
  ├── monitoring.sh → 監視の基礎
  ├── サービス管理 → SLI/SLO の設計
  ├── トラブルシューティング → インシデント対応
  └── 自動化の動機 → Toil の削減

必要な追加スキル:
  ├── Prometheus / Grafana（監視）
  ├── Kubernetes（オーケストレーション）
  ├── Terraform（IaC）
  └── Go / Python（ツール開発）

年収目安（日本）: 600万〜1200万円
```

### 8.2 セキュリティエンジニア

```
Born2beRoot での経験:
  ├── ハードニング → セキュリティベースライン
  ├── 暗号化 → 暗号学の基礎
  ├── ログ監査 → SIEM/SOC の基礎
  └── AppArmor → MAC/ゼロトラスト

必要な追加スキル:
  ├── ペネトレーションテスト
  ├── SIEM (Splunk, ELK)
  ├── 脅威モデリング
  └── コンプライアンス (SOC2, GDPR)

年収目安（日本）: 500万〜1500万円
```

### 8.3 DevOps エンジニア

```
Born2beRoot での経験:
  ├── 手動設定の苦労 → IaC の動機
  ├── SSH/ネットワーク → インフラの基礎
  ├── スクリプティング → CI/CD パイプライン
  └── サービス管理 → デプロイメント

必要な追加スキル:
  ├── Docker / Kubernetes
  ├── CI/CD (GitHub Actions, GitLab CI)
  ├── Terraform / Ansible
  └── クラウド (AWS/GCP/Azure)

年収目安（日本）: 500万〜1200万円
```

### 8.4 クラウドアーキテクト

```
Born2beRoot での経験:
  ├── ネットワーク設計 → VPC 設計
  ├── ストレージ設計 → EBS/S3 の選定
  ├── セキュリティ設計 → Well-Architected Framework
  └── システム全体の理解 → アーキテクチャ設計

必要な追加スキル:
  ├── マルチアカウント設計
  ├── マイクロサービスアーキテクチャ
  ├── コスト最適化
  └── ディザスタリカバリ

年収目安（日本）: 700万〜1500万円
```

---

## 9. 次のステップ

Born2beRoot を完了した後、以下の分野へ学習を拡げることを推奨する:

### 即座に取り組むべきもの（Born2beRoot 直後）

| 優先度 | テーマ | 理由 | 推奨リソース |
|--------|--------|------|-------------|
| 1 | Docker | VM の次はコンテナ仮想化 | Docker 公式チュートリアル |
| 2 | Ansible | Born2beRoot の設定を自動化 | Ansible for DevOps (本) |
| 3 | Git (advanced) | バージョン管理の深い理解 | Pro Git (無料オンライン) |

### 中期的に取り組むもの（1-3ヶ月後）

| 優先度 | テーマ | 理由 | 推奨リソース |
|--------|--------|------|-------------|
| 4 | Terraform | インフラ全体をコードで管理 | Terraform Up & Running (本) |
| 5 | Kubernetes | コンテナオーケストレーション | Kubernetes in Action (本) |
| 6 | CI/CD | GitHub Actions, GitLab CI でデプロイ自動化 | GitHub Actions 公式ドキュメント |

### 長期的に取り組むもの（3-6ヶ月後）

| 優先度 | テーマ | 理由 | 推奨リソース |
|--------|--------|------|-------------|
| 7 | 監視 | Prometheus, Grafana で本格的な監視 | Prometheus Up & Running (本) |
| 8 | ネットワーク | TCP/IP を深く学ぶ | TCP/IP Illustrated (本) |
| 9 | セキュリティ | ペネトレーションテスト | OverTheWire (オンライン演習) |
| 10 | クラウド | AWS/GCP の資格取得 | AWS Solutions Architect 学習ガイド |

### 42 Tokyo の他プロジェクトとの関連

```
Born2beRoot (システム管理の基礎)
  │
  ├── ft_server / Inception (Docker + マイクロサービス)
  │     └── Born2beRoot の知識が直接活きる:
  │           - ネットワーク設計
  │           - サービス管理
  │           - セキュリティ設定
  │           - Dockerfile でのユーザー管理
  │           - AppArmor プロファイルの理解
  │
  ├── NetPractice (ネットワーク)
  │     └── Born2beRoot で学んだ TCP/IP, NAT, ファイアウォールの知識が基礎
  │           - IP アドレスとサブネットの理解
  │           - ルーティングの概念
  │           - ポートフォワーディング
  │
  └── ft_transcendence (フルスタック Web アプリ)
        └── Born2beRoot で学んだ:
              - サーバー設計
              - セキュリティ（認証、ファイアウォール）
              - デプロイメント
              - SSL/TLS の基礎
              - データベースのストレージ設計
```

### 推奨する学習順序

```
Week 1-2:  Docker 基礎（Born2beRoot の VM から コンテナへ）
Week 3-4:  Docker Compose（マルチコンテナ構成）
Week 5-6:  Ansible 基礎（Born2beRoot の設定を Playbook 化）
Week 7-8:  Terraform 基礎（VirtualBox → AWS の橋渡し）
Week 9-10: Kubernetes 入門（Docker → オーケストレーション）
Week 11-12: CI/CD パイプライン構築
```

---

## 10. まとめ

Born2beRoot は単なる「VM のセットアップ課題」ではない。このプロジェクトを通じて:

1. **基礎の基礎**: OS、ネットワーク、ストレージ、セキュリティの基本原則を体得する
2. **セキュリティマインド**: 「なぜこの設定が必要か」を常に考える習慣が身につく
3. **自動化への渇望**: 手動設定の限界を体感し、IaC への動機を得る
4. **トラブルシューティング力**: 各レイヤーの理解が、問題の切り分け能力を高める
5. **キャリアの基盤**: SRE、セキュリティ、DevOps、クラウドアーキテクトへの土台

**最終メッセージ**: Born2beRoot で経験する「面倒な手動設定」こそが、将来の自動化スキルの原動力となる。すべてのコマンドの意味を理解し、「なぜ」を常に問い続けること。それが、このプロジェクトから得られる最大の学びである。
