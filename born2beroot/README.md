# Born2beRoot

42 School system administration project - Virtual Machine setup and Linux server hardening.

## Project Structure

```
born2beroot/
├── monitoring.sh          # System monitoring script (broadcast via wall)
├── signature.txt          # VM disk SHA1 signature (placeholder)
├── README.md              # This file
├── iac/                   # Infrastructure as Code (Ansible)
│   ├── Makefile           # make mandatory / make bonus / make verify
│   ├── Vagrantfile        # VM provisioning (for testing)
│   ├── ansible.cfg        # Ansible settings
│   ├── inventory.ini      # Target host definition
│   ├── group_vars/
│   │   └── all.yml        # Configuration variables (username, hostname, etc.)
│   ├── playbook.yml       # Mandatory requirements playbook
│   ├── playbook_bonus.yml # Bonus requirements playbook (WordPress + Fail2ban)
│   └── templates/
│       └── wp-config.php.j2  # WordPress config template
├── .docs/
│   └── subject.md         # Project subject in markdown format
└── .wiki/
    ├── README.md           # Wiki index
    ├── 01-background.md    # Background knowledge
    ├── 02-prerequisites.md # Prerequisites
    ├── 03-requirements.md  # Detailed requirements
    ├── 04-evaluation.md    # Evaluation criteria & defense Q&A
    ├── 05-solution-terraform.md # Terraform conceptual approach
    ├── 06-design.md        # Design principles
    ├── 07-study-guide.md   # Study guide
    ├── 08-learning-goals.md # Learning goals
    └── 09-comprehension-check.md # Comprehension check Q&A
```

## IaC Quick Start

### Prerequisites

- Ansible installed on your host machine (`pip install ansible` or `apt install ansible`)
- SSH access to a Debian 12 VM with LUKS + LVM (configured during OS installation)

### 1. Configure variables

Edit `iac/group_vars/all.yml` with your 42 login:

```yaml
b2br_username: "your_login"
b2br_hostname: "your_login42"
```

### 2. Set inventory

Edit `iac/inventory.ini` with your VM's SSH connection details:

```ini
[born2beroot]
b2br ansible_host=127.0.0.1 ansible_port=2222 ansible_user=your_login
```

### 3. Run

```bash
cd iac

# Apply mandatory requirements (hostname, SSH, UFW, password, sudo, AppArmor, cron)
make mandatory

# Apply bonus requirements (WordPress + Fail2ban)
make bonus

# Verify all settings
make verify
```

### What the playbooks configure

**Mandatory** (`playbook.yml`):
- Hostname set to `login42`
- User added to `sudo` and `user42` groups
- SSH on port 4242, root login disabled
- UFW active with only port 4242 open
- Password policy: 30-day expiry, 2-day min, 7-day warning, strength rules
- sudo: 3 retries, logging, TTY mode, restricted PATH
- AppArmor enabled
- `monitoring.sh` deployed + cron every 10 minutes

**Bonus** (`playbook_bonus.yml`):
- lighttpd + MariaDB + PHP + WordPress
- Fail2ban protecting SSH on port 4242
- UFW port 80 for WordPress

### Note

LUKS encryption and LVM partitioning must be configured during the Debian installation process. The Ansible playbooks handle all post-install configuration.

## monitoring.sh

The monitoring script collects and broadcasts system information every 10 minutes.
See the script source for details on each metric collected.

## Note

Wiki documentation is written in Japanese with technical terms in English.
