/* REVIEW-ONLY: FUN_002b80d4 @ 0x002b80d4 */


undefined4 FUN_002b80d4(int param_1)

{
  ushort uVar1;
  bool bVar2;
  bool bVar3;
  
  bVar2 = *(char *)(param_1 + 0x20c) == '\0';
  if (*(short *)(param_1 + 0x206) == 0) {
    *(bool *)(param_1 + 0x209) = bVar2;
  }
  else {
    uVar1 = *(ushort *)(param_1 + 0x206);
    *(ushort *)(param_1 + 0x206) = uVar1 - 1;
    bVar3 = *(char *)(param_1 + 0x209) != '\0';
    if (uVar1 < 9) {
      *(bool *)(param_1 + 0x209) = bVar2 && bVar3;
    }
    else {
      *(bool *)(param_1 + 0x209) = bVar2 || bVar3;
      if (*(ushort *)(param_1 + 0x206) < 0x15) {
        *(undefined2 *)(param_1 + 0x206) = 0;
      }
    }
  }
  FUN_0005f09c();
  return 0;
}

