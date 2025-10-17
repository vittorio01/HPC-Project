# Installation Guide
Because the GlobalProtect VPN service offered by our university laks of support for linux distros, we find a workaround for connecting to the cluster without configuring a windows virtual machine.

Our guide solves this problem by using an older version (v1.4.9) of [GlobalProtect-openconnect](https://github.com/yuezk/GlobalProtect-openconnect/) which does not require a paid licence to be used. 

## Ubuntu 25.04
Because the official PPA of the packet provides only the last version from github, it is necessary to download and compile manually the older version of the application. 

First download few requirements.
```
sudo apt install -y openconnect git
```
The clone the source code of 1.4.9 version of `GlobalProtect-openconnect` from the GitHub repository (please do not use newer versions since they include a 10 days trial or a license).
```
$ wget -P /tmp https://github.com/yuezk/GlobalProtect-openconnect/archive/refs/tags/v1.4.9.tar.gz
$ tar -xvf /tmp/v1.4.9.tar.gz -C /tmp
```
Move the extracted repository in the `/opt` folder.
```
sudo mv /tmp/GlobalProtect-openconnect-1.4.9 /opt/GlobalProtect-openconnect
```
Then move into that directory and start the installation. A dedicated script will automatically install the requirements for building and installing the program. 
```
cd /opt/Globalprotect-openconnect
sudo bash /opt/Globalprotect-openconnect/scripts/install-ubuntu.sh
```
After the execution of the script the application `gpclient` and the service `gpservice` should be automatically enabled. 
To connect to the ssh first edit the configuration file located in `/etc/gpservice/gp.conf`
```
$ sudo vim /etc/gpservice/gp.conf
```
And add inside the file the exact following lines:
```
[*]
openconnect-args= --csd-wrapper /usr/libexec/openconnect/hipreport.sh
```
You can now run the global protect app from your application launcher or terminal and access using your UNITN credentials.

## Arch Linux
First get the package `globalprotect-openconnect` from the extra repository. Use pacman or
whatever AUR helper you use:
```
$ sudo pacman -S globalprotect-openconnect
```

After install, the VPN will still not work. We need to update `/etc/gpservice/gp.conf`.
You can do it with either vim or nano, remember to run sudo since it's a system file.
```
$ sudo vim /etc/gpservice/gp.conf
```

And add inside the file the exact following lines:
```
[*]
openconnect-args= --csd-wrapper /usr/lib/openconnect/hipreport.sh
```

You can now run the global protect app from your application launcher or terminal and access using your UNITN credentials.

## NixOS 25.05

Add the following options in your `configuration.nix` or in a separated module.
```
environment.systemPackages = with pkgs; [
    openconnect
    globalprotect-openconnect
  ];
services.globalprotect.enable=true;
services.globalprotect.csdWrapper="${pkgs.openconnect}/libexec/openconnect/hipreport.sh";
```
Then rebuild the system configuration

```
sudo nixos-rebuild switch
```
The global protect app should now be visible on your application launcher.
