/* REVIEW-ONLY: FUN_002d2f60 @ 0x002d2f60 */


undefined8 FUN_002d2f60(undefined8 param_1)

{
  ushort uVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  undefined8 uVar5;
  int iVar6;
  
  iVar6 = (int)param_1;
  FUN_00254d18(param_1,iVar6 + 0x10,1,0x50e1f8);
  uVar5 = FUN_00471ac8(DAT_00487d94,DAT_0050e1f8);
  iVar4 = FUN_00238138(uVar5);
  bVar2 = *(char *)(iVar4 + 0xe0) == '\x02';
  if (*(char *)(iVar6 + 0x20c) != '\0') {
    bVar2 = !bVar2;
  }
  uVar1 = *(ushort *)(iVar6 + 0x206);
  if (uVar1 == 0) {
    *(bool *)(iVar6 + 0x209) = bVar2;
  }
  else {
    *(ushort *)(iVar6 + 0x206) = uVar1 - 1;
    if (uVar1 < 9) {
      bVar3 = false;
      if (*(char *)(iVar6 + 0x209) != '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(iVar6 + 0x209) = bVar3;
    }
    else {
      bVar3 = true;
      if (*(char *)(iVar6 + 0x209) == '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(iVar6 + 0x209) = bVar3;
      if (*(ushort *)(iVar6 + 0x206) < 0x15) {
        *(undefined2 *)(iVar6 + 0x206) = 0;
      }
    }
  }
  return 0;
}

