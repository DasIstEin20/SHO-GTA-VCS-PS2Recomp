/* REVIEW-ONLY: FUN_00212e28 @ 0x00212e28 */


void FUN_00212e28(int param_1)

{
  undefined1 uVar1;
  undefined4 *puVar2;
  int iVar3;
  
  uVar1 = *(undefined1 *)(param_1 + 0x31);
  *(undefined1 *)(param_1 + 0x31) = 0;
  *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + 8;
  iVar3 = *(int *)(param_1 + 0x10);
  puVar2 = *(undefined4 **)(iVar3 + 0x14);
  while (puVar2 != (undefined4 *)0x0) {
    *(undefined4 *)(iVar3 + 0x14) = *puVar2;
    *puVar2 = *(undefined4 *)(*(int *)(param_1 + 0x10) + 0x10);
    *(undefined4 **)(*(int *)(param_1 + 0x10) + 0x10) = puVar2;
    iVar3 = *(int *)(param_1 + 8);
    *(undefined4 *)(iVar3 + -8) = 7;
    *(undefined4 **)(iVar3 + -4) = puVar2;
    *(byte *)((int)puVar2 + 5) = *(byte *)((int)puVar2 + 5) & 0xfe;
    *(byte *)((int)puVar2 + 5) = *(byte *)((int)puVar2 + 5) & 0xfd;
    FUN_00212d7c(param_1,puVar2);
    iVar3 = *(int *)(param_1 + 0x10);
    puVar2 = *(undefined4 **)(iVar3 + 0x14);
  }
  *(undefined1 *)(param_1 + 0x31) = uVar1;
  *(int *)(param_1 + 8) = *(int *)(param_1 + 8) + -8;
  return;
}

