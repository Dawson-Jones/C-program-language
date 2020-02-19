/*
一个数组, 如果里面的数, 相加可以得到目标值, 返回true, 否则false
*/

#include <stdio.h>

int rec_subset(int *arr, int i, int s){
    // 如果只剩下一个元素, 那么这个元素是s为 true, 否则 false
	if (i == 1)
		return arr[i-1] == s;
    // 如果元素大于目标值, 只能不选择
	if (arr[i-1] > s)
		return rec_subset(arr, i-1, s);
    // 如果元素等于目标值, 直接返回真
	else if(arr[i-1]==s) return 1;
    // 不只一个元素且元素的值小于目标值
    // option1: 选择这个元素 s变为s-这个元素
    // option2: 不选择这个元素, 目标值还是s
	else{
		return (rec_subset(arr, i-1, s-arr[i-1]) || rec_subset(arr, i-1, s));
	}
}

int main(){
	int arr[6] = {3,34,4,12,5,2};
	printf("res: %d\n",rec_subset(arr, 6, 9));
	printf("res: %d\n",rec_subset(arr, 6, 10));
	printf("res: %d\n",rec_subset(arr, 6, 11));
	printf("res: %d\n",rec_subset(arr, 6, 13));
	return 0;
}
