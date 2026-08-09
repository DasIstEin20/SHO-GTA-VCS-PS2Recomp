/* REVIEW-ONLY: FUN_0036038c @ 0x0036038c */


void FUN_0036038c(int param_1,int param_2,short *param_3)

{
  int iVar1;
  short *psVar2;
  uint uVar3;
  int iVar4;
  short *psVar5;
  int iVar6;
  int iVar7;
  
  param_1 = param_1 * 0x4c;
  iVar6 = 0;
  psVar5 = param_3;
  do {
    uVar3 = *(uint *)(param_1 + 0x1b3e8);
    if ((uVar3 & 0x100000) == 0) {
      *(undefined4 *)(param_1 + 0x1b408) = 0x7f;
    }
    else {
      iVar1 = *(int *)(param_1 + 0x1b40c);
      if (iVar1 == 0) {
        trap(7);
      }
      iVar7 = ((iVar1 - *(int *)(param_1 + 0x1b410)) * 0x7f) / iVar1;
      *(int *)(param_1 + 0x1b408) = iVar7;
      if (iVar7 < 0) {
        *(undefined4 *)(param_1 + 0x1b408) = 0;
      }
      iVar7 = *(int *)(param_1 + 0x1b410) + 1;
      *(int *)(param_1 + 0x1b410) = iVar7;
      if (iVar1 <= iVar7) {
        *(uint *)(param_1 + 0x1b3e8) = uVar3 & 0xffefffff;
        *(undefined4 *)(param_1 + 0x1b410) = 0;
        if ((uVar3 & 0x10000) == 0) {
          if ((uVar3 & 0x20000) == 0) {
            if ((uVar3 & 0x40000) != 0) {
              *(uint *)(param_1 + 0x1b3e8) = uVar3 & 0xffebffff;
              *(undefined4 *)(param_1 + 0x1b3ec) = *(undefined4 *)(param_1 + 0x1b64c);
            }
          }
          else {
            *(uint *)(param_1 + 0x1b3e8) = uVar3 & 0xffedffff;
            *(undefined4 *)(param_1 + 0x1b3ec) = *(undefined4 *)(param_1 + 0x1b64c);
            *(undefined4 *)(param_1 + 0x1b3f0) = *(undefined4 *)(param_1 + 0x1b650);
            *(undefined4 *)(param_1 + 0x1b400) = *(undefined4 *)(param_1 + 0x1b660);
            *(undefined4 *)(param_1 + 0x1b404) = *(undefined4 *)(param_1 + 0x1b664);
            *(undefined4 *)(param_1 + 0x1b3f8) = *(undefined4 *)(param_1 + 0x1b658);
            *(undefined4 *)(param_1 + 0x1b3fc) = *(undefined4 *)(param_1 + 0x1b65c);
          }
        }
        else {
          *(uint *)(param_1 + 0x1b3e8) = uVar3 & 0xffeeffff;
          *(undefined4 *)(param_1 + 0x1b3ec) = *(undefined4 *)(param_1 + 0x1b64c);
          *(undefined4 *)(param_1 + 0x1b3f0) = *(undefined4 *)(param_1 + 0x1b650);
          *(undefined4 *)(param_1 + 0x1b3d8) = *(undefined4 *)(param_1 + 0x1b638);
          *(undefined4 *)(param_1 + 0x1b3dc) = *(undefined4 *)(param_1 + 0x1b63c);
          *(undefined4 *)(param_1 + 0x1b400) = *(undefined4 *)(param_1 + 0x1b660);
          *(undefined4 *)(param_1 + 0x1b404) = *(undefined4 *)(param_1 + 0x1b664);
          *(undefined4 *)(param_1 + 0x1b3f8) = *(undefined4 *)(param_1 + 0x1b658);
          *(undefined4 *)(param_1 + 0x1b3fc) = *(undefined4 *)(param_1 + 0x1b65c);
        }
      }
    }
    if ((*(uint *)(param_1 + 0x1b3e8) & 0x100) == 0) {
      iVar1 = *(int *)(param_1 + 0x1b408);
LAB_003604b4:
      iVar7 = ((*(int *)(param_1 + 0x1b3dc) * iVar1) / 0x7f) * (int)*psVar5;
      iVar1 = ((*(int *)(param_1 + 0x1b3d8) * iVar1) / 0x7f) * (int)*psVar5;
      iVar7 = iVar7 + ((uint)(iVar7 >> 0x1f) >> 0x14);
      iVar1 = iVar1 + ((uint)(iVar1 >> 0x1f) >> 0x14);
    }
    else {
      if ((*(uint *)(param_1 + 0x1b3e8) & 0x200) == 0) {
        iVar1 = *(int *)(param_1 + 0x1b408);
        goto LAB_003604b4;
      }
      iVar1 = ((*(int *)(param_1 + 0x1b3d8) * *(int *)(param_1 + 0x1b408)) / 0x7f) *
              (int)param_3[iVar6 * 2];
      iVar7 = ((*(int *)(param_1 + 0x1b3dc) * *(int *)(param_1 + 0x1b408)) / 0x7f) *
              (int)(param_3 + iVar6 * 2)[1];
      iVar1 = iVar1 + ((uint)(iVar1 >> 0x1f) >> 0x14);
      iVar7 = iVar7 + ((uint)(iVar7 >> 0x1f) >> 0x14);
    }
    psVar2 = (short *)(iVar6 * 4 + param_2);
    iVar4 = (int)*psVar2 + (iVar1 >> 0xc) + 0x7fff;
    iVar1 = (int)psVar2[1] + (iVar7 >> 0xc) + 0x7fff;
    iVar6 = iVar6 + 1;
    if (iVar4 < -0x8000) {
      iVar4 = -0x8000;
    }
    if (iVar1 < -0x8000) {
      iVar1 = -0x8000;
    }
    *psVar2 = (short)iVar4;
    psVar5 = psVar5 + 1;
    psVar2[1] = (short)iVar1;
    if (0x1b < iVar6) {
      return;
    }
  } while( true );
}

