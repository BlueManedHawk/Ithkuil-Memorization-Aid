# ëšho'hlorẓûţc hwomùaržrıtéu-erţtenļıls

This repository hosts a piece of software intended as a memorization aid for the various parts of the constructed language Ithkuil.  It is intended to be used not on its own, but as a supplementary tool within your broader learning of the language.

The software works by having you select a category of the language which you wish to learn, after which it will present you with an endless series of multiple-choice questions regarding the category you selected.  Questions which you answered recently will be given lower priority for selection, especially if you answered them correctly.  You can switch to a different category at any time, and you can set timers for the questions if you want.

The software is intended to be simple.  This means that there are no points systems, no online leaderboards, no formal "lessons".  This gives you, the user, the freedom to use this software in whatever way best suits your method of learning.

The questions are not taken randomly from a massive list; instead, they are procedurally constructed based on information and templates supplied with the program and provided to it at runtime.  This seeks to avoid the issue of questions becoming repetitive.

This is not a big, fancy, professional project.  This is just something I'm doing in my spare time, and I probably won't have much of it to spare on this.  If there's sufficient demand for it, i may choose to professionalize it (and frankly, in the unlikely event that it reaches a certain point, i'll realistically be morally obligated to).

## Asset Format

The questions are obtained from a collection of `.json` files in the `Assets` directory of this repository.  Each JSON file coresponds to one set of questions in one button on the main menu.  The files only have two entries, each of which is in array.

The second array in each JSON file is an array of structures, each of which contains a digitization of all of the data for a particular option for a morpholoogical component; for example, in `Assets/Case.json`, each structure contains the name, TLA, Vc, description, etc. for a particular case, such as Orientitative.  Each of these values is assigned to a key—this is important for the first array.

The first array in each JSON file is an array of pairs (also arrays) of strings.  The first string is the format of a certain category of questions, and the second string is the format of the corresponding answers.  Each one is formatted by having the name of a key in the file's second array's structures begun with U+0098 and terminated with U+009C (the control characters for "start of string" and "string terminator", respectively).

When a question needs to be contructed, the program retrieves an entry from the first array of the file.  For the question, it tries to find an entry in the second array of the file that has entries for all of the formatted keys in both the question and the answer.  It then replaces the formatted string in the question and in the answer, and then constructs three more answers from the other entries in the second array, making sure that they're not identical to the correct answer.  Then, it pushes the finished product out to the rest of the program.

This is not a perfect format (and it certainly doesn't help that i'm not great at explaining it), and in the future it will probably need to be expanded to accomodate other types of questions, or fixed to be more efficient. But it works for now.
