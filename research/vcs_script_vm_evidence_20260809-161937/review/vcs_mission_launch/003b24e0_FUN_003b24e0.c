/* REVIEW-ONLY: FUN_003b24e0 @ 0x003b24e0 */


undefined8 FUN_003b24e0(undefined8 param_1)

{
  undefined4 uVar1;
  undefined8 uVar2;
  int iVar3;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,1,0x50e1f8);
  if (0 < DAT_004cd6ec) {
    if (DAT_0050e1f8 < 0xfffe) {
      return 0;
    }
    DAT_0050e1f8 = 0xffff - DAT_0050e1f8;
  }
  FUN_002f4e88();
  uVar1 = DAT_00487718;
  iVar3 = *(int *)(&DAT_007407d0 + (DAT_0050e1f8 + 1) * 4) -
          *(int *)(&DAT_007407d0 + DAT_0050e1f8 * 4);
  if (iVar3 < 1) {
    iVar3 = DAT_004cd6e8;
  }
  FUN_001791b8(DAT_00487718,*(int *)(&DAT_007407d0 + DAT_0050e1f8 * 4) + 8,0);
  FUN_00179188(uVar1,DAT_0048771c + DAT_004cd6d8,iVar3);
  uVar2 = FUN_00255ad8(DAT_004cd6d8);
  FUN_002f4ea0();
  *(undefined1 *)((int)uVar2 + 0x20a) = 1;
  *(undefined1 *)((int)uVar2 + 0x217) = 1;
  DAT_004cd6e7 = 1;
  FUN_0045d318(DAT_0048771c + DAT_004cd704 * 4 + 8,0,(int)DAT_004cd706 << 2);
  FUN_00256130(uVar2);
  return 0;
}

