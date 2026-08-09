/* REVIEW-ONLY: FUN_002b8714 @ 0x002b8714 */


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_002b8714(int param_1,int param_2)

{
  ushort uVar1;
  int iVar2;
  
  if (param_2 == 0x37b) {
    uVar1 = *(ushort *)(param_1 + 0x204);
    *(ushort *)(param_1 + 0x204) = uVar1 + 1;
    *(uint *)(param_1 + (uint)uVar1 * 4 + 0x14) = *(uint *)(param_1 + 0x10) | 0x800000;
  }
  else {
    uVar1 = *(ushort *)(param_1 + 0x204);
    *(ushort *)(param_1 + 0x204) = uVar1 + 1;
    *(uint *)(param_1 + (uint)uVar1 * 4 + 0x14) = *(uint *)(param_1 + 0x10) | 0xc00000;
  }
  iVar2 = *(int *)(param_1 + 0x10);
  *(int *)(param_1 + 0x10) = iVar2 + 1;
  *(int *)(param_1 + 0x10) = iVar2 + 2;
  *(int *)(param_1 + 0x10) = iVar2 + 3;
                    /* WARNING: Subroutine does not return */
  FUN_0006189c(param_1,param_1 + 0x10,1,0x1d698);
}

