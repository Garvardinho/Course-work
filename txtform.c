#include <stdio.h> 
#include "project.h"

#define NO  0
#define YES 1

int strform()
{
	FILE *code_file; // Исходный файл
	FILE *temp_file; // Промежуточный
	FILE *output_file; // Конечный
	char *ptr; // Указатель на очередной символ строки
	char *start_ptr; // Указатель, используемый для запоминания начала части строки
	char *del_ptr; // Указатель, используемый для запоминания конца части строки
	char line[1024]; // Строка
	int string_flags[500] = {0}; // Массив, элементы которого указывают на количество отступов в каждой строке
	int open_brace_flag; // Флаг открывающей фигурной скобки
	int loop_start_indent; // Переменная для запоминания отступа при работе с вложенными циклами
	int loop_flag; // Флаг цикла / условия / функции
	int for_flag; // Флаг цикла for
	int carry_cnt; // Счетчик переносов
	int flag; // Флаг для работы с разделителями
	int i, j; // Индексы
	
	loop_flag = NO;
	open_brace_flag = NO;
	flag = NO;
	for_flag = NO;
	loop_start_indent = 0;
	carry_cnt = 0;
	i = 0;
	j = 0;

	// Открытие файлов
	code_file = fopen( "test.txt", "rt" );
	if( !code_file )
		return 1;

	temp_file = fopen( "temp.txt", "wt" );
	if( !temp_file )
		return 1;
	
	while( ptr = fgets( line, 100, code_file ) )
	{
		start_ptr = ptr;
		del_ptr = ptr;
		string_flags[i + 1] = string_flags[i];

		if( loop_flag )
			string_flags[i + 1]++;

		while( *ptr )
		{
			if( *ptr != '\n' )
				carry_cnt = 0;

			// Обработка разделителей
			if( *ptr == ' ' || *ptr == '\t' )
			{
				if( !flag )
				{
					del_ptr = ptr + 1;

					if( *(ptr + 1) == ' ' || *(ptr + 1) == '\t' || *(ptr + 1) == '(')
						flag = YES;

					if( ptr == line || *(ptr - 1) == '\n' || *(ptr - 1) == '#' || *(ptr - 1) == ')' || ( *(ptr - 1) == '<' && line[0] == '#' ) )
					{
						flag = YES;
						del_ptr--;
					}

					if( *ptr == '\t' )
						*ptr = ' ';
				}
			}
			else
			{
				switch( *ptr )
				{
					// Обработка открывающей фигурной скобки
					case '{':			
						{
							if( loop_flag )
							{
								string_flags[i + 1] = string_flags[i];
								string_flags[i]--;
							}

							start_ptr = ptr;
							while( *ptr != '}' ) // Если на этой же строке стоит и закрывающая скобка, то не делать перенос
							{
								ptr++;
								if( *ptr == '\0' )
									break;
							}
							
							if( !*ptr ) // Если до фигурной скобки есть какое-либо содержимое и в конце нет закрывающей, ее следует перенести
							{
								ptr = start_ptr;
								while( ptr != line )
								{
									ptr--;
									if( *ptr != ' ' && *ptr != '\t' && *ptr != '\n' ) 
									{
										sprt_add( '\n', 1, start_ptr );
										start_ptr++;
										i++;
										break;
									}
								}
								if( !loop_flag )
									string_flags[i + 1]++;
							}

							loop_flag = NO;
							ptr = start_ptr;
							open_brace_flag = YES;
						}
						break;

					// Обработка закрывающей фигурной скобки
					case '}':
						{
							start_ptr = ptr;
							
							while( ptr != line )
							{
								if( *ptr != ' ' || *ptr != '\t' )
									break;

								ptr--;
							}
							if( !open_brace_flag && ptr != line ) // Добавление переноса в случае необходимости
							{
								sprt_add( '\n', 1, start_ptr );
								string_flags[i]--;
								string_flags[i + 1]--;
								start_ptr++;
								i++;
							}
							else if( ptr == line )
							{
								string_flags[i]--;
								string_flags[i + 1]--;
							}

							ptr = start_ptr;
						}
						break;

					// Обработка открывающей угловой скобки
					case '<':
						{
							if( line[0] != '#' ) // Если угловая скобка не является частью директивы
							{
								if( *(ptr - 1) != '\t' && *(ptr - 1) != ' ')
								{
									sprt_add( ' ', 1, ptr );
									ptr++;
								}

								if( *(ptr + 1) != '\t' && *(ptr + 1) != ' ')
									sprt_add( ' ', 1, ptr + 1 );
							}
						}
						break;

					// Обработка закрывающей угловой скобки		
					case '>':
						{							
							if( *(ptr - 1) != '\t' && *(ptr - 1) != ' ' && line[0] != '#' ) // Если угловая скобка является частью арифметического выражения
							{
								sprt_add( ' ', 1, ptr );
								ptr++;
							} 
							else if( *(ptr + 1) != '\t' && *(ptr + 1) != ' ')
								sprt_add( ' ', 1, ptr + 1 );
							else if( ( *(ptr - 1) == ' ' || *(ptr - 1) == '\t' ) && line[0] == '#' ) // Если угловая скобка является частью директивы
								sprt_del( del_ptr - 1, ptr );
						}
						break;

					// Обработка символов в кавычках
					case '"':
						{
							ptr++;
							while( *ptr != '"' )
							{
								if( *ptr == '\\' )
									ptr++;

								ptr++;
							}
						}
						break;

					// Обработка комментариев
					case '/':
						{
							if( *(ptr + 1) == '/' ) // Однострочный
							{
								start_ptr = ptr - 1;
								while( *start_ptr == ' ' && start_ptr != line )
									start_ptr--;

								sprt_del( start_ptr + 1, ptr );

								while( *ptr++ );

								flag = NO;
							}
							else if( *(ptr + 1) == '*' ) // Многострочный
							{
								ptr++;
								while( !( *ptr == '*' && *(ptr + 1) == '/' ) )
								{
									if( *ptr == '\0' )
									{
										fputs( line, temp_file );
										i++;
										string_flags[i + 1] = string_flags[i];
										ptr = fgets( line, 100, code_file );
										continue;
									}
									ptr++;
								}
							}
						}
						break;

					// Обработка открывающей скобки
					case '(':
						{
							start_ptr = ptr - 1;

							if( *(start_ptr - 1) != '=' );
							{
								while( *start_ptr == ' ' || *start_ptr == '\t' )
									start_ptr--;

								start_ptr++;
								sprt_del( start_ptr, ptr ); // Удаляем лишние пробелы после скобки
								start_ptr++;
								ptr = start_ptr;
							}
							
							if( *ptr == '\t' || *ptr == ' ' )
							{
								if( *ptr == '\t' )
									*ptr = ' ';

								while( *ptr )
								{
									if( *ptr != ' ' && *ptr != '\t' )
										break;

									ptr++;

									if( *ptr == ')' )
									{
										sprt_del( start_ptr, ptr ); // Удаляем все пробелы, если между скобками стоят только разделители
										flag = NO;
										break;
									}
								}
								ptr = start_ptr;
							}
							else
							{
								if( *ptr != ')' )
									sprt_add( ' ', 1, ptr );
							}
		
							if( for_flag )
								j++;
						}
						break;

					// Обработка закрывающей скобки
					case ')':
						{
							start_ptr = ptr; // Если не нашли точку с запятой или фигурную скобку в конце - перед нами условие / цикл / функция
							while( *ptr != ';' && *ptr != '{' && !loop_flag )
							{
								if( !*ptr )
								{
									string_flags[i + 1]++;
									loop_start_indent = string_flags[i];
									loop_flag = YES;
									break;
								}

								ptr++;
							}

							ptr = start_ptr;
							if( for_flag )
							{
 								j--;
								if( j == 0 )
									for_flag = NO;
							}

							if( *(ptr - 1) != ' ' && *(ptr - 1) != '(' )
							{
								sprt_add( ' ', 1, ptr );
								ptr++;
							}
							else
								flag = NO;
						}
						break;

					// Обработка точки с запятой
					case ';':
						{
							if( loop_flag && !for_flag)
							{
								string_flags[i + 1] = loop_start_indent;
								loop_flag = NO;
							}

							start_ptr = ptr;

							if( *(start_ptr + 1) != ' ' && *(start_ptr + 1) != '\t' )
								sprt_add( ' ', 1, start_ptr + 1 );

							if( !open_brace_flag ) // Перенос, если в строке нет фигурных скобок
							{
								ptr = start_ptr;
								if( !for_flag )
								{
									while( *ptr )
									{
										ptr++;

										if( *ptr == '/' || *ptr == ' ' || *ptr == '\t' || *ptr == '\n' )
										{
											if( *ptr == '/' || *ptr == '\n' )
												break;
										}
										else
										{
											sprt_add( '\n', 1, (start_ptr + 1) );
											i++;
											string_flags[i + 1] = string_flags[i]; // По причине добавления переноса следует помнить об отступах в каждой строке
											break;
										}
									}
									ptr = start_ptr;
								}
							}
							
						}
						break;
				
					// Обработка одинарной кавычки
					case '\'':
						{
							if( *(ptr + 1) == '\\' )
								ptr++;

							ptr++;
						}
						break;
					
					// Обработка запятой
					case ',':
						{
							if( *(ptr + 1) != ' ' || *(ptr + 1) != '\t' )
								sprt_add( ' ', 1, (ptr + 1) );
						}
						break;

					// Обработка переносов
					case '\n':
						{
							if( ptr == line && *(ptr + 1) == '\0' )
							{
								if( carry_cnt || loop_flag )
								{
									*ptr = '\0';
									i--;
									break;
								}
								carry_cnt++;
							}
						}
						break;

					// Обработка цилка for
					case 'f':
						{
							if( *(ptr + 1) == 'o' && *(ptr + 2) == 'r' )
								for_flag = YES;
						}
						break;

					// Обработка сложения и вычитания
					case '-':
					case '+':
						{
							if( *(ptr - 1) != ' ' && *(ptr - 1) != '\n' )
								sprt_add( ' ', 1, ptr );

							ptr++;
							if( *(ptr + 1) != ' ' && *(ptr + 1) != '\t' && *(ptr + 1) != '=' )
								sprt_add( ' ', 1, (ptr + 1) );
						}
						break;

					// Обработка знака равно
					case '=':
						{
							if( *(ptr - 1) != '+' && *(ptr - 1) != '-' && *(ptr - 1) != '*' && *(ptr - 1) != '/' && *(ptr - 1) != ' ' && *(ptr - 1) != '\t' && *(ptr - 1) != '=' )
								sprt_add( ' ', 1, ptr );

							if( *(ptr + 1) != '=' && *(ptr + 1) != ' ' && *(ptr + 1) != '\t' )
								sprt_add( ' ', 1, (ptr + 1) );
						}
						break;
				}	

				// Удаление лишних пробелов
				if( flag )
				{
					sprt_del( del_ptr, ptr );
					ptr = del_ptr;
					flag = NO;
				}

			}
			ptr++;
		}
		if( open_brace_flag )
			open_brace_flag = NO;

		fputs( line, temp_file );
		i++;
	}
	fcloseall();

	// Открытие промежуточного и конечного файл для перезаписи кода программы с отступами
	temp_file = fopen( "temp.txt", "rt" );
	output_file = fopen( "result.doc", "wb" );
	if( !output_file )
		return 1;

	// Выводим код программы с отступами
	i = 0;
	while( ptr = fgets( line, 100, temp_file ) )
	{
		if( string_flags[i] > 0 )
			sprt_add( sprt_form, string_flags[i] * sprt_qnt, ptr );

		fputs( line, output_file );
		i++;
	}
	fcloseall();

	// Удаляем промежуточный файл
	temp_file = fopen( "temp.txt", "wt" );
	fclose( temp_file );
	remove( "temp.txt" ); 

	return 0;
}

// Функция добавления разделителей
void sprt_add( char symb, int n, char *src )
{
	char swap;
	char *add_ptr;

	add_ptr = src;
	
	while( n-- )
	{
		while( *src )
		{
			swap = *(src + 1);
			*(src + 1) = *add_ptr;
			*add_ptr = swap;
			src++;
		}

		*add_ptr = symb;
		src = add_ptr;
	}
}

// Функция удаления разделителей
void sprt_del( char *del, char *ptr )
{
	char *start;

	start = del;
	while( *del++ = *ptr++ );
}
