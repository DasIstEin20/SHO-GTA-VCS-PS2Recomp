/* REVIEW-ONLY: FUN_003b22b8 @ 0x003b22b8 */


undefined8 FUN_003b22b8(undefined8 param_1)

{
  bool bVar1;
  ushort uVar2;
  int iVar3;
  
  iVar3 = (int)param_1;
  FUN_00254d18(param_1,iVar3 + 0x10,1,0x50e1f8);
  uVar2 = (ushort)DAT_0050e1f8;
  bVar1 = (ushort)DAT_0050e1f8 != 0;
  *(ushort *)(iVar3 + 0x206) = (ushort)DAT_0050e1f8;
  if (bVar1) {
    if (uVar2 < 9) {
      *(undefined1 *)(iVar3 + 0x209) = 1;
    }
    else {
      *(undefined1 *)(iVar3 + 0x209) = 0;
    }
  }
  return 0;
}

