## mcall
1. 确保栈空间足够。线程栈有8m，协裎栈默认2k。
2. 
Runtime.mcall eliminates the need for fake scheduler states that
exist just to run a bit of code on the m->g0 stack
(Grecovery, Gstackalloc).

The elimination of the scheduler as a loop that stops and
starts using gosave and gogo fixes a bad interaction with the
way cgo uses the m->g0 stack.  Cgo runs external (gcc-compiled)
C functions on that stack, and then when calling back into Go,
it sets m->g0->sched.sp below the added call frames, so that
other uses of m->g0's stack will not interfere with those frames.
Unfortunately, gogo (longjmp) back to the scheduler loop at
this point would end up running scheduler with the lower
sp, which no longer points at a valid stack frame for
a call to scheduler.  If scheduler then wrote any function call
arguments or local variables to where it expected the stack
frame to be, it would overwrite other data on the stack.
I realized this possibility while debugging a problem with
calling complex Go code in a Go -> C -> Go cgo callback.
This wasn't the bug I was looking for, it turns out, but I believe
it is a real bug nonetheless.  Switching to runtime.mcall, which
only adds new frames to the stack and never jumps into
functions running in existing ones, fixes this bug.

* Move cgo-related code out of proc.c into cgocall.c.
* Add very large comment describing cgo call sequences.
* Simpilify, regularize cgo function implementations and names.
* Add test suite as misc/cgo/test.

Now the Go -> C path calls cgocall, which calls asmcgocall,
and the C -> Go path calls cgocallback, which calls cgocallbackg.

The shuffling, which affects mainly the callback case, moves
most of the callback implementation to cgocallback running
on the m->curg stack (not the m->g0 scheduler stack) and
only while accounted for with $GOMAXPROCS (between calls
to exitsyscall and entersyscall).

The previous callback code did not block in startcgocallback's
approximation to exitsyscall, so if, say, the garbage collector
were running, it would still barge in and start doing things
like call malloc.  Similarly endcgocallback's approximation of
entersyscall did not call matchmg to kick off new OS threads
when necessary, which caused the bug in issue 1560.

Fixes issue 1560.
  