# 05 - Terraform 詳解 (Terraform In-Depth)

Born2beRoot の要件を Infrastructure as Code (IaC) の観点から整理し、Terraform の概念を深く掘り下げる。ステートファイル、モジュール、ループ構文、リモートステート、CI/CD 統合、クラウド実装例、Ansible との役割分担までを詳述する。

---

## 1. Infrastructure as Code (IaC) とは何か

### 1.1 概要

Infrastructure as Code (IaC) とは、サーバー、ネットワーク、ストレージなどのインフラストラクチャを、コードとして宣言的に定義し、自動的にプロビジョニング（Provisioning）する手法である。

従来の手動セットアップ（Born2beRoot で実際に行う方法）と IaC の違い:

| 項目 | 手動セットアップ | IaC |
|------|----------------|-----|
| 再現性 | 手順書に依存、ヒューマンエラーの可能性 | コードから完全に再現可能 |
| バージョン管理 | 手順書の変更履歴管理が困難 | Git で全ての変更を追跡可能 |
| スケーラビリティ | 台数が増えると作業量が線形に増加 | 1台でも100台でもほぼ同じ労力 |
| ドキュメント | 手順書とサーバーの実態が乖離しがち | コード自体がドキュメント |
| テスト | 手動で確認 | 自動テスト、CI/CD パイプラインに統合可能 |
| 監査 | 設定の確認に各サーバーにログインが必要 | コードのレビューで設定を確認可能 |
| 障害復旧 | 手順書を見ながら再構築（数時間） | `terraform apply`（数分） |
| コスト | 人的コストが台数に比例して増加 | 初期投資後は追加コスト最小 |
| ロールバック | 手順書を逆順に実行（困難） | Git revert + terraform apply で即座に復元 |
| 環境一貫性 | 環境ごとの微妙な差異（スノーフレーク問題） | 同一コードから完全同一環境を生成 |

#### スノーフレーク問題（Snowflake Problem）とは

手動管理されたサーバーは、時間の経過とともに微妙に異なる設定が蓄積され、雪の結晶（Snowflake）のように一つ一つがユニークな存在になってしまう。これを **スノーフレークサーバー** と呼ぶ。

```
手動管理の結果（スノーフレーク問題）:

Server A: Debian 12.1, OpenSSH 9.2p1, UFW 0.36, Python 3.11.2
Server B: Debian 12.2, OpenSSH 9.2p1, UFW 0.36, Python 3.11.4  ← apt upgrade のタイミング差
Server C: Debian 12.1, OpenSSH 9.4p1, UFW 0.36.2, Python 3.11.2  ← 個別にパッチ適用

→ 3台とも「同じ構成」のはずが、微妙に異なる
→ 「Server B でだけ動かない」というバグの温床になる
```

IaC を使えば、全てのサーバーが同一のコードから生成されるため、この問題が根本的に解消される。これが **Immutable Infrastructure（不変のインフラ）** という考え方であり、サーバーに変更を加えるのではなく、新しいサーバーを作り直すアプローチである。

#### Configuration Drift（設定ドリフト）

スノーフレーク問題と密接に関連する概念が **Configuration Drift** である。これは、時間の経過とともにサーバーの実際の状態が意図した状態から「漂流（drift）」してしまう現象を指す。

```
設定ドリフトの例:

Day 0: IaC で構築（望ましい状態と一致）
Day 10: 緊急対応で手動パッチ適用 → ドリフト発生
Day 20: 別の管理者が設定を微調整 → さらにドリフト
Day 30: IaC のコードと実態が大きく乖離

→ terraform plan を実行すると大量の差分が表示される
→ どこまでが意図的な変更で、どこまでが不本意な変更か分からない
```

Terraform は `terraform plan` でドリフトを検出できるため、定期的に plan を実行してドリフトを監視する運用が推奨される。

### 1.2 IaC のアプローチ

IaC には大きく2つのアプローチがある:

**宣言的 (Declarative)**:
- 「望ましい状態（Desired State）」を記述する
- ツールが現在の状態と望ましい状態の差分を計算し、必要な変更を適用する
- 冪等性（Idempotency）が保証される: 何度実行しても同じ結果になる
- 例: Terraform, Ansible (一部), CloudFormation, Kubernetes YAML

```hcl
# 宣言的: 「SSH がポート 4242 で動作し、root ログインが禁止されている状態」を宣言
resource "ssh_config" "main" {
  port              = 4242
  permit_root_login = false
}
# → Terraform が「現在の状態」と比較し、差分があれば変更を適用
# → 既に port 4242 なら何もしない（冪等性）
```

**命令的 (Imperative)**:
- 「何をするか（手順）」を記述する
- 実行するたびに同じ手順が繰り返される
- 冪等性は開発者が自分で保証する必要がある
- 例: シェルスクリプト, Chef (一部)

```bash
# 命令的: SSH の設定を変更する手順を記述する
sed -i 's/#Port 22/Port 4242/' /etc/ssh/sshd_config
sed -i 's/#PermitRootLogin yes/PermitRootLogin no/' /etc/ssh/sshd_config
systemctl restart sshd
# → 2回目の実行で「#Port 22」が見つからず、意図通りに動かない可能性
# → 冪等性が保証されない
```

#### 宣言的 vs 命令的の詳細比較

| 特性 | 宣言的 | 命令的 |
|------|--------|--------|
| 記述内容 | 「何であるべきか」| 「何をするか」|
| 冪等性 | ツールが保証 | 開発者が保証 |
| 状態管理 | ツールが差分を計算 | 開発者が管理 |
| 学習曲線 | 独自言語の習得が必要 | スクリプト言語の知識で可能 |
| デバッグ | 差分の理解が必要 | ステップ実行で追跡可能 |
| 再実行の安全性 | 高い | 低い（副作用の可能性）|
| ドリフト修正 | 自動的に修正可能 | 手動で差分を確認・修正 |

#### 冪等性（Idempotency）の重要性

冪等性とは、同じ操作を何度実行しても、結果が最初の1回と同じになる性質のことである。数学では f(f(x)) = f(x) と表される。

```bash
# 冪等でない操作:
echo "Port 4242" >> /etc/ssh/sshd_config
# → 実行するたびに行が追加される
# → 2回実行すると「Port 4242」が2行になってしまう

# 冪等な操作（宣言的ツールが内部で行うこと）:
# Terraform の場合: Port が 4242 でなければ 4242 に設定する
# → 何度実行しても Port は 4242 のまま

# シェルスクリプトで冪等性を自前で実装する例:
if ! grep -q "^Port 4242" /etc/ssh/sshd_config; then
    sed -i 's/^#\?Port .*/Port 4242/' /etc/ssh/sshd_config
fi
# → 既に設定済みなら何もしない
```

IaC では冪等性が非常に重要である。なぜなら、インフラの変更は何度も apply される可能性があり、意図しない重複や矛盾が発生してはならないからだ。

#### Convergence（収束）モデル

宣言的 IaC ツールは「収束（Convergence）」というモデルで動作する:

```
収束モデル:

  現在の状態 ──────→ 差分計算 ──────→ 望ましい状態
  (SSH: port 22)     (port を変更)     (SSH: port 4242)

  1回目の実行: port 22 → 4242 に変更
  2回目の実行: port 4242 → 変更なし（既に収束済み）
  3回目の実行: port 4242 → 変更なし（既に収束済み）

  → 何回実行しても「望ましい状態」に収束する
```

### 1.3 IaC ツールの分類

```
IaC ツール
├── プロビジョニングツール（インフラの作成・管理）
│   ├── Terraform       ← 今回のフォーカス（マルチクラウド対応）
│   ├── OpenTofu        (Terraform の OSS フォーク)
│   ├── CloudFormation  (AWS 専用)
│   ├── Pulumi          (汎用言語で記述: Python, TypeScript, Go)
│   ├── CDK             (AWS Cloud Development Kit)
│   ├── Bicep           (Azure 専用)
│   └── Crossplane      (Kubernetes ネイティブの IaC)
│
├── 構成管理ツール（OS・ソフトウェアの設定）
│   ├── Ansible         ← Born2beRoot の設定に最適（エージェントレス）
│   ├── Chef            (Ruby ベース)
│   ├── Puppet          (独自 DSL)
│   └── SaltStack       (Python ベース)
│
├── イメージビルドツール
│   ├── Packer          (VM イメージの作成)
│   ├── Docker          (コンテナイメージの作成)
│   └── cloud-init      (クラウド初期設定)
│
└── コンテナオーケストレーション
    ├── Kubernetes
    ├── Docker Compose
    └── Nomad (HashiCorp)
```

#### プロビジョニングツールの比較

| ツール | 言語 | マルチクラウド | 状態管理 | 学習曲線 | ライセンス |
|--------|------|:------------:|:--------:|:--------:|:----------:|
| Terraform | HCL | ○ | State ファイル | 中 | BSL |
| OpenTofu | HCL | ○ | State ファイル | 中 | MPL (OSS) |
| CloudFormation | JSON/YAML | × (AWS) | AWS 管理 | 中 | - |
| Pulumi | Python/TS/Go | ○ | Pulumi Cloud | 低〜中 | Apache 2.0 |
| CDK | TypeScript/Python | × (AWS) | CloudFormation 経由 | 中 | Apache 2.0 |
| Bicep | Bicep | × (Azure) | Azure 管理 | 低 | MIT |

#### 構成管理ツールの比較

| ツール | 定義言語 | エージェント | プッシュ/プル | 冪等性 |
|--------|----------|:----------:|:----------:|:------:|
| Ansible | YAML | 不要（SSH） | プッシュ | ○ |
| Chef | Ruby (DSL) | 必要 | プル | ○ |
| Puppet | Puppet DSL | 必要 | プル | ○ |
| SaltStack | YAML | 両方対応 | 両方 | ○ |

Born2beRoot の文脈では:
- **Terraform** → VM の作成、ネットワーク設定、セキュリティグループ、ディスク
- **Ansible / cloud-init** → OS 内部の設定（ユーザー、パスワードポリシー、SSH、UFW 等）
- **Packer** → Born2beRoot 設定済みのマシンイメージの作成

### 1.4 IaC の歴史と進化

```
時代の流れ:

1990年代: 手動管理の時代
  → サーバー管理者が1台1台手動で設定
  → Runbook（手順書）の文化
  → CFEngine (1993) が最初の構成管理ツール

2000年代: スクリプト自動化の時代
  → シェルスクリプト、Perl で自動化
  → Puppet (2005) の登場
  → 「Infrastructure as Code」という用語が生まれる

2009-2013: 構成管理ツールの成熟
  → Chef (2009) の登場
  → Ansible (2012) ← エージェントレス革命
  → Docker (2013) ← コンテナ革命

2014-2018: クラウドネイティブ IaC
  → Terraform (2014) ← マルチクラウド IaC の標準に
  → Kubernetes (2015~) ← コンテナオーケストレーション
  → AWS CDK (2018)

2019-現在: GitOps と Policy as Code
  → GitOps (ArgoCD, Flux) ← Git を信頼できる唯一の情報源に
  → Policy as Code (OPA, Sentinel) ← ポリシーもコードで管理
  → OpenTofu (2023) ← Terraform の OSS フォーク
  → 全てをコードとして管理する文化の確立
```

---

## 2. Terraform の基本概念

### 2.1 概要

Terraform は、HashiCorp が開発した IaC ツール。HCL (HashiCorp Configuration Language) という独自の設定言語を使い、インフラストラクチャを宣言的に定義する。

2023年にライセンスが BSL (Business Source License) に変更され、コミュニティフォークとして **OpenTofu** が登場した。基本的な概念と構文は共通しているため、本ドキュメントの内容はどちらにも適用できる。

### 2.2 Terraform の基本概念

**Provider**: インフラストラクチャのプラットフォームとのインターフェース。API の抽象化レイヤーとして機能し、各クラウドプロバイダーやサービスとの通信を担当する。

```hcl
# AWS Provider の設定例
provider "aws" {
  region  = "ap-northeast-1"  # 東京リージョン
  profile = "default"          # AWS CLI のプロファイル
}

# GCP Provider の設定例
provider "google" {
  project = "my-project-id"
  region  = "asia-northeast1"
}

# 複数の Provider を同時に使用することも可能（マルチクラウド構成）
# AWS と GCP を同時に管理できる
```

**Resource**: 管理するインフラの各要素。Provider が提供するリソースタイプを使って定義する。

```hcl
# リソースの基本構文
resource "<PROVIDER>_<TYPE>" "<NAME>" {
  # 設定パラメータ
}

# 例: AWS EC2 インスタンス
resource "aws_instance" "web" {
  ami           = "ami-0123456789"
  instance_type = "t3.micro"

  tags = {
    Name = "Born2beRoot"
  }
}
```

**Data Source**: 既存のリソースの参照（作成せずに情報を取得）。Terraform の管理外にある既存リソースの情報を読み取るのに使う。

```hcl
# 最新の Debian 12 AMI を検索
data "aws_ami" "debian" {
  most_recent = true
  owners      = ["136693071363"]  # Debian 公式

  filter {
    name   = "name"
    values = ["debian-12-amd64-*"]
  }

  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

# Data Source の値を Resource で使用
resource "aws_instance" "born2beroot" {
  ami = data.aws_ami.debian.id  # ← Data Source から取得
}
```

**Variable**: パラメータの外部化。環境ごとの差異をコード変更なしに実現する。

```hcl
# 変数の定義
variable "ssh_port" {
  description = "SSH のリスニングポート"
  type        = number
  default     = 4242

  # 入力値のバリデーション
  validation {
    condition     = var.ssh_port >= 1024 && var.ssh_port <= 65535
    error_message = "ポートは 1024-65535 の範囲で指定してください。"
  }
}

# 変数の使用
resource "aws_security_group_rule" "ssh" {
  from_port = var.ssh_port
  to_port   = var.ssh_port
}
```

**Output**: 適用後の情報出力。他の Terraform 設定やスクリプトから参照できる。

```hcl
output "server_ip" {
  description = "サーバーのパブリック IP"
  value       = aws_instance.web.public_ip
}

output "ssh_command" {
  description = "SSH 接続コマンド"
  value       = "ssh kaztakam@${aws_instance.web.public_ip} -p 4242"
}
```

**Locals**: 中間値の計算やよく使う値のエイリアスを定義する。変数とは異なり、外部から値を注入できない。

```hcl
locals {
  # 共通のタグ
  common_tags = {
    Project     = "born2beroot"
    Environment = var.environment
    ManagedBy   = "terraform"
  }

  # 計算値
  disk_size_mb = var.disk_size_gb * 1024
}

resource "aws_instance" "born2beroot" {
  tags = local.common_tags  # ← locals の参照
}
```

### 2.3 HCL (HashiCorp Configuration Language) の基本構文

HCL は Terraform 専用の設定言語で、JSON より人間が読みやすく、プログラミング言語の要素も持つ。

