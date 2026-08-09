/* REVIEW-ONLY: FUN_0005e3ec @ 0x0005e3ec */


void FUN_0005e3ec(undefined4 *param_1)

{
  uint uVar1;
  
  FUN_003526b8((int)param_1 + 0x20f,0xffff354c,param_1[2]);
  param_1[1] = 0;
  *param_1 = 0;
  param_1[4] = 0;
  uVar1 = 0;
  do {
    param_1[uVar1 + 5] = 0;
    uVar1 = uVar1 + 1 & 0xffff;
  } while (uVar1 < 0x10);
  *(undefined2 *)(param_1 + 0x81) = 0;
  param_1[0x80] = 0;
  *(undefined1 *)(param_1 + 0x82) = 0;
  *(undefined1 *)((int)param_1 + 0x209) = 0;
  *(undefined1 *)((int)param_1 + 0x20a) = 0;
  *(undefined1 *)((int)param_1 + 0x20b) = 0;
  uVar1 = 0;
  do {
    param_1[uVar1 + 0x15] = 0;
    uVar1 = uVar1 + 1 & 0xffff;
  } while (uVar1 < 0x6a);
  param_1[0x7f] = 0;
  *(undefined2 *)((int)param_1 + 0x206) = 0;
  *(undefined1 *)(param_1 + 0x83) = 0;
  *(undefined1 *)((int)param_1 + 0x20d) = 1;
  *(undefined1 *)((int)param_1 + 0x20e) = 0;
  *(undefined1 *)((int)param_1 + 0x217) = 0;
  return;
}

