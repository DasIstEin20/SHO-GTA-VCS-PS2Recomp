/* REVIEW-ONLY: FUN_00256308 @ 0x00256308 */


void FUN_00256308(undefined8 param_1)

{
  byte bVar1;
  uint uVar2;
  undefined4 uVar3;
  ushort uVar4;
  undefined4 *puVar5;
  uint uVar6;
  undefined4 *puVar7;
  uint uVar8;
  int iVar9;
  
  iVar9 = (int)param_1;
  uVar4 = *(short *)(iVar9 + 0x204) - 1;
  *(ushort *)(iVar9 + 0x204) = uVar4;
  uVar2 = *(uint *)(iVar9 + (uint)uVar4 * 4 + 0x14);
  *(uint *)(iVar9 + 0x10) = uVar2;
  if ((uVar2 & 0x800000) != 0) {
    if ((uVar2 & 0x400000) == 0) {
      uVar2 = *(uint *)(iVar9 + 0x10);
    }
    else {
      *(bool *)(iVar9 + 0x209) = *(char *)(iVar9 + 0x209) == '\0';
      uVar2 = *(uint *)(iVar9 + 0x10);
    }
    uVar6 = uVar2 & 0x3fffff;
    *(uint *)(iVar9 + 0x10) = uVar6;
    bVar1 = *(byte *)(DAT_0048771c + uVar6);
    *(uint *)(iVar9 + 0x10) = uVar6 + 1;
    puVar5 = (undefined4 *)(iVar9 + (*(int *)(iVar9 + 0x1fc) + (uint)bVar1) * 4 + 0x54);
    uVar8 = (uint)*(byte *)(DAT_0048771c + uVar6 + 1);
    *(uint *)(iVar9 + 0x10) = uVar6 + 2;
    bVar1 = *(byte *)(DAT_0048771c + uVar6 + 2);
    *(uint *)(iVar9 + 0x10) = uVar6 + 3;
    if (uVar8 != 0) {
      puVar7 = &DAT_0050e1f8;
      uVar6 = uVar8;
      do {
        uVar3 = *puVar5;
        uVar6 = uVar6 - 1;
        puVar5 = puVar5 + 1;
        *puVar7 = uVar3;
        puVar7 = puVar7 + 1;
      } while (uVar6 != 0);
    }
    *(uint *)(iVar9 + 0x10) = *(int *)(iVar9 + 0x10) + (uVar2 >> 0x18);
    *(uint *)(iVar9 + 0x1fc) = *(int *)(iVar9 + 0x1fc) - (uint)bVar1;
    FUN_002550f8(param_1,iVar9 + 0x10,uVar8);
  }
  return;
}

