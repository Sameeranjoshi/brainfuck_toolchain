.section .text
.globl main

main:
	.extern malloc
	.extern memset
	.extern putchar
# Prologue
	pushq %rbp                # Save the base pointer
	movq %rsp, %rbp          # Set base pointer to stack pointer
	pushq %rbx                # Save rbx
	pushq %r12                # Save r12
# Memory allocation call
	movq $30000, %rdi         # Allocate 30,000 bytes
	call malloc                # Call malloc function
# Save the return pointer
	movq %rax, %r12           # Store returned pointer in r12
	movq %rax, %rbx           # Store original pointer in rbx
# Zero out the allocated memory
	movq %r12, %rdi           # Destination pointer
	movq $0, %rsi              # Value to set (zero)
	movq $30000, %rdx         # Number of bytes
	call memset                # Call memset function
	addq $1, %r12
	addb $1, (%r12)
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	call getchar
	movb %al, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
#LOOP 
.L6_start:
	cmpb $0, (%r12)
	je .L6_end

	subb $1, (%r12)
	subb $1, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	addq $1, %r12

	jmp .L6_start
.L6_end:
#LOOP END
#LOOP 
.L15_start:
	cmpb $0, (%r12)
	je .L15_end


	jmp .L15_start
.L15_end:
#LOOP END
	subb $1, (%r12)
	addb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
#LOOP 
.L27_start:
	cmpb $0, (%r12)
	je .L27_end

	subq $1, %r12
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	addq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
#LOOP 
.L34_start:
	cmpb $0, (%r12)
	je .L34_end

	addq $1, %r12
	addq $1, %r12
	addq $1, %r12
#LOOP 
.L38_start:
	cmpb $0, (%r12)
	je .L38_end

	addq $1, %r12
	subq $1, %r12
	call getchar
	movb %al, (%r12)
	subb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	addq $1, %r12
#LOOP 
.L55_start:
	cmpb $0, (%r12)
	je .L55_end

	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12

	jmp .L55_start
.L55_end:
#LOOP END
#LOOP 
.L60_start:
	cmpb $0, (%r12)
	je .L60_end

#LOOP 
.L61_start:
	cmpb $0, (%r12)
	je .L61_end


	jmp .L61_start
.L61_end:
#LOOP END
	addb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
#LOOP 
.L68_start:
	cmpb $0, (%r12)
	je .L68_end

	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
#LOOP 
.L78_start:
	cmpb $0, (%r12)
	je .L78_end

	addb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
#LOOP 
.L82_start:
	cmpb $0, (%r12)
	je .L82_end

#LOOP 
.L83_start:
	cmpb $0, (%r12)
	je .L83_end

	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	subb $1, (%r12)
#LOOP 
.L90_start:
	cmpb $0, (%r12)
	je .L90_end

	subb $1, (%r12)

	jmp .L90_start
.L90_end:
#LOOP END
	addq $1, %r12
#LOOP 
.L94_start:
	cmpb $0, (%r12)
	je .L94_end

	subq $1, %r12
	addq $1, %r12
#LOOP 
.L97_start:
	cmpb $0, (%r12)
	je .L97_end

	addb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
#LOOP 
.L104_start:
	cmpb $0, (%r12)
	je .L104_end

	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
#LOOP 
.L111_start:
	cmpb $0, (%r12)
	je .L111_end

	subq $1, %r12
	subq $1, %r12

	jmp .L111_start
.L111_end:
#LOOP END
	subb $1, (%r12)
	addb $1, (%r12)

	jmp .L104_start
.L104_end:
#LOOP END
#LOOP 
.L118_start:
	cmpb $0, (%r12)
	je .L118_end

	subb $1, (%r12)
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	subb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)

	jmp .L118_start
.L118_end:
#LOOP END
	subq $1, %r12
	subb $1, (%r12)

	jmp .L97_start
.L97_end:
#LOOP END
	subb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	addq $1, %r12
	subq $1, %r12
	subq $1, %r12
	subq $1, %r12
	addq $1, %r12
#LOOP 
.L138_start:
	cmpb $0, (%r12)
	je .L138_end

	addq $1, %r12
#LOOP 
.L140_start:
	cmpb $0, (%r12)
	je .L140_end

	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	addq $1, %r12
	subq $1, %r12
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	subq $1, %r12
	addq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
#LOOP 
.L166_start:
	cmpb $0, (%r12)
	je .L166_end

	subq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	call getchar
	movb %al, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	addq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)
#LOOP 
.L187_start:
	cmpb $0, (%r12)
	je .L187_end


	jmp .L187_start
.L187_end:
#LOOP END
	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	call getchar
	movb %al, (%r12)
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	addq $1, %r12
	addb $1, (%r12)
	addb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	addq $1, %r12
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12

	jmp .L166_start
.L166_end:
#LOOP END

	jmp .L140_start
