# Installation Guide

# Arch Linux
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
