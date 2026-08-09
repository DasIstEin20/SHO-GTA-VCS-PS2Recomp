/* REVIEW-ONLY: FUN_0017b168 @ 0x0017b168 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0017b168(void)

{
  int iVar1;
  undefined4 uVar2;
  bool bVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined1 auStack_30 [16];
  
  if (DAT_ffff3e91 != '\0') {
    if (_DAT_ffff9e29 == 0) {
      _DAT_ffff9e29 = 1;
      FUN_00029794(0x1f390);
      FUN_0035ca60(0xffff3e99);
    }
    iVar1 = FUN_0002a9b4(0x1f390);
    if (iVar1 == 0) {
      DAT_ffff5dd1 = 0;
      FUN_00187780();
      iVar1 = FUN_00187760(0);
      bVar3 = false;
      if ((*(short *)(iVar1 + 0x22) != 0) && (bVar3 = false, *(short *)(iVar1 + 0x54) == 0)) {
        bVar3 = true;
      }
      if (bVar3) {
        FUN_0017baac();
      }
      else {
        FUN_0009a7c4();
        iVar1 = FUN_00187760(0);
        if (*(short *)(iVar1 + 0x2a) == 0) {
          _DAT_ffff3e95 = _DAT_ffff3e95 + 1;
        }
        else {
          _DAT_ffff3e95 = _DAT_ffff3e95 + 0x14;
        }
        FUN_00308878(1);
        FUN_0030843c();
        FUN_003089e4(2);
        uVar2 = FUN_0006f400(0x42200000,0x43dc0000,auStack_30,0x43880000);
        FUN_003088b4(uVar2);
        iVar6 = 0;
        iVar1 = 0;
        while( true ) {
          if (_DAT_ffff59e9 == 0) {
            iVar4 = FUN_002ba60c(0x994);
            iVar5 = 0;
            if (iVar4 != 0) {
              FUN_001f25a0(iVar4);
              iVar5 = iVar4;
            }
            iVar4 = *(int *)(iVar5 + 0x14);
            _DAT_ffff59e9 = iVar5;
          }
          else {
            iVar4 = *(int *)(_DAT_ffff59e9 + 0x14);
          }
          if (iVar4 <= iVar6) break;
          FUN_0017b3d8(0x3f19999a,((undefined4 *)(_DAT_ffffa431 + iVar1))[1],
                       *(undefined4 *)(_DAT_ffffa431 + iVar1));
          iVar6 = iVar6 + 1;
          iVar1 = iVar1 + 8;
        }
        if (DAT_0001e8c4 != '\0') {
          FUN_002182c8(0x1e0c0);
        }
        if (_DAT_ffff59e9 == 0) {
          iVar6 = FUN_002ba60c(0x994);
          iVar1 = 0;
          if (iVar6 != 0) {
            FUN_001f25a0(iVar6);
            iVar1 = iVar6;
          }
          iVar6 = *(int *)(iVar1 + 0x14);
          _DAT_ffff59e9 = iVar1;
        }
        else {
          iVar6 = *(int *)(_DAT_ffff59e9 + 0x14);
        }
        if ((_DAT_ffff5ddd + *(int *)(_DAT_ffffa431 + (iVar6 + -1) * 8)) - _DAT_ffff3e95 < -0x100) {
          FUN_0017baac();
        }
      }
    }
  }
  return;
}

