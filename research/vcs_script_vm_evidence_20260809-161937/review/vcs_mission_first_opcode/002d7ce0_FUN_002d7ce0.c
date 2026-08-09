/* REVIEW-ONLY: FUN_002d7ce0 @ 0x002d7ce0 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_002d7ce0(undefined8 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined8 uVar3;
  int iVar4;
  int iVar5;
  float fVar6;
  undefined4 uVar7;
  undefined4 uStack_60;
  undefined4 uStack_5c;
  float fStack_58;
  
  iVar5 = (int)param_1;
  FUN_00254d18(param_1,iVar5 + 0x10,5,0x50e1f8);
  uVar2 = DAT_0050e204;
  iVar4 = DAT_0050e1f8;
  if (DAT_0050e1f8 < 0) {
    iVar4 = (&DAT_0072aca8)[DAT_0050e1f8 * -7];
  }
  if ((_DAT_0050e208 == -100.0) || (_DAT_0050e208 <= -250.0)) {
    uVar7 = DAT_0050e200;
    fVar6 = (float)FUN_00196758(DAT_0050e200,DAT_0050e204);
    uVar1 = *(undefined4 *)(iVar5 + 0x10);
    uStack_60 = uVar7;
    fStack_58 = fVar6 + 0.5;
  }
  else {
    uVar1 = *(undefined4 *)(iVar5 + 0x10);
    uStack_60 = DAT_0050e200;
    fStack_58 = _DAT_0050e208;
  }
  uVar3 = FUN_00254f50(param_1,uVar1);
  FUN_001e2cc0(uVar3);
  uStack_5c = uVar2;
  DAT_0050e1f8 = FUN_001ddb18(&uStack_60,iVar4,(undefined1)DAT_0050e1fc,0,0,0,0);
  FUN_002550f8(param_1,iVar5 + 0x10,1);
  return 0;
}

