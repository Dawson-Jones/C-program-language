/* 
当选择了数组的一个数, 不能选择相邻的数, 能相加的最大值
*/

#include <stdio.h>

int max(int x, int y){
    /*
     * 条件运算符要求有3个操作对象，称三目(元)运算符
     * 一般形式为：表达式1 ? 表达式2 : 表达式3;
    */
    return (x>y)?x:y;
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

int memory[200];
int memory_rec_opt(int *arr, int i){
    if (i == 0)
        return memory[i] = *arr;
    else if(i == 1) return memory[i] = max(*arr, ++*arr);
    else{
        if (memory[i] == 0)
            return memory[i] = max(memory_rec_opt(arr, i-2) + arr[i], memory_rec_opt(arr, i-1));
        else return memory[i];
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
	int arr[] = {1,2,4,1,7,8,3};
	// int arr[] = {4,1,1,9,1};
	int res = rec_opt(arr, 6);
	printf("rec_res: %d\n", res);
    int res_2 = dp_opt(arr, 7);
	printf("dp_res: %d\n", res_2);
	printf("ms_res: %d\n", memory_rec_opt(arr, 6));
}
