#include <stdio.h>

int main(){
	char name[50];

	fgets(name, sizeof(name), stdin);

	for(int i=0; name[i]!='\0'; i++){
	printf("%c\n", name[i]);
	}
}