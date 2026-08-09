/* REVIEW-ONLY: FUN_002b7fe8 @ 0x002b7fe8 */


undefined4 FUN_002b7fe8(int param_1)

{
  float *pfVar1;
  int *piVar2;
  
  pfVar1 = (float *)FUN_0005e3d0(param_1,param_1 + 0x10,0);
  piVar2 = (int *)FUN_0005e3d0(param_1,param_1 + 0x10,0);
  *pfVar1 = (float)*piVar2;
  return 0;
}

