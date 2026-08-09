/* REVIEW-ONLY: FUN_003b1e30 @ 0x003b1e30 */


undefined8 FUN_003b1e30(undefined8 param_1)

{
  int *piVar1;
  float *pfVar2;
  int iVar3;
  
  iVar3 = (int)param_1 + 0x10;
  piVar1 = (int *)FUN_002552f0(param_1,iVar3,0);
  pfVar2 = (float *)FUN_002552f0(param_1,iVar3,0);
  *piVar1 = (int)*pfVar2;
  return 0;
}

