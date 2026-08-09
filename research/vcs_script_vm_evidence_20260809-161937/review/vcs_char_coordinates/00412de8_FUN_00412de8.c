/* REVIEW-ONLY: FUN_00412de8 @ 0x00412de8 */


undefined8 FUN_00412de8(undefined8 param_1)

{
  undefined4 uVar1;
  undefined8 uVar2;
  float fVar3;
  undefined4 uVar4;
  undefined4 uStack_50;
  undefined4 uStack_4c;
  float fStack_48;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,4,0x50e1f8);
  uVar2 = FUN_00471ac8(DAT_00487d94,DAT_0050e1f8);
  fVar3 = DAT_0050e204;
  uVar1 = DAT_0050e1fc;
  uStack_50 = DAT_0050e1fc;
  uStack_4c = DAT_0050e200;
  fStack_48 = DAT_0050e204;
  uVar4 = DAT_0050e200;
  FUN_0010d138(&uStack_50,0);
  if ((fVar3 == -100.0) || (fVar3 <= -250.0)) {
    fVar3 = (float)FUN_00196758(uVar1,uVar4,fVar3);
  }
  FUN_004129b8(uVar1,uVar4,fVar3,param_1,uVar2,(int)param_1 + 0x20f,0);
  return 0;
}

