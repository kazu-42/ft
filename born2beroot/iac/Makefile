# Born2beRoot IaC - Makefile
#
# Usage:
#   make mandatory   - Apply mandatory configuration
#   make bonus       - Apply bonus configuration (WordPress + Fail2ban)
#   make all         - Apply both mandatory and bonus
#   make verify      - Run verification checks
#   make vagrant-up  - Create VM with Vagrant (for testing)

ANSIBLE_FLAGS ?= --ask-become-pass
INVENTORY     := inventory.ini

.PHONY: all mandatory bonus verify vagrant-up vagrant-destroy lint

all: mandatory bonus

mandatory:
	ansible-playbook -i $(INVENTORY) playbook.yml $(ANSIBLE_FLAGS)

bonus:
	ansible-playbook -i $(INVENTORY) playbook_bonus.yml $(ANSIBLE_FLAGS)

verify:
	ansible-playbook -i $(INVENTORY) playbook.yml $(ANSIBLE_FLAGS) --tags verify
	@echo "---"
	ansible-playbook -i $(INVENTORY) playbook_bonus.yml $(ANSIBLE_FLAGS) --tags verify

lint:
	ansible-lint playbook.yml playbook_bonus.yml

vagrant-up:
	vagrant up

vagrant-destroy:
	vagrant destroy -f
