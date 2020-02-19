/* 
当选择了数组的一个数, 不能选择相邻的数, 能相加的最大值
*/

#include <stdio.h>

int max(int x, int y){
	if(x>y) return x;
	return y;
}

int rec_opt(int *arr, int i){
	if (i == 0) return *arr;
	else if (i == 1) return max(*arr, ++*arr);
	else{
		int a = rec_opt(arr, i-2) + arr[i];
		int b = rec_opt(arr, i-1);
		return max(a, b);
	}
}


int dp_opt(int *arr, int length){
    int opt[length];
    opt[0] = arr[0];
    opt[1] = max(arr[0], arr[1]);

    for(int i=2;i<length;i++){
        int a = opt[i-2] + arr[i];
        int b = opt[i-1];
        opt[i] = max(a, b);
    }
    return opt[length-1];
}

int main(){
	// int arr[] = {1,2,4,1,7,8,3};
	int arr[] = {4,1,1,9,1};
	int res = rec_opt(arr, 4);
	printf("rec_res: %d\n", res);
    int res_2 = dp_opt(arr, 5);
	printf("dp_res: %d\n", res_2);
}
