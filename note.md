# Register allocating

## Some proofs

### Lemma 0

For any node $a,b$ that alive inside process(function) $F$, $a$ interfere with $b$ if and only if one of the following sustain:
1. There existing an instruction $i$ define $a$ satisfying:
   1. There is an instruction $i'$ use $b$ after $i$
   2. There is no instruction define $b$ between $i$ and $i'$
2. There existing an instruction $i$ define $b$ satisfying:
   1. There is an instruction $i'$ use $a$ after $i$
   2. There is no instruction define $a$ between $i$ and $i'$

Note: The "after" or "before" of an instruction refers to the order of execution control flow

#### Prove

Since live inside the function, thus for every instruction uses $a$ or $b$, there must be an instruction define the node before the use inside the process(function).

If $a$ interfere with $b$, then there must be an overlap of live range of $a$ and $b$, that is, there exists two pairs of instruction:
1. $i_a$ define $a$ and $i'_a$ uses $a$, no instruction define $a$ between $i_a$ and $i'_a$
2. $i_b$ define $b$ and $i'_b$ uses $b$, no instruction define $b$ between $i_b$ and $i'_b$

And $i_a,i'_a$ overlap on control flow order with $i_b,i'_b$

There for either $i_a$ is between $i_b,i'_b$ or $i_b$ is between $i_a,i'_a$ in control flow order, otherwise they will not interfere.

### Lemma 1

For any node $a,b$ that alive inside process(function) $F$, if $a$ does not interfere with $b$ before re-writing, then $a$ does not interfere with $b$ after re-writing.

#### Prove

If a node $a$ is spilled node, then:
1. Before every instruction uses this node, will add a mov to define it.
2. After every instruction defines this node, will add a mov to use it.

Thus, for every instruction $i$ uses $a$, there must be an instruction define $a$ right before $i$.

Assume $a,b$ interfere after re-writing, it must satisfy [Lemma 0](#lemma-0)

Assume there exists an instruction $i$ define $a$ satisfying:
1. There is an instruction $i'$ use $b$ after $i$
2. There is no instruction define $b$ between $i$ and $i'$

If $b$ is spilled, there would be an instruction define $b$ right before $i'$, conflict with the assumption.

If $b$ is not spilled, remove re-writing does not change the control flow order of $i, i'$ and will not add any instruction define $b$ between $i$ and $i'$ by remove re-writing. Hence according to [Lemma 0](#lemma-0) again, $a,b$ interferes before re-writing, which conflicts with the condition.

### Lemma 2

If a move instruction $\text{mov } a, b$, is calculated as coalesced in the previous instruction list, then it **may not** be calculated as coalesced in the instruction list after re-writing for spill.

#### Prove

Assume the following code piece with:
1. `r101` need to spill.
2. number color-able register `K=4`. 
3. `rax`, `rbx` are pre-colored registers. 
4. `rbx` live out of the block.

##### Before re-writing

```asm
mov r101,  7
mov rax,   8     // r101 interfere with rax
mov rbx,   r101
mov r100,  6
mov rax,   r100  // target mov instruction
mov r102,  1
mov r103,  2
mov r104,  3
mov r100,  1
mov r101,  9     // r100 interfere with r101
add rbx,   r101
add rbx,   r100
add rbx,   r102
add rbx,   r103
add rbx,   r104
```

According to [Lemma 0](#lemma-0), due to `mov rax, 8` it can be found that `r101` conflict with `rax`.

Similarly due to `mov r101, 9` can be found that `r101` conflict with `r100` before re-writing.

It can be seen that `rax` only conflict with `r101` and `rbx`

It can be seen that `r100` only conflict with `r101`

According to George method, instruction `mov rax, r100` can be coalesced since the only neighbor of `r100` is `r101`, which already interfere with `rax`.

##### After re-writing

```asm
mov r101,  7
mov [mem], r101  // re-writing: save r101
mov rax,   8     // r101 interfere with rax
mov r101,  [mem] // re-writing: fetch r101
mov rbx,   r101
mov r100,  6
mov rax,   r100  // target mov instruction
mov r102,  1
mov r103,  2
mov r104,  3
mov r100,  1
mov r101,  9     // r100 interfere with r101
mov [mem], r101  // re-writing: save r101
mov r101,  [mem] // re-writing: fetch r101
add rbx,   r101
add rbx,   r100
add rbx,   r102
add rbx,   r103
add rbx,   r104
```

Due to the fetch instruction after `mov rax, 8`, `rax` no longer interfere with `r101`

Due to `mov r101, 9` can be found that `r101` still interfere with `r102`, `r103`, `r104` and `r100`

Thus the neighbor of `r100` is on significant degree and not a neighbor of `rax`

Hence according to George method, instruction `mov rax, r100` cannot be coalesced.