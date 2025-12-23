; -------- SimpleLang Compiler Output --------
; Variables:
; a -> 0x10
; b -> 0x11
; c -> 0x12
; -------------------------------------------

LOADI A, 10
STORE A, 0x10
LOADI A, 20
STORE A, 0x11
LOAD A, 0x10
PUSH A
LOAD A, 0x11
POP B
ADD A, B
STORE A, 0x12
LOAD A, 0x12
CMP A, 30
JNZ if_end_0
LOAD A, 0x12
PUSH A
LOADI A, 1
POP B
SUB A, B
STORE A, 0x12
if_end_0:
