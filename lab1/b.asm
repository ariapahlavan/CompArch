	.ORIG x3000
	
	LEA R0, LABEL
	LDB R1, R0, #0
	LDW R1, R0, #0
	STW R1, R0, #0
	STB R1, R0, #0
	HALT
LABEL	.FILL x4500
	.END
