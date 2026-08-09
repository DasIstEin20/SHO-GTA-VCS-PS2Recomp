/* REVIEW-ONLY: FUN_003b1e98 @ 0x003b1e98 */


undefined8 FUN_003b1e98(undefined8 param_1)

{
  float *pfVar1;
  int *piVar2;
  int iVar3;
  
  iVar3 = (int)param_1 + 0x10;
  pfVar1 = (float *)FUN_002552f0(param_1,iVar3,0);
  piVar2 = (int *)FUN_002552f0(param_1,iVar3,0);
  *pfVar1 = (float)*piVar2;
  return 0;
}

