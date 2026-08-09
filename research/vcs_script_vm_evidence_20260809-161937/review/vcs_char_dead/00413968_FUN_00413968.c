/* REVIEW-ONLY: FUN_00413968 @ 0x00413968 */


undefined8 FUN_00413968(undefined8 param_1)

{
  ushort uVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  long lVar5;
  long lVar6;
  int iVar7;
  
  iVar7 = (int)param_1;
  FUN_00254d18(param_1,iVar7 + 0x10,1,0x50e1f8);
  lVar5 = FUN_00471ac8(DAT_00487d94,DAT_0050e1f8);
  bVar2 = true;
  if (lVar5 != 0) {
    lVar6 = FUN_0020c2f8(lVar5);
    if (lVar6 == 0) {
      iVar4 = *(int *)((int)lVar5 + 0x8c4);
      if (((iVar4 == 0x3a) || (iVar4 == 0x39)) ||
         (bVar2 = false, *(int *)((int)lVar5 + 0x8a4) == 0x26)) {
        bVar2 = true;
      }
    }
    else {
      iVar4 = FUN_00238138(lVar5);
      bVar2 = *(char *)(iVar4 + 0xe0) == '\x01';
    }
  }
  if (*(char *)(iVar7 + 0x20c) != '\0') {
    bVar2 = !bVar2;
  }
  uVar1 = *(ushort *)(iVar7 + 0x206);
  if (uVar1 == 0) {
    *(bool *)(iVar7 + 0x209) = bVar2;
  }
  else {
    *(ushort *)(iVar7 + 0x206) = uVar1 - 1;
    if (uVar1 < 9) {
      bVar3 = false;
      if (*(char *)(iVar7 + 0x209) != '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(iVar7 + 0x209) = bVar3;
    }
    else {
      bVar3 = true;
      if (*(char *)(iVar7 + 0x209) == '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(iVar7 + 0x209) = bVar3;
      if (*(ushort *)(iVar7 + 0x206) < 0x15) {
        *(undefined2 *)(iVar7 + 0x206) = 0;
      }
    }
  }
  return 0;
}

