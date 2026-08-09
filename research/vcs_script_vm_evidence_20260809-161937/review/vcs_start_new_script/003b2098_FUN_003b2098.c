/* REVIEW-ONLY: FUN_003b2098 @ 0x003b2098 */


undefined8 FUN_003b2098(undefined8 param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = (int)param_1 + 0x10;
  FUN_00254d18(param_1,iVar2,1,0x50e1f8);
  iVar1 = DAT_0050e1f8;
  if (DAT_0050e1f8 < 0) {
    iVar1 = 0;
  }
  iVar1 = FUN_00255ad8(iVar1);
  FUN_00254d18(param_1,iVar2,0x60,iVar1 + 0x54);
  return 0;
}

