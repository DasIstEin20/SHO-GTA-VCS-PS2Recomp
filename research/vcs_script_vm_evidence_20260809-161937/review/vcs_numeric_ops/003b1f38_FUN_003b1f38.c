/* REVIEW-ONLY: FUN_003b1f38 @ 0x003b1f38 */


undefined8 FUN_003b1f38(int param_1)

{
  float *pfVar1;
  
  pfVar1 = (float *)FUN_002552f0(param_1,param_1 + 0x10,0);
  *pfVar1 = ABS(*pfVar1);
  return 0;
}

