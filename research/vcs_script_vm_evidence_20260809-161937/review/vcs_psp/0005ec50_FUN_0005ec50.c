/* REVIEW-ONLY: FUN_0005ec50 @ 0x0005ec50 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005ec50(int param_1)

{
  int iVar1;
  ushort uVar2;
  
  if (((*(char *)(param_1 + 0x20d) != '\0') && (_DAT_ffff9f41 != 0)) &&
     (*(int *)(_DAT_ffff0e25 + _DAT_ffff9f41) == 1)) {
    iVar1 = FUN_0015d0e8(&LAB_00034740 + (uint)DAT_ffff9e99 * 0x160);
    if ((iVar1 == 0) &&
       (iVar1 = FUN_0015d10c(&LAB_00034740 + (uint)DAT_ffff9e99 * 0x160), iVar1 == 0)) {
      return;
    }
    uVar2 = *(ushort *)(param_1 + 0x204);
    if (1 < uVar2) {
      do {
        uVar2 = uVar2 - 1;
      } while (1 < uVar2);
      *(ushort *)(param_1 + 0x204) = uVar2;
    }
    FUN_0005f09c(param_1);
    *(undefined4 *)(param_1 + 0x1fc) = 0;
    FUN_002b5728();
    *(undefined4 *)(_DAT_ffff0e25 + _DAT_ffff9f41) = 0;
    *(undefined1 *)(param_1 + 0x20e) = 1;
    *(undefined4 *)(param_1 + 0x200) = 0;
  }
  return;
}

