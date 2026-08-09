/* REVIEW-ONLY: FUN_00287cc8 @ 0x00287cc8 */


undefined8 FUN_00287cc8(int param_1)

{
  ushort uVar1;
  bool bVar2;
  bool bVar3;
  long lVar4;
  
  lVar4 = FUN_003775e0(0x6f47d0);
  bVar2 = lVar4 != 0;
  if (*(char *)(param_1 + 0x20c) != '\0') {
    bVar2 = !bVar2;
  }
  uVar1 = *(ushort *)(param_1 + 0x206);
  if (uVar1 == 0) {
    *(bool *)(param_1 + 0x209) = bVar2;
  }
  else {
    *(ushort *)(param_1 + 0x206) = uVar1 - 1;
    if (uVar1 < 9) {
      bVar3 = false;
      if (*(char *)(param_1 + 0x209) != '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(param_1 + 0x209) = bVar3;
    }
    else {
      bVar3 = true;
      if (*(char *)(param_1 + 0x209) == '\0') {
        bVar3 = bVar2;
      }
      *(bool *)(param_1 + 0x209) = bVar3;
      if (*(ushort *)(param_1 + 0x206) < 0x15) {
        *(undefined2 *)(param_1 + 0x206) = 0;
      }
    }
  }
  return 0;
}

