/* REVIEW-ONLY: FUN_003b23d8 @ 0x003b23d8 */


undefined8 FUN_003b23d8(undefined8 param_1)

{
  ushort uVar1;
  int iVar2;
  
  iVar2 = (int)param_1;
  FUN_00254d18(param_1,iVar2 + 0x10,2,0x50e1f8);
  uVar1 = *(ushort *)(iVar2 + 0x204);
  *(ushort *)(iVar2 + 0x204) = uVar1 + 1;
  *(undefined4 *)(iVar2 + (uint)uVar1 * 4 + 0x14) = *(undefined4 *)(iVar2 + 0x10);
  *(undefined4 *)(iVar2 + 0x10) = DAT_0050e1f8;
  return 0;
}