```hcl
# =============================================================================
# データ型
# =============================================================================

# 文字列 (string)
variable "hostname" {
  type    = string
  default = "kaztakam42"
}

# 数値 (number)
variable "ssh_port" {
  type    = number
  default = 4242
}

# ブール値 (bool)
variable "enable_monitoring" {
  type    = bool
  default = true
}

# リスト (list)
variable "allowed_ports" {
  type    = list(number)
  default = [4242, 80, 443]
}

# マップ (map)
variable "tags" {
  type = map(string)
  default = {
    project = "born2beroot"
    env     = "education"
  }
}

# オブジェクト (object) - 異なる型のフィールドを持つ
variable "vm_config" {
  type = object({
    name   = string
    cpus   = number
    memory = number
  })
  default = {
    name   = "Born2beRoot"
    cpus   = 1
    memory = 1024
  }
}

# タプル (tuple) - 異なる型の要素を持つリスト
variable "mixed_list" {
  type    = tuple([string, number, bool])
  default = ["born2beroot", 4242, true]
}

# =============================================================================
# 文字列補間（String Interpolation）
# =============================================================================

output "ssh_command" {
  value = "ssh ${var.username}@localhost -p ${var.ssh_port}"
}

# ヒアドキュメント（複数行の文字列）
resource "local_file" "config" {
  content = <<-EOT
    hostname: ${var.hostname}
    port: ${var.ssh_port}
    users:
      - ${var.username}
  EOT
}

# =============================================================================
# 条件式（Conditional Expression / 三項演算子）
# =============================================================================

resource "aws_instance" "born2beroot" {
  instance_type = var.environment == "production" ? "t3.medium" : "t3.micro"
  # production なら t3.medium、それ以外なら t3.micro
}

# =============================================================================
# for 式（リスト内包表記・マップ内包表記）
# =============================================================================

# リストの変換
output "uppercase_names" {
  value = [for name in var.usernames : upper(name)]
  # ["kaztakam", "admin"] → ["KAZTAKAM", "ADMIN"]
}

# マップの生成
output "port_map" {
  value = { for port in var.allowed_ports : "port-${port}" => port }
  # [4242, 80] → { "port-4242" = 4242, "port-80" = 80 }
}

# フィルタリング
output "large_volumes" {
  value = [for k, v in var.lvm_volumes : k if v.size > 3]
  # サイズが 3GB 以上のボリューム名だけを取得
}

# =============================================================================
# 組み込み関数（よく使うもの）
# =============================================================================

# ファイルの読み込み
locals {
  cloud_init = file("${path.module}/scripts/cloud-init.yaml")
}

# テンプレートのレンダリング
locals {
  rendered = templatefile("${path.module}/templates/sshd.tftpl", {
    port              = var.ssh_port
    permit_root_login = "no"
  })
}

# 文字列操作
output "hostname_lower" {
  value = lower(replace(var.hostname, " ", "-"))
}

# リスト操作
output "first_port" {
  value = element(var.allowed_ports, 0)
}

# マージ（マップの結合）
output "all_tags" {
  value = merge(local.common_tags, { Name = "born2beroot" })
}

# JSON エンコード/デコード
output "config_json" {
  value = jsonencode(var.vm_config)
}

# CIDR 操作（ネットワーク計算）
output "subnets" {
  value = cidrsubnet("10.0.0.0/16", 8, 1)  # → "10.0.1.0/24"
}
```

### 2.4 Terraform のワークフロー

```
┌─────────┐    ┌─────────┐    ┌──────────┐    ┌──────────┐
│  Write   │───→│  Plan   │───→│  Apply   │───→│  State   │
│ (.tf)    │    │ (diff)  │    │ (execute)│    │(.tfstate)│
└─────────┘    └─────────┘    └──────────┘    └──────────┘
     │              │               │               │
     │        差分を表示       変更を実行      状態を記録
     │      「+ 作成」                        次回の Plan で
     │      「- 削除」                        参照される
     │      「~ 変更」を表示
     │
     └── HCL でインフラを記述
```

```bash
# 0. ワークスペースの初期化
terraform init
# → Provider プラグインのダウンロード
# → バックエンドの初期化
# → モジュールのダウンロード

# 1. コードのフォーマット
terraform fmt
# → HCL ファイルを標準スタイルに整形
# → チーム全体でコードスタイルを統一

# 2. 構文の検証
terraform validate
# → HCL の文法エラー、型の不一致、必須パラメータの不足を検出

# 3. 実行計画の確認
terraform plan
# → 「何が変更されるか」を表示するが、実際には変更しない
# → レビュー用に保存: terraform plan -out=tfplan

# 4. 変更の適用
terraform apply
# → Plan の内容を確認後、yes を入力して適用
# → 保存済み Plan から適用: terraform apply tfplan

# 5. 現在の状態を確認
terraform show
# → 管理中のリソースの現在の状態を表示

# 6. 状態の一覧
terraform state list
# → 管理中の全リソースの識別子を一覧表示

# 7. リソースの詳細
terraform state show aws_instance.born2beroot
# → 特定リソースの全属性を表示

# 8. インフラの削除
terraform destroy
# → 管理中の全リソースを削除
```

#### terraform plan の出力例

```
$ terraform plan

Terraform will perform the following actions:

  # aws_instance.born2beroot will be created
  + resource "aws_instance" "born2beroot" {
      + ami                    = "ami-0123456789abcdef0"
      + instance_type          = "t3.micro"
      + id                     = (known after apply)
      + public_ip              = (known after apply)
      + tags                   = {
          + "Name" = "Born2beRoot"
        }
    }

  # aws_security_group.born2beroot will be created
  + resource "aws_security_group" "born2beroot" {
      + name        = "born2beroot-sg"
      + description = "Born2beRoot Security Group"
      + ingress     = [
          + {
              + from_port   = 4242
              + to_port     = 4242
              + protocol    = "tcp"
              + cidr_blocks = ["0.0.0.0/0"]
            },
        ]
    }

Plan: 2 to add, 0 to change, 0 to destroy.

# +  = 新規作成
# -  = 削除
# ~  = 変更（in-place）
# -/+ = 削除してから再作成（forces replacement）
```

---

## 3. ステートファイルの仕組み

### 3.1 ステートとは何か

Terraform はインフラの現在の状態を **ステートファイル** (`terraform.tfstate`) に JSON 形式で記録する。これにより、コードの望ましい状態と現在の状態を比較し、差分のみを適用できる。

```
                    ┌──────────────────┐
                    │   .tf ファイル     │
                    │  (望ましい状態)    │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │  terraform plan   │
                    │  差分を計算        │
                    └────────┬─────────┘
                             │
              ┌──────────────┼──────────────┐
              │              │              │
    ┌─────────▼──────┐  ┌───▼───────┐  ┌──▼───────┐
    │ .tfstate        │  │ 実際の     │  │ Plan     │
    │(記録された状態)  │  │ インフラ   │  │ (差分)   │
    └────────────────┘  └───────────┘  └──────────┘
```

### 3.2 ステートファイルの中身

```json
{
  "version": 4,
  "terraform_version": "1.5.0",
  "serial": 3,
  "lineage": "a1b2c3d4-e5f6-7890-abcd-ef0123456789",
  "resources": [
    {
      "mode": "managed",
      "type": "aws_instance",
      "name": "born2beroot",
      "provider": "provider[\"registry.terraform.io/hashicorp/aws\"]",
      "instances": [
        {
          "schema_version": 1,
          "attributes": {
            "id": "i-0123456789abcdef0",
            "ami": "ami-0123456789",
            "instance_type": "t3.micro",
            "public_ip": "54.250.100.200",
            "private_ip": "10.0.1.50",
            "tags": {
              "Name": "Born2beRoot"
            },
            "root_block_device": [
              {
                "volume_size": 30,
                "encrypted": true
              }
            ]
          }
        }
      ]
    }
  ]
}
```

### 3.3 ステートファイルの重要性

| 役割 | 説明 |
|------|------|
| **差分計算の基礎** | ステート（現在）とコード（望ましい）を比較して差分を算出 |
| **リソースの追跡** | ステートに記録されたリソースのみが Terraform の管理下にある |
| **メタデータの保持** | リソースの依存関係、Provider 情報を保持 |
| **パフォーマンス** | API で毎回状態を取得する代わりにステートを参照（高速化） |
| **リソース ID の対応** | コード上の名前と実際のリソース ID を紐づける |

#### ステートがない場合の問題

```
ステートが失われた場合:

1. Terraform は「管理しているリソースがない」と認識する
2. terraform plan → 全リソースを「新規作成」と表示
3. terraform apply → 既存リソースとは別に新しいリソースを作成
4. 結果: 同じリソースが2つ存在する（孤児リソース問題）

対処法:
  terraform import aws_instance.born2beroot i-0123456789abcdef0
  → 既存リソースをステートに取り込む
```

### 3.4 リモートステート (Remote State)

チームでの共同作業では、ステートファイルをリモートに保存する。ローカルに置くと以下の問題が発生する:

```
ローカルステートの問題:

開発者 A のPC: terraform.tfstate (version 5)
開発者 B のPC: terraform.tfstate (version 3)  ← 古い

→ 開発者 B が apply すると、開発者 A の変更が上書きされる
→ 最悪の場合、リソースが破壊される
```

#### S3 + DynamoDB によるリモートステート (AWS)

```hcl
terraform {
  backend "s3" {
    bucket         = "my-terraform-state"
    key            = "born2beroot/terraform.tfstate"
    region         = "ap-northeast-1"
    dynamodb_table = "terraform-locks"  # ステートロック用
    encrypt        = true               # ステートファイルの暗号化
  }
}
```

- **S3**: ステートファイルの保存先。バージョニングを有効にして履歴を保持
- **DynamoDB**: ステートロック。複数人が同時に `terraform apply` しないようにする
- **encrypt**: ステートファイルにはパスワード等の機密情報が含まれるため暗号化必須

#### GCS によるリモートステート (GCP)

```hcl
terraform {
  backend "gcs" {
    bucket = "my-terraform-state"
    prefix = "born2beroot"
  }
}
```

#### Terraform Cloud によるリモートステート

```hcl
terraform {
  cloud {
    organization = "my-org"
    workspaces {
      name = "born2beroot"
    }
  }
}
```

### 3.5 ステートロック（State Locking）

チーム開発では、2人が同時に `terraform apply` を実行する危険がある。ステートロックはこれを防止する。

```
開発者 A: terraform apply 開始
  → ステートをロック取得 (DynamoDB に LockID を書き込み)
  → 変更を適用中...

開発者 B: terraform apply 開始
  → ステートのロック取得を試みる
  → エラー: "Error: Error acquiring the state lock"
  → "Lock Info: ID: a1b2c3, Operation: OperationTypeApply, Who: userA@host"
  → 開発者 A の操作が完了するまで待つ必要がある

開発者 A: terraform apply 完了
  → ステートのロックを解放

開発者 B: terraform apply 開始（今度は成功）
  → ステートをロック取得
  → 変更を適用中...
```

#### ロックが残ってしまった場合の対処

```bash
# ロックの強制解除（注意: 他の誰かが本当に apply 中でないことを確認）
terraform force-unlock <LOCK_ID>

# 例
terraform force-unlock a1b2c3d4-e5f6-7890-abcd-ef0123456789
```

### 3.6 ステートの注意点とベストプラクティス

1. **ステートファイルは機密情報を含む**: パスワード、API キー等がプレーンテキストで記録される可能性がある。必ず暗号化し、アクセス制御を行う。
2. **ステートファイルは Git に含めない**: `.gitignore` に `*.tfstate` と `*.tfstate.backup` を追加する。
3. **ステートの手動編集は避ける**: `terraform state` コマンドを使用する。
4. **ステートのバックアップ**: リモートバックエンドのバージョニングを有効にする。
5. **ステートの分割**: 大規模プロジェクトではステートを分割して管理する（ブラストレディウスの縮小）。

```bash
# ステート操作コマンド
terraform state list                                    # 管理リソースの一覧
terraform state show aws_instance.born2beroot           # リソースの詳細表示
terraform state mv <old> <new>                          # リソースの名前変更/移動
terraform state rm <resource>                           # ステートから除外（インフラは削除しない）
terraform state pull                                    # リモートステートをローカルに取得
terraform state push                                    # ローカルステートをリモートにプッシュ
terraform import aws_instance.born2beroot i-01234567    # 既存リソースをインポート
```

#### .gitignore の設定

```gitignore
# Terraform
*.tfstate
*.tfstate.backup
*.tfstate.lock.info
.terraform/
.terraform.lock.hcl
*.tfvars        # 機密情報を含む可能性
crash.log
override.tf
override.tf.json
*_override.tf
*_override.tf.json
```

---

## 4. モジュールの作成と利用

### 4.1 モジュールとは

モジュールは Terraform コードの再利用可能なパッケージ。Born2beRoot の各要件をモジュール化する例:

```
born2beroot-terraform/
├── main.tf               # ルートモジュール
├── variables.tf
├── outputs.tf
├── versions.tf           # Terraform と Provider のバージョン制約
├── .gitignore
├── modules/
│   ├── vm/               # VM 作成モジュール
│   │   ├── main.tf
│   │   ├── variables.tf
│   │   └── outputs.tf
│   ├── network/          # ネットワーク設定モジュール
│   │   ├── main.tf
│   │   ├── variables.tf
│   │   └── outputs.tf
│   └── security/         # セキュリティ設定モジュール
│       ├── main.tf
│       ├── variables.tf
│       └── outputs.tf
├── environments/
│   ├── dev/
│   │   └── terraform.tfvars
│   ├── staging/
│   │   └── terraform.tfvars
│   └── prod/
│       └── terraform.tfvars
├── scripts/
│   ├── cloud-init.yaml   # cloud-init 設定
│   └── monitoring.sh     # 監視スクリプト
└── tests/
    └── born2beroot_test.go  # terratest
```

### 4.2 モジュールの定義

```hcl
# =====================================================
# modules/security/variables.tf
# セキュリティモジュールの入力変数
# =====================================================

variable "vpc_id" {
  description = "VPC ID"
  type        = string
}

variable "ssh_port" {
  description = "SSH ポート番号"
  type        = number
  default     = 4242

  validation {
    condition     = var.ssh_port >= 1024 && var.ssh_port <= 65535
    error_message = "ポートは 1024-65535 の範囲で指定してください。"
  }
}

variable "allowed_ssh_cidrs" {
  description = "SSH 接続を許可する CIDR ブロック"
  type        = list(string)
  default     = ["0.0.0.0/0"]
}

variable "tags" {
  description = "共通タグ"
  type        = map(string)
  default     = {}
}
```

```hcl
# =====================================================
# modules/security/main.tf
# セキュリティグループの定義（UFW に相当）
# =====================================================

resource "aws_security_group" "born2beroot" {
  name        = "born2beroot-sg"
  description = "Born2beRoot security group - SSH on custom port only"
  vpc_id      = var.vpc_id

  # インバウンド: SSH (port 4242) のみ
  # Born2beRoot の `ufw allow 4242` に相当
  ingress {
    description = "SSH on custom port"
    from_port   = var.ssh_port
    to_port     = var.ssh_port
    protocol    = "tcp"
    cidr_blocks = var.allowed_ssh_cidrs
  }

  # アウトバウンド: 全て許可
  # Born2beRoot の `ufw default allow outgoing` に相当
  egress {
    description = "Allow all outbound"
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = merge(var.tags, {
    Name = "born2beroot-sg"
  })

  lifecycle {
    create_before_destroy = true
  }
}

# 追加のセキュリティ: SSH ブルートフォース対策のアラーム
resource "aws_cloudwatch_metric_alarm" "ssh_brute_force" {
  alarm_name          = "born2beroot-ssh-brute-force"
  comparison_operator = "GreaterThanThreshold"
  evaluation_periods  = 2
  metric_name         = "NetworkIn"
  namespace           = "AWS/EC2"
  period              = 300
  statistic           = "Sum"
  threshold           = 10000000  # 10MB in 5 min
  alarm_description   = "SSH ブルートフォース攻撃の可能性"

  dimensions = {
    InstanceId = var.instance_id
  }

  tags = var.tags
}
```

