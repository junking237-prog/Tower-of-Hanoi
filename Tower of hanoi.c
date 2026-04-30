#include <stdio.h>
#include<math.h>
#include<stdbool.h>
int main(){
	int n,i,c;
	bool t;
	printf("-----------Tower of Hanoi solution------------\n");
	printf("enter the number of rings that your tower of hanoi have: ");
	scanf("%d",&n);
	if(n==1){
		printf("For 1 ring, the solution is: \n");
	    printf("1-->2  or  1-->3 \n");
	}
	else if(n==2){
		printf("For 2 ring, the solution is: \n");
	    printf(" 1-->2 \n 1-->3 \n 2-->3 \n");
	}
	else if(n==3){
		printf("For 4 ring, the solution is: \n");
	    printf(" 1-->2 \n 1-->3 \n 2-->3 \n");
	    printf(" 1-->2 \n 3-->1 \n 3-->2 \n 1-->2 \n");
	}
	else if(n>=4){
	    printf("For %d ring, the solution is: \n",n);
	    i=pow(2,n)-1;
	    c=0;
		while(i>0){
			if(i>0){
				printf(" 1-->2 \n");
				i--;
			}
			if(i>0){
				printf(" 1-->3 \n");
				i--;
			}
			if(i>0){
				printf(" 2-->3 \n");
				i--;
			}
			if(i>0){
				printf(" 1-->2 \n");
				i--;
			}
			if(i>0){
				printf(" 3-->1 \n");
				i--;
			}
			if(i>0){
				printf(" 3-->2 \n");
				i--;
			}
			if(i>0){
				printf(" 1-->2 \n");
				i--;
			}
			if(i>0){
				t=c%2;
				if(!t){
				    printf(" 1-->3 \n");
				}
				else if(t){
					printf(" 3-->1 \n");
				}
				i--;
			}
			if(i>0){
				printf(" 2-->3 \n");
				i--;
			}
			if(i>0){
				printf(" 2-->1 \n");
				i--;
			}
			if(i>0){
				printf(" 3-->1 \n");
				i--;
			}
			if(i>0){
				printf(" 2-->3 \n");
				i--;
			}
			c++;
		}	
	}
	printf("that's what you should have done to win the tower of hanoi game. ");
	return 0;
}