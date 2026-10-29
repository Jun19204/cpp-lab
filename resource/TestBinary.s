  add x3, x1, x2
  sub x3, x1, x2
L1:
  lw  x3, 4(x2)
  sw  x3, 4(x2)
  beq x1, x2, L1
