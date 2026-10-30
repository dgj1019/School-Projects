	.data
MultiplicandPrompt:	.asciiz "Enter the multiplicand: "
MultiplierPrompt:	.asciiz "Enter the multiplier: "
MultSign:		.asciiz " * "
EqualSign:		.asciiz " = "
NewLine:		.asciiz "\n"

	.text

main:
	la $a0, MultiplicandPrompt
	li $v0, 4
	syscall
	
	li $v0, 5
	syscall
	move $s0, $v0
	
	la $a0, MultiplierPrompt
	li $v0, 4
	syscall
	
	li $v0, 5
	syscall
	move $s1, $v0
	
	move $a0, $s0
	move $a1, $s1
	jal Recursive
	move $s2, $v0
	
	move $a0, $s0
	li $v0, 1
	syscall
	
	la $a0, MultSign
	li $v0, 4
	syscall
	
	move $a0, $s1
	li $v0, 1
	syscall
	
	la $a0, EqualSign
	li $v0, 4
	syscall
	
	move $a0, $s2
	li $v0, 1
	syscall
	
	la $a0, NewLine
	li $v0, 4
	syscall
	
	li $v0, 10
	syscall
	
	
Recursive:
	addiu $sp, $sp, -12
	sw $ra, 8($sp)
	sw $a0, 4($sp)
	sw $a1, 0($sp)
	
	li $t0, 1
	beq $a1, $t0, BaseCase
	
	addi $a1, $a1, -1
	jal Recursive
	
	lw $a0, 4($sp)
	add $v0, $a0, $v0
	j finish
	
BaseCase:
	move $v0, $a0
	
finish:
	lw $a1, 0($sp)
	lw $a0, 4($sp)
	lw $ra, 8($sp)
	addi $sp, $sp, 12
	jr $ra
	
 
 	
	
	
