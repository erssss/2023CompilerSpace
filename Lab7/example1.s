	.cpu cortex-a72
	.arch armv8-a
	.fpu vfpv3-d16
	.arch_extension crc
	.text
	.global main
	.type main , %function
main:
	push {r3, r4, r5, r6, fp, lr}
	mov fp, sp
	sub sp, sp, #24
.L34:
	ldr r4, =1
	str r4, [fp, #-24]
	ldr r4, [fp, #-24]
	str r4, [fp, #-16]
	ldr r4, [fp, #-24]
	ldr r5, [fp, #-16]
	add r6, r4, r5
	str r6, [fp, #-20]
	ldr r4, [fp, #-24]
	ldr r4, [fp, #-16]
	str r6, [fp, #-12]
	ldr r4, [fp, #-24]
	ldr r4, [fp, #-16]
	str r6, [fp, #-8]
	ldr r4, [fp, #-24]
	ldr r4, [fp, #-16]
	str r6, [fp, #-4]
	ldr r4, [fp, #-24]
	ldr r4, [fp, #-16]
	str r6, [fp, #-12]
	ldr r4, =5
	str r4, [fp, #-24]
	ldr r4, [fp, #-24]
	ldr r5, [fp, #-16]
	add r6, r4, r5
	mov r0, r6
	bl putint
	mov r4, r0
	mov r0, #0
	add sp, sp, #24
	pop {r3, r4, r5, r6, fp, lr}
	bx lr

	.ident "zm"