```hcl
# =====================================================
# modules/security/outputs.tf
# セキュリティモジュールの出力
# =====================================================

output "security_group_id" {
  description = "セキュリティグループ ID"
  value       = aws_security_group.born2beroot.id
}

output "security_group_name" {
  description = "セキュリティグループ名"
  value       = aws_security_group.born2beroot.name
}
```

### 4.3 モジュールの呼び出し

```hcl
# main.tf（ルートモジュール）
module "security" {
  source = "./modules/security"

  vpc_id            = module.network.vpc_id
  ssh_port          = var.ssh_port
  allowed_ssh_cidrs = ["10.0.0.0/8"]  # 内部ネットワークのみ
  instance_id       = module.vm.instance_id

  tags = local.common_tags
}

module "vm" {
  source = "./modules/vm"

  ami_id             = data.aws_ami.debian.id
  instance_type      = local.instance_type
  subnet_id          = module.network.public_subnet_id
  security_group_ids = [module.security.security_group_id]
  user_data          = file("scripts/cloud-init.yaml")

  tags = local.common_tags
}

# モジュールの出力を参照
output "vm_ip" {
  value = module.vm.public_ip
}

output "ssh_command" {
  value = "ssh kaztakam@${module.vm.public_ip} -p ${var.ssh_port}"
}
```

### 4.4 Terraform Registry のモジュール

Terraform Registry (registry.terraform.io) にはコミュニティの公開モジュールがある。

```hcl
# Registry のモジュールを使用する例
module "vpc" {
  source  = "terraform-aws-modules/vpc/aws"
  version = "5.0.0"

  name = "born2beroot-vpc"
  cidr = "10.0.0.0/16"

  azs             = ["ap-northeast-1a"]
  public_subnets  = ["10.0.1.0/24"]
  private_subnets = ["10.0.2.0/24"]

  tags = local.common_tags
}
```

### 4.5 モジュール設計のベストプラクティス

1. **単一責任**: 1モジュール = 1つの論理的な単位
2. **明示的な入出力**: variables.tf と outputs.tf を必ず定義
3. **バリデーション**: 入力変数に validation ブロックを追加
4. **バージョニング**: Registry 公開時は Semantic Versioning に従う
5. **テスト**: terratest 等でモジュールのテストを書く
6. **ドキュメント**: description を充実させる（terraform-docs で自動生成）

---

## 5. 条件分岐とループ

### 5.1 count によるループ

```hcl
# 同じ構成の VM を複数台作成
resource "aws_instance" "born2beroot" {
  count         = var.instance_count  # 例: 3
  ami           = data.aws_ami.debian.id
  instance_type = "t3.micro"

  tags = {
    Name = "born2beroot-${count.index}"  # born2beroot-0, 1, 2
  }
}

# 参照方法
output "instance_ids" {
  value = aws_instance.born2beroot[*].id  # Splat 式
}

output "first_ip" {
  value = aws_instance.born2beroot[0].public_ip
}
```

#### count の注意点: インデックスベースの問題

```
count = 3 で作成:
  [0] = server-a
  [1] = server-b
  [2] = server-c

server-b を削除したい場合（count = 2 に変更）:
  [0] = server-a  ← 変更なし
  [1] = server-c  ← 元の [2] が [1] に繰り上がる！

  → server-b が削除されるのではなく、server-c が destroy → re-create される
  → 意図しない再作成が発生する
```

### 5.2 for_each によるループ

```hcl
# Born2beRoot の Bonus パーティション構成を for_each で表現
variable "lvm_volumes" {
  type = map(object({
    size  = number
    mount = string
  }))
  default = {
    root    = { size = 10, mount = "/" }
    swap    = { size = 2,  mount = "swap" }
    home    = { size = 5,  mount = "/home" }
    var     = { size = 3,  mount = "/var" }
    srv     = { size = 3,  mount = "/srv" }
    tmp     = { size = 3,  mount = "/tmp" }
    var-log = { size = 4,  mount = "/var/log" }
  }
}

resource "aws_ebs_volume" "lvm" {
  for_each = var.lvm_volumes

  availability_zone = "ap-northeast-1a"
  size              = each.value.size
  encrypted         = true  # LUKS に相当

  tags = {
    Name       = "born2beroot-${each.key}"
    MountPoint = each.value.mount
  }
}

# 参照方法
output "volume_ids" {
  value = { for k, v in aws_ebs_volume.lvm : k => v.id }
}

output "home_volume_id" {
  value = aws_ebs_volume.lvm["home"].id
}
```

#### for_each の利点

```
for_each = { "a" = ..., "b" = ..., "c" = ... } で作成:
  ["a"] = server-a
  ["b"] = server-b
  ["c"] = server-c

"b" を削除した場合（マップから "b" を除去）:
  ["a"] = server-a  ← 変更なし
  ["c"] = server-c  ← 変更なし
  ["b"] = 削除       ← "b" のみが削除される

→ 他のリソースに影響しない！
```

**ルール**: リソースが一意に識別される場合は `for_each` を使い、単純な数のスケーリングには `count` を使う。Born2beRoot のパーティション定義のように、各要素が名前で識別される場合は `for_each` が適切。

### 5.3 dynamic ブロック

リソース内のネストされたブロックを動的に生成する。

```hcl
# UFW のルールを動的に生成
variable "firewall_rules" {
  type = list(object({
    port        = number
    protocol    = string
    description = string
  }))
  default = [
    { port = 4242, protocol = "tcp", description = "SSH" },
    # Bonus: WordPress 用
    { port = 80,   protocol = "tcp", description = "HTTP" },
    { port = 443,  protocol = "tcp", description = "HTTPS" },
  ]
}

resource "aws_security_group" "born2beroot" {
  name   = "born2beroot-sg"
  vpc_id = var.vpc_id

  dynamic "ingress" {
    for_each = var.firewall_rules
    content {
      from_port   = ingress.value.port
      to_port     = ingress.value.port
      protocol    = ingress.value.protocol
      cidr_blocks = ["0.0.0.0/0"]
      description = ingress.value.description
    }
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }
}
```

### 5.4 条件分岐

```hcl
# 環境に応じてリソースを作成/作成しない
resource "aws_instance" "monitoring" {
  count = var.environment == "production" ? 1 : 0
  # production なら 1台作成、それ以外なら作成しない

  ami           = data.aws_ami.debian.id
  instance_type = "t3.small"
}

# 条件に応じた値の選択
locals {
  instance_type = var.environment == "production" ? "t3.medium" : "t3.micro"
  disk_size     = var.environment == "production" ? 100 : 30
  monitoring    = var.enable_monitoring ? "enabled" : "disabled"
}

# null を使った条件付きパラメータ
resource "aws_instance" "born2beroot" {
  ami           = data.aws_ami.debian.id
  instance_type = local.instance_type
  key_name      = var.ssh_key_name != "" ? var.ssh_key_name : null
  # key_name が空なら設定しない
}
```

---

## 6. Born2beRoot 要件のクラウド実装例

### 6.1 AWS での完全実装

```hcl
# =============================================================================
# versions.tf
# =============================================================================
terraform {
  required_version = ">= 1.6.0"
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = "~> 5.0"
    }
  }

  backend "s3" {
    bucket         = "born2beroot-terraform-state"
    key            = "born2beroot/terraform.tfstate"
    region         = "ap-northeast-1"
    dynamodb_table = "terraform-locks"
    encrypt        = true
  }
}

# =============================================================================
# provider.tf
# =============================================================================
provider "aws" {
  region = "ap-northeast-1"  # 東京リージョン

  default_tags {
    tags = {
      Project   = "born2beroot"
      ManagedBy = "terraform"
    }
  }
}

# =============================================================================
# data.tf - 既存リソースの参照
# =============================================================================

# 最新の Debian 12 AMI を取得
data "aws_ami" "debian" {
  most_recent = true
  owners      = ["136693071363"]  # Debian 公式

  filter {
    name   = "name"
    values = ["debian-12-amd64-*"]
  }

  filter {
    name   = "virtualization-type"
    values = ["hvm"]
  }
}

# 現在の AWS アカウント情報
data "aws_caller_identity" "current" {}

# 現在のリージョン
data "aws_region" "current" {}

# =============================================================================
# network.tf - VPC とサブネット
# =============================================================================

resource "aws_vpc" "main" {
  cidr_block           = "10.0.0.0/16"
  enable_dns_support   = true
  enable_dns_hostnames = true

  tags = { Name = "born2beroot-vpc" }
}

resource "aws_subnet" "public" {
  vpc_id                  = aws_vpc.main.id
  cidr_block              = "10.0.1.0/24"
  availability_zone       = "${data.aws_region.current.name}a"
  map_public_ip_on_launch = true

  tags = { Name = "born2beroot-public" }
}

resource "aws_internet_gateway" "main" {
  vpc_id = aws_vpc.main.id
  tags   = { Name = "born2beroot-igw" }
}

resource "aws_route_table" "public" {
  vpc_id = aws_vpc.main.id

  route {
    cidr_block = "0.0.0.0/0"
    gateway_id = aws_internet_gateway.main.id
  }

  tags = { Name = "born2beroot-public-rt" }
}

resource "aws_route_table_association" "public" {
  subnet_id      = aws_subnet.public.id
  route_table_id = aws_route_table.public.id
}

# =============================================================================
# security.tf - セキュリティグループ（UFW に相当）
# =============================================================================

resource "aws_security_group" "born2beroot" {
  name        = "born2beroot-sg"
  description = "Born2beRoot - Port ${var.ssh_port} only"
  vpc_id      = aws_vpc.main.id

  # SSH (port 4242) のみ
  ingress {
    description = "SSH on port ${var.ssh_port}"
    from_port   = var.ssh_port
    to_port     = var.ssh_port
    protocol    = "tcp"
    cidr_blocks = var.allowed_ssh_cidrs
  }

  egress {
    description = "Allow all outbound"
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }

  tags = { Name = "born2beroot-sg" }
}

# =============================================================================
# compute.tf - EC2 インスタンス（VM に相当）
# =============================================================================

resource "aws_key_pair" "born2beroot" {
  key_name   = "born2beroot-key"
  public_key = file(var.ssh_public_key_path)
}

resource "aws_instance" "born2beroot" {
  ami                    = data.aws_ami.debian.id
  instance_type          = var.instance_type
  subnet_id              = aws_subnet.public.id
  vpc_security_group_ids = [aws_security_group.born2beroot.id]
  key_name               = aws_key_pair.born2beroot.key_name

  # cloud-init で OS 内部設定を自動化
  user_data = file("scripts/cloud-init.yaml")

  # ルートボリューム（暗号化あり = LUKS に相当）
  root_block_device {
    volume_size = var.disk_size_gb
    volume_type = "gp3"
    encrypted   = true

    tags = { Name = "born2beroot-root" }
  }

  # IMDSv2 を強制（セキュリティ強化）
  metadata_options {
    http_tokens = "required"
  }

  tags = {
    Name     = "born2beroot"
    Hostname = var.hostname
  }
}

# =============================================================================
# storage.tf - 追加 EBS ボリューム（LVM パーティションに相当）
# =============================================================================

resource "aws_ebs_volume" "lvm" {
  for_each = {
    for k, v in var.lvm_volumes : k => v
    if k != "root" && k != "swap"
  }

  availability_zone = aws_instance.born2beroot.availability_zone
  size              = each.value.size
  type              = "gp3"
  encrypted         = true  # LUKS に相当

  tags = {
    Name       = "born2beroot-${each.key}"
    MountPoint = each.value.mount
  }
}

resource "aws_volume_attachment" "lvm" {
  for_each = aws_ebs_volume.lvm

  device_name = "/dev/xvd${substr("fghijklm", index(keys(aws_ebs_volume.lvm), each.key), 1)}"
  volume_id   = each.value.id
  instance_id = aws_instance.born2beroot.id
}

# =============================================================================
# outputs.tf
# =============================================================================

output "instance_id" {
  description = "EC2 インスタンス ID"
  value       = aws_instance.born2beroot.id
}

output "public_ip" {
  description = "パブリック IP"
  value       = aws_instance.born2beroot.public_ip
}

output "ssh_command" {
  description = "SSH 接続コマンド"
  value       = "ssh ${var.username}@${aws_instance.born2beroot.public_ip} -p ${var.ssh_port}"
}

output "requirements_summary" {
  description = "Born2beRoot 要件のサマリー"
  value = {
    ssh_port    = var.ssh_port
    root_login  = "disabled"
    encryption  = "EBS encryption (LUKS equivalent)"
    firewall    = "Security Group: port ${var.ssh_port} only"
    monitoring  = "cloud-init configured"
  }
}
```

### 6.2 GCP での実装

```hcl
# =============================================================================
# GCP Provider
# =============================================================================
provider "google" {
  project = var.gcp_project_id
  region  = "asia-northeast1"  # 東京
  zone    = "asia-northeast1-a"
}

# =============================================================================
# ファイアウォールルール（UFW に相当）
# =============================================================================

# SSH のみ許可
resource "google_compute_firewall" "allow_ssh" {
  name    = "born2beroot-allow-ssh"
  network = "default"

  allow {
    protocol = "tcp"
    ports    = [tostring(var.ssh_port)]
  }

  source_ranges = var.allowed_ssh_cidrs
  target_tags   = ["born2beroot"]
}

# その他の全インバウンドを拒否
resource "google_compute_firewall" "deny_all" {
  name     = "born2beroot-deny-all"
  network  = "default"
  priority = 65534  # 最低優先度

  deny {
    protocol = "all"
  }

  source_ranges = ["0.0.0.0/0"]
  target_tags   = ["born2beroot"]
}

# =============================================================================
# Compute Engine インスタンス（VM に相当）
# =============================================================================
resource "google_compute_instance" "born2beroot" {
  name         = "born2beroot"
  machine_type = "e2-micro"
  zone         = "asia-northeast1-a"
  tags         = ["born2beroot"]

  boot_disk {
    initialize_params {
      image = "debian-cloud/debian-12"
      size  = var.disk_size_gb
      type  = "pd-balanced"
    }
    # GCP はデフォルトで暗号化（LUKS に相当）
  }

  # 追加ディスク（LVM パーティションに相当）
  dynamic "attached_disk" {
    for_each = google_compute_disk.data
    content {
      source      = attached_disk.value.self_link
      device_name = "born2beroot-${attached_disk.key}"
    }
  }

  network_interface {
    network = "default"
    access_config {}  # 外部 IP を付与
  }

  # cloud-init
  metadata = {
    user-data = file("scripts/cloud-init.yaml")
  }
}

# 追加ディスク
resource "google_compute_disk" "data" {
  for_each = {
    for k, v in var.lvm_volumes : k => v
    if k != "root" && k != "swap"
  }

  name = "born2beroot-${each.key}"
  size = each.value.size
  type = "pd-balanced"
  zone = "asia-northeast1-a"
}
```

