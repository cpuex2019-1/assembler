    addi    $1, $0, 220
    addi    $2, $0, 100
    sw  $1, 320($0)
    addlw $3, $2, $1
    addi    $4, $0, 16256   # 1.0の上位16bit
    slli    $4, $4, 16
    sw $4, 320($0)
    addlf $f1, $2, $1