/* REVIEW-ONLY: FUN_002b8048 @ 0x002b8048 */


undefined4 FUN_002b8048(int param_1)

{
  int *piVar1;
  int iVar2;
  
  piVar1 = (int *)FUN_0005e3d0(param_1,param_1 + 0x10,0);
  iVar2 = *piVar1;
  if (iVar2 < 0) {
    iVar2 = -iVar2;
  }
  *piVar1 = iVar2;
  return 0;
}

