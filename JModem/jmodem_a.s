	.file	"jmodem_a.c"
	.comm	handle,2,2
	.comm	syst,16,4
	.globl	user_abort
	.bss
	.align 2
	.type	user_abort, @object
	.size	user_abort, 2
user_abort:
	.zero	2
	.comm	in_buffer,4,4
	.comm	out_buffer,4,4
	.comm	comp_buffer,4,4
	.comm	file_buffer,4,4
	.comm	int_buffer,4,4
	.comm	start,4,4
	.comm	finish,4,4
	.comm	buff,4,4
	.globl	abrt
	.data
	.type	abrt, @object
	.size	abrt, 9
abrt:
	.string	"Aborted!"
	.globl	okay
	.type	okay, @object
	.size	okay, 9
okay:
	.string	"Okay    "
	.globl	retry
	.type	retry, @object
	.size	retry, 9
retry:
	.string	"Retry   "
	.globl	done
	.type	done, @object
	.size	done, 9
done:
	.string	"Done!   "
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%esi
	pushl	%ebx
	andl	$-16, %esp
	subl	$64, %esp
	.cfi_offset 6, -12
	.cfi_offset 3, -16
	movl	8(%ebp), %eax
	movw	%ax, 28(%esp)
	movw	$0, 42(%esp)
	movzwl	28(%esp), %eax
	movzwl	%ax, %eax
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	get_inp
	movl	%eax, 52(%esp)
	movl	52(%esp), %eax
	testl	%eax, %eax
	jne	.L2
	call	disp
	movl	$1, %eax
	jmp	.L42
.L2:
	movzwl	28(%esp), %eax
	movzwl	%ax, %eax
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	get_fun
	movl	%eax, 56(%esp)
	cmpl	$0, 56(%esp)
	jne	.L4
	call	disp
	movl	$8, %eax
	jmp	.L42
