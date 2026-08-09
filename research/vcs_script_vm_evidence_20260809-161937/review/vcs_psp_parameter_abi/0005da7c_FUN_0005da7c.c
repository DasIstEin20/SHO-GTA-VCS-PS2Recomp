/* REVIEW-ONLY: FUN_0005da7c @ 0x0005da7c */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_0005da7c(int param_1,int *param_2)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  
  uVar4 = (uint)*(byte *)(_DAT_ffff3534 + *param_2);
  *param_2 = *param_2 + 1;
  if (uVar4 < 0xcd) {
    if (uVar4 < 0x6d) {
      if (uVar4 < 0xd) {
        iVar5 = uVar4 + 0x5d;
      }
      else {
        iVar5 = uVar4 + *(int *)(param_1 + 0x1fc) + -0xd;
      }
    }
    else {
      iVar5 = *param_2;
      bVar1 = *(byte *)(_DAT_ffff3534 + iVar5);
      *param_2 = iVar5 + 1;
      iVar6 = *(int *)(param_1 + 0x1fc);
      bVar2 = *(byte *)(_DAT_ffff3534 + iVar5 + 1);
      *param_2 = iVar5 + 2;
      iVar5 = *(int *)(param_1 + ((uint)bVar1 + iVar6) * 4 + 0x54);
      if ((iVar5 < 0) || ((int)(uint)bVar2 <= iVar5)) {
        return -0x4164;
      }
      iVar5 = uVar4 + *(int *)(param_1 + 0x1fc) + iVar5 + -0x6d;
    }
    iVar5 = param_1 + iVar5 * 4 + 0x54;
  }
  else {
    bVar1 = *(byte *)(_DAT_ffff3534 + *param_2);
    *param_2 = *param_2 + 1;
    if (uVar4 < 0xe6) {
      iVar5 = (uVar4 - 0xcd) * 0x100;
    }
    else {
      iVar5 = *param_2;
      bVar2 = *(byte *)(_DAT_ffff3534 + iVar5);
      *param_2 = iVar5 + 1;
      iVar6 = *(int *)(param_1 + 0x1fc);
      bVar3 = *(byte *)(_DAT_ffff3534 + iVar5 + 1);
      *param_2 = iVar5 + 2;
      iVar5 = *(int *)(param_1 + ((uint)bVar2 + iVar6) * 4 + 0x54);
      if ((iVar5 < 0) || ((int)(uint)bVar3 <= iVar5)) {
        return -0x4164;
      }
      iVar5 = (uVar4 - 0xe6) * 0x100 + iVar5;
    }
    iVar5 = _DAT_ffff3534 + ((uint)bVar1 + iVar5) * 4;
  }
  return iVar5;
}

