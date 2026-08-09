/* REVIEW-ONLY: FUN_003b1d20 @ 0x003b1d20 */


undefined8 FUN_003b1d20(undefined8 param_1)

{
  float *pfVar1;
  int iVar2;
  
  iVar2 = (int)param_1 + 0x10;
  pfVar1 = (float *)FUN_002552f0(param_1,iVar2,0);
  FUN_00254d18(param_1,iVar2,1,0x50e1f8);
  *pfVar1 = *pfVar1 + DAT_0050e1f8 * DAT_004cd410;
  return 0;
}

