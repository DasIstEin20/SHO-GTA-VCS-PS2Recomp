/* REVIEW-ONLY: FUN_0006041c @ 0x0006041c */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0006041c(void)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  
  iVar1 = *(int *)(_DAT_ffff0e24 + *(int *)(_DAT_ffff0e24 + 3) + 3);
  _DAT_ffff9f6c = *(undefined2 *)(_DAT_ffff0e24 + iVar1 + 8);
  _DAT_ffff9f6e = *(undefined2 *)(_DAT_ffff0e24 + iVar1 + 10);
  _DAT_ffff9f70 = *(undefined4 *)(_DAT_ffff0e24 + iVar1 + 0xc);
  _DAT_ffff9f74 = *(short *)(_DAT_ffff0e24 + iVar1 + 0x10);
  _DAT_ffff9f76 = *(undefined2 *)(_DAT_ffff0e24 + iVar1 + 0x12);
  iVar1 = iVar1 + 0x14;
  uVar2 = 0;
  if (0 < _DAT_ffff9f74) {
    do {
      puVar3 = (undefined4 *)(_DAT_ffff0e24 + iVar1);
      iVar1 = iVar1 + 4;
      *(undefined4 *)(uVar2 * 4 + 0x4f340) = *puVar3;
      uVar2 = uVar2 + 1 & 0xffff;
    } while ((int)uVar2 < (int)_DAT_ffff9f74);
  }
  return;
}

