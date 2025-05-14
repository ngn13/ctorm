.section .text

.global cu_streq
.global cu_strlen
.global cu_strcmpu
.global cu_startswith
.global cu_endswith
.global cu_contains

.type cu_streq, @function
.type cu_strlen, @function
.type cu_strcmpu, @function
.type cu_startswith, @function
.type cu_endswith, @function
.type cu_contains, @function

.L__cu_ret_true:
  orb $1, %al
  ret

.L__cu_ret_false:
  xor %al, %al
  ret

// WARN: these functions do NOT check for NULL pointers for extra speed

// checks if two given strings (rdi, rsi) are exactly the same
cu_streq:
  // get the current char
  movb (%rdi), %al

  // compare the chars
  cmpb (%rsi), %al
  jne .L__cu_ret_false

  // check if we reached the end ('\0')
  test %al, %al
  je .L__cu_ret_true

  // move onto the next char
  inc %rdi
  inc %rsi

  // loop
  jmp cu_streq

// get the length of a string (rdi)
cu_strlen:
  mov %rdi, %rax

  .Lcu_strlen_loop:
    inc %rax
    cmpb $0, -1(%rax)
    jne .Lcu_strlen_loop

  sub %rdi, %rax
  dec %rax

  ret

// check if a string (rdi) starts with an another given string (rsi)
cu_startswith:
  // get the current char
  movb (%rsi), %al

  // check if we reached the end
  test %al, %al
  je .L__cu_ret_true

  // compare the char
  cmpb (%rdi), %al
  jne .L__cu_ret_false

  // move onto the next char
  inc %rdi
  inc %rsi

  // loop
  jmp cu_startswith

// compare two strings (rdi, rsi) until a given byte (dl) is reached
cu_strcmpu:
  // get the current char
  movb (%rdi), %al

  // compare the chars
  cmpb (%rsi), %al
  jne .L__cu_ret_false

  // check if we reached the given byte
  cmp %al, %dl
  je .L__cu_ret_true

  // check if we reached the end ('\0')
  test %al, %al
  je .L__cu_ret_true

  // move onto the next char
  inc %rdi
  inc %rsi

  // loop
  jmp cu_strcmpu

// check if a given byte (si) is present in a string (rdi)
cu_contains:
  mov %rsi, %rax

  .L__cu_contains_loop:
    // check if we reached the NULL termination char
    cmpb $0, (%rdi)
    je .L__cu_ret_false

    // compare the chars, return true if char matches
    cmpb (%rdi), %al
    je .L__cu_ret_true

    // move to the next char
    inc %rdi

  // if not matches, loop
  jmp .L__cu_contains_loop
