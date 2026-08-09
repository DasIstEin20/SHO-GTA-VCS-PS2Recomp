/* REVIEW-ONLY: FUN_003b2230 @ 0x003b2230 */


undefined8 FUN_003b2230(int param_1)

{
  ushort uVar1;
  bool bVar2;
  
  uVar1 = *(ushort *)(param_1 + 0x206);
  bVar2 = *(char *)(param_1 + 0x20c) != '\0';
  if (uVar1 == 0) {
    *(bool *)(param_1 + 0x209) = bVar2;
  }
  else {
    *(ushort *)(param_1 + 0x206) = uVar1 - 1;
    if (uVar1 < 9) {
      *(bool *)(param_1 + 0x209) = *(char *)(param_1 + 0x209) != '\0' && bVar2;
    }
    else {
      *(bool *)(param_1 + 0x209) = *(char *)(param_1 + 0x209) != '\0' || bVar2;
      if (*(ushort *)(param_1 + 0x206) < 0x15) {
        *(undefined2 *)(param_1 + 0x206) = 0;
      }
    }
  }
  FUN_00256308();
  return 0;
}

