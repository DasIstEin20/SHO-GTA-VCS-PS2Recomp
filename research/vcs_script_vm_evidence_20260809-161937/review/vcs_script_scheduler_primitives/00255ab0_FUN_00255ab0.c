/* REVIEW-ONLY: FUN_00255ab0 @ 0x00255ab0 */


void FUN_00255ab0(int *param_1,int *param_2)

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

