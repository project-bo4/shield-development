
![code](https://raw.githubusercontent.com/project-bo4/shield-development/master/assets/readme_header.jpg)

## SHIELD
A very experimental modification platform for Call of Duty®: Black Ops 4 run by community, aiming at improving both functionality and performance of original game.


## DEMONWARE
As of May11, 2023 we merged 'demonware' branch into 'master'. it includes a built-in demonware server emulator which allows player to start game without connection to official online servers.


## INSTRUCTIONS

> You should have publisher files required for playing online under LPC folder of your game directory.
> if its not the case then start original game through battlenet launcher once to get those downloaded.

1- Clone repository with its sub-modules and use generate.bat to make visual studio solution then compile project and copy ``d3d11.dll`` into your bo4 folder.

2- Start BlackOps4.exe

*In case you wanted to revert back to original battlenet servers just delete ``d3d11.dll``. you can put it back later whenever you want to re-install client.*

## SHIELD DOCUMENTATION

Documentation on shield can be found [Here](https://shield-bo4.gitbook.io/).

## SUBMITTING ISSUES
Github issues section is only for reporting programmatically errors of client. please dont use it for requesting features or seeking help with personal issues such as faulty game data or similar problems. use battlenet's scan and repair feature to fix those problems. 


## NOTES
- Base SDK(well kinda...) used by this project is developed by [Maurice Heumann](https://github.com/momo5502); Thanks to the guy.

- There are some 3rd-party project/tools that have influenced and helped the project in particular ways; If you belive there is something originated from you and want to be credited please contact any of our social media accounts.

- This Project is created purely for educational purposes. its free and open-sourced under gnu license. developers are not responsible or liable for misuse of this product.


## Roadmap
### CLIENT-SIDE:
- [x] Online Battlenet Connection Requirement Removal
- [x] Built-in Demonware Server Emulation
- [x] BlackBox Crash Reporting Component
- [x] In-game Interactable Console
- [x] Gsc & Lua Modding Support
- [ ] Enable Aim-Assist for Game Controllers
- [ ] Miscellaneous Features such as 1st person view fov 
- [ ] Fix *uncommon* runtime black screen issue along other instabilities

### SERVER-SIDE:
- [x] Fully Functional Online Server Emulator
- [ ] Implement Basic Matchmaking with QOS Logics
- [ ] Try to Create Dedicated Server off Public Ship Builds
