# Venom RootKit
A simple windows rootkit that I have wrote, In order to explore a bit about the world of rootkits and windows kernel in general.
The Venom rootkit uses a few well known methods that are commonly being used by other famous rootkits. Some of the main features are listed in the "Features" section below.

# Flow
The flow of the rootkit is as follows:
We start by dropping the rootkit .sys file and the UM .dll file to disk.
Then, we load the rootkit driver, we can do so using some exploit or projects like DSEFix and KDMapper.
Once the rootkit is loaded, it creates a device and a sym link, so that the UM client will be able to talk with it.
Then it performs the IRP hook over the nsiproxy driver. And then it performs an APC injection of the UM dll to an arbitriry thread within "explorer.exe" (It can easily be changed). The APC injection is first queening a kernel APC and then a user APC, so we can avoid Microsoft ETW event on user mode APC created from kernel, as described [here](https://medium.com/@philiptsukerman/bypassing-the-microsoft-windows-threat-intelligence-kernel-apc-injection-sensor-92266433e0b0).

# Features
- [x] Dynamic APC injection to load the UM dll.
- [x] Process Hiding.
- [x] Token elevation to "NT AUTHORITY\SYSTEM".
- [x] Command execution.
- [x] TCP port hiding by hooking NSI.
- [x] C&C server communication. 
- [ ] Logging.
- [ ] File hiding.
- [ ] Anti VM/Debug (Maybe implement through TLS callbacks).
- [ ] Dynamic config for UM client.

# Some other projects I have taken inspiration from
 - [BlackBone](https://github.com/DarthTon/Blackbone)
 - [Win_Rootkit](https://github.com/alal4465/Win_Rootkit)

# Thanks
I want to thanks [@omerk2511](https://github.com/omerk2511) for helping and guiding.
