# Venom RootKit
Simple Windows rootkit that I have written, To explore a bit about the world of rootkits and Windows kernel in general.
The Venom rootkit uses a few well-known methods that are commonly used by other famous rootkits. Some of the main features are listed in the "Features" section below.

# How Venom works
The Venom rootkit is composed of 3 major components.
1. The driver file (Venom) which we load on the target machine.
2. The client dll (Venom Client) which is loaded by the driver via APC Injection.
3. The C&C server to whom the client connects, thus allowing control over the rootkit instances.

The core of the rootkit is of course the driver, which contains all the logic, and the client exists just to control it.
Venom works by first hooking the NSI driver that will later be used to hide a port of our choice.
It then creates a Kernel APC that creates a User APC which in turn runs a shellcode that loads the Venom client dll into a process of our choice.
You can read more about why I create a Kernel APC that creates a User APC [here](https://medium.com/@philiptsukerman/bypassing-the-microsoft-windows-threat-intelligence-kernel-apc-injection-sensor-92266433e0b0).
Lastly, the Venom client connects back to the C&C server from which We can control the driver and utilize all of its features.

# Demo
A little demo of the port hiding feature.
![Port Hiding Demo](https://camo.githubusercontent.com/eeb97ea73446b3739615c14ee8ddf6d48636b2c26ac8938e4bd2e3cd08610e6f/68747470733a2f2f692e696d6775722e636f6d2f663551746c66312e706e67)

# Features
- [x] Dynamic APC injection to load the UM dll.
- [x] Process Hiding.
- [x] Token elevation to "NT AUTHORITY\SYSTEM".
- [x] Command execution.
- [x] TCP port hiding by irp hooking nsiproxy driver.
- [x] C&C server communication.
- [ ] Deleting the driver and dll from the disk after loading (SOON).

# Usage
1. Drop the dll in `C:\hooking.dll`
2. Drop the driver at a place of your choice
3. Run the C&C with `python main.py`
4. Load the driver 
`sc create venom type= kernel binPath= {path_to_file} displayName= venom`
`sc start venom`

# Looking forward
There are many improvements to be made in each component, especially in the client and C&C that I just made for the POC.
Currently, the client is very limited and static, so a lot to be made there.
When looking at the rootkit itself in each capability there are better ways to achieve the goal.
For example, replacing a token with the system process token is very easy to detect, a better way would be to just modify the relevant values in the token.
Or when looking at the NSI hook, IRP hooking of the dispatch routine is also very easy to detect.
So stay tuned for some future changes, and feel free to contribute your own!

# Some other projects I have taken inspiration from
 - [BlackBone](https://github.com/DarthTon/Blackbone)
 - [Win_Rootkit](https://github.com/alal4465/Win_Rootkit)

# Thanks
I want to thank [@omerk2511](https://github.com/omerk2511) for helping and guiding me.
