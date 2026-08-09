/* REVIEW-ONLY: FUN_002a19f4 @ 0x002a19f4 */


/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Removing unreachable block (ram,0x002a32b4) */
/* WARNING: Removing unreachable block (ram,0x002a302c) */
/* WARNING: Removing unreachable block (ram,0x002a2fb0) */
/* WARNING: Removing unreachable block (ram,0x002a3140) */
/* WARNING: Removing unreachable block (ram,0x002a3320) */
/* WARNING: Removing unreachable block (ram,0x002a31ac) */

void FUN_002a19f4(int param_1)

{
  short sVar1;
  undefined8 uVar2;
  undefined4 in_zero;
  undefined4 in_at;
  undefined4 extraout_at;
  undefined4 extraout_at_00;
  undefined4 extraout_at_01;
  undefined4 extraout_at_02;
  undefined4 extraout_at_03;
  uint uVar3;
  int iVar4;
  float *pfVar5;
  float *pfVar6;
  int iVar7;
  undefined4 uVar8;
  int iVar9;
  uint uVar10;
  float fVar11;
  float fVar12;
  float fVar13;
  float fVar14;
  float fVar15;
  float fVar16;
  float fVar17;
  float fVar18;
  float fVar19;
  float fVar20;
  float fVar21;
  float fVar22;
  float fVar23;
  float fVar24;
  float fVar25;
  undefined8 local_340 [2];
  undefined8 local_330 [90];
  float local_60;
  float local_5c;
  uint local_54;
  undefined4 *local_50;
  int local_4c;
  int local_48;
  
  if (*(short *)(param_1 + 0x58) == 200) {
    if (100.0 < *(float *)(param_1 + 0x38)) {
      setCopReg(2,in_zero,*(undefined8 *)(param_1 + 0x30));
      local_340[0] = getCopReg(2,0);
      FUN_002b67a8(local_340,(int)*(short *)(param_1 + 0x350));
      in_at = extraout_at;
    }
  }
  else if (*(short *)(param_1 + 0x58) == 0xc9) {
    setCopReg(2,in_zero,*(undefined8 *)(param_1 + 0x30));
    local_330[0] = getCopReg(2,0);
    FUN_002b6ef8(local_330,(int)*(short *)(param_1 + 0x350));
    in_at = extraout_at_00;
  }
  if (*(char *)(param_1 + 0x360) != '\0') {
    if (*(short *)(param_1 + 0x58) == 200) {
      uVar10 = iRam00000e6c - *(int *)(param_1 + 0x35c);
      local_54 = (uint)(uVar10 < 0x28);
      if (uVar10 == 0x14) {
                    /* WARNING: Subroutine does not return */
        FUN_001d18b0();
      }
      if (((local_54 == 0) && (uVar3 = uVar10 & 1, uVar10 < 0x51)) && (uVar3 != 0)) {
        if (uVar3 == 0) {
                    /* WARNING: Subroutine does not return */
          FUN_001d18b0(0,0x3e4ccccd);
        }
                    /* WARNING: Subroutine does not return */
        FUN_001d18b0(uVar3,0x3e4ccccd);
      }
      if (uVar10 == 0x3c) {
        *(uint *)(param_1 + 0x44) = *(uint *)(param_1 + 0x44) | 0x200000;
      }
      if (uVar10 == 0x52) {
        FUN_000ea7ac(&DAT_00026098,0xff,0xff,0xff);
        FUN_000ea884(0,&DAT_00026098,0);
        FUN_000eac6c(&DAT_00026098);
        FUN_000ea884(0x3f800000,&DAT_00026098,1);
        (**(code **)(*(int *)(param_1 + 0x5c) + 0x9c))
                  (param_1 + *(short *)(*(int *)(param_1 + 0x5c) + 0x98));
        in_at = extraout_at_01;
      }
    }
    else {
      uVar10 = iRam00000e6c - *(int *)(param_1 + 0x35c);
      local_54 = (uint)(uVar10 < 0x28);
      if (uVar10 == 0x14) {
                    /* WARNING: Subroutine does not return */
        FUN_001d18b0();
      }
      if (((local_54 == 0) && (uVar3 = uVar10 & 1, uVar10 < 0x3d)) && (uVar3 != 0)) {
        if (uVar3 == 0) {
                    /* WARNING: Subroutine does not return */
          FUN_001d18b0(0,0x3e99999a);
        }
                    /* WARNING: Subroutine does not return */
        FUN_001d18b0(uVar3,0x3e99999a);
      }
      if (uVar10 == 0x1e) {
        *(uint *)(param_1 + 0x44) = *(uint *)(param_1 + 0x44) | 0x200000;
      }
      if (uVar10 == 0x3e) {
        FUN_000ea7ac(&DAT_00026098,200,200,200);
        FUN_000ea884(0,&DAT_00026098,0);
        FUN_000eac6c(&DAT_00026098);
        FUN_000ea884(0x3f800000,&DAT_00026098,1);
        (**(code **)(*(int *)(param_1 + 0x5c) + 0x9c))
                  (param_1 + *(short *)(*(int *)(param_1 + 0x5c) + 0x98));
        in_at = extraout_at_02;
      }
    }
  }
  if (((*(short *)(param_1 + 0x58) != 200) && (*(short *)(param_1 + 0x352) != 0)) &&
     (((uint)*(ushort *)(param_1 + 0x56) + iRam00000e6c & 7) != 0)) {
    *(uint *)(param_1 + 0x44) = *(uint *)(param_1 + 0x44) & 0xffff7fff | 0x8000;
    FUN_002024e8(param_1);
    FUN_0022b788(param_1);
    iVar9 = 0;
    if (*(short *)(param_1 + 0x58) < iRam00000e38) {
      iVar9 = *(int *)(DAT_003493bc + *(short *)(param_1 + 0x58) * 4);
    }
    if (*(short *)(param_1 + 0x352) == 0) {
      if (*(char *)(DAT_00345294 + *(short *)(param_1 + 0x58) * 0x14 + 0xc) == '\x01') {
        if ((*(int *)(param_1 + 0x50) != 0) && (**(char **)(param_1 + 0x50) == '\x01')) {
          if ((*(int *)(param_1 + 0x40) != 0) && ((*(uint *)(param_1 + 0x44) & 1) != 0)) {
                    /* WARNING: Subroutine does not return */
            FUN_00219478(*(undefined4 *)(param_1 + 0x40));
          }
          *(undefined4 *)(param_1 + 0x40) = 0;
          if (*(int *)(param_1 + 0x50) != 0) {
            if (**(char **)(param_1 + 0x50) == '\x01') {
              uVar8 = *(undefined4 *)(*(int *)(param_1 + 0x50) + 4);
              FUN_002c4e78();
              FUN_00259294(uVar8);
            }
            *(undefined4 *)(param_1 + 0x50) = 0;
          }
        }
        if (*(int *)(param_1 + 0x50) == 0) {
          sVar1 = *(short *)(param_1 + 0x58);
          *(undefined2 *)(param_1 + 0x58) = 0xffff;
          FUN_0009aa98(param_1,(int)sVar1);
        }
      }
      else {
        FUN_001c6c28((int)*(short *)(param_1 + 0x58),4);
      }
    }
    else if ((*(int *)(param_1 + 0x50) != 0) && (**(char **)(param_1 + 0x50) == '\x02')) {
      (**(code **)(*(int *)(param_1 + 0x5c) + 0x44))
                (param_1 + *(short *)(*(int *)(param_1 + 0x5c) + 0x40));
      iVar9 = (int)*(short *)(iVar9 + 0x40);
      if (iVar9 != -1) {
        FUN_000e4d6c(0x2d3518,2);
        iVar4 = 0;
        if (iVar9 < iRam00000e38) {
          iVar4 = *(int *)(DAT_003493bc + iVar9 * 4);
        }
        uVar8 = (**(code **)(*(int *)(iVar4 + 0x20) + 0x24))
                          (iVar4 + *(short *)(*(int *)(iVar4 + 0x20) + 0x20));
        *(undefined4 *)(param_1 + 0x50) = uVar8;
        FUN_000e4d74(0x2d3518);
        if (*(int *)(param_1 + 0x50) != 0) {
          if (*(int *)(param_1 + 0x40) == 0) {
            uVar10 = *(uint *)(param_1 + 0x44);
          }
          else {
            if ((*(uint *)(param_1 + 0x44) & 1) != 0) {
                    /* WARNING: Subroutine does not return */
              FUN_00219478(*(undefined4 *)(param_1 + 0x40));
            }
            uVar10 = *(uint *)(param_1 + 0x44);
          }
          *(int *)(param_1 + 0x40) = *(int *)(*(int *)(param_1 + 0x50) + 4) + 0x10;
          *(uint *)(param_1 + 0x44) = uVar10 & 0xfffffffe;
          FUN_002024e8(param_1);
        }
      }
    }
    return;
  }
  local_48 = param_1 + 0x70;
  local_4c = param_1 + 0x80;
  if (*(short *)(param_1 + 0x58) != 200) {
    fVar22 = local_60 - 10.0;
    if (fVar22 < 0.0) {
      fVar22 = fVar22 + local_5c;
    }
    fVar14 = *(float *)(*(short *)(param_1 + 0x354) * 0x14 + 0xc);
    do {
      if (fVar14 <= fVar22) {
        if (fVar22 <= local_5c) {
          fVar14 = fRam0000000c - *(float *)(*(short *)(param_1 + 0x354) * 0x14 + 0xc);
          if (fVar14 < 0.0) {
            fVar14 = fVar14 + local_5c;
            sVar1 = *(short *)(param_1 + 0x354);
          }
          else {
            sVar1 = *(short *)(param_1 + 0x354);
          }
          pfVar5 = (float *)(sVar1 * 0x14);
          fVar14 = (fVar22 - pfVar5[3]) / fVar14;
          fVar22 = fVar22 + 20.0;
          fVar15 = 1.0 - fVar14;
          fVar16 = pfVar5[2];
          fVar23 = fRam00000008 * fVar14;
          fVar20 = fRam00000004 * fVar14 + pfVar5[1] * fVar15;
          fVar14 = fRam00000000 * fVar14 + *pfVar5 * fVar15;
          if (local_5c < fVar22) {
            fVar22 = fVar22 - local_5c;
          }
          uVar10 = (uint)*(short *)(param_1 + 0x354);
          pfVar5 = (float *)(uVar10 * 0x14);
          fVar21 = pfVar5[3];
          while ((fVar22 < fVar21 || (local_5c < fVar22))) {
            uVar10 = (uVar10 + 1) % 0;
            pfVar5 = (float *)(uVar10 * 0x14);
            fVar21 = pfVar5[3];
          }
          fVar21 = fRam0000000c - pfVar5[3];
          if (fVar21 < 0.0) {
            fVar21 = fVar21 + local_5c;
            fVar12 = pfVar5[3];
          }
          else {
            fVar12 = pfVar5[3];
          }
          fVar21 = (fVar22 - fVar12) / fVar21;
          fVar12 = pfVar5[2];
          fVar22 = fVar22 + 30.0;
          fVar17 = 1.0 - fVar21;
          fVar18 = fRam00000008 * fVar21;
          fVar19 = fRam00000004 * fVar21 + pfVar5[1] * fVar17;
          fVar21 = fRam00000000 * fVar21 + *pfVar5 * fVar17;
          if (local_5c < fVar22) {
            fVar22 = fVar22 - local_5c;
          }
          uVar10 = (uint)*(short *)(param_1 + 0x354);
          fVar24 = *(float *)(uVar10 * 0x14 + 0xc);
          while ((fVar22 < fVar24 || (local_5c < fVar22))) {
            uVar10 = (uVar10 + 1) % 0;
            fVar24 = *(float *)(uVar10 * 0x14 + 0xc);
          }
          *(float *)(param_1 + 0x30) = (fVar21 + fVar14) * 0.5;
          *(float *)(param_1 + 0x34) = (fVar19 + fVar20) * 0.5;
          *(float *)(param_1 + 0x38) =
               (fVar18 + fVar12 * fVar17 + fVar23 + fVar16 * fVar15) * 0.5 + 1.0;
          setCopReg(2,in_zero,CONCAT44(fVar19 - fVar20,fVar21 - fVar14));
          uVar2 = getCopReg(2,0);
          setCopReg(2,in_at,uVar2);
                    /* WARNING: Bad instruction - Truncating control flow here */
          halt_baddata();
        }
        sVar1 = *(short *)(param_1 + 0x354);
      }
      else {
        sVar1 = *(short *)(param_1 + 0x354);
      }
      *(short *)(param_1 + 0x354) = (short)(((int)sVar1 + 1U) % 0);
      fVar14 = *(float *)(*(short *)(param_1 + 0x354) * 0x14 + 0xc);
    } while( true );
  }
  fVar22 = *(float *)(DAT_003526c4 + *(short *)(param_1 + 0x350) * 4 + 0x1c) - 30.0;
  if (fVar22 < 0.0) {
    fVar22 = fVar22 + *(float *)(DAT_003526c4 + 0xc);
  }
  fVar23 = fVar22 + 60.0;
  iVar9 = *(short *)(param_1 + 0x354) + 1;
  fVar14 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + *(short *)(param_1 + 0x354) * 0x14 + 0xc);
  if (iVar9 < *(int *)(DAT_003526c4 + 0x40)) {
    fVar15 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 * 0x14 + 0xc);
  }
  else {
    iVar9 = 0;
    fVar15 = *(float *)(DAT_003526c4 + 0xc);
  }
  do {
    if (fVar14 <= fVar22) {
      if (fVar22 <= fVar15) {
        local_50 = &DAT_000260c8;
        if ((*(float *)(DAT_003526c4 + 0x14) <=
             *(float *)(DAT_003526c4 + *(short *)(param_1 + 0x350) * 4 + 0x1c)) &&
           (*(float *)(DAT_003526c4 + *(short *)(param_1 + 0x350) * 4 + 0x28) <
            *(float *)(DAT_003526c4 + 0x14))) {
          FUN_0006076c(0,0xe65,*(undefined4 *)(param_1 + 0x60),0x1e);
          in_at = extraout_at_03;
        }
        fVar14 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 * 0x14 + 0xc) -
                 *(float *)(*(int *)(DAT_003526c4 + 0x44) + *(short *)(param_1 + 0x354) * 0x14 + 0xc
                           );
        if (fVar14 < 0.0) {
          fVar14 = fVar14 + *(float *)(DAT_003526c4 + 0xc);
        }
        pfVar5 = (float *)(*(int *)(DAT_003526c4 + 0x44) + *(short *)(param_1 + 0x354) * 0x14);
        fVar14 = (fVar22 - pfVar5[3]) / fVar14;
        pfVar6 = (float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 * 0x14);
        fVar20 = pfVar6[2];
        fVar15 = pfVar5[2];
        fVar21 = 1.0 - fVar14;
        fVar22 = *pfVar6 * fVar14 + *pfVar5 * fVar21;
        fVar16 = pfVar6[1] * fVar14 + pfVar5[1] * fVar21;
        if (*(float *)(DAT_003526c4 + 0xc) < fVar23) {
          fVar23 = fVar23 - *(float *)(DAT_003526c4 + 0xc);
        }
        fVar12 = fVar23 + 60.0;
        iVar9 = *(short *)(param_1 + 0x354) * 0x14;
        fVar17 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 + 0xc);
        iVar4 = *(short *)(param_1 + 0x354) + 1;
        if (iVar4 < *(int *)(DAT_003526c4 + 0x40)) {
          fVar18 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar4 * 0x14 + 0xc);
          iVar7 = iVar4;
        }
        else {
          fVar18 = *(float *)(DAT_003526c4 + 0xc);
          iVar7 = 0;
        }
        while ((fVar23 < fVar17 || (fVar18 < fVar23))) {
          iVar4 = iVar4 % *(int *)(DAT_003526c4 + 0x40);
          iVar9 = iVar4 * 0x14;
          iVar4 = iVar4 + 1;
          fVar17 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 + 0xc);
          if (iVar4 < *(int *)(DAT_003526c4 + 0x40)) {
            fVar18 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar4 * 0x14 + 0xc);
            iVar7 = iVar4;
          }
          else {
            fVar18 = *(float *)(DAT_003526c4 + 0xc);
            iVar7 = 0;
          }
        }
        fVar17 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar7 * 0x14 + 0xc) -
                 *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 + 0xc);
        if (fVar17 < 0.0) {
          fVar17 = fVar17 + *(float *)(DAT_003526c4 + 0xc);
        }
        pfVar5 = (float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9);
        fVar17 = (fVar23 - pfVar5[3]) / fVar17;
        pfVar6 = (float *)(*(int *)(DAT_003526c4 + 0x44) + iVar7 * 0x14);
        fVar23 = pfVar6[2];
        fVar19 = pfVar5[2];
        fVar18 = 1.0 - fVar17;
        fVar25 = *pfVar6 * fVar17 + *pfVar5 * fVar18;
        fVar24 = pfVar6[1] * fVar17 + pfVar5[1] * fVar18;
        if (*(float *)(DAT_003526c4 + 0xc) < fVar12) {
          fVar12 = fVar12 - *(float *)(DAT_003526c4 + 0xc);
        }
        iVar9 = *(short *)(param_1 + 0x354) + 1;
        fVar13 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + *(short *)(param_1 + 0x354) * 0x14 + 0xc
                           );
        if (iVar9 < *(int *)(DAT_003526c4 + 0x40)) {
          fVar11 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 * 0x14 + 0xc);
        }
        else {
          fVar11 = *(float *)(DAT_003526c4 + 0xc);
        }
        while ((fVar12 < fVar13 || (fVar11 < fVar12))) {
          iVar4 = iVar9 % *(int *)(DAT_003526c4 + 0x40);
          iVar9 = iVar4 + 1;
          fVar13 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar4 * 0x14 + 0xc);
          if (iVar9 < *(int *)(DAT_003526c4 + 0x40)) {
            fVar11 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 * 0x14 + 0xc);
          }
          else {
            fVar11 = *(float *)(DAT_003526c4 + 0xc);
          }
        }
        *(float *)(param_1 + 0x30) = (fVar25 + fVar22) * 0.5;
        *(float *)(param_1 + 0x34) = (fVar24 + fVar16) * 0.5;
        *(float *)(param_1 + 0x38) =
             (fVar23 * fVar17 + fVar19 * fVar18 + fVar20 * fVar14 + fVar15 * fVar21) * 0.5 + 4.3;
        setCopReg(2,in_zero,CONCAT44(fVar24 - fVar16,fVar25 - fVar22));
        uVar2 = getCopReg(2,0);
        setCopReg(2,in_at,uVar2);
                    /* WARNING: Bad instruction - Truncating control flow here */
        halt_baddata();
      }
      sVar1 = *(short *)(param_1 + 0x354);
    }
    else {
      sVar1 = *(short *)(param_1 + 0x354);
    }
    *(short *)(param_1 + 0x354) = (short)((sVar1 + 1) % *(int *)(DAT_003526c4 + 0x40));
    iVar9 = *(short *)(param_1 + 0x354) + 1;
    fVar14 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + *(short *)(param_1 + 0x354) * 0x14 + 0xc);
    if (iVar9 < *(int *)(DAT_003526c4 + 0x40)) {
      fVar15 = *(float *)(*(int *)(DAT_003526c4 + 0x44) + iVar9 * 0x14 + 0xc);
    }
    else {
      iVar9 = 0;
      fVar15 = *(float *)(DAT_003526c4 + 0xc);
    }
  } while( true );
}

