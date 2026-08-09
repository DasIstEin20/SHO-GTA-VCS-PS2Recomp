/* REVIEW-ONLY: FUN_0005e5e8 @ 0x0005e5e8 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_0005e5e8(int param_1)

{
  short sVar1;
  char cVar2;
  uint uVar3;
  code *pcVar4;
  short *psVar5;
  int iVar6;
  short local_10;
  short sStack_e;
  
  sVar1 = *(short *)(_DAT_ffff3534 + *(int *)(param_1 + 0x10));
  *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 2;
  uVar3 = (uint)sVar1;
  if ((uVar3 & 0x8000) == 0) {
    *(undefined1 *)(param_1 + 0x20c) = 0;
  }
  else {
    *(undefined1 *)(param_1 + 0x20c) = 1;
  }
  uVar3 = uVar3 & 0x7fff;
  pcVar4 = (code *)(&PTR_LAB_00380ab4)[uVar3 * 2];
  sStack_e = (short)((uint)*(undefined4 *)(&DAT_00380ab0 + uVar3 * 8) >> 0x10);
  iVar6 = (int)sStack_e;
  local_10 = (short)*(undefined4 *)(&DAT_00380ab0 + uVar3 * 8);
  if ((iVar6 == 0) && (local_10 == 0 && pcVar4 == FUN_00000000)) {
    iVar6 = -1;
  }
  else {
    param_1 = param_1 + local_10;
    if (iVar6 != 0) {
      psVar5 = (short *)(*(int *)(pcVar4 + param_1) + iVar6 * 8);
      pcVar4 = *(code **)(psVar5 + 2);
      param_1 = param_1 + *psVar5;
    }
    cVar2 = (*pcVar4)(param_1);
    iVar6 = (int)cVar2;
  }
  return iVar6;
}

