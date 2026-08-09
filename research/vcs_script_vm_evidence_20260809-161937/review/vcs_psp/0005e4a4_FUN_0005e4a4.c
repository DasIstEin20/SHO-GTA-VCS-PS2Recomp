/* REVIEW-ONLY: FUN_0005e4a4 @ 0x0005e4a4 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005e4a4(int param_1)

{
  bool bVar1;
  int iVar2;
  
  _DAT_ffffc640 = param_1;
  if (*(char *)(param_1 + 0x20a) != '\0') {
    FUN_0005ec50(param_1);
  }
  if (((*(char *)(param_1 + 0x217) != '\0') && (DAT_ffffc689 == '\x01')) &&
     (*(short *)(param_1 + 0x204) == 1)) {
    *(undefined2 *)(param_1 + 0x204) = 0;
    *(undefined4 *)(param_1 + 0x10) = *(undefined4 *)(param_1 + 0x14);
  }
  if (_DAT_ffffc51c < *(uint *)(param_1 + 0x200)) {
    if (*(char *)(param_1 + 0x20b) != '\0') {
      bVar1 = false;
      if ((_LAB_000348c8_2 != 0) && (bVar1 = false, _LAB_000348fc == 0)) {
        bVar1 = true;
      }
      if (bVar1) {
        *(undefined4 *)(param_1 + 0x200) = 0;
        iVar2 = 0;
        do {
          if (*(int *)(iVar2 * 0xc0 + 0x4f5f0) != 0) {
            *(undefined4 *)(iVar2 * 0xc0 + 0x4f5fc) = 0;
          }
          iVar2 = (iVar2 + 1) * 0x10000 >> 0x10;
        } while (iVar2 < 8);
        if (iRam0004fbf0 != 0) {
          uRam0004fbfc = 0;
        }
      }
    }
  }
  else {
    do {
      iVar2 = FUN_0005e5e8(param_1);
    } while (iVar2 == 0);
  }
  return;
}

