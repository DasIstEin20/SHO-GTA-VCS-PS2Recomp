/* REVIEW-ONLY: FUN_00212d7c @ 0x00212d7c */


void FUN_00212d7c(int param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 *puVar3;
  
  puVar3 = (undefined4 *)0x0;
  if ((*(byte *)(*(int *)(param_2 + 8) + 6) & 8) == 0) {
    puVar3 = (undefined4 *)
             FUN_003209bc(*(int *)(param_2 + 8),3,*(undefined4 *)(*(int *)(param_1 + 0x10) + 0x60));
  }
  if (puVar3 != (undefined4 *)0x0) {
    puVar1 = *(undefined4 **)(param_1 + 8);
    *puVar1 = *puVar3;
    puVar1[1] = puVar3[1];
    iVar2 = *(int *)(param_1 + 8);
    *(undefined4 *)(iVar2 + 8) = 7;
    *(int *)(iVar2 + 0xc) = param_2;
    iVar2 = *(int *)(param_1 + 8);
    *(int *)(param_1 + 8) = iVar2 + 0x10;
    FUN_0006e4c4(param_1,iVar2,0);
  }
  return;
}

