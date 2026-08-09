/* REVIEW-ONLY: FUN_003a7c70 @ 0x003a7c70 */


undefined8 FUN_003a7c70(undefined8 param_1)

{
  ushort uVar1;
  bool bVar2;
  bool bVar3;
  long lVar4;
  int iVar5;
  
  iVar5 = (int)param_1;
  FUN_00254d18(param_1,iVar5 + 0x10,2,0x50e1f8);
  lVar4 = FUN_00258ec0(param_1,(undefined2)DAT_0050e1f8,(undefined2)DAT_0050e1fc);
  bVar2 = lVar4 != 0;
  if (*(char *)(iVar5 + 0x20c) != '\0') {
    bVar2 = !bVar2;
  }
  uVar1 = *(ushort *)(iVar5 + 0x206);
  if (uVar1 == 0) {
    *(bool *)(iVar5 + 0x209) = bVar2;
  }
  else {
    *(ushort *)(iVar5 + 0x206) = uVar1 - 1;
    if (uVar1 < 9) {
      bVar3 = false;
      if (*(char *)(iVar5 + 0x209) != '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(iVar5 + 0x209) = bVar3;
    }
    else {
      bVar3 = true;
      if (*(char *)(iVar5 + 0x209) == '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(iVar5 + 0x209) = bVar3;
      if (*(ushort *)(iVar5 + 0x206) < 0x15) {
        *(undefined2 *)(iVar5 + 0x206) = 0;
      }
    }
  }
  return 0;
}