.L4:
	movzwl	28(%esp), %eax
	movzwl	%ax, %eax
	movl	12(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	get_prt
	movl	%eax, 60(%esp)
	cmpl	$0, 60(%esp)
	jne	.L5
	call	disp
	movl	$8, %eax
	jmp	.L42
.L5:
	movl	60(%esp), %eax
	movzbl	(%eax), %eax
	movsbl	%al, %eax
	movl	%eax, (%esp)
	call	get_port
	movw	%ax, port
	movl	$9216, (%esp)
	call	allocate_memory
	movl	%eax, in_buffer
	movl	in_buffer, %eax
	testl	%eax, %eax
	jne	.L6
	movl	$4, %eax
	jmp	.L42
.L6:
	movl	$9216, (%esp)
	call	allocate_memory
	movl	%eax, out_buffer
	movl	out_buffer, %eax
	testl	%eax, %eax
	jne	.L7
	movl	$4, %eax
	jmp	.L42
.L7:
	movl	$9216, (%esp)
	call	allocate_memory
	movl	%eax, comp_buffer
	movl	comp_buffer, %eax
	testl	%eax, %eax
	jne	.L8
	movl	$4, %eax
	jmp	.L42
.L8:
	movl	$9216, (%esp)
	call	allocate_memory
	movl	%eax, file_buffer
	movl	file_buffer, %eax
	testl	%eax, %eax
	jne	.L9
	movl	$4, %eax
	jmp	.L42
.L9:
	movl	$9216, (%esp)
	call	allocate_memory
	movl	%eax, int_buffer
	movl	int_buffer, %eax
	testl	%eax, %eax
	jne	.L10
	movl	$4, %eax
	jmp	.L42
.L10:
	movl	$0, 8(%esp)
	movl	$0, 4(%esp)
	movl	$1, (%esp)
	call	screen
	movw	$512, syst+2
	movl	$0, syst+4
	movw	$0, syst
	movl	$okay, syst+12
	movl	56(%esp), %eax
	movzbl	(%eax), %eax
	movsbl	%al, %eax
	cmpl	$82, %eax
	je	.L12
	cmpl	$83, %eax
	je	.L13
	jmp	.L11
.L12:
	movl	$0, 12(%esp)
	leal	52(%esp), %eax
	movl	%eax, 8(%esp)
	movl	$handle, 4(%esp)
	movl	$2, (%esp)
	call	file_io
	testw	%ax, %ax
	jne	.L14
	movl	in_buffer, %eax
	movl	%eax, buff
	movzwl	port, %eax
	movzwl	%ax, %eax
	movl	%eax, (%esp)
	call	open_chan
	movl	$0, 8(%esp)
	movl	$0, 4(%esp)
	movl	$4, (%esp)
	call	screen
	call	rx_sync
	movw	%ax, 42(%esp)
	cmpw	$0, 42(%esp)
	jne	.L15
	movl	$0, 8(%esp)
	movl	$0, 4(%esp)
	movl	$12, (%esp)
	call	screen
.L15:
	movw	$-1, 46(%esp)
	movw	$10, 44(%esp)
	jmp	.L16
.L24:
	movl	$start, (%esp)
	call	time
	movl	$0, 8(%esp)
	movl	$syst, 4(%esp)
	movl	$10, (%esp)
	call	screen
	movl	in_buffer, %eax
	movl	%eax, 4(%esp)
	movl	$syst+2, (%esp)
	call	recv_blk
	movw	%ax, 42(%esp)
	cmpw	$0, 42(%esp)
	je	.L17
	jmp	.L18
.L17:
	movl	in_buffer, %edx
	movzwl	syst+2, %eax
	movzwl	%ax, %eax
	movl	%edx, 8(%esp)
	movl	%eax, 4(%esp)
	movl	$0, (%esp)
	call	calc_crc
	testw	%ax, %ax
	jne	.L19
	movl	buff, %eax
	movzbl	3(%eax), %eax
	movzwl	syst, %edx
	addl	$1, %edx
	cmpb	%dl, %al
	jne	.L19
	movl	$okay, syst+12
	movw	$10, 44(%esp)
	movzwl	syst+2, %eax
	subl	$6, %eax
	movw	%ax, syst+2
	movl	out_buffer, %eax
	movb	$6, (%eax)
	movl	out_buffer, %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	write_chan
	movl	buff, %eax
	movzbl	2(%eax), %eax
	movzbl	%al, %eax
	andl	$2, %eax
	testl	%eax, %eax
	je	.L20
	movl	file_buffer, %edx
	movl	buff, %eax
	leal	4(%eax), %ecx
	movzwl	syst+2, %eax
	movzwl	%ax, %eax
	movl	%edx, 8(%esp)
	movl	%ecx, 4(%esp)
	movl	%eax, (%esp)
	call	decode
	movw	%ax, syst+2
	jmp	.L21
.L20:
	movzwl	syst+2, %eax
	movzwl	%ax, %edx
	movl	buff, %eax
	leal	4(%eax), %ecx
	movl	file_buffer, %eax
	movl	%edx, 8(%esp)
	movl	%ecx, 4(%esp)
	movl	%eax, (%esp)
	call	memcpy
.L21:
	movzwl	syst+2, %eax
	movzwl	%ax, %eax
	movl	%eax, 12(%esp)
	movl	$file_buffer, 8(%esp)
	movl	$handle, 4(%esp)
	movl	$3, (%esp)
	call	file_io
	movw	%ax, 46(%esp)
	movl	syst+4, %edx
	movzwl	46(%esp), %eax
	addl	%edx, %eax
	movl	%eax, syst+4
	movzwl	syst, %eax
	addl	$1, %eax
	movw	%ax, syst
	movl	$finish, (%esp)
	call	time
	movl	finish, %edx
	movl	start, %eax
	cmpl	%eax, %edx
	je	.L22
	movzwl	46(%esp), %eax
	movl	finish, %ecx
	movl	start, %edx
	subl	%edx, %ecx
	movl	%ecx, %ebx
	movl	$0, %edx
	divl	%ebx
	movw	%ax, syst+8
.L22:
	movl	buff, %eax
	movzbl	2(%eax), %eax
	movzbl	%al, %eax
	andl	$4, %eax
	testl	%eax, %eax
	je	.L23
	movl	$0, 12(%esp)
	leal	52(%esp), %eax
	movl	%eax, 8(%esp)
	movl	$handle, 4(%esp)
	movl	$4, (%esp)
	call	file_io
	movzwl	port, %eax
	movzwl	%ax, %eax
	movl	%eax, (%esp)
	call	close_chan
	movw	$0, 42(%esp)
	jmp	.L11
.L23:
	jmp	.L16
.L19:
	movl	out_buffer, %eax
	movb	$21, (%eax)
	movl	$retry, syst+12
	movl	out_buffer, %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	write_chan
.L16:
	cmpw	$0, 46(%esp)
	je	.L18
	movzwl	user_abort, %eax
	testw	%ax, %ax
	jne	.L18
	cmpw	$0, 42(%esp)
	jne	.L18
	movzwl	44(%esp), %eax
	leal	-1(%eax), %edx
	movw	%dx, 44(%esp)
	testw	%ax, %ax
	jne	.L24
.L18:
	movzwl	port, %eax
	movzwl	%ax, %eax
	movl	%eax, (%esp)
	call	close_chan
	movl	$0, 12(%esp)
	leal	52(%esp), %eax
	movl	%eax, 8(%esp)
	movl	$handle, 4(%esp)
	movl	$5, (%esp)
	call	file_io
	movw	$7, 42(%esp)
	jmp	.L11
.L14:
	movw	$3, 42(%esp)
	jmp	.L11
.L13:
	movl	$0, 12(%esp)
	leal	52(%esp), %eax
	movl	%eax, 8(%esp)
	movl	$handle, 4(%esp)
	movl	$1, (%esp)
	call	file_io
	testw	%ax, %ax
	jne	.L25
	movl	out_buffer, %eax
	movl	%eax, buff
	movl	$0, syst+4
	movzwl	port, %eax
	movzwl	%ax, %eax
	movl	%eax, (%esp)
	call	open_chan
	movw	$-1, 48(%esp)
	movl	$0, 8(%esp)
	movl	$0, 4(%esp)
	movl	$4, (%esp)
	call	screen
	call	tx_sync
	movw	%ax, 42(%esp)
	cmpw	$0, 42(%esp)
	jne	.L26
	movl	$0, 8(%esp)
	movl	$0, 4(%esp)
	movl	$11, (%esp)
	call	screen
	jmp	.L27
.L26:
	jmp	.L27
.L35:
	movl	$start, (%esp)
	call	time
	movzwl	syst+2, %eax
	movzwl	%ax, %eax
	movl	%eax, 12(%esp)
	movl	$file_buffer, 8(%esp)
	movl	$handle, 4(%esp)
	movl	$6, (%esp)
	call	file_io
	movw	%ax, 48(%esp)
	cmpw	$0, 48(%esp)
	jne	.L28
	jmp	.L29
.L28:
	movl	syst+4, %edx
	movzwl	48(%esp), %eax
	addl	%edx, %eax
	movl	%eax, syst+4
	movl	$0, 8(%esp)
	movl	$syst, 4(%esp)
	movl	$10, (%esp)
	call	screen
	movl	buff, %eax
	movzwl	syst, %edx
	addl	$1, %edx
	movw	%dx, syst
	movzwl	syst, %edx
	movb	%dl, 3(%eax)
	movzwl	syst+2, %eax
	cmpw	48(%esp), %ax
	je	.L30
	movl	buff, %eax
	movb	$4, 2(%eax)
	jmp	.L31
.L30:
	movl	buff, %eax
	movb	$1, 2(%eax)
.L31:
	movl	comp_buffer, %ecx
	movl	file_buffer, %edx
	movzwl	48(%esp), %eax
	movl	%ecx, 8(%esp)
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	encode
	movw	%ax, 50(%esp)
	movzwl	50(%esp), %eax
	cmpw	48(%esp), %ax
	jnb	.L32
	movl	buff, %eax
	movzwl	50(%esp), %edx
	addl	$6, %edx
	movw	%dx, (%eax)
	movl	buff, %eax
	movl	buff, %edx
	movzbl	2(%edx), %edx
	orl	$2, %edx
	movb	%dl, 2(%eax)
	movzwl	50(%esp), %edx
	movl	comp_buffer, %eax
	movl	buff, %ecx
	addl	$4, %ecx
	movl	%edx, 8(%esp)
	movl	%eax, 4(%esp)
	movl	%ecx, (%esp)
	call	memcpy
	jmp	.L33
.L32:
	movl	buff, %eax
	movzwl	48(%esp), %edx
	addl	$6, %edx
	movw	%dx, (%eax)
	movzwl	48(%esp), %edx
	movl	file_buffer, %eax
	movl	buff, %ecx
	addl	$4, %ecx
	movl	%edx, 8(%esp)
	movl	%eax, 4(%esp)
	movl	%ecx, (%esp)
	call	memcpy
.L33:
	movl	out_buffer, %edx
	movl	buff, %eax
	movzwl	(%eax), %eax
	movzwl	%ax, %eax
	movl	%edx, 8(%esp)
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	calc_crc
	movl	out_buffer, %edx
	movl	buff, %eax
	movzwl	(%eax), %eax
	movzwl	%ax, %eax
	movl	%edx, 8(%esp)
	movl	$syst, 4(%esp)
	movl	%eax, (%esp)
	call	send_blk
	movw	%ax, 42(%esp)
	movl	$finish, (%esp)
	call	time
	movl	finish, %edx
	movl	start, %eax
	cmpl	%eax, %edx
	je	.L34
	movzwl	48(%esp), %eax
	movl	finish, %ecx
	movl	start, %edx
	subl	%edx, %ecx
	movl	%ecx, %esi
	movl	$0, %edx
	divl	%esi
	movw	%ax, syst+8
.L34:
	movl	buff, %eax
	movzbl	2(%eax), %eax
	cmpb	$4, %al
	jne	.L27
	jmp	.L29
.L27:
	movzwl	user_abort, %eax
	testw	%ax, %ax
	jne	.L29
	cmpw	$0, 42(%esp)
	je	.L35
.L29:
	movzwl	port, %eax
	movzwl	%ax, %eax
	movl	%eax, (%esp)
	call	close_chan
	cmpw	$0, 42(%esp)
	je	.L36
	movl	$abrt, syst+12
	jmp	.L37
.L36:
	movl	$done, syst+12
.L37:
	movl	$0, 12(%esp)
	leal	52(%esp), %eax
	movl	%eax, 8(%esp)
	movl	$handle, 4(%esp)
	movl	$4, (%esp)
	call	file_io
	movl	$0, 8(%esp)
	movl	$syst, 4(%esp)
	movl	$10, (%esp)
	call	screen
	jmp	.L43
.L25:
	movw	$1, 42(%esp)
.L43:
	nop
.L11:
	movl	in_buffer, %eax
	movl	%eax, (%esp)
	call	free
	movl	out_buffer, %eax
	movl	%eax, (%esp)
	call	free
	movl	comp_buffer, %eax
	movl	%eax, (%esp)
	call	free
	movl	file_buffer, %eax
	movl	%eax, (%esp)
	call	free
	cmpw	$0, 42(%esp)
	je	.L39
	movl	$finish, (%esp)
	call	time
	movl	$0, start
	movl	finish, %eax
	addl	$5, %eax
	movl	%eax, finish
	jmp	.L40
.L41:
	movl	$start, (%esp)
	call	time
.L40:
	movl	finish, %edx
	movl	start, %eax
	cmpl	%eax, %edx
	ja	.L41
.L39:
	movl	$0, 8(%esp)
	movl	$0, 4(%esp)
	movl	$13, (%esp)
	call	screen
	movzwl	42(%esp), %eax
.L42:
	leal	-8(%ebp), %esp
	popl	%ebx
	.cfi_restore 3
	popl	%esi
	.cfi_restore 6
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.globl	send_blk
	.type	send_blk, @function
send_blk:
.LFB3:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movl	8(%ebp), %eax
	movw	%ax, -28(%ebp)
	movw	$10, -10(%ebp)
	jmp	.L45
.L55:
	movzwl	-28(%ebp), %eax
	movl	16(%ebp), %edx
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	write_chan
	call	flush
.L47:
	movb	$0, -11(%ebp)
	leal	-11(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	read_chan
	movzbl	-11(%ebp), %eax
	cmpb	$6, %al
	je	.L46
	movzbl	-11(%ebp), %eax
	cmpb	$24, %al
	je	.L46
	movzbl	-11(%ebp), %eax
	cmpb	$21, %al
	je	.L46
	movzbl	-11(%ebp), %eax
	movzbl	%al, %eax
	testl	%eax, %eax
	je	.L46
	movzwl	user_abort, %eax
	testw	%ax, %ax
	je	.L47
.L46:
	movzbl	-11(%ebp), %eax
	cmpb	$24, %al
	jne	.L48
	jmp	.L49
.L48:
	movzbl	-11(%ebp), %eax
	cmpb	$6, %al
	jne	.L50
	cmpw	$9, -10(%ebp)
	jne	.L51
	movl	12(%ebp), %eax
	movzwl	2(%eax), %eax
	leal	512(%eax), %edx
	movl	12(%ebp), %eax
	movw	%dx, 2(%eax)
	movl	12(%ebp), %eax
	movzwl	2(%eax), %eax
	cmpw	$8192, %ax
	jbe	.L53
	movl	12(%ebp), %eax
	movw	$8192, 2(%eax)
	jmp	.L53
.L51:
	movl	12(%ebp), %eax
	movzwl	2(%eax), %eax
	shrw	%ax
	movl	%eax, %edx
	movl	12(%ebp), %eax
	movw	%dx, 2(%eax)
	movl	12(%ebp), %eax
	movzwl	2(%eax), %eax
	cmpw	$63, %ax
	ja	.L53
	movl	12(%ebp), %eax
	movw	$64, 2(%eax)
.L53:
	movl	12(%ebp), %eax
	movl	$okay, 12(%eax)
	movl	$0, %eax
	jmp	.L58
.L50:
	movl	12(%ebp), %eax
	movl	$retry, 12(%eax)
	movl	$0, 8(%esp)
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$10, (%esp)
	call	screen
.L45:
	movzwl	-10(%ebp), %eax
	leal	-1(%eax), %edx
	movw	%dx, -10(%ebp)
	testw	%ax, %ax
	je	.L49
	movzwl	user_abort, %eax
	testw	%ax, %ax
	je	.L55
.L49:
	movb	$24, -11(%ebp)
	movw	$0, -10(%ebp)
	jmp	.L56
.L57:
	leal	-11(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	write_chan
	movzwl	-10(%ebp), %eax
	addl	$1, %eax
	movw	%ax, -10(%ebp)
.L56:
	cmpw	$5, -10(%ebp)
	jbe	.L57
	movl	$7, %eax
.L58:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE3:
	.size	send_blk, .-send_blk
	.globl	recv_blk
	.type	recv_blk, @function
recv_blk:
.LFB4:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	movw	$10, -16(%ebp)
	movl	12(%ebp), %eax
	movl	%eax, -12(%ebp)
	jmp	.L60
.L65:
	movl	12(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$2, (%esp)
	call	read_chan
	movw	%ax, -14(%ebp)
	cmpw	$2, -14(%ebp)
	jne	.L61
	movl	-12(%ebp), %eax
	movzwl	(%eax), %edx
	movl	8(%ebp), %eax
	movw	%dx, (%eax)
	movl	8(%ebp), %eax
	movzwl	(%eax), %eax
	cmpw	$9216, %ax
	jbe	.L62
	jmp	.L63
.L62:
	movl	-12(%ebp), %eax
	leal	2(%eax), %edx
	movl	8(%ebp), %eax
	movzwl	(%eax), %eax
	subl	$2, %eax
	movzwl	%ax, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	read_chan
	movw	%ax, -14(%ebp)
	movzwl	-14(%ebp), %edx
	movl	8(%ebp), %eax
	movzwl	(%eax), %eax
	movzwl	%ax, %eax
	subl	$2, %eax
	cmpl	%eax, %edx
	jne	.L61
	movl	$0, %eax
	jmp	.L68
.L61:
	movb	$21, -17(%ebp)
	leal	-17(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	write_chan
	call	flush
.L60:
	movzwl	-16(%ebp), %eax
	leal	-1(%eax), %edx
	movw	%dx, -16(%ebp)
	testw	%ax, %ax
	je	.L63
	movzwl	user_abort, %eax
	testw	%ax, %ax
	je	.L65
.L63:
	movb	$24, -17(%ebp)
	movw	$0, -16(%ebp)
	jmp	.L66
.L67:
	leal	-17(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	write_chan
	movzwl	-16(%ebp), %eax
	addl	$1, %eax
	movw	%ax, -16(%ebp)
.L66:
	cmpw	$5, -16(%ebp)
	jbe	.L67
	movl	$7, %eax
.L68:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE4:
	.size	recv_blk, .-recv_blk
	.globl	rx_sync
	.type	rx_sync, @function
rx_sync:
.LFB5:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$40, %esp
	call	flush
	jmp	.L70
.L76:
	movb	$0, -11(%ebp)
	leal	-11(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	read_chan
	movzbl	-11(%ebp), %eax
	cmpb	$24, %al
	jne	.L71
	jmp	.L72
.L71:
	movzbl	-11(%ebp), %eax
	cmpb	$6, %al
	jne	.L73
	movl	$0, %eax
	jmp	.L79
.L73:
	movzbl	-11(%ebp), %eax
	cmpb	$21, %al
	jne	.L75
	movb	$6, -11(%ebp)
	leal	-11(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	write_chan
	movl	$0, %eax
	jmp	.L79
.L75:
	movb	$21, -11(%ebp)
	leal	-11(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	write_chan
.L70:
	movzwl	user_abort, %eax
	testw	%ax, %ax
	je	.L76
.L72:
	movb	$24, -11(%ebp)
	movw	$0, -10(%ebp)
	jmp	.L77
.L78:
	leal	-11(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$1, (%esp)
	call	write_chan
	movzwl	-10(%ebp), %eax
	addl	$1, %eax
	movw	%ax, -10(%ebp)
.L77:
	cmpw	$7, -10(%ebp)
	jle	.L78
	movl	$7, %eax
.L79:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE5:
	.size	rx_sync, .-rx_sync
	.globl	tx_sync
	.type	tx_sync, @function
tx_sync:
.LFB6:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	call	rx_sync
	movw	%ax, -10(%ebp)
	cmpw	$0, -10(%ebp)
	je	.L81
	movzwl	-10(%ebp), %eax
	jmp	.L82
.L81:
	call	flush
	movw	$5, timer
	nop
.L83:
	movzwl	timer, %eax
	testw	%ax, %ax
	jne	.L83
	movl	$0, %eax
.L82:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE6:
	.size	tx_sync, .-tx_sync
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04.4) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
