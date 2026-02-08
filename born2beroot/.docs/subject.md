# Born2beRoot - Project Subject

## Introduction

This project aims to introduce you to the wonderful world of virtualization.
You will create your first machine in VirtualBox (or UTM if VirtualBox is not available)
under specific instructions. At the end of the project, you will be able to set up
your own operating system while implementing strict rules.

## General Guidelines

- The use of VirtualBox (or UTM if VirtualBox is not available) is mandatory.
- You only have to turn in a `signature.txt` file at the root of your repository.
  You must paste in it the signature of your machine's virtual disk.

## Mandatory Part

### Virtual Machine Setup

- Create a virtual machine with the latest stable version of **Debian** (recommended)
  or **Rocky Linux**.
- No graphical interface shall be used (X.org, Wayland, or equivalent).
- You must set up your operating system with **at least 2 encrypted partitions using LVM**.

### Hostname and User

- The hostname of your virtual machine must be your **login ending with 42**
  (e.g., `kaztakam42`).
- In addition to the root user, a user with your login as username must be present.
- This user must belong to the `user42` and `sudo` groups.

### SSH Configuration

- An SSH service must be running on **port 4242 only**.
- It must **not** be possible to connect using SSH as root.

### Firewall

- Configure your operating system with the **UFW** firewall (or **firewalld** for Rocky).
- Leave only **port 4242** open.
- The firewall must be active when the virtual machine launches.

### Password Policy

#### Expiration

- Password must expire every **30 days**.
- Minimum number of days allowed before the modification of a password: **2**.
- The user must receive a warning message **7 days** before their password expires.

#### Strength

- Minimum **10 characters** long.
- Must contain an **uppercase letter**, a **lowercase letter**, and a **number**.
- Must not contain more than **3 consecutive identical characters**.
- Must not include the user's **name**.
- Must have at least **7 characters** that are not part of the former password
  (does not apply to root).

### sudo Configuration

- Authentication using sudo must be limited to **3 attempts** in the event of an
  incorrect password.
- A custom message of your choice must be displayed if an error due to a wrong
  password occurs when using sudo.
- Each action using sudo must be **archived**, both inputs and outputs.
  The log file must be saved in **`/var/log/sudo/`**.
- **TTY mode** must be enabled for security reasons.
- The paths that can be used by sudo must be restricted to:
  **`/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/snap/bin`**

### Monitoring Script

- Create a script called **`monitoring.sh`** developed in **bash**.
- At server startup, the script must display some information on **all terminals**
  every **10 minutes** (see `wall`).
- The banner is optional; no error must be visible.
- The following information must be displayed:
  - The architecture of your operating system and its kernel version
  - The number of physical processors
  - The number of virtual processors
  - The current available RAM on your server and its utilization rate as a percentage
  - The current available memory on your server and its utilization rate as a percentage
  - The current utilization rate of your processors as a percentage
  - The date and time of the last reboot
  - Whether LVM is active or not
  - The number of active connections
  - The number of users using the server
  - The IPv4 address of your server and its MAC address
  - The number of commands executed with the sudo program

## Bonus Part

Bonus will only be assessed if the mandatory part is **PERFECT**.

### Partition Structure

Set up partitions correctly to get a structure similar to:
```
NAME                    TYPE  MOUNTPOINT
sda                     disk
├─sda1                  part  /boot
├─sda2                  part
│ └─sda5                part
│   └─sda5_crypt        crypt
│     ├─LVMGroup-root   lvm   /
│     ├─LVMGroup-swap   lvm   [SWAP]
│     ├─LVMGroup-home   lvm   /home
│     ├─LVMGroup-var    lvm   /var
│     ├─LVMGroup-srv    lvm   /srv
│     ├─LVMGroup-tmp    lvm   /tmp
│     └─LVMGroup-var--log lvm /var/log
```

### WordPress

Set up a functional WordPress website with:
- **lighttpd** web server
- **MariaDB** database
- **PHP**

### Free Choice Service

Set up a service that you think is useful (not NGINX or Apache2).
You must justify your choice during the defense.

## Submission

Turn in your work in your Git repository as usual. Only the work inside your
repository will be evaluated during the defense. The `signature.txt` file must
be at the root of your repository.
