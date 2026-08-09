/* REVIEW-ONLY: FUN_002d30e8 @ 0x002d30e8 */


undefined8 FUN_002d30e8(undefined8 param_1)

{
  bool bVar1;
  int iVar2;
  undefined8 uVar3;
  long lVar4;
  undefined8 uVar5;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,2,0x50e1f8);
  uVar3 = FUN_00471ac8(DAT_00487d94,DAT_0050e1f8);
  uVar3 = FUN_00238138(uVar3);
  lVar4 = FUN_0024a5b8();
  if (lVar4 != 0) {
    uVar5 = FUN_0024a5b8();
    FUN_00210708(uVar5);
  }
  if (DAT_0050e1fc == 0) {
    iVar2 = FUN_0024a5b8();
    bVar1 = false;
    if (((*(ulong *)(iVar2 + 0xcc8) & 0x8000000000000) != 0) && (*(int *)(iVar2 + 0x480) != 0)) {
      lVar4 = FUN_00219d38();
      bVar1 = lVar4 != 0;
    }
    if (bVar1) {
      iVar2 = FUN_00285440(0);
      *(ushort *)(iVar2 + 0x96) = *(ushort *)(iVar2 + 0x96) | 0x20;
    }
    else {
      FUN_0024aac8(uVar3,1);
    }
    lVar4 = FUN_002d3038();
    if (lVar4 != 0) {
      uVar3 = FUN_0024a5b8();
      FUN_00201da8(uVar3);
    }
  }
  else {
    iVar2 = FUN_0024a5b8();
    bVar1 = false;
    if (((*(ulong *)(iVar2 + 0xcc8) & 0x8000000000000) != 0) && (*(int *)(iVar2 + 0x480) != 0)) {
      lVar4 = FUN_00219d38();
      bVar1 = lVar4 != 0;
    }
    if (bVar1) {
      iVar2 = FUN_00285440(0);
      *(ushort *)(iVar2 + 0x96) = *(ushort *)(iVar2 + 0x96) & 0xffdf;
    }
    else {
      FUN_0024aac8(uVar3,0);
    }
    lVar4 = FUN_002d3038();
    if ((lVar4 != 0) && (iVar2 = FUN_0024a5b8(), *(int *)(iVar2 + 0x8c4) != 0x16)) {
      uVar3 = FUN_0024a5b8();
      FUN_00201da8(uVar3);
    }
  }
  return 0;
}