---

## 7. CI/CD との統合

### 7.1 なぜ CI/CD に統合するのか

手動で `terraform apply` を実行する運用には以下の問題がある:

1. **ヒューマンエラー**: 間違った環境で apply してしまう
2. **レビュー不足**: Plan の結果を誰もチェックしない
3. **監査証跡**: 誰がいつ何を変更したか追跡できない
4. **一貫性**: 個人の環境差による問題

CI/CD に統合すると:
- PR 時に自動で `terraform plan` が実行される
- Plan 結果が PR のコメントに投稿され、レビュー可能になる
- main ブランチへのマージ時に自動で `terraform apply` が実行される
- 全ての変更が Git の履歴として残る

### 7.2 GitHub Actions での Terraform 自動化

```yaml
# .github/workflows/terraform.yml
name: Terraform CI/CD

on:
  push:
    branches: [main]
    paths: ['terraform/**']
  pull_request:
    branches: [main]
    paths: ['terraform/**']

env:
  TF_VERSION: '1.6.0'
  WORKING_DIR: 'terraform'

permissions:
  contents: read
  pull-requests: write  # PR にコメントするため

jobs:
  # ====================================
  # PR 時: フォーマット、検証、Plan
  # ====================================
  plan:
    if: github.event_name == 'pull_request'
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Terraform
        uses: hashicorp/setup-terraform@v3
        with:
          terraform_version: ${{ env.TF_VERSION }}

      - name: Terraform Init
        run: terraform init
        working-directory: ${{ env.WORKING_DIR }}

      - name: Terraform Format Check
        run: terraform fmt -check -recursive
        working-directory: ${{ env.WORKING_DIR }}

      - name: Terraform Validate
        run: terraform validate
        working-directory: ${{ env.WORKING_DIR }}

      - name: tflint (静的解析)
        uses: terraform-linters/setup-tflint@v4
        with:
          tflint_version: latest

      - run: tflint --init && tflint
        working-directory: ${{ env.WORKING_DIR }}

      - name: Terraform Plan
        id: plan
        run: terraform plan -no-color -out=tfplan 2>&1 | tee plan.txt
        working-directory: ${{ env.WORKING_DIR }}
        continue-on-error: true

      # Plan 結果を PR にコメント
      - name: Comment Plan on PR
        uses: actions/github-script@v7
        with:
          script: |
            const fs = require('fs');
            const plan = fs.readFileSync('${{ env.WORKING_DIR }}/plan.txt', 'utf8');
            const status = '${{ steps.plan.outcome }}';
            const emoji = status === 'success' ? '✅' : '❌';

            const body = `#### ${emoji} Terraform Plan
            \`\`\`
            ${plan.slice(0, 60000)}
            \`\`\`
            *Pushed by: @${{ github.actor }}*
            `;

            github.rest.issues.createComment({
              issue_number: context.issue.number,
              owner: context.repo.owner,
              repo: context.repo.repo,
              body: body
            });

      - name: Fail if plan failed
        if: steps.plan.outcome == 'failure'
        run: exit 1

  # ====================================
  # main マージ時: Apply
  # ====================================
  apply:
    if: github.ref == 'refs/heads/main' && github.event_name == 'push'
    runs-on: ubuntu-latest
    environment: production  # 環境保護ルール適用
    steps:
      - uses: actions/checkout@v4

      - name: Setup Terraform
        uses: hashicorp/setup-terraform@v3
        with:
          terraform_version: ${{ env.TF_VERSION }}

      - name: Terraform Init
        run: terraform init
        working-directory: ${{ env.WORKING_DIR }}

      - name: Terraform Apply
        run: terraform apply -auto-approve
        working-directory: ${{ env.WORKING_DIR }}

      # Slack 通知
      - name: Notify Slack
        if: always()
        uses: slackapi/slack-github-action@v1
        with:
          payload: |
            {
              "text": "Terraform Apply: ${{ job.status }}"
            }
```

### 7.3 CI/CD パイプラインの全体像

```
Developer → Feature Branch → Pull Request → Review → Merge → Deploy

                   ┌──────────────────────────────┐
                   │       Pull Request            │
                   │                                │
                   │  CI:                           │
                   │  1. terraform fmt -check       │
                   │  2. terraform validate         │
                   │  3. tflint (静的解析)           │
                   │  4. checkov (セキュリティ)      │
                   │  5. terraform plan             │
                   │  6. Plan 結果を PR にコメント   │
                   │                                │
                   │  レビュー:                      │
                   │  - コードの変更内容              │
                   │  - Plan の差分                  │
                   │  - セキュリティスキャン結果      │
                   └───────────────┬────────────────┘
                                   │ Approve + Merge
                   ┌───────────────▼────────────────┐
                   │       main ブランチ             │
                   │                                │
                   │  CD:                           │
                   │  1. terraform init             │
                   │  2. terraform apply            │
                   │  3. 検証テスト実行              │
                   │  4. Slack/Teams 通知           │
                   └────────────────────────────────┘
```

### 7.4 Terraform Cloud / Enterprise

HashiCorp が提供するマネージドサービス:

| 機能 | 説明 |
|------|------|
| リモートステート管理 | S3/DynamoDB の設定不要 |
| ステートロック | 自動的にロック管理 |
| Plan/Apply の Web UI | ブラウザで Plan 結果を確認 |
| ポリシー管理（Sentinel） | セキュリティポリシーの強制 |
| チームアクセス制御 | RBAC によるアクセス管理 |
| VCS 統合 | GitHub/GitLab との自動連携 |
| コスト推定 | Apply 前のクラウドコスト推定 |
| Private Registry | プライベートモジュールの公開 |

---

## 8. Ansible との役割分担

### 8.1 Terraform vs Ansible

| 項目 | Terraform | Ansible |
|------|-----------|---------|
| 得意分野 | インフラの作成・管理 | OS の設定・構成管理 |
| アプローチ | 宣言的 | 宣言的 + 手続き的 |
| ステート | あり（.tfstate） | なし（冪等性で担保） |
| エージェント | 不要（API 呼び出し） | 不要（SSH で接続） |
| 実行頻度 | インフラ変更時 | 設定変更時、定期実行 |
| Born2beRoot | VM作成、ネットワーク、ディスク | SSH設定、UFW、パスワード、sudo |

### 8.2 Born2beRoot での役割分担

```
Terraform の責務（インフラ層）:
├── VM の作成（CPU、メモリ、ディスク）
├── ネットワーク設定（VPC、サブネット、ルートテーブル）
├── セキュリティグループ（ネットワークレベルの UFW に相当）
├── ディスクの暗号化設定（LUKS に相当）
├── SSH キーペアの登録
└── cloud-init の設定配布

Ansible の責務（OS 構成層）:
├── ホスト名の設定（hostnamectl）
├── ユーザー・グループの作成（adduser, groupadd）
├── SSH 設定（sshd_config の編集）
├── UFW の設定（ホスト内ファイアウォール）
├── パスワードポリシーの設定（login.defs, pam_pwquality）
├── sudo の設定（sudoers.d）
├── AppArmor の設定
├── monitoring.sh の配置
└── cron の設定
```

### 8.3 Terraform → Ansible の連携方法

```
方法 1: Terraform の output を Ansible inventory に変換
  terraform output -json > tf_output.json
  → Python スクリプトで Ansible inventory を生成

方法 2: Terraform の provisioner で Ansible を実行
  provisioner "local-exec" {
    command = "ansible-playbook -i '${self.public_ip},' playbook.yml"
  }

方法 3: cloud-init で初回設定 + Ansible で継続管理
  Terraform → cloud-init で初回設定を完了
  Ansible → その後の設定変更を管理（推奨）

方法 4: 動的 Inventory
  ansible-playbook -i terraform-inventory playbook.yml
  → Terraform のステートから自動的にホスト一覧を取得
```

### 8.4 Ansible Playbook の完全例

```yaml
# playbook.yml - Born2beRoot の完全設定
---
- name: Born2beRoot Configuration
  hosts: born2beroot
  become: yes
  vars:
    username: kaztakam
    hostname: kaztakam42
    ssh_port: 4242
    password_max_days: 30
    password_min_days: 2
    password_warn_age: 7
    password_min_length: 10
    sudo_max_tries: 3
    monitoring_interval: 10

  tasks:
    # --- ホスト名 ---
    - name: Set hostname
      hostname:
        name: "{{ hostname }}"

    - name: Update /etc/hosts
      lineinfile:
        path: /etc/hosts
        regexp: '^127\.0\.1\.1'
        line: "127.0.1.1\t{{ hostname }}"

    # --- ユーザーとグループ ---
    - name: Create user42 group
      group:
        name: user42
        state: present

    - name: Add user to groups
      user:
        name: "{{ username }}"
        groups: sudo,user42
        append: yes

    # --- SSH ---
    - name: Configure SSH
      lineinfile:
        path: /etc/ssh/sshd_config
        regexp: "{{ item.regexp }}"
        line: "{{ item.line }}"
      loop:
        - { regexp: '^#?Port', line: 'Port {{ ssh_port }}' }
        - { regexp: '^#?PermitRootLogin', line: 'PermitRootLogin no' }
        - { regexp: '^#?X11Forwarding', line: 'X11Forwarding no' }
        - { regexp: '^#?MaxAuthTries', line: 'MaxAuthTries 3' }
      notify: restart sshd

    # --- UFW ---
    - name: Install UFW
      apt:
        name: ufw
        state: present
        update_cache: yes

    - name: UFW - deny incoming
      ufw:
        direction: incoming
        policy: deny

    - name: UFW - allow outgoing
      ufw:
        direction: outgoing
        policy: allow

    - name: UFW - allow SSH port
      ufw:
        rule: allow
        port: "{{ ssh_port }}"
        proto: tcp

    - name: UFW - enable
      ufw:
        state: enabled

    # --- パスワードポリシー ---
    - name: Install libpam-pwquality
      apt:
        name: libpam-pwquality
        state: present

    - name: Configure login.defs
      lineinfile:
        path: /etc/login.defs
        regexp: "^{{ item.key }}"
        line: "{{ item.key }}\t{{ item.value }}"
      loop:
        - { key: "PASS_MAX_DAYS", value: "{{ password_max_days }}" }
        - { key: "PASS_MIN_DAYS", value: "{{ password_min_days }}" }
        - { key: "PASS_WARN_AGE", value: "{{ password_warn_age }}" }

    - name: Configure PAM pwquality
      lineinfile:
        path: /etc/pam.d/common-password
        regexp: 'pam_pwquality\.so'
        line: >-
          password requisite pam_pwquality.so retry=3
          minlen={{ password_min_length }} ucredit=-1 dcredit=-1
          lcredit=-1 maxrepeat=3 reject_username difok=7 enforce_for_root

    - name: Apply password policy to existing users
      command: >-
        chage -M {{ password_max_days }}
        -m {{ password_min_days }}
        -W {{ password_warn_age }} {{ item }}
      loop:
        - "{{ username }}"
        - root
      changed_when: true

    # --- sudo ---
    - name: Create sudo log directory
      file:
        path: /var/log/sudo
        state: directory
        mode: '0750'

    - name: Configure sudo
      copy:
        content: |
          Defaults    passwd_tries={{ sudo_max_tries }}
          Defaults    badpass_message="Wrong password. Access denied."
          Defaults    logfile="/var/log/sudo/sudo.log"
          Defaults    log_input
          Defaults    log_output
          Defaults    iolog_dir="/var/log/sudo"
          Defaults    requiretty
          Defaults    secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin"
        dest: /etc/sudoers.d/sudo_config
        mode: '0440'
        validate: '/usr/sbin/visudo -cf %s'

    # --- monitoring.sh ---
    - name: Deploy monitoring script
      copy:
        src: scripts/monitoring.sh
        dest: /usr/local/bin/monitoring.sh
        mode: '0755'

    - name: Configure cron for monitoring
      cron:
        name: "Born2beRoot monitoring"
        minute: "*/{{ monitoring_interval }}"
        job: "/usr/local/bin/monitoring.sh"
        user: root

    # --- AppArmor ---
    - name: Ensure AppArmor is enabled
      systemd:
        name: apparmor
        enabled: yes
        state: started

  handlers:
    - name: restart sshd
      service:
        name: sshd
        state: restarted
```

#### Ansible の実行

```bash
# 構文チェック
ansible-playbook playbook.yml --syntax-check

# ドライラン（変更をプレビュー）
ansible-playbook -i inventory.ini playbook.yml --check --diff

# 実行
ansible-playbook -i inventory.ini playbook.yml

# 特定のタスクだけ実行（タグを使用）
ansible-playbook -i inventory.ini playbook.yml --tags "ssh,ufw"

# 詳細ログ
ansible-playbook -i inventory.ini playbook.yml -vvv
```

---

## 9. cloud-init の完全例

クラウド環境では **cloud-init** を使って VM の初回起動時に自動設定を行うのが一般的である。

### 9.1 cloud-init とは

cloud-init は、クラウドインスタンスの初回起動時に自動的に実行される初期設定ツール。AWS, GCP, Azure など主要なクラウドプロバイダーが対応している。

```
cloud-init の実行フェーズ:

1. Generator → cloud-init を有効にするか判断
2. Local     → ネットワーク設定前の初期化
3. Network   → ネットワーク設定後の処理
4. Config    → パッケージ、ユーザー、ファイル配置
5. Final     → runcmd の実行、最終処理
```

### 9.2 Born2beRoot の cloud-init 完全設定

```yaml
#cloud-config
# =============================================================================
# Born2beRoot - cloud-init 完全設定
# VM の初回起動時に自動的に実行される
# =============================================================================

# --- ホスト名 ---
hostname: kaztakam42
manage_etc_hosts: true
fqdn: kaztakam42.local

# --- ロケールとタイムゾーン ---
locale: en_US.UTF-8
timezone: Asia/Tokyo

# --- グループの作成 ---
groups:
  - user42

# --- ユーザーの作成 ---
users:
  - default
  - name: kaztakam
    groups: [sudo, user42]
    shell: /bin/bash
    lock_passwd: false
    # mkpasswd --method=sha-512 で生成
    passwd: $6$rounds=4096$salt$hash_here
    ssh_authorized_keys:
      - ssh-ed25519 AAAA... kaztakam@42tokyo

# --- パッケージ ---
package_update: true
package_upgrade: true
packages:
  - ufw
  - libpam-pwquality
  - apparmor
  - apparmor-utils
  - sudo
  - openssh-server
  - vim
  - cron

