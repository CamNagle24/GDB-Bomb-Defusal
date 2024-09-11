# GDB Bomb Defusal
This was very similar to Project 2 where we utilized the restrictions on operators like +, -, *, or / to translate our functions into assembly code line by line. This showed me how registers worked and the different operations we can do on them. It got me much more acquainted with the low-level details of x86 and also gave me a greater appreciation for the features of “high-level” languages like C.

<img src="assets/AllFunctions.png" width="600">

### What I Learned
- 

<img src="assets/FunctionsGDB.png" width="600">

Also utilizing caller/callee-save registers

With adjusting the size of the stack, also using push and pop?

<img src="assets/Bomb.png" width="600">

In part 2 the goal is to defuse all the stages of a bomb without having it blow up to lose points. However, the trick is that when using the gdb you cannot see the C source code. You have to go line by line with a binary program in assembly stepping through/into its functions and loops. This made me understand assembly a lot more because I could enable the registers and see what was in them after completing each line. I could jump to different functions and set breakpoints and ones I didn’t want to run, (like the explode_bomb function).

## <a href="https://www.youtube.com/watch?v=UNz9k9E9IWM"> Demo Video on YouTube </a>

## Setup

If you want to play around with Bitwise Puzzle, feel free to clone my repo. To start, please enter the following commands on your terminal:

```
git clone https://github.com/CamNagle24/Bitwise-Puzzle
make
```

At this point, you are free to type in whatever sequence of commands you like.

```
./btest
```

./btest will give you all the tests that the code tests and passes.
