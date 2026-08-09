/* REVIEW-ONLY: FUN_00255358 @ 0x00255358 */


void FUN_00255358(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  
  FUN_0045f6d8((int)param_1 + 0x20f,0x4ae4a8,param_1[2]);
  param_1[1] = 0;
  *param_1 = 0;
  uVar2 = 0;
  param_1[4] = 0;
  do {
    iVar1 = uVar2 + 5;
    uVar2 = uVar2 + 1 & 0xffff;
    param_1[iVar1] = 0;
  } while (uVar2 < 0x10);
  *(undefined2 *)(param_1 + 0x81) = 0;
  uVar2 = 0;
  param_1[0x80] = 0;
  *(undefined1 *)(param_1 + 0x82) = 0;
  *(undefined1 *)((int)param_1 + 0x209) = 0;
  *(undefined1 *)((int)param_1 + 0x20a) = 0;
  *(undefined1 *)((int)param_1 + 0x20b) = 0;
  do {
    iVar1 = uVar2 + 0x15;
    uVar2 = uVar2 + 1 & 0xffff;
    param_1[iVar1] = 0;
  } while (uVar2 < 0x6a);
  *(undefined1 *)((int)param_1 + 0x217) = 0;
  *(undefined1 *)((int)param_1 + 0x20d) = 1;
  param_1[0x7f] = 0;
  *(undefined2 *)((int)param_1 + 0x206) = 0;
  *(undefined1 *)(param_1 + 0x83) = 0;
  *(undefined1 *)((int)param_1 + 0x20e) = 0;
  return;
}