# --- 設定ファイルの配置 ---
write_files:
  # sudo 設定
  - path: /etc/sudoers.d/sudo_config
    owner: root:root
    permissions: '0440'
    content: |
      Defaults passwd_tries=3
      Defaults badpass_message="Wrong password. Access denied."
      Defaults logfile="/var/log/sudo/sudo.log"
      Defaults log_input
      Defaults log_output
      Defaults iolog_dir="/var/log/sudo"
      Defaults requiretty
      Defaults secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin"

  # SSH 設定（sshd_config.d ディレクトリ配下に配置）
  - path: /etc/ssh/sshd_config.d/born2beroot.conf
    owner: root:root
    permissions: '0644'
    content: |
      Port 4242
      PermitRootLogin no
      PasswordAuthentication yes
      PubkeyAuthentication yes
      X11Forwarding no
      MaxAuthTries 3
      LoginGraceTime 60
      ClientAliveInterval 300
      ClientAliveCountMax 2

  # monitoring.sh
  - path: /usr/local/bin/monitoring.sh
    owner: root:root
    permissions: '0755'
    content: |
      #!/bin/bash
      ARCH=$(uname -a)
      PCPU=$(grep "physical id" /proc/cpuinfo | sort -u | wc -l)
      VCPU=$(grep -c "^processor" /proc/cpuinfo)
      URAM=$(free -m | awk '/Mem:/ {print $3}')
      TRAM=$(free -m | awk '/Mem:/ {print $2}')
      PRAM=$(free -m | awk '/Mem:/ {printf("%.2f"), $3/$2*100}')
      UDISK=$(df -BG --total | awk '/total/ {print $3}' | tr -d 'G')
      TDISK=$(df -BG --total | awk '/total/ {print $2}' | tr -d 'G')
      PDISK=$(df --total | awk '/total/ {print $5}')
      CPUL=$(top -bn1 | awk '/Cpu/ {printf("%.1f"), $2+$4}')
      LB=$(who -b | awk '{print $3" "$4}')
      LVM=$(if [ $(lsblk | grep -c "lvm") -gt 0 ]; then echo yes; else echo no; fi)
      TCP=$(ss -t | grep -c ESTAB)
      ULOG=$(who | wc -l)
      IP=$(hostname -I | awk '{print $1}')
      MAC=$(ip link | awk '/ether/ {print $2}' | head -1)
      SUDO=$(grep -c COMMAND /var/log/sudo/sudo.log 2>/dev/null || echo 0)

      wall "
      #Architecture: $ARCH
      #CPU physical: $PCPU
      #vCPU: $VCPU
      #Memory Usage: ${URAM}/${TRAM}MB (${PRAM}%)
      #Disk Usage: ${UDISK}/${TDISK}GB (${PDISK})
      #CPU load: ${CPUL}%
      #Last boot: $LB
      #LVM use: $LVM
      #Connections TCP: $TCP ESTABLISHED
      #User log: $ULOG
      #Network: IP $IP ($MAC)
      #Sudo: $SUDO cmd
      "

  # パスワード品質設定
  - path: /etc/security/pwquality.conf
    owner: root:root
    permissions: '0644'
    content: |
      minlen = 10
      ucredit = -1
      dcredit = -1
      lcredit = -1
      maxrepeat = 3
      reject_username
      difok = 7
      enforce_for_root

# --- 起動時コマンド ---
runcmd:
  # sudo ログディレクトリ
  - mkdir -p /var/log/sudo

  # login.defs
  - sed -i 's/^PASS_MAX_DAYS.*/PASS_MAX_DAYS\t30/' /etc/login.defs
  - sed -i 's/^PASS_MIN_DAYS.*/PASS_MIN_DAYS\t2/' /etc/login.defs
  - sed -i 's/^PASS_WARN_AGE.*/PASS_WARN_AGE\t7/' /etc/login.defs

  # PAM 設定
  - >-
    sed -i '/pam_pwquality/c\password requisite pam_pwquality.so retry=3
    minlen=10 ucredit=-1 dcredit=-1 lcredit=-1 maxrepeat=3
    reject_username difok=7 enforce_for_root' /etc/pam.d/common-password

  # 既存ユーザーへのポリシー適用
  - chage -M 30 -m 2 -W 7 kaztakam
  - chage -M 30 -m 2 -W 7 root

  # UFW
  - ufw default deny incoming
  - ufw default allow outgoing
  - ufw allow 4242/tcp
  - ufw --force enable

  # SSH
  - systemctl restart sshd

  # AppArmor
  - systemctl enable apparmor
  - systemctl start apparmor

  # cron
  - "(crontab -l 2>/dev/null; echo '*/10 * * * * /usr/local/bin/monitoring.sh') | crontab -"

  # user42 グループ（念のため）
  - groupadd -f user42

final_message: "Born2beRoot cloud-init completed in $UPTIME seconds."
```

### 9.3 cloud-init の検証

```bash
# 構文チェック
cloud-init schema --config-file cloud-init.yaml

# ログの確認
cat /var/log/cloud-init.log
cat /var/log/cloud-init-output.log

# ステータス確認
cloud-init status
cloud-init status --long

# 再実行（テスト用）
sudo cloud-init clean && sudo cloud-init init
```

---

## 10. 手動セットアップ vs IaC の詳細比較

| 作業 | 手動 | IaC (Terraform + Ansible/cloud-init) |
|------|------|--------------------------------------|
| VM 作成 | VirtualBox GUI | `resource "aws_instance"` |
| パーティション | インストーラ画面 | cloud-init / Preseed |
| LUKS 暗号化 | パスフレーズ入力 | `encrypted = true` |
| LVM 設定 | インストーラで手動 | `for_each` でボリューム定義 |
| ユーザー作成 | `adduser` | cloud-init `users` / Ansible `user` |
| SSH 設定 | `nano sshd_config` | cloud-init `write_files` / Ansible `lineinfile` |
| UFW 設定 | `ufw allow` | Security Group + Ansible `ufw` |
| パスワードポリシー | 複数ファイル編集 | Ansible task / cloud-init `runcmd` |
| sudo 設定 | `visudo` | cloud-init `write_files` |
| monitoring.sh | 手動配置 + crontab | cloud-init / Ansible `copy` + `cron` |
| 再現時間 | 1-3 時間 | 5-15 分（コード作成後） |
| エラー率 | 高い | 低い |
| スケーリング | 台数 x 作業時間 | ほぼ一定 |
| ドリフト検出 | 手動確認 | `terraform plan` |
| ロールバック | 困難 | `git revert` + `apply` |

---

## 11. 学習の意義と発展

### 11.1 Born2beRoot から実務へ

| Born2beRoot の学習 | 実務での応用 |
|-------------------|-------------|
| VM の手動作成 | Terraform による自動作成 |
| パーティションの手動設定 | クラウドの disk リソース定義 |
| SSH の手動設定 | Ansible による一括管理 |
| UFW の手動設定 | Security Group / Network Policy |
| パスワードポリシーの手動設定 | PAM モジュールの自動配布 |
| monitoring.sh の手動配置 | Prometheus + Grafana |
| cron の手動設定 | systemd timer / K8s CronJob |
| LUKS 暗号化 | AWS EBS / GCP Disk Encryption |
| AppArmor | SELinux / Pod Security |

### 11.2 学習の意義

Born2beRoot を手動で行うことには重要な学習上の意義がある:

1. **基礎の理解**: IaC を使う前に、裏で何が行われているかを理解する必要がある
2. **トラブルシューティング**: 自動化が失敗した時に手動で修正できる能力
3. **セキュリティの意識**: 各設定がなぜ必要かを理解する
4. **設計判断**: 何を自動化し、何を手動にするかの判断力
5. **抽象化の理解**: クラウドサービスが「裏で何をしているか」の直感
6. **デバッグ能力**: cloud-init や Ansible が失敗した時に手動で同じ操作ができる

### 11.3 IaC のベストプラクティス

| プラクティス | 説明 |
|-------------|------|
| モジュール化 | 再利用可能な単位に分割する |
| 環境の分離 | dev / staging / prod を明確に分ける |
| ステートの保護 | リモートバックエンド + 暗号化 + ロック |
| コードレビュー | `terraform plan` の結果をレビューに含める |
| 自動テスト | validate, tflint, terratest で品質担保 |
| バージョン固定 | Provider と Terraform のバージョンを固定 |
| シークレット管理 | Vault, AWS Secrets Manager で機密情報管理 |
| Policy as Code | Sentinel / OPA でポリシーを強制 |
| 命名規則 | 一貫した命名規則を適用 |
| ドキュメント | description の充実、terraform-docs |

### 11.4 発展的なツール

```
Terraform エコシステム:
├── tflint        → 静的解析、ベストプラクティスのチェック
├── checkov       → セキュリティスキャン
├── terraform-docs → ドキュメント自動生成
├── terratest     → Go による統合テスト
├── Terragrunt    → DRY な Terraform（ラッパーツール）
├── Infracost     → コスト推定
├── Atlantis      → PR ベースの Terraform 自動化
└── Spacelift     → Terraform の CI/CD プラットフォーム
```

---

## 12. Workspace とライフサイクル管理

### 12.1 Terraform Workspace

Workspace は、同じ Terraform コードを異なる環境（dev / staging / production）に適用する仕組みである。Workspace ごとに独立したステートファイルが管理される。

```bash
# Workspace の操作
terraform workspace list          # Workspace の一覧（* が現在の Workspace）
terraform workspace new dev       # 新しい Workspace を作成
terraform workspace new staging
terraform workspace new prod
terraform workspace select dev    # Workspace を切り替え
terraform workspace show          # 現在の Workspace を表示
terraform workspace delete dev    # Workspace を削除
```

```
Workspace の仕組み:

ローカルバックエンド:
  terraform.tfstate.d/
  ├── dev/
  │   └── terraform.tfstate
  ├── staging/
  │   └── terraform.tfstate
  └── prod/
      └── terraform.tfstate

S3 バックエンド（key = "born2beroot/terraform.tfstate" の場合）:
  s3://my-bucket/
  ├── env:/dev/born2beroot/terraform.tfstate
  ├── env:/staging/born2beroot/terraform.tfstate
  └── env:/prod/born2beroot/terraform.tfstate
```

```hcl
# Workspace をコード内で参照する
resource "aws_instance" "born2beroot" {
  ami           = data.aws_ami.debian.id
  instance_type = terraform.workspace == "prod" ? "t3.medium" : "t3.micro"

  tags = {
    Name        = "born2beroot-${terraform.workspace}"
    Environment = terraform.workspace
  }
}

# Workspace ごとの変数設定
locals {
  environment_config = {
    dev = {
      instance_type = "t3.micro"
      disk_size     = 20
      monitoring    = false
      backup        = false
    }
    staging = {
      instance_type = "t3.small"
      disk_size     = 30
      monitoring    = true
      backup        = false
    }
    prod = {
      instance_type = "t3.medium"
      disk_size     = 50
      monitoring    = true
      backup        = true
    }
  }

  config = local.environment_config[terraform.workspace]
}

resource "aws_instance" "born2beroot" {
  instance_type = local.config.instance_type

  root_block_device {
    volume_size = local.config.disk_size
    encrypted   = true
  }
}
```

#### Workspace vs ディレクトリ分離

Workspace 以外にも環境分離の方法がある。それぞれに長所と短所がある:

| アプローチ | 構成 | 長所 | 短所 |
|-----------|------|------|------|
| Workspace | 1つのディレクトリ | コードの重複なし、切り替えが簡単 | 環境間の差異が大きい場合に複雑化 |
| ディレクトリ分離 | env/dev/, env/prod/ | 環境ごとに独立、明示的 | コードの重複、同期コスト |
| Terragrunt | DRY な構成 | コード重複なし、柔軟 | 追加ツールの学習コスト |

```
ディレクトリ分離パターン:

environments/
├── dev/
│   ├── main.tf        # module "born2beroot" { source = "../../modules" }
│   ├── variables.tf
│   ├── terraform.tfvars  # dev 固有の値
│   └── backend.tf        # dev 用のステートバックエンド
├── staging/
│   ├── main.tf        # module "born2beroot" { source = "../../modules" }
│   ├── variables.tf
│   ├── terraform.tfvars  # staging 固有の値
│   └── backend.tf        # staging 用のステートバックエンド
└── prod/
    ├── main.tf        # module "born2beroot" { source = "../../modules" }
    ├── variables.tf
    ├── terraform.tfvars  # prod 固有の値
    └── backend.tf        # prod 用のステートバックエンド

modules/
├── born2beroot/       # 共通モジュール
│   ├── main.tf
│   ├── variables.tf
│   └── outputs.tf
```

### 12.2 Lifecycle ルール

Terraform のリソースには `lifecycle` ブロックで挙動を制御できる。これはインフラ管理において非常に重要な機能である。

```hcl
# =============================================================================
# create_before_destroy
# =============================================================================
# デフォルトでは Terraform は古いリソースを削除してから新しいリソースを作成する。
# create_before_destroy を true にすると、新しいリソースを先に作成してから
# 古いリソースを削除する（ゼロダウンタイムデプロイ）。

resource "aws_instance" "born2beroot" {
  ami           = data.aws_ami.debian.id
  instance_type = "t3.micro"

  lifecycle {
    create_before_destroy = true
  }
}
# → AMI が変更された場合:
#   1. 新しい AMI で新インスタンスを作成
#   2. 新インスタンスが正常に起動したことを確認
#   3. 古いインスタンスを削除
# → ダウンタイムが最小化される

# =============================================================================
# prevent_destroy
# =============================================================================
# 重要なリソースの誤削除を防止する。terraform destroy を実行しても
# このリソースは削除されない（エラーが発生する）。

resource "aws_ebs_volume" "important_data" {
  availability_zone = "ap-northeast-1a"
  size              = 100
  encrypted         = true

  lifecycle {
    prevent_destroy = true
  }
}
# → terraform destroy 実行時:
#   Error: Instance cannot be destroyed
#   Resource aws_ebs_volume.important_data has lifecycle.prevent_destroy set,
#   but the plan calls for this resource to be destroyed.
# → コードから lifecycle ブロックを削除してから destroy する必要がある

# =============================================================================
# ignore_changes
# =============================================================================
# 特定の属性の変更を無視する。外部から変更される属性に対して使用する。

resource "aws_instance" "born2beroot" {
  ami           = data.aws_ami.debian.id
  instance_type = "t3.micro"

  tags = {
    Name = "born2beroot"
  }

  lifecycle {
    # AWS コンソールから手動でタグを追加しても Terraform は無視する
    ignore_changes = [tags]
  }
}

resource "aws_security_group" "born2beroot" {
  name   = "born2beroot-sg"
  vpc_id = var.vpc_id

  lifecycle {
    # セキュリティグループのルールを外部ツール（AWS WAF など）が管理する場合
    ignore_changes = [ingress, egress]
  }
}

# 全ての変更を無視（Terraform 管理から実質的に除外）
resource "aws_instance" "legacy" {
  ami           = "ami-existing"
  instance_type = "t3.micro"

  lifecycle {
    ignore_changes = all
  }
}

# =============================================================================
# replace_triggered_by
# =============================================================================
# 指定したリソースや属性が変更されたとき、このリソースを強制的に再作成する。
# Terraform 1.2 以降で利用可能。

resource "aws_instance" "born2beroot" {
  ami           = data.aws_ami.debian.id
  instance_type = "t3.micro"
  user_data     = file("scripts/cloud-init.yaml")

  lifecycle {
    # cloud-init の設定が変更されたらインスタンスを再作成
    replace_triggered_by = [
      null_resource.cloud_init_hash
    ]
  }
}

