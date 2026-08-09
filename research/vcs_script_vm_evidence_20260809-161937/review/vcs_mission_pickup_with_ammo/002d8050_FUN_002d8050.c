/* REVIEW-ONLY: FUN_002d8050 @ 0x002d8050 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_002d8050(undefined8 param_1)

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
  FUN_00254d18(param_1,iVar5 + 0x10,6,0x50e1f8);
  uVar2 = DAT_0050e204;
  iVar4 = DAT_0050e1f8;
  if (DAT_0050e1f8 < 0) {
    iVar4 = (&DAT_0072aca8)[DAT_0050e1f8 * -7];
  }
  if ((_DAT_0050e20c == -100.0) || (_DAT_0050e20c <= -250.0)) {
    uVar7 = _DAT_0050e208;
    fVar6 = (float)FUN_00196758(DAT_0050e204,_DAT_0050e208);
    uVar1 = *(undefined4 *)(iVar5 + 0x10);
    uStack_5c = uVar7;
    fStack_58 = fVar6 + 0.5;
  }
  else {
    uVar1 = *(undefined4 *)(iVar5 + 0x10);
    uStack_5c = _DAT_0050e208;
    fStack_58 = _DAT_0050e20c;
  }
  uVar3 = FUN_00254f50(param_1,uVar1);
  FUN_001e2cc0(uVar3);
  uStack_60 = uVar2;
  DAT_0050e1f8 = FUN_001ddb18(&uStack_60,iVar4,(undefined1)DAT_0050e1fc,DAT_0050e200,0,0,0);
  FUN_002550f8(param_1,iVar5 + 0x10,1);
  return 0;
}

