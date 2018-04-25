# LD41: The Textual-Moba

A text-mode Moba for Ludum Dare 41, theme: Combine 2 Incompatible Genres.


## Description

_Destroy the enemy Fonxus before it destroys yours !_

This is, obviously, a text-based MOBA. Move around the map and defeat your foes with command lines. Exciting hum ? The game is **not multiplayer** (but maybe in the future ?). Your allies are played by (rather stupid) bots.

Teams are composed of a warrior, a ranger and a mage. You can play any of them. Redshirts spawn regularly and walk down the lanes. You should probably follow them ! Fight happen in the middle of the lanes initially, you can use your skills to help your team, which is imba because bots don't know how to use skills... :(

The game works well but could need more balance. The problem is that we need a correct AI before we can really balance the game, and we lacked time (and experience) to make a proper one. Anyway, we are still happy of the result given the complexity of the gameplay for a LD game. So that's it.

_We hope you will have fun !_


## How to play

First choose a class by typing `warrior`, `ranger` or `mage`. You probably should avoid the warrior for the first game because of its short range. Actually, **you should play ranger** because otherwise the game will be very hard. See below.

Then you can type commands. The game is self-documented: type `help` to get help in bad English.

However, a lot of game mechanics are not properly explained in game, so here is a (not so) quick summary:

- You play in the blue team, on the left of the map, against the red team on the right.
- Every time you perform an action (move, attack, etc.) the turn ends and the other characters (AI) plays.
- You play first, then the other blue heros, then the blueshirts, then the blue turrets. The red team play afterward in the same order (heros, redshirts, turrets).
- You can move around the map from node to node (old adventure games call them "rooms"). Use the `go <direction>` command for this, or just `g <direction>`. You can use the command `direction`, `dir` or `d` to list the paths you can take. You can move around the map using only four directions: `blue` (toward your base), `red` (toward the enemy base), `top` and `bot`.
- There is two lanes: top and bot. On each lane, you will find two turrets (displayed as T on the map) plus the Fonxus turret. The jungle and the river (the part of the map between the lanes) are pretty much useless as bots never go there.
- You can look who is on your node with the command `look` or `l`. It's quite useless because it is called automatically before your turn.
- You can attack with the command `attack <target>` or `a <target>`, where `<target>` is the index of the enemy when you use the command `look`.
- Inside a node, characters are placed in four "rows". There is a back row and a front row for each team. When you move from node to node, you always appear in the back row. You can change row with the command `move front` or `move back`. You can't go in the red rows and enemies can't go in the blue rows. Rows look like this:

```
  blue back row  |  blue front row  |  red front row  | red back row
```

- Rows are used to compute ranges. If you are in the (blue) front row, an enemy in the (red) front row will be at a distance of 1. If the enemy is in the back row, _the distance will be 2 if there is an enemy in the front row, 1 otherwise_. In short, empty rows don't count. This mean that you can attack an enemy in the back row from the back row even if your range is only 1 if there is nobody in the front rows.
- Ranger and mage have a range of 2, the warrior has a range of 1. This makes the warrior really hard to play (among other things...).
- Warrior and mage are hard-coded to go on the bottom lane, the ranger go top. If you play ranger you can go with the others which gives you a great advantage. Don't worry about the top lane, it is unlikely the enemy ranger manages to destroy your turret.
- Blueshirts and redshirts spawn every 10 turns, with the first wave on turn 5. For each team, two go top and two go bot. Blue/redshirts can only move forward and attack on sight.
- _Blueshirts and redshirds are always in the front row_. This mean you can follow them and be "hidden" behind them. If your character has a range of 2, he will be able to attack the enemies in the front row. Nobody has a range above 2 (except the mage fireball, but the AI don't use it, so you are safe).
- Turrets and heroes always attack a random _closest_ enemy first. It mean that if you are in the back row with blueshirts in the front row, you are _really_ safe. **However** they will keep attacking you if they can once they focused you: if a turret attack you because there is nobody in the front row, and during the next turn a blueshirt comes, _the turret will keep attacking you_ !
- _Take care if you play the "LD version" !_ Turrets are bugged and will remember you even after you leave the area if they don't find someone else to attack.
- Your character has 1 skill that has some cooldown and consume mana. The warrior has a heal over time, command `use endure`. The ranger throw a bomb at a range of 2 max that damages all the enemies of a row, command `use bomb front` of `use bomb back` depending on which row you target. The mage has a fireball that deal huge damage at a single enemy with a range of 3, command `use fireball <target>`.
- Your skill become stronger at lever 4.


### Tips

Play ranger (or maybe mage) and push the bottom lane with your allies. Hero AI are quite dumb. They only move forward with redshirts/blueshirt an go back otherwise. They return to the Fonxus to get heal if they fall below 1/4 of their max HP. Destroy the two bot towers to gain some xp, then you can try to push top or just keep attacking the Fonxus.


## Mini-postmortem by Draklaw

Ok, the mechanics of this game are _way_ too complicated for a Ludum Dare. This lead to three issues.

First, we lacked time to implement everything. Heros should have had 3 skills instead of one. Heroes AI should be a bit more interesting and the player should be able to give them orders (follow, push this lane, etc.). We wanted to implement objects, gold and shopping. And neutral creeps. And... Well, much more things.

Second, 3 days are not enough to balance such a game. You can't really balance the game before all mechanics are implemented, and then you just don't have the time. Completing a game takes at least half an hour, so it is really hard to iterate the design.

Last, but not least, I don't think people will invest enough time in a broken LD game to really grasp the mechanics. But the game becomes interesting only if you don care about placement, ranges and such. It is not _that_ interesting it its current state, mostly because of bugs, poor AI and missing featuers, but I think the base design works.

The good news is that I might keep working on it. It might lead to something interesting.


## Known bugs

- Enemy heroes don't gain levels. For some reason, I wrote a test that explicitly forbid them... Which is a good thing, otherwise the game would probably be unbeatable.
- Turrets can focus you even with blueshirts in front of you if you don't play the fixed game.

**Balance issues:**
- The warrior sucks. His ability is far too weak.
- Fonxus don't heal mana.
- Red fonxus heal the enemies way too much when you try to destroy it.


## Compilation

Clone the repository. We use a submodule to include our "game engine", Lair. So you should clone with `--recursive`:
```
git clone --recursive <clone-url>
```

The only dependency of the project is [Lair](https://github.com/draklaw/lair), our homebrew game engine. However, Lair has several dependencies. Refer to Lair's `Readme.md` for more information. Using a package manager is highly recommended. We build the Windows version using MSYS2.

We use cmake to compile. Once the dependencies are installed, assuming they are in a standard place where cmake can find them, all you need to do is
```
mkdir <some-build-directory>
cd <some-build-directory>
cmake <project-root>
make
```

If, as suggested above, you choose to do an out-of-source build, you must make sure that the game can find the assets folder. Just copy or link the asset folder in the directory of the executable, and you're good to go. If the game complain about missing DLLs (typical under Windows), you have to copy them to the executable directory. Now enjoy the game !