resource "null_resource" "cloud_init_hash" {
  triggers = {
    cloud_init = filemd5("scripts/cloud-init.yaml")
  }
}

# =============================================================================
# precondition / postcondition
# =============================================================================
# Terraform 1.2 以降で利用可能。リソースの作成前後に条件をチェックする。

resource "aws_instance" "born2beroot" {
  ami           = data.aws_ami.debian.id
  instance_type = var.instance_type
  subnet_id     = var.subnet_id

  lifecycle {
    precondition {
      condition     = data.aws_ami.debian.architecture == "x86_64"
      error_message = "AMI は x86_64 アーキテクチャである必要があります。"
    }

    postcondition {
      condition     = self.public_ip != ""
      error_message = "パブリック IP が割り当てられませんでした。サブネットの設定を確認してください。"
    }
  }
}
```

### 12.3 Moved ブロック（リファクタリング支援）

Terraform 1.1 以降で導入された `moved` ブロックは、リソースの名前変更やモジュールへの移動時にステートを自動的に更新する。

```hcl
# リソース名の変更
# before: resource "aws_instance" "web" { ... }
# after:  resource "aws_instance" "born2beroot" { ... }

moved {
  from = aws_instance.web
  to   = aws_instance.born2beroot
}
# → terraform plan 実行時:
#   # aws_instance.web has moved to aws_instance.born2beroot
#   → No changes. Your infrastructure matches the configuration.
# → リソースは削除・再作成されない！

# モジュールへの移動
# before: resource "aws_security_group" "sg" { ... }（ルートモジュール内）
# after:  module.security 内に移動

moved {
  from = aws_security_group.sg
  to   = module.security.aws_security_group.born2beroot
}

# for_each への移行
# before: resource "aws_ebs_volume" "data" { count = 3, ... }
# after:  resource "aws_ebs_volume" "data" { for_each = var.volumes, ... }

moved {
  from = aws_ebs_volume.data[0]
  to   = aws_ebs_volume.data["home"]
}
moved {
  from = aws_ebs_volume.data[1]
  to   = aws_ebs_volume.data["var"]
}
moved {
  from = aws_ebs_volume.data[2]
  to   = aws_ebs_volume.data["tmp"]
}
```

---

## 13. Import と State 操作の詳細

### 13.1 terraform import

既存のリソース（手動で作成されたものや別のツールで管理されていたもの）を Terraform の管理下に取り込む。Born2beRoot で手動構築した環境を後から Terraform 管理に移行する場合に必要になる。

```bash
# 基本構文
terraform import <RESOURCE_ADDRESS> <RESOURCE_ID>

# AWS EC2 インスタンスのインポート
terraform import aws_instance.born2beroot i-0123456789abcdef0

# セキュリティグループのインポート
terraform import aws_security_group.born2beroot sg-0123456789abcdef0

# VPC のインポート
terraform import aws_vpc.main vpc-0123456789abcdef0

# GCP のインスタンスのインポート
terraform import google_compute_instance.born2beroot \
  projects/my-project/zones/asia-northeast1-a/instances/born2beroot
```

#### import の手順

```
手動リソースを Terraform 管理に移行する手順:

1. 既存リソースを確認
   aws ec2 describe-instances --instance-ids i-0123456789abcdef0

2. Terraform コードを書く（リソース定義）
   resource "aws_instance" "born2beroot" {
     ami           = "ami-..."
     instance_type = "t3.micro"
     # ... 既存の設定に合わせる
   }

3. インポート実行
   terraform import aws_instance.born2beroot i-0123456789abcdef0

4. Plan で差分を確認
   terraform plan
   # → 差分がゼロになるまでコードを調整する

5. 差分がなくなれば完了
   # 以降は Terraform で管理される
```

### 13.2 Import ブロック（Terraform 1.5+）

Terraform 1.5 以降では、コード内に `import` ブロックを記述できるようになった。これにより import 操作もコードレビューの対象にできる。

```hcl
# import ブロックの定義
import {
  to = aws_instance.born2beroot
  id = "i-0123456789abcdef0"
}

import {
  to = aws_security_group.born2beroot
  id = "sg-0123456789abcdef0"
}

# terraform plan で確認
# → import されるリソースと既存コードの差分が表示される

# terraform apply で import を実行
# → ステートにリソースが追加される
```

#### コード自動生成（Terraform 1.5+）

```bash
# import 対象のリソースの HCL コードを自動生成
terraform plan -generate-config-out=generated.tf

# → generated.tf に import 対象のリソースの HCL コードが生成される
# → 生成されたコードを確認・調整してから apply する
```

### 13.3 State 操作の詳細

```bash
# =============================================================================
# terraform state list - 管理リソースの一覧
# =============================================================================
$ terraform state list
aws_instance.born2beroot
aws_security_group.born2beroot
aws_vpc.main
aws_subnet.public
aws_ebs_volume.lvm["home"]
aws_ebs_volume.lvm["var"]
module.security.aws_security_group.born2beroot

# フィルタリング
$ terraform state list aws_ebs_volume.lvm
aws_ebs_volume.lvm["home"]
aws_ebs_volume.lvm["var"]
aws_ebs_volume.lvm["tmp"]

# =============================================================================
# terraform state show - リソースの詳細
# =============================================================================
$ terraform state show aws_instance.born2beroot
# aws_instance.born2beroot:
resource "aws_instance" "born2beroot" {
    ami                          = "ami-0123456789abcdef0"
    arn                          = "arn:aws:ec2:ap-northeast-1:123456789012:instance/i-01234567"
    associate_public_ip_address  = true
    availability_zone            = "ap-northeast-1a"
    cpu_core_count               = 1
    cpu_threads_per_core         = 2
    id                           = "i-0123456789abcdef0"
    instance_state               = "running"
    instance_type                = "t3.micro"
    private_ip                   = "10.0.1.50"
    public_ip                    = "54.250.100.200"
    ...
}

# =============================================================================
# terraform state mv - リソースの移動・名前変更
# =============================================================================
# リソースの名前変更
terraform state mv aws_instance.web aws_instance.born2beroot

# モジュール内への移動
terraform state mv aws_security_group.sg module.security.aws_security_group.main

# モジュール全体の移動
terraform state mv module.old_security module.new_security

# for_each のキー変更
terraform state mv 'aws_ebs_volume.lvm["var_log"]' 'aws_ebs_volume.lvm["var-log"]'

# =============================================================================
# terraform state rm - ステートからの除外
# =============================================================================
# Terraform の管理から除外するが、実際のリソースは削除しない
terraform state rm aws_instance.born2beroot
# → 次の plan では「新規作成」と表示されるが、
#   実際の AWS 上のインスタンスはそのまま残る

# 使用例: 別の Terraform プロジェクトに移管する場合
#   1. プロジェクト A でステートから除外
#   2. プロジェクト B でインポート

# =============================================================================
# terraform state pull / push - リモートステートの操作
# =============================================================================
# リモートステートをローカルにダウンロード
terraform state pull > terraform.tfstate.backup

# ローカルステートをリモートにアップロード（危険！通常は使わない）
terraform state push terraform.tfstate

# =============================================================================
# terraform state replace-provider - Provider の変更
# =============================================================================
# Provider のレジストリが変更された場合
terraform state replace-provider \
  registry.terraform.io/hashicorp/aws \
  registry.terraform.io/alternative/aws
```

### 13.4 terraform taint と replace

```bash
# taint は非推奨（Terraform 0.15.2 以降）
# 代わりに -replace オプションを使用する

# 特定のリソースを次の apply で強制的に再作成
terraform plan -replace=aws_instance.born2beroot
# → Plan 出力:
#   # aws_instance.born2beroot will be replaced, as requested
#   -/+ resource "aws_instance" "born2beroot" { ... }

terraform apply -replace=aws_instance.born2beroot

# 使用例:
# - cloud-init の変更が反映されない場合（user_data の変更では再作成されないことがある）
# - インスタンスが不安定な状態になった場合
# - セキュリティパッチ適用後にクリーンな状態で再起動したい場合
```

---

## 14. Terraform テストフレームワーク

### 14.1 terraform test（Terraform 1.6+）

Terraform 1.6 以降では、組み込みのテストフレームワークが利用可能になった。

```hcl
# tests/born2beroot.tftest.hcl

# =============================================================================
# 変数のバリデーションテスト
# =============================================================================
run "validate_ssh_port_range" {
  command = plan

  variables {
    ssh_port = 22  # 1024 未満は無効
  }

  expect_failures = [
    var.ssh_port
  ]
}

run "validate_valid_ssh_port" {
  command = plan

  variables {
    ssh_port = 4242
  }

  # エラーが発生しないことを確認
}

# =============================================================================
# Plan レベルのテスト（実際にリソースを作成しない）
# =============================================================================
run "security_group_allows_only_ssh" {
  command = plan

  assert {
    condition     = length(aws_security_group.born2beroot.ingress) == 1
    error_message = "セキュリティグループのインバウンドルールは SSH のみであるべきです。"
  }

  assert {
    condition     = aws_security_group.born2beroot.ingress[0].from_port == 4242
    error_message = "SSH ポートは 4242 であるべきです。"
  }
}

run "instance_uses_encrypted_volume" {
  command = plan

  assert {
    condition     = aws_instance.born2beroot.root_block_device[0].encrypted == true
    error_message = "ルートボリュームは暗号化されているべきです（LUKS に相当）。"
  }
}

# =============================================================================
# Apply レベルのテスト（実際にリソースを作成して検証）
# =============================================================================
run "full_integration_test" {
  command = apply

  variables {
    ssh_port      = 4242
    instance_type = "t3.micro"
    disk_size_gb  = 30
  }

  assert {
    condition     = output.public_ip != ""
    error_message = "パブリック IP が割り当てられていません。"
  }

  assert {
    condition     = output.ssh_command != ""
    error_message = "SSH コマンドが生成されていません。"
  }
}
```

```bash
# テストの実行
terraform test

# 詳細出力
terraform test -verbose

# 特定のテストファイルのみ実行
terraform test -filter=tests/born2beroot.tftest.hcl
```

### 14.2 terratest（Go ベースの統合テスト）

```go
// tests/born2beroot_test.go
package test

import (
    "fmt"
    "testing"
    "time"

    "github.com/gruntwork-io/terratest/modules/terraform"
    "github.com/gruntwork-io/terratest/modules/ssh"
    "github.com/gruntwork-io/terratest/modules/retry"
    "github.com/stretchr/testify/assert"
)

func TestBorn2beRootInfrastructure(t *testing.T) {
    t.Parallel()

    terraformOptions := terraform.WithDefaultRetryableErrors(t, &terraform.Options{
        TerraformDir: "../",
        Vars: map[string]interface{}{
            "ssh_port":      4242,
            "instance_type": "t3.micro",
            "disk_size_gb":  30,
            "environment":   "test",
        },
    })

    // テスト終了後にリソースを削除
    defer terraform.Destroy(t, terraformOptions)

    // Terraform Apply
    terraform.InitAndApply(t, terraformOptions)

    // Output の取得
    publicIP := terraform.Output(t, terraformOptions, "public_ip")
    sshCommand := terraform.Output(t, terraformOptions, "ssh_command")

    // パブリック IP が割り当てられていることを確認
    assert.NotEmpty(t, publicIP)
    assert.Contains(t, sshCommand, "4242")

    // SSH 接続テスト（ポート 4242）
    sshHost := ssh.Host{
        Hostname:    publicIP,
        SshUserName: "kaztakam",
        SshKeyPair:  loadKeyPair(t),
    }

    // SSH が起動するまで待機（最大 5 分）
    retry.DoWithRetry(t, "SSH connection test", 30, 10*time.Second, func() (string, error) {
        return "", ssh.CheckSshConnectionE(t, sshHost)
    })

    // SSH ポートが 4242 であることを確認
    output := ssh.CheckSshCommand(t, sshHost, "ss -tlnp | grep sshd")
    assert.Contains(t, output, ":4242")

    // root ログインが禁止されていることを確認
    output = ssh.CheckSshCommand(t, sshHost, "grep '^PermitRootLogin' /etc/ssh/sshd_config")
    assert.Contains(t, output, "no")

    // UFW が有効であることを確認
    output = ssh.CheckSshCommand(t, sshHost, "sudo ufw status")
    assert.Contains(t, output, "Status: active")
    assert.Contains(t, output, "4242")

    // AppArmor が有効であることを確認
    output = ssh.CheckSshCommand(t, sshHost, "sudo aa-status")
    assert.Contains(t, output, "apparmor module is loaded")

    // パスワードポリシーが設定されていることを確認
    output = ssh.CheckSshCommand(t, sshHost, "grep PASS_MAX_DAYS /etc/login.defs")
    assert.Contains(t, output, "30")

    // sudo ログが設定されていることを確認
    output = ssh.CheckSshCommand(t, sshHost, "cat /etc/sudoers.d/sudo_config")
    assert.Contains(t, output, "logfile")
    assert.Contains(t, output, "requiretty")

    // monitoring.sh が配置されていることを確認
    output = ssh.CheckSshCommand(t, sshHost, "ls -la /usr/local/bin/monitoring.sh")
    assert.Contains(t, output, "-rwxr-xr-x")

    // cron が設定されていることを確認
    output = ssh.CheckSshCommand(t, sshHost, "crontab -l")
    assert.Contains(t, output, "monitoring.sh")
}
```

```bash
# terratest の実行
cd tests
go test -v -timeout 30m
```

### 14.3 その他のテスト・検証ツール

```bash
# =============================================================================
# tflint - 静的解析
# =============================================================================
# インストール
curl -s https://raw.githubusercontent.com/terraform-linters/tflint/master/install_linux.sh | bash

# 設定ファイル
cat > .tflint.hcl << 'EOF'
plugin "aws" {
  enabled = true
  version = "0.27.0"
  source  = "github.com/terraform-linters/tflint-ruleset-aws"
}

rule "terraform_naming_convention" {
  enabled = true
}

rule "terraform_documented_outputs" {
  enabled = true
}

rule "terraform_documented_variables" {
  enabled = true
}
EOF

# 実行
tflint --init
tflint

# 出力例:
# 3 issue(s) found:
# Warning: "t2.micro" is previous generation instance type (aws_instance_previous_type)
# Warning: Missing version constraint for provider "aws" (terraform_required_providers)
# Notice: variable "hostname" should have a description (terraform_documented_variables)

# =============================================================================
# checkov - セキュリティスキャン
# =============================================================================
pip install checkov

# 実行
checkov -d .

# 出力例:
# Passed checks: 15, Failed checks: 3, Skipped checks: 0
# Check: CKV_AWS_135: "Ensure that EC2 is EBS optimized"
#   FAILED for resource: aws_instance.born2beroot
# Check: CKV_AWS_88: "EC2 instance should not have public IP"
#   FAILED for resource: aws_instance.born2beroot

# 特定のチェックをスキップ
checkov -d . --skip-check CKV_AWS_88

# =============================================================================
# terraform-docs - ドキュメント自動生成
# =============================================================================
# インストール
brew install terraform-docs

