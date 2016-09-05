# K2, more todo here
# uses 8,9,10,11,12,14   ( 13 is unlucky ;)
      .text   
      .set noreorder
      .globl  main
main:
      li    $14,0x4242
back: sub   $14,0x2424
      bgezal $14,back
      li   $8,0x42424242
      la   $12,str

      li    $10,0x3333
      sub   $10,0x3303        # filler to avoid 0x00 in li
loop: sub   $12,0x3333
      lw    $9,0x3333($12)
      sub   $10,0x4
      xor   $11,$9,$8
      sw    $11,0x3333($12)
      add   $12,0x4
      bnez  $10,loop
	
	
.data
str:	.ascii	"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
