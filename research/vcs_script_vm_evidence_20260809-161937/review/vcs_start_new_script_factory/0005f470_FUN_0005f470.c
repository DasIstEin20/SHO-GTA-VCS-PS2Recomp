/* REVIEW-ONLY: FUN_0005f470 @ 0x0005f470 */


int FUN_0005f470(undefined4 param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = iRam00018018;
  FUN_0005e6c0(iRam00018018,0x18018);
  iVar2 = DAT_0000ef10 + 1;
  *(int *)(iVar1 + 8) = DAT_0000ef10;
  DAT_0000ef10 = iVar2;
  FUN_0005e3ec(iVar1);
  *(undefined4 *)(iVar1 + 0x10) = param_1;
  FUN_0005e6f8(iVar1,0x18014);
  *(undefined1 *)(iVar1 + 0x208) = 1;
  return iVar1;
}

