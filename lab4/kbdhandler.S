#include "i8042.h"

.data

.text
.globl _kbd_int_handler_ass
_kbd_int_handler_ass:

		pushl %ebp
		movl %esp, %ebp				//setup base pointer
		pushl %ebx
		movl $0, %eax
		movl $MAX_ATTEMPTS, %ecx
start1:
		inb $STAT_REG				//read status
		testb $OUT_BUF_STATUS,%al
		jnz out_buf_ready			//can read if OUT_BUF_STATUS is set to 1
		pushl %ecx					//preserve number of attempts
		movl $ASS_NUM_LOOPS, %ecx
loop1:
		loopl loop1					//wait
		popl %ecx					//get back the number of attempts
		loopl start1
		movl $1, %eax 				//max number of attempts reached, return 1
		jmp end1
out_buf_ready:
 		movb $PARITY_ERROR, %bl
		orb $TIMEOUT, %bl
		testb %bl, %al
		jz success					//if neither PARITY_ERROR nor TIMEOUT is set, no errors occurred
		movl $1, %eax				//else return 1
		jmp end1
success:
		inb $OUT_BUF				//read scancode
		movl 8(%ebp), %ebx			//get argument(address of variable that keeps the scancode)
		movl $0, (%ebx)				//variable is of type long, set  it to 0 before writing
		movb %al, (%ebx)			//write value from keyboard to variable
		movl $0, %eax				//on success, return 0
end1:	popl %ebx

		leave
		ret

