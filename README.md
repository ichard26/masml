# masml

> My ASM Language

Richard's silly ASM-like(-ish) language. This little project was meant both as a better
implementation over my original Python one *and* as a way to get better at C. This is my
second major piece of software written in C. 🎉

## Usage

To get a debug build (note it has ASAN), simply run `make` from the root of your checkout.
Otherwise, run `make build-release` for an optimized build. Afterwards, you'll have a
`masal` executable with which you can pass a MASML program to.

> The preferred file extension for MASML programs is `.masml`.

For example, you can run the `examples/factor-finder.masml` program I wrote to test the
parser and VM. You should see the following output:

```console
$ ./masml examples/factor-finder.masml
[OUTPUT] 1.000000
[OUTPUT] 27.000000
[OUTPUT] 3.000000
[OUTPUT] 9.000000
[OUTPUT] 9.000000
[OUTPUT] 3.000000
[OUTPUT] 27.000000
[OUTPUT] 1.000000
```

There are three flags available: `--show-result`, `--debug-parser`, and `--debug-vm`. The
first simply shows the value of the first VM register on termination. The other two enable
debug output for the parser and VM respectively.

For the parser, they show you the parsed instructions, what registers they're using and if
they have an argument (and if so, whether it's a variable or a constant). For the VM, they
log each instruction executed along with some details about the VM's internal state before
the instruction is executed.

Here's an example rerun with some flags. (`--debug-vm` is *very* noisy so it isn't
included here)

```console
$ ./masml examples/factor-finder.masml --show-result --debug-parser
[LINE 1  ] #0   SET-REGISTER  $1      27
[LINE 2  ] #1   STORE         $1      &product -> ram[0]
[LINE 5  ] #2   ADD           $1      1
[LINE 6  ] #3   STORE         $1      &loop_until -> ram[1]
[LINE 7  ] #4   SET-REGISTER  $2      1
[LINE 12 ] #5   LOAD          $1      &product -> ram[0]
[LINE 13 ] #6   MODULO        $1      (null)
[LINE 14 ] #7   GOTO-IF-NOT   $1      13
[LINE 17 ] #8   LOAD          $1      &loop_until -> ram[1]
[LINE 18 ] #9   ADD           $2      1
[LINE 19 ] #10  EQUAL         $1      (null)
[LINE 20 ] #11  GOTO-IF-NOT   $1      5
[LINE 22 ] #12  EXIT          (null)  (null)
[LINE 25 ] #13  LOAD          $1      &product -> ram[0]
[LINE 26 ] #14  DIVIDE        $1      (null)
[LINE 27 ] #15  PRINT         $2      (null)
[LINE 28 ] #16  PRINT         $1      (null)
[LINE 29 ] #17  GOTO          (null)  8
[OUTPUT] 1.000000
[OUTPUT] 27.000000
[OUTPUT] 3.000000
[OUTPUT] 9.000000
[OUTPUT] 9.000000
[OUTPUT] 3.000000
[OUTPUT] 27.000000
[OUTPUT] 1.000000
[RESULT] 1.000000
```

### Platform compatibility

`masal.c` targets C11 without using any POSIX specific features as far as I know, but I've
only built and tested this code on my Ubuntu 20.04.04 x86-64 machine. It *should* work on
other platforms, but I can't make any guarantees. And no, I'm not providing a VS build
configuration, sorry Windows folks.

Anyway, I'm still pretty awful at C. Expecting me to write portable C first try is a bit
much :)

### Language syntax

Each line in a program is an instruction. Each instruction can read registers and/or an
numerical constant (as an argument) and write to a register if it produces a result (eg.
`EQUAL`). There are two registers: `$1` and `$2`. Arguments come in two types: variables
(eg. `&daylily`) and numerical constants (eg. `27`).

Only `LOAD` and `STORE` can read and write a variable respectively. Actually, `PRINT` can
also read a variable. All other instructions only interact with the two VM registers
(which are simply double-precision float stack variables in the VM).

Each instruction can have a register and/or an argument specified. How the register and
argument are used is instruction-dependant. If a register specifier doesn't start with a
dollarsign (`$`), it will be instead treated as an argument.

Here are the available instructions and their semantics:

**LOAD**

Read a variable and store it in the target register.

**STORE**

Read the target register and set a variable.

**SET-REGISTER**

Write a numerical constant to the target register.

**SWAP**

Replace the value stored in register A with what's in register B and vice versa.

**ADD** / **SUBTRACT** / **MULTIPLY** / **DIVIDE** / **MODULO**

Perform X mathematical operation with register A as the left operand and register B as the
right operand if no numerical constant is specified. Otherwise, the left operand is the
target register and the right operand is the constant.

In either case, the result is written to the target register.

**EQUAL**

Compare the specified numerical constant against the target register. If a numerical
constant is not given, compare the two registers instead.

`1.0` will be written to the target register if the two operands are equal, otherwise
`0.0` is written.

**NOT**

Read the target register and write back `0.0` if it's non-zero, otherwise `1.0`.

**GOTO**

Unconditional jump to instruction X. Remember that instruction indexes start at zero!

**GOTO-IF** / **GOTO-IF-NOT**

If the target register is non-zero (or zero with `GOTO-IF-NOT`), jump to instruction X.
Remember that instruction indexes start at zero!

**PRINT**

If a variable is specified, print its value, otherwise print the target register's value.

**EXIT**

Stop execution.

______________________________________________________________________

To include a comment, prefix the line with `#`. Comments and empty lines are ignored in
the parser (lines with only whitespace probably break the parser right now though).

## Tooling

### Vim syntax highlighting

A Vim syntax file can be found at `tools/masml.vim`. To install it and set up syntax
highlighting:

1. Create these files and directories if they don't exist:

   ```php
   $HOME/.vim/ftdetect/masml.vim
   $HOME/.vim/syntax/masml.vim
   ```

1. Put the following in `$HOME/.vim/ftdetect/masml.vim`:

   ```vim
   autocmd BufRead,BufNewFile *.masml set filetype=masml
   ```

1. Copy and paste the contents of `tools/masml.vim` into `$HOME/.vim/syntax/masml.vim`

## Possible improvements

- Implement goto labels since specifying instruction indexes is error-prone
- Support `GOTO 0` (or a label pointing to the first instruction) properly (it may or may
  not crash currently ¯\\\_(ツ)\_/¯)
- Support an (practically) unlimited amount of variables (currently RAM is implemented
  simply as `double ram[1000] = {0}`)
- Support programs with lines of (practically) unlimited length (limit is 64 characters
  right now)
