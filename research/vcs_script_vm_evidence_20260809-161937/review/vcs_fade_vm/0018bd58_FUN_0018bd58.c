/* REVIEW-ONLY: FUN_0018bd58 @ 0x0018bd58 */


undefined4 FUN_0018bd58(int param_1)

{
  ushort uVar1;
  bool bVar2;
  bool bVar3;
  int iVar4;
  
  iVar4 = FUN_002189d4(0x1e0c0);
  bVar3 = (iVar4 != 0) != (*(char *)(param_1 + 0x20c) != '\0');
  if (*(short *)(param_1 + 0x206) == 0) {
    *(bool *)(param_1 + 0x209) = bVar3;
  }
  else {
    uVar1 = *(ushort *)(param_1 + 0x206);
    *(ushort *)(param_1 + 0x206) = uVar1 - 1;
    bVar2 = *(char *)(param_1 + 0x209) != '\0';
    if (uVar1 < 9) {
      *(bool *)(param_1 + 0x209) = bVar3 && bVar2;
    }
    else {
      *(bool *)(param_1 + 0x209) = bVar3 || bVar2;
      if (*(ushort *)(param_1 + 0x206) < 0x15) {
        *(undefined2 *)(param_1 + 0x206) = 0;
      }
    }
  }
  return 0;
}

