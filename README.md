# unDefender
### Killing your preferred antimalware by abusing native symbolic links and NT paths

unDefender is the C++ implementation of a technique originally described by [@jonasLyk](https://twitter.com/jonasLyk) in this [Twitter thread](https://twitter.com/jonasLyk/status/1378143191279472644).  
At its core, this technique revolves around changing the \Device\BootDevice symbolic link in the Windows Object Manager so that when Defender's WdFilter driver is unloaded and loaded again by its Tamper Protection feature, another file is mapped in memory in place of the original WdFilter.sys, rendering it effectively useless!

__Requirements__
- Compile unDefender.exe in Release x64 configuration;
- Place unDefender.exe and the provided WdFilter.sys in the same folder;
- Run an elevated cmd.exe or powershell.exe and navigate to said folder;
- `.\unDefender.exe`
- Profit :)

__Tested on__
- [x] Windows 10 20H2
- [x] Windows 10 21H1
- [ ] Windows 11
