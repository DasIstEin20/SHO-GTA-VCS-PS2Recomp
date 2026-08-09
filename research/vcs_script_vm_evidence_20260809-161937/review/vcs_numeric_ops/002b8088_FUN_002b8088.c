/* REVIEW-ONLY: FUN_002b8088 @ 0x002b8088 */


undefined4 FUN_002b8088(int param_1)

{
  float *pfVar1;
  float fVar2;
  
  pfVar1 = (float *)FUN_0005e3d0(param_1,param_1 + 0x10,0);
  fVar2 = *pfVar1;
  if (fVar2 < 0.0) {
    fVar2 = -fVar2;
  }
  *pfVar1 = fVar2;
  return 0;
}

