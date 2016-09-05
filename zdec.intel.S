#  K2,
#  XOR DECODE (gas)
#  I UESD TO CODE INTEL ASM AND WE DID SHIT IN ALL CAPS
#  USES: esi,ebx,ecx
# 0x851805e0 0x42925923
        .text
        .globl  _start
_start:
main:    jmp  down
back:    popl %esi
         movl $string,%esi
         movl $0x851805e0,%ebx
         xorl %ecx,%ecx
         movb $0x42,%ecx
decode:  xorl %ebx,(%esi)
         addl $0x42925923,%ebx
         inc  %esi
         inc  %esi
         inc  %esi
         inc  %esi
         loop decode
         jmp  shellcode
down:    call back
shellcode:
.data
string:
.string "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
