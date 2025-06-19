
.global strhash
.p2align 2
.type strhash, %function

strhash:
.fnstart
	MOV		R4, #0
calcLenLoop:
	LDRB	R5, [R0, R4]
	CMP		R5, #0
	BEQ		calcLenLoopExit
	ADD		R4, R4, #1
	B		calcLenLoop
calcLenLoopExit:
	MOV		R6, #0
	MOV		R7, R4
forLoop:
	CMP		R6, R4
	BGT		forLoopExit
	LDRB	R5, [R0, R6]
	CMP		R5, #48
	BLT		forLoopExit
	CMP		R5, #57
	BGT		forLoopElseIf1
	CMP		R5, #48
	IT		EQ
	ADDEQ	R7, R7, #5
	CMP		R5, #49
	IT		EQ
	ADDEQ	R7, R7, #12
	CMP		R5, #50
	IT		EQ
	ADDEQ	R7, R7, #7
	CMP		R5, #51
	IT		EQ
	ADDEQ	R7, R7, #6
	CMP		R5, #52
	IT		EQ
	ADDEQ	R7, R7, #4
	CMP		R5, #53
	IT		EQ
	ADDEQ	R7, R7, #11
	CMP		R5, #54
	IT		EQ
	ADDEQ	R7, R7, #6
	CMP		R5, #55
	IT		EQ
	ADDEQ	R7, R7, #3
	CMP		R5, #56
	IT		EQ
	ADDEQ	R7, R7, #10
	CMP		R5, #57
	IT		EQ
	ADDEQ	R7, R7, #23
	ADD		R6, R6, #1
	B		forLoop
forLoopElseIf1:
	CMP		R5, #65
	BLT		forLoopExit
	CMP		R5, #90
	BGT		forLoopElseIf2
	LSL		R8, R5, #1
	ADD		R7, R7, R8
	ADD		R6, R6, #1
	B		forLoop
forLoopElseIf2:
	CMP		R5, #97
	BLT		forLoopExit
	CMP		R5, #122
	BGT		forLoopExit
	SUB		R8, R5, #97
	MUL		R8, R8, R8
	ADD		R7, R7, R8
	ADD		R6, R6, #1
	B		forLoop
forLoopExit:
	MOV		R0, R7
	CMP		R0, #9
	BLE		strHashEnd
digitSum:
	MOV		R4, R0
	MOV		R5, #0
digitSum1000:
	CMP		R4, #1000
	BLT		digitSum100
	SUB		R4, R4, #1000
	ADD		R5, R5, #1
	B		digitSum1000
digitSum100:
	CMP		R4, #100
	BLT		digitSum10
	SUB		R4, R4, #100
	ADD		R5, R5, #1
	B		digitSum100
digitSum10:
	CMP		R4, #10
	BLT		digitSum1
	SUB		R4, R4, #10
	ADD		R5, R5, #1
	B		digitSum10
digitSum1:
	CMP		R4, #1
	BLT		digitSumEnd
	SUB		R4, R4, #1
	ADD		R5, R5, #1
	B		digitSum1
digitSumEnd:
	MOV		R0, R5
	CMP		R0, #9
	BLE		strHashEnd
successiveSubtract:
	SUB		R0, R0, #7
	CMP		R0, #9
	BGT		successiveSubtract
strHashEnd:
	NOP
fibonacci:
	PUSH	{LR}
	CMP		R0, #0
	BEQ		fibonacciZero
	CMP		R0, #1
	BEQ		fibonacciOne
	SUB		R0, R0, #1
	PUSH	{R0, LR}
	BL		fibonacci
	MOV		R1, R0
	POP		{R0, LR}
	SUB		R0, R0, #1
	PUSH	{R1, LR}
	BL		fibonacci
	POP		{R1, LR}
	ADD		R0, R0, R1
	POP		{LR}
	BX		LR
fibonacciZero:
	MOV		R0, #0
	POP		{LR}
	BX		LR
fibonacciOne:
	MOV		R0, #1
	POP		{LR}
	BX		LR
.fnend
