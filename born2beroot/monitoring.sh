#!/bin/bash

# =============================================================================
# monitoring.sh - Born2beRoot System Monitoring Script
# =============================================================================
# This script collects system information and broadcasts it to all logged-in
# users via the wall command. It is intended to be run every 10 minutes via cron.
#
# Usage:
#   sudo /usr/local/bin/monitoring.sh
#
# Cron entry (every 10 minutes):
#   */10 * * * * /usr/local/bin/monitoring.sh
# =============================================================================

# --- Architecture and Kernel Version ---
ARCH=$(uname -a)

# --- Physical CPUs ---
PCPU=$(grep "physical id" /proc/cpuinfo | sort -u | wc -l)

# --- Virtual CPUs ---
VCPU=$(grep -c "^processor" /proc/cpuinfo)

# --- RAM Usage ---
RAM_TOTAL=$(free -m | awk '/^Mem:/ {print $2}')
RAM_USED=$(free -m | awk '/^Mem:/ {print $3}')
RAM_PERCENT=$(free | awk '/^Mem:/ {printf("%.2f"), $3/$2*100}')

# --- Disk Usage ---
DISK_TOTAL=$(df -BG --total | awk '/^total/ {print $2}' | sed 's/G//')
DISK_USED=$(df -BM --total | awk '/^total/ {print $3}' | sed 's/M//')
DISK_PERCENT=$(df --total | awk '/^total/ {print $5}')

# --- CPU Load ---
CPU_LOAD=$(top -bn1 | grep "^%Cpu" | awk '{printf("%.1f%%"), $2+$4}')

# --- Last Boot ---
LAST_BOOT=$(who -b | awk '{print $3" "$4}')

# --- LVM Active ---
LVM_USE=$(if [ "$(lsblk | grep -c "lvm")" -gt 0 ]; then echo yes; else echo no; fi)

# --- TCP Connections ---
TCP_CONN=$(ss -t state established | tail -n +2 | wc -l)

# --- Logged In Users ---
USER_LOG=$(who | wc -l)

# --- Network (IPv4 and MAC) ---
IP=$(hostname -I | awk '{print $1}')
MAC=$(ip link show | awk '/ether/ {print $2}' | head -1)

# --- Number of sudo commands executed ---
SUDO_LOG=$(journalctl _COMM=sudo 2>/dev/null | grep -c "COMMAND" || \
           grep -c "COMMAND" /var/log/sudo/sudo.log 2>/dev/null || \
           grep -c "COMMAND" /var/log/auth.log 2>/dev/null || \
           echo "0")

# --- Broadcast via wall ---
wall "
	#Architecture: ${ARCH}
	#CPU physical: ${PCPU}
	#vCPU: ${VCPU}
	#Memory Usage: ${RAM_USED}/${RAM_TOTAL}MB (${RAM_PERCENT}%)
	#Disk Usage: ${DISK_USED}/${DISK_TOTAL}Gb (${DISK_PERCENT})
	#CPU load: ${CPU_LOAD}
	#Last boot: ${LAST_BOOT}
	#LVM use: ${LVM_USE}
	#Connections TCP: ${TCP_CONN} ESTABLISHED
	#User log: ${USER_LOG}
	#Network: IP ${IP} (${MAC})
	#Sudo: ${SUDO_LOG} cmd
"
