/* REVIEW-ONLY: FUN_00117a44 @ 0x00117a44 */


void FUN_00117a44(undefined4 param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  *(byte *)(*(int *)(param_2 + 0x20) + 5) = *(byte *)(*(int *)(param_2 + 0x20) + 5) | 1;
  iVar4 = 0;
  if (0 < *(int *)(param_2 + 0x28)) {
    iVar3 = 0;
    do {
      piVar1 = (int *)(*(int *)(param_2 + 8) + iVar3);
      if (*piVar1 == 4) {
        iVar2 = piVar1[1];
        *(byte *)(iVar2 + 5) = *(byte *)(iVar2 + 5) | 1;
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 8;
    } while (iVar4 < *(int *)(param_2 + 0x28));
  }
  iVar4 = 0;
  iVar3 = 0;
  if (0 < *(int *)(param_2 + 0x24)) {
    do {
      iVar2 = *(int *)(*(int *)(param_2 + 0x1c) + iVar3);
      iVar4 = iVar4 + 1;
      *(byte *)(iVar2 + 5) = *(byte *)(iVar2 + 5) | 1;
      iVar3 = iVar3 + 4;
    } while (iVar4 < *(int *)(param_2 + 0x24));
  }
  iVar4 = 0;
  iVar3 = 0;
  if (0 < *(int *)(param_2 + 0x34)) {
    do {
      iVar2 = *(int *)(*(int *)(param_2 + 0x10) + iVar3);
      if ((*(byte *)(iVar2 + 5) & 0x11) == 0) {
        FUN_001177f8(param_1,iVar2);
      }
      iVar4 = iVar4 + 1;
      iVar3 = iVar3 + 4;
    } while (iVar4 < *(int *)(param_2 + 0x34));
  }
  iVar3 = 0;
  iVar4 = 0;
  if (0 < *(int *)(param_2 + 0x38)) {
    do {
      iVar2 = *(int *)(*(int *)(param_2 + 0x18) + iVar4);
      iVar3 = iVar3 + 1;
      *(byte *)(iVar2 + 5) = *(byte *)(iVar2 + 5) | 1;
      iVar4 = iVar4 + 0xc;
    } while (iVar3 < *(int *)(param_2 + 0x38));
  }
  return;
}

