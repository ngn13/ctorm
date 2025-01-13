.section .text

.type cu_streq, @function
.type cu_strlen, @function
.type cu_startswith, @function
.type cu_strcmp_until, @function

.global cu_streq
.global cu_strlen
.global cu_startswith
.global cu_strcmp_until

.L__cu_ret_true:
  mov $1, %rax
  ret

.L__cu_ret_false:
  mov $0, %rax
  ret

/*

 * checks if two given strings (rdi, rsi) are EXACTLY the same
 * no NULL checking is done for extra speed

*/
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
    cmpb $0, (%rax)
    jne .Lcu_strlen_loop

  sub %rdi, %rax
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

/*

 * compare two strings until a given strings (rdi, rsi)
 * until a given byte (dl) is reached

*/
cu_strcmp_until:
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
  jmp cu_strcmp_until