# Markdown 形式でドキュメント生成
terraform-docs markdown table . > README.md

# 生成例:
# ## Requirements
# | Name | Version |
# |------|---------|
# | terraform | >= 1.6.0 |
# | aws | ~> 5.0 |
#
# ## Inputs
# | Name | Description | Type | Default | Required |
# |------|-------------|------|---------|:--------:|
# | ssh_port | SSH ポート番号 | `number` | `4242` | no |
```

---

## 15. Policy as Code の詳細

### 15.1 Policy as Code とは

Policy as Code は、組織のセキュリティポリシーやコンプライアンス要件をコードとして定義し、Terraform の plan/apply 時に自動的に検証する仕組みである。

```
Policy as Code のフロー:

コード変更 → terraform plan → ポリシーチェック → 合格 → terraform apply
                                    ↓
                                  不合格 → エラー（apply をブロック）
```

### 15.2 Sentinel（HashiCorp 公式）

Sentinel は Terraform Cloud/Enterprise で利用可能なポリシー言語。

```python
# policy/born2beroot-security.sentinel
# Born2beRoot のセキュリティ要件をポリシーとして定義

import "tfplan/v2" as tfplan

# ルール 1: EC2 インスタンスのルートボリュームは暗号化必須（LUKS に相当）
main = rule {
    all tfplan.resource_changes as _, rc {
        rc.type is "aws_instance" implies
        rc.change.after.root_block_device[0].encrypted is true
    }
}

# ルール 2: セキュリティグループで SSH (22) が全世界に開放されていないこと
deny_ssh_22_open = rule {
    all tfplan.resource_changes as _, rc {
        rc.type is "aws_security_group" implies
        all rc.change.after.ingress as ingress {
            not (ingress.from_port is 22 and
                 "0.0.0.0/0" in ingress.cidr_blocks)
        }
    }
}

# ルール 3: 承認されたインスタンスタイプのみ使用可能
allowed_types = ["t3.micro", "t3.small", "t3.medium"]
enforce_instance_type = rule {
    all tfplan.resource_changes as _, rc {
        rc.type is "aws_instance" implies
        rc.change.after.instance_type in allowed_types
    }
}
```

### 15.3 Open Policy Agent (OPA) / Conftest

OPA は OSS のポリシーエンジン。Rego という言語でポリシーを記述する。Conftest は OPA をベースにした設定ファイルの検証ツール。

```rego
# policy/born2beroot.rego
package born2beroot

import future.keywords.in
import future.keywords.if

# terraform plan -out=tfplan && terraform show -json tfplan > plan.json

# ルール 1: EBS ボリュームは暗号化必須
deny[msg] if {
    resource := input.resource_changes[_]
    resource.type == "aws_instance"
    root_device := resource.change.after.root_block_device[0]
    not root_device.encrypted
    msg := sprintf(
        "EC2 インスタンス '%s' のルートボリュームが暗号化されていません（LUKS に相当する暗号化が必要）",
        [resource.name]
    )
}

# ルール 2: セキュリティグループで SSH ポート 22 が開放されていないこと
deny[msg] if {
    resource := input.resource_changes[_]
    resource.type == "aws_security_group"
    ingress := resource.change.after.ingress[_]
    ingress.from_port == 22
    "0.0.0.0/0" in ingress.cidr_blocks
    msg := sprintf(
        "セキュリティグループ '%s' で SSH ポート 22 が全世界に開放されています（Born2beRoot ではポート %d を使用）",
        [resource.name, 4242]
    )
}

# ルール 3: 必須タグのチェック
required_tags := {"Project", "ManagedBy", "Environment"}

deny[msg] if {
    resource := input.resource_changes[_]
    resource.type == "aws_instance"
    tags := resource.change.after.tags
    missing := required_tags - {key | tags[key]}
    count(missing) > 0
    msg := sprintf(
        "EC2 インスタンス '%s' に必須タグが不足: %v",
        [resource.name, missing]
    )
}

# ルール 4: SSH ポートが 4242 であることを確認
deny[msg] if {
    resource := input.resource_changes[_]
    resource.type == "aws_security_group"
    ingress := resource.change.after.ingress[_]
    ingress.description == "SSH"
    ingress.from_port != 4242
    msg := sprintf(
        "セキュリティグループ '%s' の SSH ポートが 4242 ではありません（Born2beRoot 要件違反）",
        [resource.name]
    )
}
```

```bash
# Conftest でポリシーチェック
terraform plan -out=tfplan
terraform show -json tfplan > plan.json

conftest test plan.json --policy policy/

# 出力例:
# FAIL - plan.json - born2beroot - EC2 インスタンス 'web' のルートボリュームが暗号化されていません
# PASS - plan.json - born2beroot - セキュリティグループ SSH ポートチェック
# 2 tests, 1 passed, 1 warning, 1 failure
```

### 15.4 CI/CD パイプラインへのポリシー統合

```yaml
# .github/workflows/terraform-policy.yml
name: Terraform Policy Check

on:
  pull_request:
    paths: ['terraform/**']

jobs:
  policy-check:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Terraform
        uses: hashicorp/setup-terraform@v3

      - name: Terraform Init & Plan
        run: |
          terraform init
          terraform plan -out=tfplan
          terraform show -json tfplan > plan.json
        working-directory: terraform

      - name: Install Conftest
        run: |
          LATEST=$(curl -s https://api.github.com/repos/open-policy-agent/conftest/releases/latest | jq -r .tag_name | sed 's/v//')
          curl -Lo conftest.tar.gz "https://github.com/open-policy-agent/conftest/releases/download/v${LATEST}/conftest_${LATEST}_Linux_x86_64.tar.gz"
          tar xzf conftest.tar.gz
          sudo mv conftest /usr/local/bin/

      - name: Run Policy Check
        run: conftest test terraform/plan.json --policy policy/ --output json

      - name: Checkov Security Scan
        uses: bridgecrewio/checkov-action@master
        with:
          directory: terraform/
          framework: terraform
          output_format: github_failed_only
```

---

## 16. Packer との連携

### 16.1 Packer とは

Packer は HashiCorp 製のマシンイメージ作成ツール。Born2beRoot の設定済み VM イメージを事前にベイク（bake）しておくことで、Terraform でのデプロイ時間を短縮できる。

```
Packer のワークフロー:

Debian 12 ベースイメージ
  → Packer がプロビジョニング実行
    → SSH 設定
    → UFW 設定
    → パスワードポリシー設定
    → sudo 設定
    → monitoring.sh 配置
    → AppArmor 設定
  → Born2beRoot 設定済みイメージ（AMI / GCE Image）
    → Terraform がこのイメージからインスタンスを起動
    → cloud-init は最小限の設定（ホスト名、ユーザー）のみ
```

### 16.2 Packer テンプレート

```hcl
# born2beroot.pkr.hcl

packer {
  required_plugins {
    amazon = {
      version = ">= 1.2.0"
      source  = "github.com/hashicorp/amazon"
    }
    ansible = {
      version = ">= 1.1.0"
      source  = "github.com/hashicorp/ansible"
    }
  }
}

# 変数定義
variable "aws_region" {
  type    = string
  default = "ap-northeast-1"
}

variable "ssh_port" {
  type    = number
  default = 4242
}

variable "version" {
  type    = string
  default = "1.0.0"
}

# ソースイメージの定義
source "amazon-ebs" "born2beroot" {
  ami_name      = "born2beroot-${var.version}-{{timestamp}}"
  instance_type = "t3.micro"
  region        = var.aws_region

  source_ami_filter {
    filters = {
      name                = "debian-12-amd64-*"
      root-device-type    = "ebs"
      virtualization-type = "hvm"
    }
    most_recent = true
    owners      = ["136693071363"]  # Debian 公式
  }

  ssh_username = "admin"

  # AMI の暗号化
  encrypt_boot = true

  tags = {
    Name        = "Born2beRoot"
    Version     = var.version
    BaseOS      = "Debian 12"
    Builder     = "Packer"
    BuildDate   = "{{timestamp}}"
  }

  # 古い AMI の自動削除（最新 3 世代を保持）
  ami_description = "Born2beRoot pre-configured image v${var.version}"
}

# ビルド手順
build {
  sources = ["source.amazon-ebs.born2beroot"]

  # シェルスクリプトで基本設定
  provisioner "shell" {
    inline = [
      "sudo apt-get update",
      "sudo apt-get upgrade -y",
      "sudo apt-get install -y ufw libpam-pwquality apparmor apparmor-utils sudo openssh-server vim cron",
    ]
  }

  # Ansible で詳細設定
  provisioner "ansible" {
    playbook_file = "ansible/playbook.yml"
    extra_arguments = [
      "--extra-vars", "ssh_port=${var.ssh_port}",
      "--extra-vars", "packer_build=true",
    ]
  }

  # イメージの検証
  provisioner "shell" {
    inline = [
      "echo '=== Born2beRoot Image Validation ==='",
      "sudo ufw status | grep -q 'Status: active' && echo 'UFW: OK' || echo 'UFW: FAIL'",
      "sudo aa-status | grep -q 'apparmor module is loaded' && echo 'AppArmor: OK' || echo 'AppArmor: FAIL'",
      "grep -q 'PASS_MAX_DAYS.*30' /etc/login.defs && echo 'Password Policy: OK' || echo 'Password Policy: FAIL'",
      "grep -q 'Port ${var.ssh_port}' /etc/ssh/sshd_config.d/*.conf && echo 'SSH Port: OK' || echo 'SSH Port: FAIL'",
      "test -x /usr/local/bin/monitoring.sh && echo 'Monitoring: OK' || echo 'Monitoring: FAIL'",
    ]
  }

  # クリーンアップ
  provisioner "shell" {
    inline = [
      "sudo apt-get clean",
      "sudo rm -rf /var/lib/apt/lists/*",
      "sudo rm -rf /tmp/*",
      "sudo rm -f /root/.bash_history",
      "history -c",
    ]
  }
}
```

```bash
# Packer ビルド
packer init born2beroot.pkr.hcl
packer validate born2beroot.pkr.hcl
packer build born2beroot.pkr.hcl

# 特定のバージョンでビルド
packer build -var 'version=1.1.0' born2beroot.pkr.hcl
```

### 16.3 Packer + Terraform の連携

```hcl
# Terraform で Packer が作成したイメージを使用
data "aws_ami" "born2beroot_packer" {
  most_recent = true
  owners      = ["self"]  # 自分のアカウントの AMI

  filter {
    name   = "name"
    values = ["born2beroot-*"]
  }

  filter {
    name   = "tag:Builder"
    values = ["Packer"]
  }
}

resource "aws_instance" "born2beroot" {
  # Packer で作成済みのイメージを使用
  ami           = data.aws_ami.born2beroot_packer.id
  instance_type = "t3.micro"

  # cloud-init は最小限（ホスト名とユーザーの個別設定のみ）
  user_data = <<-EOT
    #cloud-config
    hostname: ${var.hostname}
    users:
      - name: ${var.username}
        groups: [sudo, user42]
        ssh_authorized_keys:
          - ${var.ssh_public_key}
  EOT

  tags = {
    Name    = "born2beroot"
    AMI     = data.aws_ami.born2beroot_packer.id
    Version = data.aws_ami.born2beroot_packer.tags["Version"]
  }
}
```

```
Packer + Terraform の利点:

Without Packer（cloud-init のみ）:
  Terraform Apply → インスタンス起動 → cloud-init 実行（5-10分）
  → パッケージダウンロード、設定適用、サービス再起動
  → 全ての設定が完了するまで SSH 接続不可

With Packer:
  事前: Packer Build → 設定済みイメージ作成（15-30分、1回だけ）
  運用: Terraform Apply → インスタンス起動（1-2分）
  → 既に全ての設定が適用済み
  → 即座に SSH 接続可能
```

---

## 17. マルチリージョン・マルチアカウント構成

### 17.1 マルチリージョン

```hcl
# 複数リージョンに Born2beRoot 環境を展開

# 東京リージョン
provider "aws" {
  alias  = "tokyo"
  region = "ap-northeast-1"
}

# 大阪リージョン（DR サイト）
provider "aws" {
  alias  = "osaka"
  region = "ap-northeast-3"
}

# 東京の Born2beRoot
module "born2beroot_tokyo" {
  source    = "./modules/born2beroot"
  providers = { aws = aws.tokyo }

  hostname      = "born2beroot-tokyo"
  instance_type = "t3.micro"
  ssh_port      = 4242
}

# 大阪の Born2beRoot（DR）
module "born2beroot_osaka" {
  source    = "./modules/born2beroot"
  providers = { aws = aws.osaka }

  hostname      = "born2beroot-osaka"
  instance_type = "t3.micro"
  ssh_port      = 4242
}

output "tokyo_ip" {
  value = module.born2beroot_tokyo.public_ip
}

output "osaka_ip" {
  value = module.born2beroot_osaka.public_ip
}
```

### 17.2 マルチアカウント（AWS Organizations）

```hcl
# セキュリティアカウントで監査ログを集約
provider "aws" {
  alias  = "security"
  region = "ap-northeast-1"

  assume_role {
    role_arn = "arn:aws:iam::${var.security_account_id}:role/TerraformRole"
  }
}

# 開発アカウントで Born2beRoot を実行
provider "aws" {
  alias  = "dev"
  region = "ap-northeast-1"

  assume_role {
    role_arn = "arn:aws:iam::${var.dev_account_id}:role/TerraformRole"
  }
}

# セキュリティアカウントに CloudTrail ログを保存
resource "aws_s3_bucket" "audit_logs" {
  provider = aws.security
  bucket   = "born2beroot-audit-logs"
}

# 開発アカウントに Born2beRoot をデプロイ
module "born2beroot" {
  source    = "./modules/born2beroot"
  providers = { aws = aws.dev }

  hostname = "born2beroot-dev"
}
```

---

## 18. セキュリティベストプラクティス

### 18.1 シークレット管理

```hcl
# 悪い例: ハードコードされたシークレット
resource "aws_instance" "bad" {
  user_data = <<-EOT
    DB_PASSWORD=mysecretpassword  # 絶対にやってはいけない
  EOT
}

# 良い例 1: 変数（環境変数から注入）
variable "db_password" {
  type      = string
  sensitive = true  # plan/apply の出力でマスクされる
}

# 良い例 2: AWS Secrets Manager
data "aws_secretsmanager_secret_version" "db_password" {
  secret_id = "born2beroot/db-password"
}

# 良い例 3: HashiCorp Vault
data "vault_generic_secret" "db" {
  path = "secret/born2beroot/db"
}

resource "aws_instance" "born2beroot" {
  user_data = templatefile("cloud-init.yaml", {
    db_password = data.aws_secretsmanager_secret_version.db_password.secret_string
  })
}
```

```bash
# sensitive 変数の値を渡す方法

# 環境変数（推奨）
export TF_VAR_db_password="mysecretpassword"
terraform apply

# .tfvars ファイル（.gitignore に追加必須）
echo 'db_password = "mysecretpassword"' > secrets.tfvars
terraform apply -var-file=secrets.tfvars

# CLI フラグ（コマンド履歴に残るため非推奨）
terraform apply -var='db_password=mysecretpassword'  # 非推奨
```

### 18.2 IAM の最小権限

```hcl
# Terraform 実行用の IAM ポリシー（最小権限の原則）
resource "aws_iam_policy" "terraform_born2beroot" {
  name        = "TerraformBorn2beRoot"
  description = "Born2beRoot の Terraform 実行に必要な最小権限"

  policy = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "EC2Management"
        Effect = "Allow"
        Action = [
          "ec2:RunInstances",
          "ec2:TerminateInstances",
          "ec2:DescribeInstances",
          "ec2:CreateSecurityGroup",
          "ec2:DeleteSecurityGroup",
          "ec2:AuthorizeSecurityGroupIngress",
          "ec2:RevokeSecurityGroupIngress",
          "ec2:CreateVolume",
          "ec2:DeleteVolume",
          "ec2:AttachVolume",
          "ec2:DetachVolume",
          "ec2:DescribeVolumes",
          "ec2:CreateKeyPair",
          "ec2:DeleteKeyPair",
          "ec2:DescribeKeyPairs",
          "ec2:DescribeImages",
          "ec2:DescribeSubnets",
          "ec2:DescribeVpcs",
          "ec2:DescribeSecurityGroups",
          "ec2:CreateTags",
        ]
        Resource = "*"
        Condition = {
          StringEquals = {
            "aws:RequestedRegion" = "ap-northeast-1"
          }
        }
      },
      {
        Sid    = "S3StateAccess"
        Effect = "Allow"
        Action = [
          "s3:GetObject",
          "s3:PutObject",
          "s3:ListBucket",
        ]
        Resource = [
          "arn:aws:s3:::born2beroot-terraform-state",
          "arn:aws:s3:::born2beroot-terraform-state/*",
        ]
      },
      {
        Sid    = "DynamoDBLocking"
        Effect = "Allow"
        Action = [
          "dynamodb:GetItem",
          "dynamodb:PutItem",
          "dynamodb:DeleteItem",
        ]
        Resource = "arn:aws:dynamodb:ap-northeast-1:*:table/terraform-locks"
      },
    ]
  })
}
```

### 18.3 ステートファイルの保護

```hcl
# S3 バケットのセキュリティ設定（ステートファイル保存用）
resource "aws_s3_bucket" "terraform_state" {
  bucket = "born2beroot-terraform-state"

  # 誤削除防止
  lifecycle {
    prevent_destroy = true
  }
}

