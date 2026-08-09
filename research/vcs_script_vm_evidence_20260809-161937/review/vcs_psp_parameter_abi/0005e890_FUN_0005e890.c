/* REVIEW-ONLY: FUN_0005e890 @ 0x0005e890 */


void FUN_0005e890(undefined4 param_1,undefined4 param_2,short param_3)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  
  iVar3 = 0;
  if (0 < param_3) {
    puVar2 = (undefined4 *)0x1d698;
    do {
      uVar4 = *puVar2;
      puVar1 = (undefined4 *)FUN_0005e3d0(param_1,param_2,0);
      iVar3 = iVar3 + 1;
      *puVar1 = uVar4;
      puVar2 = puVar2 + 1;
    } while (iVar3 < param_3);
  }
  return;
}

