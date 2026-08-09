/* REVIEW-ONLY: FUN_003b2108 @ 0x003b2108 */


undefined8 FUN_003b2108(undefined8 param_1)

{
  ushort uVar1;
  int iVar2;
  
  iVar2 = (int)param_1;
  FUN_00254d18(param_1,iVar2 + 0x10,1,0x50e1f8);
  uVar1 = *(ushort *)(iVar2 + 0x204);
  *(ushort *)(iVar2 + 0x204) = uVar1 + 1;
  *(undefined4 *)(iVar2 + (uint)uVar1 * 4 + 0x14) = *(undefined4 *)(iVar2 + 0x10);
  if (DAT_0050e1f8 < 0) {
    *(int *)(iVar2 + 0x10) = DAT_004cd6d8 - DAT_0050e1f8;
  }
  else {
    *(int *)(iVar2 + 0x10) = DAT_0050e1f8;
  }
  return 0;
}

