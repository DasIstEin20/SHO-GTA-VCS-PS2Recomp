/* REVIEW-ONLY: FUN_0007943c @ 0x0007943c */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0007943c(int *param_1,char param_2)

{
  undefined8 uVar1;
  bool bVar2;
  bool bVar3;
  undefined4 in_zero;
  int iVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  undefined4 uVar12;
  undefined1 local_1a0 [4];
  int local_19c;
  undefined8 local_190;
  undefined8 local_180;
  undefined8 local_170;
  int local_160;
  undefined1 local_150 [4];
  int local_14c;
  undefined1 local_140;
  undefined8 local_130;
  undefined8 local_120;
  undefined8 local_110;
  int local_100;
  int local_fc;
  int local_f8;
  float local_f0;
  float local_ec;
  undefined1 auStack_e0 [96];
  undefined1 auStack_80 [16];
  undefined4 local_70;
  undefined8 local_40 [2];
  
  iVar4 = FUN_0010cb38(*param_1);
  if (iVar4 == 0) {
    iVar4 = FUN_00102de4(*param_1);
    if (iVar4 == 0) {
      uVar12 = 0;
    }
    else {
      FUN_00079024(param_1);
      if (param_1[9] == 0) {
        uVar12 = 0;
      }
      else {
        bVar2 = *(int *)(*param_1 + 0x860) == 0;
        iVar4 = FUN_00363748(_DAT_ffff4198);
        uVar6 = iVar4 + 1;
        if ((int)uVar6 < 0) {
          uVar6 = -(-uVar6 & 3);
        }
        else {
          uVar6 = uVar6 & 3;
        }
        uVar8 = _DAT_ffff9e74 & 3;
        uVar12 = FUN_0022c1a4();
        bVar3 = false;
        if (uVar8 == uVar6 || param_2 != '\0') {
          iVar4 = FUN_00144894(*(undefined4 *)(&LAB_00034740 + (uint)DAT_ffff9e98 * 0x160));
          FUN_00079108(param_1);
          iVar9 = 0;
          iVar7 = 0;
          do {
            iVar7 = iVar4 + iVar7 * 0x79;
            FUN_001ee798(iVar4,iVar7);
            iVar10 = 0;
            if (0 < *(int *)(iVar7 + 0x780)) {
              do {
                iVar11 = iVar7 + iVar10 * 0x40;
                if (*(int *)(iVar11 + 0x10) != *param_1) {
                  FUN_00079144(local_1a0);
                  FUN_00079150(local_1a0,iVar11);
                  if (((*(byte *)(iVar11 + 0x34) & 2) == 0) &&
                     (iVar5 = FUN_0022bfcc(param_1[0x18],param_1[0x5d],param_1[9],iVar11,
                                           param_1 + 0x54,param_1 + 0x10), iVar5 != 0)) {
                    FUN_00079334(local_150);
                    FUN_00079364(uVar12,local_150,local_1a0,param_1 + 8,param_1 + 0x4c);
                    if ((local_ec < (float)param_1[0x5c]) && (0.0 <= local_f0)) {
                      FUN_0010d370(*param_1,iVar11);
                      *(undefined1 *)(param_1 + 0x1c) = local_1a0[0];
                      param_1[0x1d] = local_19c;
                      setCopReg(2,in_zero,local_190);
                      uVar1 = getCopReg(2,0);
                      *(undefined8 *)(param_1 + 0x20) = uVar1;
                      setCopReg(2,in_zero,local_180);
                      uVar1 = getCopReg(2,0);
                      *(undefined8 *)(param_1 + 0x24) = uVar1;
                      setCopReg(2,in_zero,local_170);
                      uVar1 = getCopReg(2,0);
                      *(undefined8 *)(param_1 + 0x28) = uVar1;
                      param_1[0x2c] = local_160;
                      *(undefined1 *)(param_1 + 0x30) = local_150[0];
                      param_1[0x31] = local_14c;
                      *(undefined1 *)(param_1 + 0x34) = local_140;
                      setCopReg(2,in_zero,local_130);
                      uVar1 = getCopReg(2,0);
                      *(undefined8 *)(param_1 + 0x38) = uVar1;
                      setCopReg(2,in_zero,local_120);
                      uVar1 = getCopReg(2,0);
                      *(undefined8 *)(param_1 + 0x3c) = uVar1;
                      setCopReg(2,in_zero,local_110);
                      uVar1 = getCopReg(2,0);
                      *(undefined8 *)(param_1 + 0x40) = uVar1;
                      param_1[0x44] = local_100;
                      param_1[0x45] = local_fc;
                      param_1[0x46] = local_f8;
                      param_1[0x48] = (int)local_f0;
                      param_1[0x49] = (int)local_ec;
                      bVar3 = true;
                    }
                  }
                }
                iVar10 = iVar10 + 1;
              } while (iVar10 < *(int *)(iVar7 + 0x780));
            }
            iVar9 = iVar9 + 1;
            iVar7 = iVar9 * 0x10;
          } while (iVar9 < 3);
        }
        else if (!bVar2) {
          FUN_00079108(param_1);
          FUN_00079144(auStack_e0);
          FUN_00079150(auStack_e0,*param_1 + 0x850);
          iVar4 = FUN_0022bfcc(param_1[0x18],param_1[0x5c],param_1[9],*param_1 + 0x850,
                               param_1 + 0x54,param_1 + 0x10);
          if (iVar4 != 0) {
            FUN_00079150(param_1 + 0x1c,*param_1 + 0x850);
            FUN_00079364(uVar12,param_1 + 0x30,param_1 + 0x1c,param_1 + 8,param_1 + 0x4c);
            if (((float)param_1[0x49] < (float)param_1[0x5c]) && (0.0 <= (float)param_1[0x48])) {
              bVar3 = true;
            }
          }
        }
        if (bVar3) {
          uVar12 = FUN_000798d4(param_1,local_40,0,1);
          setCopReg(2,in_zero,local_40[0]);
          uVar1 = getCopReg(2,0);
          *(undefined8 *)(param_1 + 4) = uVar1;
          *(undefined1 *)(param_1 + 0x4c) = 0;
          FUN_00079228(param_1 + 0x4c,param_1 + 4,param_1 + 0x10);
        }
        else {
          local_70 = 0;
          FUN_0010d370(*param_1,auStack_80);
          if (bVar2) {
            uVar12 = 0;
          }
          else {
            uVar12 = FUN_0007943c(param_1,1);
          }
        }
      }
    }
  }
  else {
    uVar12 = 0;
  }
  return uVar12;
}

