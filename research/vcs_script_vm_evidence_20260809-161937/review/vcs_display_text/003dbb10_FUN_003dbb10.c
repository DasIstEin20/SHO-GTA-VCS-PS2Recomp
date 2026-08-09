/* REVIEW-ONLY: FUN_003dbb10 @ 0x003dbb10 */


undefined8 FUN_003dbb10(undefined8 param_1)

{
  short sVar1;
  undefined4 uVar2;
  short *psVar3;
  undefined8 uVar4;
  short *psVar5;
  undefined2 *puVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  uint uVar10;
  int iVar11;
  int aiStack_40 [4];
  
  iVar11 = (int)param_1 + 0x10;
  FUN_00254d18(param_1,iVar11,2,0x50e1f8);
  if (DAT_0048a5a0 == 0) {
    uVar4 = FUN_002bd320(5000,iVar11);
    DAT_0048a5a0 = FUN_003fb648(uVar4);
  }
  iVar9 = DAT_0048a5a0;
  FUN_00254d18(param_1,iVar11,1,aiStack_40);
  if (aiStack_40[0] == 0) {
    aiStack_40[0] = 0;
  }
  else {
    aiStack_40[0] = DAT_0048771c + aiStack_40[0];
  }
  psVar3 = (short *)FUN_003fb4b0(iVar9,aiStack_40[0]);
  uVar2 = DAT_0050e1fc;
  if (DAT_004cd6f8 < 0x14) {
    uVar10 = (uint)DAT_004cd6f8;
    uVar7 = (uint)DAT_004cd6f8;
    *(undefined4 *)(&DAT_0050d32c + (uint)DAT_004cd6f8 * 0xa8) = DAT_0050e1f8;
    (&DAT_0050d330)[uVar10 * 0x2a] = uVar2;
    (&DAT_0050d334)[uVar7 * 0x2a] = (&DAT_0050d334)[uVar7 * 0x2a] & 0xffffffdf;
    iVar11 = 0;
    sVar1 = *psVar3;
    psVar5 = psVar3;
    while (sVar1 != 0) {
      psVar5 = psVar5 + 1;
      iVar11 = iVar11 + 1;
      sVar1 = *psVar5;
    }
    if (0x3f < iVar11) {
      psVar3[0x3f] = 0;
      iVar11 = 0x3f;
    }
    iVar9 = 0;
    if (0 < iVar11) {
      psVar5 = (short *)((int)&DAT_0050d334 + (uint)DAT_004cd6f8 * 0xa8 + 2);
      iVar8 = iVar11;
      do {
        sVar1 = *psVar3;
        iVar8 = iVar8 + -1;
        psVar3 = psVar3 + 1;
        *psVar5 = sVar1;
        psVar5 = psVar5 + 1;
        iVar9 = iVar11;
      } while (iVar8 != 0);
    }
    if (iVar9 < 0x40) {
      puVar6 = (undefined2 *)(iVar9 * 2 + (uint)DAT_004cd6f8 * 0xa8 + 0x50d336);
      do {
        *puVar6 = 0;
        iVar9 = iVar9 + 1;
        puVar6 = puVar6 + 1;
      } while (iVar9 < 0x40);
    }
    DAT_004cd6f8 = DAT_004cd6f8 + 1;
  }
  return 0;
}

