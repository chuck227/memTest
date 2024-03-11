#include <stdio.h>

void tester(){
	printf("test from function tester\n");	
}

void tester2(int a, int b){
	printf("The valud is %d\n", a+b);
}	

int tester3(int a, int b){
	int returner = 0;
	for(int i = 0; i < b; i++) returner += a;
	return returner;
}

int main(){
	printf("test from main\n");
	tester();
	tester2(1, 2);
	int test = tester3(1, 5);
	printf("We did it! %d\n", test);
}
