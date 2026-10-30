	.data
chars:		.ascii "ABCDEFGHIJKLMNOPQRSTUVWXYZ .,!-'"
msg1:		.word 0x93EA9646, 0xCDE50442, 0x34D29306, 0xD1F33720
		.word 0x56033D01, 0x394D963B, 0xDE7BEFA4
msg1end:	.word 28
LineBreak:	.asciiz "\n\n"


	.text 
main:
	la $s0, msg1 #nextbyte
	la $s1, msg1end #msgend

	
	li $s2, 0 #bits
	li $s3, 0 #buffer
	
	
WhileLoop:
	bge $s0, $s1, Exit
	bge $s2, 5, WhileLoopCon	
	
	lbu $t1, 0($s0)
	addi $s0, $s0, 1
	
	sll $s3, $s3, 8
	or $s3, $s3, $t1
	addi $s2, $s2, 8
		

WhileLoopCon:
	addi $s2, $s2, -5
	srlv $t2, $s3, $s2
	andi $t2, $t2, 0x1F
	
	la $t3, chars
	add $t3, $t3, $t2
	lbu $a0, 0($t3)
	
	li $v0, 11
	syscall


	j WhileLoop

	
Exit:
	la $a0, LineBreak
	li $v0, 4
	syscall
	
	li $v0, 10
	syscall
	
	
	
	
	
	
	
	
