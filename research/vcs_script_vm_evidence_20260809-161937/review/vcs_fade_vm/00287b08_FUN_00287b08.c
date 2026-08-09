/* REVIEW-ONLY: FUN_00287b08 @ 0x00287b08 */


undefined8 FUN_00287b08(undefined8 param_1)

{
  undefined4 uVar1;
  undefined4 uStack_40;
  undefined4 uStack_3c;
  float fStack_38;
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,4,0x50e1f8);
  uStack_40 = DAT_0050e1f8;
  if ((DAT_0050e200 == -100.0) ||
     (fStack_38 = DAT_0050e200, uStack_3c = DAT_0050e1fc, DAT_0050e200 <= -250.0)) {
    uVar1 = DAT_0050e1fc;
    fStack_38 = (float)FUN_00196758(DAT_0050e1f8,DAT_0050e1fc);
    uStack_3c = uVar1;
  }
  FUN_0037e088(0x6f47d0,&uStack_40,(undefined2)DAT_0050e204,1);
  return 0;
}

