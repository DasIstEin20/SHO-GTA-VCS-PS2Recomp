/* REVIEW-ONLY: FUN_0017f2cc @ 0x0017f2cc */


void FUN_0017f2cc(int param_1,uint *param_2,int param_3,uint *param_4)

{
  undefined1 uVar1;
  char cVar2;
  undefined2 uVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  uint uVar7;
  undefined4 *puVar8;
  uint uVar9;
  undefined8 uVar10;
  uint local_60;
  undefined4 local_5c;
  undefined8 local_58;
  undefined8 local_50;
  undefined8 local_48;
  undefined8 local_40;
  undefined8 local_38;
  
  uVar7 = *param_4;
  local_60 = param_4[1];
  local_5c = param_4[2];
  for (uVar4 = (uint)*(byte *)(param_3 + 0xd6); uVar4 < 5; uVar4 = uVar4 + 1) {
    uVar7 = uVar7 & ~(0x80 << (uVar4 & 0x1f));
  }
  uVar4 = *param_2;
  iVar6 = (uVar4 & 0xffff) + 1;
  *(char *)param_2 = (char)iVar6;
  *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
  *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar7 & 0xffff);
  uVar4 = *param_2;
  iVar6 = (uVar4 & 0xffff) + 1;
  *(char *)param_2 = (char)iVar6;
  *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
  *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((uVar7 & 0xffff) >> 8);
  uVar4 = *param_2;
  iVar6 = (uVar4 & 0xffff) + 1;
  *(char *)param_2 = (char)iVar6;
  *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
  *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar7 >> 0x10);
  uVar4 = *param_2;
  iVar6 = (uVar4 & 0xffff) + 1;
  *(char *)param_2 = (char)iVar6;
  *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
  *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar7 >> 0x18);
  if ((uVar7 & 1) != 0) {
    uVar4 = *param_2;
    uVar3 = *(undefined2 *)(*(int *)(param_1 + 0x70) + 0x58);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)uVar3;
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((ushort)uVar3 >> 8);
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0xb6);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
    if (*(char *)(param_3 + 0xb6) == '\x06') {
      uVar4 = *param_2;
      uVar5 = *(undefined4 *)(*(int *)(param_1 + 0x68) + 0x824);
      iVar6 = (uVar4 & 0xffff) + 1;
      *(char *)param_2 = (char)iVar6;
      *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
      *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)uVar5;
    }
  }
  if ((uVar7 & 0x2000) != 0) {
    uVar3 = *(undefined2 *)(param_3 + 0xb2);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)uVar3;
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((ushort)uVar3 >> 8);
  }
  if ((uVar7 & 0x1000) != 0) {
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0xb4);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 0x4000000) != 0) {
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0xb5);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 2) != 0) {
    uVar3 = *(undefined2 *)(param_3 + 0xb0);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)uVar3;
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((ushort)uVar3 >> 8);
  }
  if ((uVar7 & 4) != 0) {
    uVar9 = *(uint *)(param_3 + 0x40);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 & 0xffff);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((uVar9 & 0xffff) >> 8);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x10);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x18);
    uVar9 = *(uint *)(param_3 + 0x44);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 & 0xffff);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((uVar9 & 0xffff) >> 8);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x10);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x18);
  }
  if ((uVar7 & 8) != 0) {
    uVar9 = *(uint *)(param_3 + 0x48);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 & 0xffff);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((uVar9 & 0xffff) >> 8);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x10);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x18);
  }
  if ((uVar7 & 0x10) != 0) {
    uVar9 = *(uint *)(param_3 + 0xb8);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 & 0xffff);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((uVar9 & 0xffff) >> 8);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x10);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x18);
  }
  if ((uVar7 & 0x20) != 0) {
    uVar9 = *(uint *)(param_3 + 0xbc);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 & 0xffff);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((uVar9 & 0xffff) >> 8);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x10);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)(uVar9 >> 0x18);
  }
  cVar2 = *(char *)(param_3 + 0xd6);
  if ((uVar7 & 0x40) != 0) {
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = cVar2;
    cVar2 = *(char *)(param_3 + 0xd6);
  }
  uVar4 = 0;
  if (cVar2 != '\0') {
    iVar6 = param_3 + 0xd8;
    do {
      if ((uVar7 & 0x80 << (uVar4 & 0x1f)) != 0) {
        FUN_0017cd58(param_1,param_2,iVar6,*(undefined1 *)((int)&local_60 + uVar4));
      }
      uVar4 = uVar4 + 1;
      iVar6 = iVar6 + 0x18;
    } while (uVar4 < *(byte *)(param_3 + 0xd6));
  }
  if ((uVar7 & 0x4000) != 0) {
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0xc0);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 0x8000) != 0) {
    FUN_0012acac(param_2,param_3 + 0xc0,local_5c._1_1_,0xffff);
  }
  if ((uVar7 & 0x10000) != 0) {
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0xb7);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 0x2000000) != 0) {
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0x159);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 0x20000) != 0) {
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0x150);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 0x40000) != 0) {
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0x151);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 0x80000) != 0) {
    uVar3 = *(undefined2 *)(param_3 + 0x152);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)uVar3;
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((ushort)uVar3 >> 8);
  }
  if ((uVar7 & 0x100000) != 0) {
    uVar4 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0x154);
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 0x200000) != 0) {
    FUN_0025876c(param_2,param_3 + 0x60);
  }
  if ((uVar7 & 0x400000) != 0) {
    uVar1 = *(undefined1 *)(param_3 + 0x155);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar4 & 0xffff)) = uVar1;
  }
  if ((uVar7 & 0x800000) != 0) {
    uVar3 = *(undefined2 *)(param_3 + 0x156);
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)uVar3;
    uVar4 = *param_2;
    iVar6 = (uVar4 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(char *)((int)param_2 + (uVar4 & 0xffff)) = (char)((ushort)uVar3 >> 8);
  }
  if ((uVar7 & 0x1000000) != 0) {
    uVar7 = *param_2;
    uVar1 = *(undefined1 *)(param_3 + 0x158);
    iVar6 = (uVar7 & 0xffff) + 1;
    *(char *)param_2 = (char)iVar6;
    *(char *)((int)param_2 + 1) = (char)((uint)iVar6 >> 8);
    *(undefined1 *)((int)param_2 + (uVar7 & 0xffff)) = uVar1;
  }
  local_58 = FUN_002f1d84(*(uint *)(param_3 + 0x40));
  FUN_002f1d84(*(undefined4 *)(param_3 + 0x44));
  local_50 = FUN_002f1d84(*(undefined4 *)(param_3 + 0x48));
  local_48 = FUN_002f1d84(*(undefined4 *)(param_3 + 0x20));
  local_40 = FUN_002f1d84(*(undefined4 *)(param_3 + 0x24));
  local_38 = FUN_002f1d84(*(undefined4 *)(param_3 + 0x28));
  uVar3 = *(undefined2 *)(param_3 + 0xb0);
  FUN_002f1d84(*(undefined4 *)(param_3 + 0xb8));
  FUN_002f1d84(*(undefined4 *)(param_3 + 0xbc));
  FUN_0017e524(" PedState (%f %f %f) (%f %f %f) car: %i heading: %f %f anims: %i",
               *(undefined1 *)(param_3 + 0xd6),(undefined4)local_58,local_58._4_4_,local_38,uVar3);
  uVar7 = 0;
  if (*(char *)(param_3 + 0xd6) != '\0') {
    puVar8 = (undefined4 *)(param_3 + 0xd8);
    do {
      uVar10 = FUN_002f1d84(*puVar8);
      local_38 = uVar10;
      uVar10 = FUN_002f1d84(puVar8[1]);
      local_40 = uVar10;
      FUN_002f1d84(puVar8[2]);
      FUN_002f1d84(puVar8[3]);
      FUN_0017e524("  anim %i: blend %f %f time %f spd %f anm %i, %i flags %i",uVar7,
                   (undefined4)local_38,local_38._4_4_,puVar8[5]);
      uVar7 = uVar7 + 1;
      puVar8 = puVar8 + 6;
    } while (uVar7 < *(byte *)(param_3 + 0xd6));
  }
  return;
}

