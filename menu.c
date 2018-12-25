#include <stdio.h>
#include "project.h"

void menu()
{
	int flag;

	printf( "Do you want to use tabs or spaces?\n1. Tabs\n2. Spaces\nEnter the form number: " );
	scanf( "%d", &flag );

	if( flag == 1 )
	{
		printf( "How much tabs do you want: " );
		sprt_form = '\t';
		scanf( "%d", &sprt_qnt );
	}
	else if( flag == 2 )
	{
		printf( "How much spaces do you want: " );
		sprt_form = ' ';
		scanf( "%d", &sprt_qnt );
	}
	else
	{
		printf( "You've entered the wrong flag!" );
		getchar();
		getchar();
	}
}
