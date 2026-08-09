/* REVIEW-ONLY: FUN_00287c38 @ 0x00287c38 */


undefined8 FUN_00287c38(undefined8 param_1)

{
  float fVar1;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,2,0x50e1f8);
  if ((float)DAT_0050e1f8 <= 2.0) {
    fVar1 = 0.0;
  }
  else {
    fVar1 = (float)DAT_0050e1f8 * 0.001;
  }
  FUN_00376f70(fVar1,0x6f47d0,(undefined2)DAT_0050e1fc);
  return 0;
}

