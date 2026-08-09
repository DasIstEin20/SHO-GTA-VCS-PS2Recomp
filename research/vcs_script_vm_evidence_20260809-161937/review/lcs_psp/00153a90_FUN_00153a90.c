/* REVIEW-ONLY: FUN_00153a90 @ 0x00153a90 */


undefined4 FUN_00153a90(int param_1)

{
  short sVar1;
  undefined4 uVar2;
  uint uVar3;
  
  sVar1 = *(short *)(DAT_00334d7c + *(int *)(param_1 + 0x18));
  *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + 2;
  uVar3 = (uint)sVar1;
  if ((uVar3 & 0x8000) == 0) {
    *(undefined1 *)(param_1 + 0x216) = 0;
  }
  else {
    *(undefined1 *)(param_1 + 0x216) = 1;
  }
  uVar3 = uVar3 & 0x7fff;
  uVar2 = 0xffffffff;
  if (uVar3 < 100) {
    uVar2 = FUN_00154d50();
  }
  else if (uVar3 < 200) {
    uVar2 = FUN_00158784();
  }
  else if (uVar3 < 0x131) {
    uVar2 = FUN_0015b6d4();
  }
  else if (uVar3 < 0x195) {
    uVar2 = FUN_001d860c();
  }
  else if (uVar3 < 0x1f9) {
    uVar2 = FUN_001daca0();
  }
  else if (uVar3 < 0x25d) {
    uVar2 = FUN_00219ff0();
  }
  else if (uVar3 < 0x2c1) {
    uVar2 = FUN_0021c518();
  }
  else if (uVar3 < 0x325) {
    uVar2 = FUN_0021de94();
  }
  else if (uVar3 < 0x389) {
    uVar2 = FUN_001ddbd4();
  }
  else if (uVar3 < 0x3ed) {
    uVar2 = FUN_001e2000();
  }
  else if (uVar3 < 0x451) {
    uVar2 = FUN_000e023c();
  }
  else if (uVar3 < 0x4b5) {
    uVar2 = FUN_000e229c();
  }
  else if (uVar3 < 0x519) {
    uVar2 = FUN_0018db34();
  }
  else if (uVar3 < 0x57d) {
    uVar2 = FUN_00190874();
  }
  else if (uVar3 < 0x5d9) {
    uVar2 = FUN_0022d364();
  }
  else if (uVar3 < 0x640) {
    uVar2 = FUN_0017888c();
  }
  else if (uVar3 < 0x6a4) {
    uVar2 = FUN_0005eb04();
  }
  return uVar2;
}

