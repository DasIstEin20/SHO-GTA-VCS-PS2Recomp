/* REVIEW-ONLY: FUN_0005e6f8 @ 0x0005e6f8 */


void FUN_0005e6f8(int *param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = *param_2;
  param_1[1] = 0;
  *param_1 = iVar1;
  if (*param_2 != 0) {
    *(int **)(*param_2 + 4) = param_1;
  }
  *param_2 = (int)param_1;
  return;
}