# バージョニング（ステートの履歴保持）
resource "aws_s3_bucket_versioning" "terraform_state" {
  bucket = aws_s3_bucket.terraform_state.id
  versioning_configuration {
    status = "Enabled"
  }
}

# サーバーサイド暗号化（ステートには機密情報が含まれる）
resource "aws_s3_bucket_server_side_encryption_configuration" "terraform_state" {
  bucket = aws_s3_bucket.terraform_state.id
  rule {
    apply_server_side_encryption_by_default {
      sse_algorithm     = "aws:kms"
      kms_master_key_id = aws_kms_key.terraform_state.arn
    }
    bucket_key_enabled = true
  }
}

# パブリックアクセスの完全ブロック
resource "aws_s3_bucket_public_access_block" "terraform_state" {
  bucket = aws_s3_bucket.terraform_state.id

  block_public_acls       = true
  block_public_policy     = true
  ignore_public_acls      = true
  restrict_public_buckets = true
}

# アクセスログ
resource "aws_s3_bucket_logging" "terraform_state" {
  bucket = aws_s3_bucket.terraform_state.id

  target_bucket = aws_s3_bucket.access_logs.id
  target_prefix = "terraform-state-logs/"
}

# DynamoDB テーブル（ステートロック用）
resource "aws_dynamodb_table" "terraform_locks" {
  name         = "terraform-locks"
  billing_mode = "PAY_PER_REQUEST"
  hash_key     = "LockID"

  attribute {
    name = "LockID"
    type = "S"
  }

  # ポイントインタイムリカバリ
  point_in_time_recovery {
    enabled = true
  }
}

# KMS キー（ステートファイルの暗号化用）
resource "aws_kms_key" "terraform_state" {
  description             = "KMS key for Terraform state encryption"
  deletion_window_in_days = 30
  enable_key_rotation     = true
}
```

---

## 19. 実践的なトラブルシューティング

### 19.1 よくあるエラーと対処法

```
エラー 1: Provider の認証エラー
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Error: No valid credential sources found

原因: AWS の認証情報が設定されていない
対処:
  1. AWS CLI で認証情報を設定
     aws configure
  2. 環境変数で設定
     export AWS_ACCESS_KEY_ID="..."
     export AWS_SECRET_ACCESS_KEY="..."
  3. IAM ロール（EC2 インスタンスプロファイル）を使用
  4. AWS SSO を使用
     aws sso login --profile my-profile

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
エラー 2: ステートロックの競合
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Error: Error acquiring the state lock
Lock Info:
  ID:        a1b2c3d4
  Path:      born2beroot/terraform.tfstate
  Operation: OperationTypeApply
  Who:       user@hostname
  Created:   2024-01-15 10:30:00

原因: 別のプロセスがステートをロック中
対処:
  1. 本当に他の誰かが apply 中でないか確認
  2. プロセスがクラッシュしてロックが残っている場合:
     terraform force-unlock a1b2c3d4
  3. DynamoDB テーブルのロックを直接確認:
     aws dynamodb get-item --table-name terraform-locks \
       --key '{"LockID":{"S":"born2beroot/terraform.tfstate"}}'

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
エラー 3: リソースの依存関係エラー
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Error: Error creating Security Group: VPCIdNotFound: The vpc ID 'vpc-xxx' does not exist

原因: 依存するリソースがまだ作成されていない、または存在しない
対処:
  1. depends_on で明示的に依存関係を定義
     resource "aws_security_group" "sg" {
       depends_on = [aws_vpc.main]
       ...
     }
  2. 暗黙的な依存関係を利用（属性参照）
     vpc_id = aws_vpc.main.id  # ← 暗黙的に VPC の作成を待つ

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
エラー 4: ステートとリアルの不整合
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Error: error reading EC2 Instance (i-xxx): InvalidInstanceID.NotFound

原因: ステートにはあるが実際のリソースが削除されている
対処:
  1. ステートからリソースを除外
     terraform state rm aws_instance.born2beroot
  2. terraform refresh（非推奨、plan で代替）
     terraform plan -refresh-only
  3. Plan を実行して差分を確認
     terraform plan
     # → 削除されたリソースを「新規作成」として表示

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
エラー 5: Provider バージョンの不整合
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Error: Incompatible provider version
Provider registry.terraform.io/hashicorp/aws v5.0.0 does not
have a package available for your current platform

原因: .terraform.lock.hcl と実行環境の不一致
対処:
  1. ロックファイルを更新
     terraform init -upgrade
  2. プラットフォーム指定でロック生成
     terraform providers lock -platform=linux_amd64 -platform=darwin_arm64

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
エラー 6: cloud-init が完了しない
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
症状: インスタンスは起動するが、SSH 接続できない / 設定が適用されない

対処:
  1. インスタンスのコンソール出力を確認
     aws ec2 get-console-output --instance-id i-xxx
  2. cloud-init のログを確認（SSH 接続できる場合）
     cat /var/log/cloud-init.log
     cat /var/log/cloud-init-output.log
  3. cloud-init のステータス確認
     cloud-init status --long
  4. YAML の構文エラーを事前チェック
     cloud-init schema --config-file cloud-init.yaml

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
エラー 7: for_each / count の変更時の問題
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Error: Invalid for_each argument
The "for_each" value depends on resource attributes that cannot be
determined until apply

原因: for_each のキーが plan 時に決定できない
対処:
  1. for_each には plan 時に値が確定するデータのみ使用
     # 悪い例: aws_instance.ids が apply 後に決まる
     for_each = toset(aws_instance.born2beroot[*].id)  # NG
     # 良い例: 変数やローカル値を使用
     for_each = var.lvm_volumes  # OK
  2. 必要に応じて targeted apply
     terraform apply -target=aws_instance.born2beroot
```

### 19.2 デバッグ方法

```bash
# ログレベルの設定
export TF_LOG=DEBUG       # TRACE, DEBUG, INFO, WARN, ERROR
export TF_LOG_PATH=/tmp/terraform.log

# ログをファイルに出力して plan 実行
TF_LOG=DEBUG terraform plan 2>&1 | tee /tmp/tf-debug.log

# Provider のデバッグ
export TF_LOG_CORE=ERROR      # Terraform コアのログレベル
export TF_LOG_PROVIDER=DEBUG  # Provider のログレベル

# グラフの可視化（依存関係の確認）
terraform graph | dot -Tpng > graph.png
# → Graphviz が必要: apt install graphviz

# Plan の詳細出力（JSON 形式）
terraform plan -out=tfplan
terraform show -json tfplan | jq .

# ステートの詳細確認
terraform state pull | jq '.resources[] | select(.type == "aws_instance")'
```

### 19.3 パフォーマンスの最適化

```hcl
# 大規模環境でのパフォーマンス問題と対策

# 問題 1: Plan が遅い
# → 対策: ステートの分割（ブラストレディウスの縮小）
# ネットワーク、コンピュート、セキュリティを別々のステートで管理

# 問題 2: API 制限に引っかかる
# → 対策: parallelism の調整
terraform apply -parallelism=5  # デフォルト 10 を減らす

# 問題 3: 大量のリソースの refresh が遅い
# → 対策: targeted plan/apply
terraform plan -target=module.security
terraform apply -target=aws_instance.born2beroot

# 問題 4: ステートファイルが巨大
# → 対策: ステートの分割（関心の分離）
# state1: ネットワーク（VPC, サブネット, ルートテーブル）
# state2: セキュリティ（セキュリティグループ, IAM）
# state3: コンピュート（EC2, EBS）
# state4: 監視（CloudWatch, SNS）
```

---

## 20. Terraform のアンチパターン

### 20.1 避けるべきプラクティス

```hcl
# =============================================================================
# アンチパターン 1: ハードコードされた値
# =============================================================================
# 悪い例
resource "aws_instance" "bad" {
  ami           = "ami-0123456789"  # ← ハードコード
  instance_type = "t3.micro"       # ← ハードコード
  subnet_id     = "subnet-abc123"  # ← ハードコード
}

# 良い例
resource "aws_instance" "good" {
  ami           = data.aws_ami.debian.id        # ← Data Source
  instance_type = var.instance_type              # ← 変数
  subnet_id     = module.network.public_subnet_id # ← モジュール出力
}

# =============================================================================
# アンチパターン 2: 巨大な main.tf
# =============================================================================
# 悪い例: 全てを1ファイルに
# main.tf (1000行以上)

# 良い例: 論理的に分割
# network.tf    ← ネットワーク関連
# security.tf   ← セキュリティ関連
# compute.tf    ← コンピュート関連
# storage.tf    ← ストレージ関連
# variables.tf  ← 変数定義
# outputs.tf    ← 出力定義
# data.tf       ← Data Source
# versions.tf   ← Provider とバージョン

# =============================================================================
# アンチパターン 3: provisioner の多用
# =============================================================================
# 悪い例: provisioner で全ての設定を行う
resource "aws_instance" "bad" {
  provisioner "remote-exec" {
    inline = [
      "apt-get update",
      "apt-get install -y ufw",
      "ufw allow 4242",
      "ufw enable",
      # ... 100行のスクリプト
    ]
  }
}
# → provisioner は「最後の手段」。cloud-init や Ansible を使うべき

# 良い例: cloud-init + Ansible
resource "aws_instance" "good" {
  user_data = file("cloud-init.yaml")  # 初回設定
  # → Ansible で継続管理
}

# =============================================================================
# アンチパターン 4: ステートの手動編集
# =============================================================================
# 悪い例: terraform.tfstate を直接エディタで編集
# → JSON の構造が壊れる可能性
# → serial 番号の不整合
# → ロックの無視

# 良い例: terraform state コマンドを使用
terraform state mv aws_instance.old aws_instance.new
terraform state rm aws_instance.deprecated
terraform import aws_instance.new i-0123456789

# =============================================================================
# アンチパターン 5: terraform destroy の安易な使用
# =============================================================================
# 悪い例: 「とりあえず destroy して作り直し」
terraform destroy  # ← 全リソース削除
terraform apply    # ← 全リソース作成

# 良い例: 特定のリソースのみ再作成
terraform apply -replace=aws_instance.born2beroot

# 良い例: targeted destroy
terraform destroy -target=aws_instance.born2beroot
```

### 20.2 コードレビューチェックリスト

| チェック項目 | 確認内容 |
|-------------|---------|
| ハードコードされた値 | 変数化されているか |
| sensitive 指定 | パスワード等に `sensitive = true` があるか |
| バリデーション | 変数に `validation` ブロックがあるか |
| description | 変数と出力に description があるか |
| タグ付け | リソースに適切なタグが付与されているか |
| 暗号化 | ストレージが暗号化されているか |
| パブリックアクセス | 不要なパブリックアクセスがないか |
| ライフサイクル | 重要なリソースに `prevent_destroy` があるか |
| バージョン制約 | Provider のバージョンが固定されているか |
| ステート分離 | ブラストレディウスが適切か |
| セキュリティグループ | 0.0.0.0/0 が最小限か |
| plan 出力の確認 | 意図しない削除・再作成がないか |

---

## まとめ

Born2beRoot の手動セットアップは、Linux システム管理の基礎を深く理解するための優れた学習方法である。しかし、実務においては同様の作業を IaC で自動化することが標準的なプラクティスとなっている。

### キーポイント

1. **IaC は「コードとしてのインフラ」**: 手順書ではなくコードでインフラを管理する
2. **Terraform は宣言的**: 「望ましい状態」を記述し、ツールが差分を管理する
3. **ステートは命**: ステートファイルの管理が Terraform 運用の鍵
4. **Module で再利用**: 共通パターンをモジュール化して使い回す
5. **count / for_each**: 複数リソースを効率的に管理する
6. **リモートステート**: チーム開発では必須（S3 + DynamoDB / GCS）
7. **CI/CD 統合**: Plan → Review → Apply の自動化
8. **Ansible と組み合わせ**: Terraform（インフラ）+ Ansible（構成管理）が定番
9. **cloud-init で初期設定**: クラウド VM の初回起動時の設定を自動化
10. **手動の経験が基礎**: Born2beRoot の手動経験があるからこそ IaC の価値がわかる
11. **Workspace / ライフサイクル**: 環境管理とリソースのライフサイクル制御
12. **Import**: 既存リソースの Terraform 管理への移行
13. **テスト**: terraform test、terratest、tflint、checkov で品質と安全性を担保
14. **Policy as Code**: Sentinel / OPA でセキュリティポリシーを強制
15. **Packer**: 設定済みマシンイメージの事前作成でデプロイ時間を短縮
16. **セキュリティ**: シークレット管理、最小権限、ステート保護
17. **トラブルシューティング**: デバッグログ、ステート操作、パフォーマンス最適化

Born2beRoot で手動セットアップを経験した後に IaC を学ぶと、各リソース定義が実際に何をしているかが直感的に理解できるようになる。手動の苦労を知る者だけが、自動化の真の価値を理解できる。
