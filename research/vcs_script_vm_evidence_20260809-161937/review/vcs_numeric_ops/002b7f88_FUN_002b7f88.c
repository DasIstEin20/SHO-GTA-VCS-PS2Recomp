/* REVIEW-ONLY: FUN_002b7f88 @ 0x002b7f88 */


undefined4 FUN_002b7f88(int param_1)

{
  int *piVar1;
  float *pfVar2;
  
  piVar1 = (int *)FUN_0005e3d0(param_1,param_1 + 0x10,0);
  pfVar2 = (float *)FUN_0005e3d0(param_1,param_1 + 0x10,0);
  *piVar1 = (int)*pfVar2;
  return 0;
}