.L140_end:
#LOOP END
	subb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	addq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
#LOOP 
.L215_start:
	cmpb $0, (%r12)
	je .L215_end

	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
#LOOP 
.L221_start:
	cmpb $0, (%r12)
	je .L221_end

	subq $1, %r12

	jmp .L221_start
.L221_end:
#LOOP END
	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	subq $1, %r12
	subq $1, %r12
	addq $1, %r12
#LOOP 
.L232_start:
	cmpb $0, (%r12)
	je .L232_end


	jmp .L232_start
.L232_end:
#LOOP END
	addb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	call getchar
	movb %al, (%r12)

	jmp .L215_start
.L215_end:
#LOOP END
	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
#LOOP 
.L245_start:
	cmpb $0, (%r12)
	je .L245_end

	addb $1, (%r12)
	subb $1, (%r12)
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
#LOOP 
.L249_start:
	cmpb $0, (%r12)
	je .L249_end

	subb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	addq $1, %r12
#LOOP 
.L254_start:
	cmpb $0, (%r12)
	je .L254_end

	addb $1, (%r12)

	jmp .L254_start
.L254_end:
#LOOP END
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
#LOOP 
.L261_start:
	cmpb $0, (%r12)
	je .L261_end

	addb $1, (%r12)
#LOOP 
.L263_start:
	cmpb $0, (%r12)
	je .L263_end

#LOOP 
.L264_start:
	cmpb $0, (%r12)
	je .L264_end

	addb $1, (%r12)

	jmp .L264_start
.L264_end:
#LOOP END
#LOOP 
.L267_start:
	cmpb $0, (%r12)
	je .L267_end

	addb $1, (%r12)
	subb $1, (%r12)
	addq $1, %r12

	jmp .L267_start
.L267_end:
#LOOP END
#LOOP 
.L272_start:
	cmpb $0, (%r12)
	je .L272_end

	addq $1, %r12
#LOOP 
.L274_start:
	cmpb $0, (%r12)
	je .L274_end

	addb $1, (%r12)
	addq $1, %r12
	subq $1, %r12
	subq $1, %r12
#LOOP 
.L279_start:
	cmpb $0, (%r12)
	je .L279_end

	addb $1, (%r12)
#LOOP 
.L281_start:
	cmpb $0, (%r12)
	je .L281_end

	addq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	subq $1, %r12
	addb $1, (%r12)

	jmp .L281_start
.L281_end:
#LOOP END
	addb $1, (%r12)

	jmp .L279_start
.L279_end:
#LOOP END
	addb $1, (%r12)
	subb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	call getchar
	movb %al, (%r12)
	subq $1, %r12
	addq $1, %r12
#LOOP 
.L299_start:
	cmpb $0, (%r12)
	je .L299_end

	addq $1, %r12
	addq $1, %r12

	jmp .L299_start
.L299_end:
#LOOP END
	subq $1, %r12
#LOOP 
.L304_start:
	cmpb $0, (%r12)
	je .L304_end

	subq $1, %r12
	subb $1, (%r12)
	addq $1, %r12
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subq $1, %r12

	jmp .L304_start
.L304_end:
#LOOP END
	addb $1, (%r12)
	subq $1, %r12
	subq $1, %r12
	subq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)

	jmp .L274_start
.L274_end:
#LOOP END
#LOOP 
.L320_start:
	cmpb $0, (%r12)
	je .L320_end

	subq $1, %r12
	subb $1, (%r12)
	addq $1, %r12
	subq $1, %r12
	subq $1, %r12

	jmp .L320_start
.L320_end:
#LOOP END
	addq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
#LOOP 
.L330_start:
	cmpb $0, (%r12)
	je .L330_end

	addq $1, %r12

	jmp .L330_start
.L330_end:
#LOOP END
	addb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	addq $1, %r12
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	addq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12

	jmp .L272_start
.L272_end:
#LOOP END
#LOOP 
.L353_start:
	cmpb $0, (%r12)
	je .L353_end

	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	subq $1, %r12
	subq $1, %r12
	subq $1, %r12
#LOOP 
.L358_start:
	cmpb $0, (%r12)
	je .L358_end

	addq $1, %r12
	movb (%r12), %al
	movzbl %al, %edi
	call putchar

	jmp .L358_start
.L358_end:
#LOOP END
	subq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12

	jmp .L353_start
.L353_end:
#LOOP END
	addb $1, (%r12)
	addb $1, (%r12)
#LOOP 
.L369_start:
	cmpb $0, (%r12)
	je .L369_end

	addq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	call getchar
	movb %al, (%r12)
	subq $1, %r12
	subq $1, %r12
#LOOP 
.L376_start:
	cmpb $0, (%r12)
	je .L376_end

	subq $1, %r12
#LOOP 
.L378_start:
	cmpb $0, (%r12)
	je .L378_end

	addb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	subq $1, %r12

	jmp .L378_start
