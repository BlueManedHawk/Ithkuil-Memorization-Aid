# ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls

This repository hosts a piece of software intended as a memorization aid for the various parts of the constructed language Ithkuil.  It is intended to be used not on its own, but as a supplementary tool within your broader learning of the language.

The software works by having you select a category of the language which you wish to learn, after which it will present you with an endless series of multiple-choice questions regarding the category you selected.  You can switch to a different category at any time.

The software is intended to be simple.  This means that there are no points systems, no online leaderboards, no formal "lessons".  This gives you, the user, the freedom to use this software in whatever way best suits your method of learning.

The questions are not taken randomly from a massive list; instead, they are procedurally constructed based on information and templates supplied with the program and provided to it at runtime.  This seeks to avoid the issue of questions becoming repetitive.

This is not a big, fancy, professional project.  This is just something I'm doing in my spare time, and I probably won't have much of it to spare on this.  If there's sufficient demand for it, i may choose to professionalize it (and frankly, in the unlikely event that it reaches a certain point, i'll realistically be morally obligated to).

## Building

To build, you will need a Linux-based operating system that complies with the latest version of POSIX.  You will also need a copy of Clang 13, and you need to have the latest versions of SDL2 and SDL\_ttf installed—since you're building from scratch, you'll also need the development files.  Installing these is rather system-specific, so if you can't figure it out, please ask in the appropriate place for your system, not here.  Finally, be warned that this was developed with glibc, and may not work with musl.

If you cannot build for your system, please file an issue.  I will be happy to work on porting the software to any _open and actively maintained_ systems out there.

Download a copy of the latest release from [the releases page](https://github.com/BlueManedHawk/Ithkuil-Memorization-Aid/releases), decompress and untar it, then go into the directory in a terminal and type `make release`.  This will create a file called `ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls.elf`, which you can execute to launch the program.  **Do not move this file or any of the files within the software directory**—doing so will make the program fail to launch.

There isn't any way to install the program yet—but then again, this isn't a professional project.

## Known Issues

These two should hopefully be fixable before release N.1.

- The timer doesn't do anything yet.
- The window is fixed at a resolution of 480p36@4:3, and can't be scaled, which may lead to serious difficulty if you're lucky enough to have a fucknormously huge monitor.  I did try to fix this once, but i was unable to reconcile the discrepancies in the apparent and actual position of the cursor.  Note that when scaling is implemented, the software is still going to render at 480p36@4:3 so as to reduce resource usage and to not give an advantage to those rich enough to afford a fucknormously huge monitor.
- The current color scheme is imperfect.
- There's a piece of code that's copied in both the menu code and the questions code that should be in its own bit.

The rest of these will probably need to wait.

- Some of the glyphs that Ithkuil uses don't exist in the font that the software uses, Barlow Condensed.
- After switching categories, you still need to answer another question in the previous category for the change to take effect.
- The asset format (see [below](https://github.com/BlueManedHawk/Ithkuil-Memorization-Aid#asset-format)) works okay for now, but it will almost certainly need to change in the future to adapt to whatever requirements we end up stumbling across.
- The current JSON library has an unpleasant interface that should be replaced with a better one.
- SDL may not have been the best choice.
- Seemingly random freezes can sometimes occur.  While the process can be harmlessly killed and restarted, this is annoying.
- Perhaps most obviously, the software currently only has files for the aspects and the cases.  Digitizing the entire language will take a long time (though admittedly most of that will be from the lexicon and the VxCs affixes), and more importantly, figuring out what the best way to organize everything is will be its own ball of fish. Eventually, this might even evolve into a completely separate project.

## Asset Format

This section is mostly aimed at developers, and is not required to use the program.

The questions are obtained from a collection of `.json` files in the `Assets` directory of this repository.  Each JSON file coresponds to one set of questions in one button on the main menu.  The are a structure containing two entries, each of which is in array.

The second array in each JSON file is an array of structures, each of which contains a digitization of all of the data for a particular option for a particular category; for example, in `Assets/Case.json`, each structure contains the name, TLA, Vc, description, etc. for a particular case, such as Orientitative.

The first array in each JSON file is an array of pairs (also arrays) of strings.  The first string is the format of a certain category of questions, and the second string is the format of the corresponding answers.  Each one is formatted by having the name of a key in the file's second array's structures begun with U+0098 and terminated with U+009C (the control characters for "start of string" and "string terminator", respectively).

When a question needs to be contructed, the program retrieves an entry from the first array of the file.  The program constructs four answers based on the second member of the entry from random entries in the second array in the file.  It then selects a random entry from the second array in the file and constructs the question base on the data there, and also overwrites a random one of the answers, which it keeps track of as the correct answer.  Finally, it checks all the answers to make sure there are no duplicates, and replaces any that exist, making sure _not_ to overwrite the correct answer.  Then, it pushes the finished product out to the rest of the program.

This is not a perfect format (and it certainly doesn't help that i'm not great at explaining it), and in the future it will probably need to be expanded to accomodate other types of questions, or fixed to be more efficient. But it works for now.
