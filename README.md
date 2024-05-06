
![code](https://raw.githubusercontent.com/project-bo4/shield-development/master/assets/readme_header.jpg)

## SHIELD
A very experimental modification platform for Call of Duty®: Black Ops 4 run by the community, aiming at improving both functionality and performance of the original game.


## DEMONWARE
As of May 11, 2023, we have merged the 'demonware' branch into 'master'. It includes a built-in demonware server emulator which allows the player to start a game without an active connection to the official online servers.


## INSTRUCTIONS

> You should already have the publisher files required for playing online (found under the LPC folder of your game directory).
> If this is not the case, then run Black Ops 4 through Battle.NET Launcher once to get those files downloaded.

1- Clone repository with its sub-modules and use ```generate.bat``` to make a Visual Studio solution, then compile project and copy ``d3d11.dll`` into your BO4 folder.

2- Start BlackOps4.exe

*In case you wanted to revert back to using the original Battle.NET servers, just delete the generated ``d3d11.dll`` file from your game folder. You can re-generate and copy ```d3d11.dll``` back later whenever you want to re-install the client.*

## SHIELD DOCUMENTATION

Documentation on shield can be found [here](https://shield-bo4.gitbook.io/).

## SUBMITTING ISSUES
The Github Issues section is only for reporting programmatical errors related to the client. Please don't use it for requesting features or seeking help with personal issues such as experiencing faulty game data or similar problems. Use Battle.NET's 'Scan And Repair' feature to fix those problems. 


## NOTES
- Base SDK (well kinda...) used by this project is developed by [Maurice Heumann](https://github.com/momo5502); Thanks to the guy.

- There are some 3rd-party project/tools that have influenced and helped the project in particular ways; If you belive there is something originated from you and want to be credited, please contact any of our social media accounts.

- This Project is created purely for educational purposes. It's free and open-sourced under the GNU License. Developers are not responsible or liable for misuse of this product.


## Roadmap
### CLIENT-SIDE:
- [x] Online Battle.NET Connection Requirement Removal
- [x] Built-in Demonware Server Emulation
- [x] BlackBox Crash Reporting Component
- [x] In-game Interactable Console
- [x] GSC & Lua Modding Support
- [ ] Enable Aim-Assist for Game Controllers
- [ ] Miscellaneous Features such as 1st-Person View FOV
- [ ] Fix *uncommon* Runtime "Black Screen" issue along other instabilities

### SERVER-SIDE:
- [x] Fully Functional Online Server Emulator
- [ ] Implement Basic Matchmaking with QOS Logic
- [ ] Try to Create Dedicated Server off Public Ship Builds
