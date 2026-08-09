/* REVIEW-ONLY: FUN_002e090c @ 0x002e090c */


void FUN_002e090c(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  undefined1 *puVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  undefined1 *puVar8;
  int iVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  undefined1 auStack_c0 [32];
  int local_a0 [20];
  undefined1 auStack_50 [4];
  int *local_4c;
  
  puVar4 = auStack_c0;
  puVar8 = auStack_c0;
  FUN_00250f6c(0x43e98000);
  FUN_00250f2c(0x41500000);
  iVar9 = 0;
  iVar7 = 0;
  iVar6 = 0;
  FUN_00251054(1);
  FUN_00251030();
  FUN_0025109c(0);
  FUN_00250df8(0x3eba8826,0x3f4ac083);
  piVar3 = (int *)0x273d88;
  iVar5 = 0;
  local_4c = (int *)0x273d88;
  do {
    if (*piVar3 != 0) {
      FUN_000771ac(*piVar3,piVar3[1],piVar3[2],piVar3[3]);
      FUN_00075ca0(0x2718e8,piVar3[7]);
      FUN_00075f60(0x2718e8);
      iVar1 = FUN_00253420(0x41500000,0x2718e8);
      *(int *)(puVar4 + 0x20) = iVar1;
      iVar7 = iVar7 + iVar1 + -1;
      iVar6 = iVar6 + 1;
    }
    iVar5 = iVar5 + 1;
    piVar3 = piVar3 + 8;
    puVar4 = puVar4 + 4;
  } while (iVar5 < 0x14);
  if (0.0 < DAT_00355c3c) {
    if (DAT_00355c38 == '\0') {
      DAT_00355c40 = DAT_00355c40 - 30.0 / DAT_00355c3c;
    }
    else {
      DAT_00355c40 = DAT_00355c40 + 30.0 / DAT_00355c3c;
    }
  }
  iVar5 = 0;
  fVar12 = (float)(iVar6 + 8) * 20.0 + (float)iVar7 * 11.0;
  piVar3 = local_4c;
  do {
    iVar6 = *piVar3;
    iVar5 = iVar5 + 1;
    if (iVar6 != 0) {
      iVar9 = iVar9 + *(int *)(puVar8 + 0x20) + -1;
    }
    for (fVar11 = ((206.0 - (float)iVar5 * 20.0) - (float)iVar9 * 11.0) + DAT_00355c40;
        fVar11 < 12.0; fVar11 = fVar12 + fVar11) {
    }
    for (; 226.0 < fVar11; fVar11 = fVar11 - fVar12) {
    }
    if ((32.0 < fVar11) &&
       (fVar10 = fVar11 + (float)(*(int *)(puVar8 + 0x20) + -1) * 11.0, fVar10 < 206.0)) {
      fVar13 = 255.0;
      if (fVar11 < 52.0) {
        fVar13 = (float)(int)(((fVar11 - 32.0) / 20.0) * 255.0);
      }
      if (((186.0 < fVar10) && (fVar11 = (fVar10 - 186.0) / 20.0, fVar11 < 1.0)) && (0.0 < fVar11))
      {
        fVar13 = (float)(int)((1.0 - fVar11) * 255.0);
      }
      if (255.0 < fVar13) {
        fVar13 = 255.0;
      }
      if (iVar6 != 0) {
        FUN_000771ac(iVar6,piVar3[1],piVar3[2],piVar3[3]);
        FUN_00075ca0(0x2718e8,piVar3[7]);
        FUN_00075f60(0x2718e8);
        FUN_002d57e4(param_1,(int)fVar13);
        uVar2 = FUN_0005e9cc(auStack_50,0xff,0xff,0xff);
        FUN_00250e30(uVar2);
        FUN_00250f14();
        FUN_0025109c(0);
        FUN_002d57e4(param_1,(int)fVar13);
        uVar2 = FUN_0005e9cc(auStack_50,0,0,0);
        FUN_002510ac(uVar2);
        FUN_00252ad4(0x41700000,0x2718e8,0);
        FUN_00250dcc();
        FUN_002d57e4(param_1,0xff);
        uVar2 = FUN_0005e9cc(auStack_50,0xff,0xff,0xff);
        FUN_00250e30(uVar2);
      }
    }
    piVar3 = piVar3 + 8;
    puVar8 = puVar8 + 4;
  } while (iVar5 < 0x14);
  FUN_00250f6c((float)(DAT_00352acc + -10));
  FUN_00250f2c(0x41200000);
  return;
}

