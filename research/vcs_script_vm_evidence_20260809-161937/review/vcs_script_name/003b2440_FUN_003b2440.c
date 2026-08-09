/* REVIEW-ONLY: FUN_003b2440 @ 0x003b2440 */


undefined8 FUN_003b2440(undefined8 param_1)

{
  byte *pbVar1;
  uint uVar2;
  byte *pbVar3;
  int aiStack_20 [4];
  
  FUN_00254d18(param_1,(int)param_1 + 0x10,1,aiStack_20);
  if (aiStack_20[0] == 0) {
    aiStack_20[0] = 0;
  }
  else {
    aiStack_20[0] = DAT_0048771c + aiStack_20[0];
  }
  pbVar3 = (byte *)((int)param_1 + 0x20f);
  FUN_0045dea8(pbVar3,aiStack_20[0],8);
  uVar2 = 0;
  pbVar1 = pbVar3;
  do {
    if (*pbVar1 - 0x41 < 0x1a) {
      *pbVar1 = *pbVar1 + 0x20;
    }
    uVar2 = uVar2 + 1 & 0xff;
    pbVar1 = pbVar3 + uVar2;
  } while (uVar2 < 8);
  return 0;
}