.L378_end:
#LOOP END
	subq $1, %r12

	jmp .L376_start
.L376_end:
#LOOP END

	jmp .L369_start
.L369_end:
#LOOP END
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	subb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subq $1, %r12
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	addb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12

	jmp .L263_start
.L263_end:
#LOOP END
	subb $1, (%r12)
	subq $1, %r12
	call getchar
	movb %al, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
#LOOP 
.L408_start:
	cmpb $0, (%r12)
	je .L408_end

	addb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	addq $1, %r12

	jmp .L408_start
.L408_end:
#LOOP END
	addb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)

	jmp .L261_start
.L261_end:
#LOOP END
	addq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	addb $1, (%r12)

	jmp .L249_start
.L249_end:
#LOOP END
	addq $1, %r12

	jmp .L245_start
.L245_end:
#LOOP END
	subq $1, %r12
	subq $1, %r12
	addq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
	subb $1, (%r12)
	addq $1, %r12
#LOOP 
.L434_start:
	cmpb $0, (%r12)
	je .L434_end

	subq $1, %r12
	subq $1, %r12
	subq $1, %r12
#LOOP 
.L438_start:
	cmpb $0, (%r12)
	je .L438_end

#LOOP 
.L439_start:
	cmpb $0, (%r12)
	je .L439_end

	subq $1, %r12
	addq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
#LOOP 
.L444_start:
	cmpb $0, (%r12)
	je .L444_end

	addb $1, (%r12)
	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)

	jmp .L444_start
.L444_end:
#LOOP END
	call getchar
	movb %al, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	call getchar
	movb %al, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)

	jmp .L439_start
.L439_end:
#LOOP END
	addq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
#LOOP 
.L472_start:
	cmpb $0, (%r12)
	je .L472_end

	addq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	call getchar
	movb %al, (%r12)
#LOOP 
.L480_start:
	cmpb $0, (%r12)
	je .L480_end


	jmp .L480_start
.L480_end:
#LOOP END
	addq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subq $1, %r12
	subq $1, %r12
	addq $1, %r12
	addq $1, %r12
#LOOP 
.L489_start:
	cmpb $0, (%r12)
	je .L489_end


	jmp .L489_start
.L489_end:
#LOOP END
	subq $1, %r12
	addq $1, %r12
	subq $1, %r12
	addq $1, %r12
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)

	jmp .L472_start
.L472_end:
#LOOP END

	jmp .L438_start
.L438_end:
#LOOP END
	addq $1, %r12
	subq $1, %r12
	addq $1, %r12
	subq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	call getchar
	movb %al, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subq $1, %r12
	subq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	subq $1, %r12
	addb $1, (%r12)
	addb $1, (%r12)
	subb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	subb $1, (%r12)

	jmp .L434_start
.L434_end:
#LOOP END
	subq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	call getchar
	movb %al, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	addb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)

	jmp .L138_start
.L138_end:
#LOOP END
	addq $1, %r12
	addq $1, %r12
	subb $1, (%r12)

	jmp .L94_start
.L94_end:
#LOOP END
	addb $1, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	addq $1, %r12

	jmp .L83_start
.L83_end:
#LOOP END
	addb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	subq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	addq $1, %r12

	jmp .L82_start
.L82_end:
#LOOP END
	subq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12

	jmp .L78_start
.L78_end:
#LOOP END
	subb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	addq $1, %r12

	jmp .L68_start
.L68_end:
#LOOP END
	addb $1, (%r12)
	addq $1, %r12
	subq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	subb $1, (%r12)
	addb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	subq $1, %r12
	subq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	subq $1, %r12

	jmp .L60_start
.L60_end:
#LOOP END
	addb $1, (%r12)
	addq $1, %r12
	movb (%r12), %al
	movzbl %al, %edi
	call putchar
	subb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)
	subb $1, (%r12)

	jmp .L38_start
.L38_end:
#LOOP END
	addb $1, (%r12)
	addb $1, (%r12)
	subq $1, %r12
	addb $1, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subq $1, %r12

	jmp .L34_start
.L34_end:
#LOOP END
	addb $1, (%r12)
	subb $1, (%r12)
	addq $1, %r12
	addb $1, (%r12)
	addq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	call getchar
	movb %al, (%r12)
	subq $1, %r12
	subb $1, (%r12)
	subb $1, (%r12)
	addb $1, (%r12)
	subb $1, (%r12)
	subq $1, %r12
	addq $1, %r12
	addb $1, (%r12)
	addq $1, %r12

	jmp .L27_start
.L27_end:
#LOOP END



# Epilogue
	popq %r12                 # Restore r12
	popq %rbx                 # Restore rbx
	movq %rbp, %rsp           # Restore stack pointer
	popq %rbp                 # Restore base pointer
	ret                       # Return to the kernel
