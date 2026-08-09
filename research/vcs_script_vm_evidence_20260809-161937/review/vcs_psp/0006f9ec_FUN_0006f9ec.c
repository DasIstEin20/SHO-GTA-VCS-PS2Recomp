/* REVIEW-ONLY: FUN_0006f9ec @ 0x0006f9ec */


void FUN_0006f9ec(int param_1,char param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int local_30 [3];
  undefined1 auStack_24 [4];
  int local_20 [3];
  undefined1 auStack_14 [4];
  
  FUN_0006f9a0();
  FUN_0006fd30(param_1,0xf608,0xf610);
  uRam0000f6a9 = 0;
  if (param_2 != '\0') {
    FUN_00202684(0x17f58,0xc6,0);
    if (iRam00017f08 == 0) {
      iRam00017f08 = 1;
      FUN_00029794(0x1f390);
      FUN_0035ca60(0xf6ac);
    }
    FUN_0002a670(0x1f390);
    if (iRam000177ec == 0) {
      FUN_0036216c();
    }
    iVar1 = FUN_00251268(iRam000177ec);
    if (iVar1 == 0) {
      if (*(char *)(param_1 + 0x18c) == '\0') {
        if (iRam00017f08 == 0) {
          iRam00017f08 = 1;
          FUN_00029794(0x1f390);
          FUN_0035ca60(0xf6ac);
        }
        iVar1 = FUN_0002a9cc(0x1f390);
        if (iVar1 != 0) {
          if (iRam00017f08 == 0) {
            iRam00017f08 = 1;
            FUN_00029794(0x1f390);
            FUN_0035ca60(0xf6ac);
          }
          FUN_0002bca0(0x1f390,3,0);
          goto LAB_0006fc08;
        }
      }
      if (iRam00017f08 == 0) {
        iRam00017f08 = 1;
        FUN_00029794(0x1f390);
        FUN_0035ca60(0xf6ac);
      }
      FUN_0002a980(0x1f390);
      if (iRam00017f08 == 0) {
        iRam00017f08 = 1;
        FUN_00029794(0x1f390);
        FUN_0035ca60(0xf6ac);
      }
      uVar2 = FUN_0002a950(0x1f390);
      uVar3 = FUN_00362ed0(local_30,0xf6d0,auStack_24);
      FUN_002dea14(uVar2,uVar3);
      if (local_30[0] != 0) {
        FUN_002ba72c(local_30[0]);
      }
    }
    else {
      if (iRam00017f08 == 0) {
        iRam00017f08 = 1;
        FUN_00029794(0x1f390);
        FUN_0035ca60(0xf6ac);
      }
      FUN_0002a980(0x1f390);
      if (iRam00017f08 == 0) {
        iRam00017f08 = 1;
        FUN_00029794(0x1f390);
        FUN_0035ca60(0xf6ac);
      }
      uVar2 = FUN_0002a950(0x1f390);
      uVar3 = FUN_00362ed0(local_20,0xf6d0,auStack_14);
      FUN_002dea14(uVar2,uVar3);
      if (local_20[0] != 0) {
        FUN_002ba72c(local_20[0]);
      }
    }
  }
LAB_0006fc08:
  *(undefined4 *)(param_1 + 0xcc) = 0;
  if (*(char *)(param_1 + 0x1a8) != '\0') {
    FUN_003636f0(uRam00012278);
  }
  *(undefined1 *)(param_1 + 0x1a8) = 0;
  return;
}

