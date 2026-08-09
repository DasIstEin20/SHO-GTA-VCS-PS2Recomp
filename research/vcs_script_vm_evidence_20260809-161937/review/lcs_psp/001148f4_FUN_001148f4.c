/* REVIEW-ONLY: FUN_001148f4 @ 0x001148f4 */


void FUN_001148f4(void)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  float fVar6;
  undefined1 auStack_50 [4];
  undefined1 auStack_4c [4];
  undefined1 auStack_48 [16];
  undefined1 auStack_38 [4];
  undefined1 auStack_34 [4];
  
  if (uRam0000126c < 2) {
    if (uRam0000126c != 0) {
      FUN_00250eb8();
      FUN_00250fd0();
      FUN_00250fac((float)(DAT_00352acc + -0x1e));
      FUN_00250ecc();
      FUN_00251030();
      uVar5 = iRam00000e3c - iRam00001294;
      if (cRam000012a8 == '\0') {
        if (uVar5 < 8000) {
          FUN_00250df8(0x3fa66666);
          FUN_00250eb8();
          FUN_00114590(uVar5,0,8000);
          uVar1 = FUN_0005e9cc(auStack_4c,0xff,0xff,0x80);
          FUN_00250e30(uVar1);
          FUN_00251054(0);
          if (iRam000012a4 != 0) {
            FUN_00252ad4((float)(DAT_00352acc / 2),iRam000012a4,0,DAT_00352ad0 >> 1);
          }
        }
      }
      else if ((2999 < uVar5) && (uVar5 < 11000)) {
        FUN_00250df8(0x3fa66666);
        FUN_00250eb8();
        FUN_00114590(uVar5,3000,11000);
        uVar1 = FUN_0005e9cc(auStack_50,0xff,0xff,0x80);
        FUN_00250e30(uVar1);
        FUN_00251054(0);
        if (iRam000012a4 != 0) {
          FUN_00252ad4((float)(DAT_00352acc / 2),iRam000012a4,0,DAT_00352ad0 >> 1);
        }
      }
      if ((-1 < iRam0000128c) &&
         ((uVar5 = iRam0000128c - (iRam00000e3c - iRam00001294), 4000 < uVar5 ||
          ((uRam00000e6c & 1) != 0)))) {
        uVar1 = FUN_0005e9cc(auStack_48,0xc2,0xa5,0x78);
        FUN_00113868(uVar5,uVar1);
      }
      if ((DAT_00352c7c == '\0') && (iRam00001284 != -1)) {
        iVar4 = iRam00001288 - iRam00001284;
        if (iVar4 < 0) {
          iVar4 = 0;
        }
        iVar2 = 0;
        if (-1 < iRam00001288) {
          iVar2 = iRam00001288;
        }
        FUN_002e7698(0x351d0,"%d/%d",iVar4,iVar2);
        FUN_002501b8(0x351d0,0x2718e8);
        FUN_00183034();
        uVar1 = FUN_0005e9cc(auStack_38,200,200,200);
        FUN_00250e30(uVar1);
        FUN_00251044();
        fVar6 = (float)FUN_00250a58(0x2718e8,0);
        FUN_00252ad4(0x43ea0000,0x2718e8,0);
        uVar1 = FUN_0005e9cc(auStack_38,200,200,200);
        FUN_00250e30(uVar1);
        FUN_00251030();
        iVar4 = DAT_0033207c;
        if (DAT_0033207c == 0) {
          iVar2 = FUN_0029f0e0(0x994);
          iVar4 = 0;
          if (iVar2 != 0) {
            FUN_0010f920(iVar2);
            iVar4 = iVar2;
          }
        }
        DAT_0033207c = iVar4;
        uVar1 = FUN_0010fad4(DAT_0033207c,"KILLS");
        FUN_00252ad4(468.0 - (fVar6 + 4.0),uVar1,0);
      }
    }
  }
  else if (((uRam0000126c < 3) && (cRam000012a8 != '\0')) &&
          (uVar5 = iRam00000e3c - iRam00001294, uVar5 < 5000)) {
    FUN_00250fd0();
    FUN_00250fac((float)(DAT_00352acc + -0x14));
    FUN_00250ecc();
    iVar4 = DAT_0033207c;
    if (DAT_0033207c == 0) {
      iVar2 = FUN_0029f0e0(0x994);
      iVar4 = 0;
      if (iVar2 != 0) {
        FUN_0010f920(iVar2);
        iVar4 = iVar2;
      }
    }
    DAT_0033207c = iVar4;
    uVar1 = FUN_0010fad4(DAT_0033207c,&DAT_00310500);
    FUN_00250df8(0x3fc00000);
    FUN_00250eb8();
    FUN_00114590(uVar5,0,5000);
    uVar3 = FUN_0005e9cc(auStack_34,0x80,0xff,0x80);
    FUN_00250e30(uVar3);
    FUN_00251054(0);
    fVar6 = (float)(int)uVar5;
    if ((int)uVar5 < 0) {
      fVar6 = fVar6 + 4.2949673e+09;
    }
    FUN_00252ad4((float)(DAT_00352acc / 2),fVar6 * 0.01,uVar1,0);
  }
  return;
}

