/* REVIEW-ONLY: FUN_0005e6c0 @ 0x0005e6c0 */


void FUN_0005e6c0(int *param_1,int *param_2)

{
  int iVar1;
  
  if ((int *)param_1[1] == (int *)0x0) {
    *param_2 = *param_1;
    iVar1 = *param_1;
  }
  else {
    *(int *)param_1[1] = *param_1;
    iVar1 = *param_1;
  }
  if (iVar1 != 0) {
    *(int *)(iVar1 + 4) = param_1[1];
  }
  return;
}

