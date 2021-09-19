# Venom RootKit
A simple windows rootkit that I have wrote, In order to explore a bit about the world of rootkits and windows kernel in general.
The Venom rootkit uses a few well known methods that are commonly being used by other famous rootkits. Some of the main features are listed in the "Features" section later on.

# Features
- [x] Dynamic APC injection of user mode dll from kernel.
- [x] Process Hiding.
- [x] Token elevation to "NT AUTHORITY\SYSTEM".
- [ ] Command execution.
- [x] TCP port hiding by hooking NSI.
- [ ] C&C server communication. 
- [ ] Logging.
- [ ] File hiding.
- [ ] Anti VM/Debug.

# Some other projects I have taken inspiration from
 - [BlackBone](https://github.com/DarthTon/Blackbone)
 - [Win_Rootkit](https://github.com/alal4465/Win_Rootkit)

# Thanks
I want to thanks [@omerk2511](https://github.com/omerk2511) for helping and guiding.
