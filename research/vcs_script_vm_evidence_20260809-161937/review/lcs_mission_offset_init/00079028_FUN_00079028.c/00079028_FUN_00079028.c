/* REVIEW-ONLY: FUN_00079028 @ 0x00079028 */


void FUN_00079028(void)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  
  iVar1 = *(int *)(DAT_00334d7c + *(int *)(DAT_00334d7c + 3) + 3);
  uRam00001196 = *(undefined2 *)(DAT_00334d7c + iVar1 + 8);
  uRam00001198 = *(undefined2 *)(DAT_00334d7c + iVar1 + 10);
  uRam0000119c = *(undefined4 *)(DAT_00334d7c + iVar1 + 0xc);
  sRam000011a0 = *(short *)(DAT_00334d7c + iVar1 + 0x10);
  uRam000011a2 = *(undefined2 *)(DAT_00334d7c + iVar1 + 0x12);
  iVar1 = iVar1 + 0x14;
  uVar2 = 0;
  if (0 < sRam000011a0) {
    do {
      puVar3 = (undefined4 *)(DAT_00334d7c + iVar1);
      iVar1 = iVar1 + 4;
      *(undefined4 *)(uVar2 * 4 + 0x276288) = *puVar3;
      uVar2 = uVar2 + 1 & 0xffff;
    } while ((int)uVar2 < (int)sRam000011a0);
  }
  return;
}

