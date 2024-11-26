PARTYMOD THUG2 1.0.0

This is a patch for THUG2 to improve its input handling as well as smooth out a few other parts of the PC port.
The patch is designed to keep the game as original as possible, and leave its files unmodified.

Features and Fixes:
- Replaced input system entirely with new, modern system using the SDL2 library
- Improved window handling allowing for custom resolutions and configurable windowing (NOTE: when using widescreen or portrait resolutions, the main menu is always forced to 4:3 using black bars to preserve the menu text's positioning)
- Replaced configuration files with new INI-based system (see partymod.ini)
- Custom configurator program to handle new configuration files
- Controller glyphs are now shown in prompts when using a controller (Controller glyph style is configurable between PS2, Xbox, and Gamecube style) (NOTE: because of the way glyph code is rewritten in this version, some prompts will be incorrect)
- Restores the on-screen keyboard
- Restores quick chat and keyboard taunt binds
- Adds entropy to music shuffling to make sure it doesn't repeat between sessions (while consuming the same number of random numbers)
- Fixes too-short clipping distance causing visual errors in large levels (I.E. Skatopia)
- Fixes screen flash effect covering only part of the screen (E.G. when doing a Franklin Grind)
- Optionally disables blur for graphics drivers that show severe artifacts
- Replaces online service with OpenSpy


INSTALLATION:
1. Make sure THUG2 (English) is installed, remove the widescreen mod if it is installed (delete dinput8.dll)
2. Extract this zip folder into your THUG2 installation `game/` directory
3. Run partypatcher.exe to create the new, patched THUG2PM.exe game executable (this will be used to launch the game from now on) (this only needs to be done once)
4. Optionally (highly recommended), configure the game with partyconfig.exe
5. Launch the game from THUG2PM.exe

NOTE: if the game is installed into the "Program Files" directory, you may need to run each program as administrator. 
Also, if the game is installed into the "Program Files" directory, save files will be saved in the C:\Users\<name>\AppData\Local\VirtualStore directory.  
For more information, see here: https://answers.microsoft.com/en-us/windows/forum/all/please-explain-virtualstore-for-non-experts/d8912f80-b275-48d7-9ff3-9e9878954227