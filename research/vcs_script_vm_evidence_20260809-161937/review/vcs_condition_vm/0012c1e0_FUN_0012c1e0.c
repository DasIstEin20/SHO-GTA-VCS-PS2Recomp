/* REVIEW-ONLY: FUN_0012c1e0 @ 0x0012c1e0 */


undefined8 FUN_0012c1e0(int param_1)

{
  ushort uVar1;
  char cVar2;
  long lVar3;
  bool bVar4;
  bool bVar5;
  
  if (DAT_0048f7f8 == 0) {
    FUN_00471798();
  }
  lVar3 = FUN_00241b80(DAT_0048f7f8);
  if (lVar3 == 0) {
    cVar2 = *(char *)(param_1 + 0x20c);
  }
  else {
    lVar3 = FUN_003cbff8();
    cVar2 = *(char *)(param_1 + 0x20c);
    if (lVar3 < 6) {
      uVar1 = *(ushort *)(param_1 + 0x206);
      bVar5 = cVar2 != '\0';
      if (uVar1 == 0) goto LAB_0012c264;
      *(ushort *)(param_1 + 0x206) = uVar1 - 1;
      if (uVar1 < 9) goto LAB_0012c27c;
      cVar2 = *(char *)(param_1 + 0x209);
      goto LAB_0012c298;
    }
  }
  uVar1 = *(ushort *)(param_1 + 0x206);
  bVar5 = cVar2 == '\0';
  if (uVar1 == 0) {
LAB_0012c264:
    *(bool *)(param_1 + 0x209) = bVar5;
    return 0;
  }
  *(ushort *)(param_1 + 0x206) = uVar1 - 1;
  if (uVar1 < 9) {
LAB_0012c27c:
    bVar4 = false;
    if (*(char *)(param_1 + 0x209) != '\0') {
      bVar4 = bVar5;
    }
    *(bool *)(param_1 + 0x209) = bVar4;
    return 0;
  }
  cVar2 = *(char *)(param_1 + 0x209);
LAB_0012c298:
  bVar4 = true;
  if (cVar2 == '\0') {
    bVar4 = bVar5;
  }
  *(bool *)(param_1 + 0x209) = bVar4;
  if (*(ushort *)(param_1 + 0x206) < 0x15) {
    *(undefined2 *)(param_1 + 0x206) = 0;
  }
  return 0;
}

