.data
.dword 3,4,4,2,3,1,2,4,1,1,1,2,2,3,1,1,1,2,2,2,3,4,4,1
.text
lui x28,0x10000
lui x3,0x10000
ld x5,0(x28)
ld x6,8(x28)
ld x7,16(x28)
ld x29,24(x28)
lui x15,0x10000
lui x2,0x10000
bne x6,x7,wrong
addi x13,x7,0
addi x17,x29,0
addi x14,x14,0x400
add x14,x14,x28
sd x5,0(x14)
addi x14,x14,8
sd x29,0(x14)
addi x21,x5,0
addi x25,x0,-8
mul x25,x6,x25
add x3,x3,x25


outerouterloop:
    addi x17,x29,0
    addi x21,x21,-1
    addi x15,x2,0
    addi x15,x15,-8
    addi x25,x0,8
    mul x25,x25,x6
    add x3,x3,x25
    #addi x3,x3,16
    add x28,x3,x0
    beq x0,x0,outerloop
    

loop:
    addi x12,x12,8
    mul x20,x5,x6
    #add x31,x29,x5
    addi x25,x0,8
    add x4,x25,x4
    mul x22,x29,x4
    mul x31,x20,x25    
    addi x28,x28,32
    #addi x31,x31,32
    addi x15,x15,32
    add x15,x31,x15
    addi x12,x12,-8
    add x26,x28,x12
    ld x30,0(x26)
    #add x26,x15,x12
    #addi x28,x28,8
    ld x9, 0(x15)
    addi x12,x12,8
    mul x19,x9,x30
    add x10,x19,x10
    addi x13,x13,-1
    addi x28,x3,0
    addi x15,x16,0
    add x15,x15,x22
    bne x13,x0,loop
    addi x15,x16,0
    addi x14,x14,8
    addi x4,x0,0
    sd x10,0(x14)
    addi x10,x0,0
    addi x12,x0,0
    addi x13,x7,0
    addi x17,x17,-1
    bne x17,x0,outerloop

outerloop:
    addi x15,x15,8
    addi x16,x15,0
    bne x17,x0,loop
    bne x21,x0,outerouterloop
    beq x21,x0,exit

wrong:
    addi x9,x9,0x400
    add x9,x9,x28
    sd x0,0(x9)
    sd x0,8(x9)
    
exit:
    addi x0,x0,0