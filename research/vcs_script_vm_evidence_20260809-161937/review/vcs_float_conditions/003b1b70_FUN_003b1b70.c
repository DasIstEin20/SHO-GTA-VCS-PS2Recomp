/* REVIEW-ONLY: FUN_003b1b70 @ 0x003b1b70 */


undefined8 FUN_003b1b70(undefined8 param_1)

{
  ushort uVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  
  iVar4 = (int)param_1;
  FUN_00254d18(param_1,iVar4 + 0x10,2,0x50e1f8);
  bVar2 = DAT_0050e1fc <= DAT_0050e1f8;
  if (*(char *)(iVar4 + 0x20c) != '\0') {
    bVar2 = !bVar2;
  }
  uVar1 = *(ushort *)(iVar4 + 0x206);
  if (uVar1 == 0) {
    *(bool *)(iVar4 + 0x209) = bVar2;
  }
  else {
    *(ushort *)(iVar4 + 0x206) = uVar1 - 1;
    if (uVar1 < 9) {
      bVar3 = false;
      if (*(char *)(iVar4 + 0x209) != '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(iVar4 + 0x209) = bVar3;
    }
    else {
      bVar3 = true;
      if (*(char *)(iVar4 + 0x209) == '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(iVar4 + 0x209) = bVar3;
      if (*(ushort *)(iVar4 + 0x206) < 0x15) {
        *(undefined2 *)(iVar4 + 0x206) = 0;
      }
    }
  }
  return 0;
}

