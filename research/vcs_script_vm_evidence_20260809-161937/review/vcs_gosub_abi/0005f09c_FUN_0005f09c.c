/* REVIEW-ONLY: FUN_0005f09c @ 0x0005f09c */


void FUN_0005f09c(int param_1)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  ushort uVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  uint uVar8;
  int iVar9;
  undefined4 *puVar10;
  
  uVar4 = *(short *)(param_1 + 0x204) - 1;
  uVar8 = *(uint *)(param_1 + (uint)uVar4 * 4 + 0x14);
  *(ushort *)(param_1 + 0x204) = uVar4;
  *(uint *)(param_1 + 0x10) = uVar8;
  if ((uVar8 & 0x800000) != 0) {
    iVar9 = *(int *)(param_1 + 0x1fc);
    if ((uVar8 & 0x400000) != 0) {
      *(bool *)(param_1 + 0x209) = *(char *)(param_1 + 0x209) == '\0';
    }
    uVar5 = uVar8 & 0x3fffff;
    *(uint *)(param_1 + 0x10) = uVar5;
    bVar1 = *(byte *)(DAT_0000ef04 + uVar5);
    *(uint *)(param_1 + 0x10) = uVar5 + 1;
    bVar2 = *(byte *)(DAT_0000ef04 + uVar5 + 1);
    *(uint *)(param_1 + 0x10) = uVar5 + 2;
    bVar3 = *(byte *)(DAT_0000ef04 + uVar5 + 2);
    *(uint *)(param_1 + 0x10) = uVar5 + 3;
    puVar7 = (undefined4 *)((uint)bVar1 * 4 + param_1 + iVar9 * 4 + 0x54);
    iVar6 = 0;
    if (bVar2 != 0) {
      puVar10 = (undefined4 *)0x1d698;
      do {
        iVar6 = iVar6 + 1;
        *puVar10 = *puVar7;
        puVar7 = puVar7 + 1;
        puVar10 = puVar10 + 1;
      } while (iVar6 < (int)(uint)bVar2);
      iVar9 = *(int *)(param_1 + 0x1fc);
    }
    *(uint *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + (uVar8 >> 0x18);
    *(uint *)(param_1 + 0x1fc) = iVar9 - (uint)bVar3;
    FUN_0005e890(param_1,param_1 + 0x10,bVar2);
  }
  return;
}

