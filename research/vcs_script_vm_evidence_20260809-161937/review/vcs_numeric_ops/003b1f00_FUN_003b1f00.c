/* REVIEW-ONLY: FUN_003b1f00 @ 0x003b1f00 */


undefined8 FUN_003b1f00(undefined8 param_1)

{
  int *piVar1;
  int iVar2;
  
  piVar1 = (int *)FUN_002552f0(param_1,(int)param_1 + 0x10,0);
  iVar2 = *piVar1;
  if (iVar2 < 0) {
    iVar2 = -iVar2;
  }
  *piVar1 = iVar2;
  return 0;
}

