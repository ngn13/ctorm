.section .text

.global cu_startswith
.global cu_contains
.global cu_streq
.global cu_strcmpu
.global cu_strlen

.type cu_startswith, @function
.type cu_contains, @function
.type cu_streq, @function
.type cu_strcmpu, @function
.type cu_strlen, @function

.L_cu_ret_true_3:
  // restore the registers
  pop %edx
.L_cu_ret_true_2:
  pop %esi
  pop %edi
  mov %ebp, %esp
  pop %ebp

  // return true (1)
  orb $1, %al
  ret

.L_cu_ret_false_3:
  // restore the registers
  pop %edx
.L_cu_ret_false_2:
  pop %esi
  pop %edi
  mov %ebp, %esp
  pop %ebp

  // return false (0)
  xor %al, %al
  ret

// WARN: these functions do NOT check for NULL pointers for extra speed

// check if a string (edi) starts with an another given string (esi)
cu_startswith:
  // save esp, edi and esi
  push %ebp
  mov %esp, %ebp
  push %edi
  push %esi

  // move args from stack to registers
  movl  8(%ebp), %edi
  movl 12(%ebp), %esi

  .Lcu_startswith_char:
    // get the current char
    movb (%esi), %al

    // check if we reached the end
    test %al, %al
    je .L_cu_ret_true_2

    // compare the char
    cmpb (%edi), %al
    jne .L_cu_ret_false_2

    // move onto the next char
    inc %edi
    inc %esi

    // loop
    jmp .Lcu_startswith_char

// check if a given byte (si) is present in a string (edi)
cu_contains:
  // save esp, edi and esi
  push %ebp
  mov %esp, %ebp
  push %edi
  push %esi

  // move args to registers
  movl  8(%ebp), %edi
  movl 12(%ebp), %esi

  mov %esi, %eax

  .Lcu_contains_char:
    // check if we reached the NULL termination char
    cmpb $0, (%edi)
    je .L_cu_ret_false_2

    // compare the chars, return true if char matches
    cmpb (%edi), %al
    je .L_cu_ret_true_2

    // move to the next char
    inc %edi

    // if not matches, loop
    jmp .Lcu_contains_char

// checks if two given strings (edi, esi) are exactly the same
cu_streq:
  // save esp, edi and esi
  push %ebp
  mov %esp, %ebp
  push %edi
  push %esi

  // move args to registers
  movl  8(%ebp), %edi
  movl 12(%ebp), %esi

  .Lcu_streq_char:
    // get the current char
    movb (%edi), %al

    // compare the chars
    cmpb (%esi), %al
    jne .L_cu_ret_false_2

    // check if we reached the end ('\0')
    test %al, %al
    je .L_cu_ret_true_2

    // move onto the next char
    inc %edi
    inc %esi

    // loop
    jmp .Lcu_streq_char

// compare two strings (edi, esi) until a given byte (dl) is reached
cu_strcmpu:
  // save esp, edi and esi
  push %ebp
  mov %esp, %ebp
  push %edi
  push %esi
  push %edx

  // move args to registers
  movl  8(%ebp), %edi
  movl 12(%ebp), %esi
  movl 16(%ebp), %edx

  .Lcu_strcmpu_char:
    // get the current char
    movb (%edi), %al

    // compare the chars
    cmpb (%esi), %al
    jne .L_cu_ret_false_3

    // check if we reached the given byte
    cmp %al, %dl
    je .L_cu_ret_true_3

    // check if we reached the end ('\0')
    test %al, %al
    je .L_cu_ret_true_3

    // move onto the next char
    inc %edi
    inc %esi

    // loop
    jmp .Lcu_strcmpu_char

// get the length of a string (edi)
cu_strlen:
  // save esp and edi
  push %ebp
  mov %esp, %ebp
  push %edi

  // move arg to register
  movl 8(%ebp), %edi

  mov %edi, %eax

  .Lcu_strlen_char:
    inc %eax
    cmpb $0, -1(%eax)
    jne .Lcu_strlen_char

  sub %edi, %eax
  dec %eax

  // restore the registers
  pop %edi
  mov %ebp, %esp
  pop %ebp
  ret
