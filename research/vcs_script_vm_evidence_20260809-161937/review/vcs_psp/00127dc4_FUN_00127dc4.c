/* REVIEW-ONLY: FUN_00127dc4 @ 0x00127dc4 */


void FUN_00127dc4(int param_1,int param_2,int param_3)

{
  undefined8 uVar1;
  undefined4 in_zero;
  int iVar2;
  int unaff_gp;
  undefined4 local_30;
  undefined4 uStack_2c;
  undefined4 local_28;
  
  if (*(int *)(param_1 + 0x8b4) == 0x12) {
    FUN_001a68f0(param_1);
  }
  FUN_0010cccc(param_1,0x45);
  FUN_0010cbdc(param_1,1);
  FUN_0010ca24(param_1,param_2);
  FUN_00104d60(param_1,0x33);
  FUN_0010cad4(param_1,param_2);
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffdff;
  *(uint *)(param_1 + 0x1c8) = *(uint *)(param_1 + 0x1c8) & 0xffffefff;
  *(uint *)(param_1 + 0x1d0) = *(uint *)(param_1 + 0x1d0) | 0x400;
  if (*(int *)(param_2 + 0x354) == 6) {
    iVar2 = FUN_002fb35c(param_2,0);
    if (iVar2 == 0) {
      FUN_002fb370(param_2,0,param_1);
    }
  }
  else if (param_3 < 0) {
    iVar2 = FUN_002fb35c(param_2,0);
    if (iVar2 == 0) {
      FUN_002fb370(param_2,0,param_1);
    }
    else {
      iVar2 = FUN_002fb35c(param_2,1);
      if (iVar2 == 0) {
        FUN_002fb370(param_2,1,param_1);
      }
      else {
        iVar2 = FUN_002fb35c(param_2,2);
        if (iVar2 == 0) {
          FUN_002fb370(param_2,2,param_1);
        }
        else {
          iVar2 = FUN_002fb35c(param_2,3);
          if (iVar2 == 0) {
            FUN_002fb370(param_2,3,param_1);
          }
        }
      }
    }
  }
  else {
    iVar2 = FUN_002fb35c(param_2,param_3);
    if (iVar2 == 0) {
      FUN_002fb370(param_2,param_3,param_1);
    }
  }
  iVar2 = FUN_0010944c(param_1);
  if (iVar2 != 0) {
    FUN_000c2fa4(0x1f3f0);
    FUN_0033e52c(param_2);
  }
  if ((*(int *)(param_2 + 900) == 0) ||
     (iVar2 = FUN_0010944c(*(undefined4 *)(param_2 + 900)), iVar2 == 0)) {
    *(uint *)(param_2 + 0x48) = *(uint *)(param_2 + 0x48) & 0xfffffe0f | 0x30;
  }
  else {
    *(uint *)(param_2 + 0x48) = *(uint *)(param_2 + 0x48) & 0xfffffe0f;
  }
  FUN_0008db8c(param_1,2);
  setCopReg(2,in_zero,*(undefined8 *)(param_2 + 0x30));
  uVar1 = getCopReg(2,0);
  local_30 = (undefined4)uVar1;
  uStack_2c = (undefined4)((ulonglong)uVar1 >> 0x20);
  *(undefined4 *)(param_1 + 0x30) = local_30;
  *(undefined4 *)(param_1 + 0x34) = uStack_2c;
  *(undefined4 *)(param_1 + 0x38) = local_28;
  FUN_0008da2c(param_1);
  if ((*(byte *)(param_2 + 0x265) & 2) != 0) {
    *(byte *)(param_2 + 0x265) = *(byte *)(param_2 + 0x265) & 0xfd;
    *(int *)(unaff_gp + 0x156c) = *(int *)(unaff_gp + 0x156c) + -1;
  }
  if ((*(byte *)(param_2 + 0x265) & 4) != 0) {
    *(byte *)(param_2 + 0x265) = *(byte *)(param_2 + 0x265) & 0xfb;
    *(int *)(unaff_gp + 0x1570) = *(int *)(unaff_gp + 0x1570) + -1;
  }
  if ((*(byte *)(param_2 + 0x265) & 0x10) == 0) {
    *(byte *)(param_2 + 0x265) = *(byte *)(param_2 + 0x265) | 0x10;
    FUN_00202314(0x3f800000,unaff_gp + 0x1e78,*(undefined4 *)(param_2 + 0x60),0x12);
  }
  FUN_00069370(0xc47a0000,*(undefined4 *)(param_1 + 0x50),0x10);
  FUN_001b1a34(param_1,param_2,*(int *)(param_2 + 900) == param_1);
  FUN_0010450c(param_1);
  if ((*(byte *)(param_2 + 0x266) & 2) != 0) {
    *(uint *)(param_1 + 0x1c8) = *(uint *)(param_1 + 0x1c8) & 0xfff7ffff;
  }
  *(uint *)(param_1 + 0x1c8) = *(uint *)(param_1 + 0x1c8) | 0x100000;
  return;
}

