/* REVIEW-ONLY: FUN_0009e4c8 @ 0x0009e4c8 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0009e4c8(int param_1)

{
  ushort uVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  
  if (_DAT_ffffbe18 == 0) {
    FUN_00361eec();
  }
  iVar4 = FUN_00150ab0(_DAT_ffffbe18);
  if ((iVar4 == 0) || (iVar4 = FUN_002cf87c(), 5 < iVar4)) {
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
  }
  else {
    bVar2 = *(char *)(param_1 + 0x20c) != '\0';
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
  }
  return 0;
}

