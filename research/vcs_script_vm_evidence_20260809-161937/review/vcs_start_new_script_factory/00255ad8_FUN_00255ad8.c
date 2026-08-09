/* REVIEW-ONLY: FUN_00255ad8 @ 0x00255ad8 */


int FUN_00255ad8(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = DAT_004cd6c8;
  FUN_00255a78(DAT_004cd6c8,0x4cd6c8);
  *(int *)(iVar1 + 8) = DAT_0048772c;
  DAT_0048772c = DAT_0048772c + 1;
  FUN_00255358(iVar1);
  *(undefined4 *)(iVar1 + 0x10) = param_1;
  FUN_00255ab0(iVar1,0x4cd6c4);
  *(undefined1 *)(iVar1 + 0x208) = 1;
  return iVar1;
}

