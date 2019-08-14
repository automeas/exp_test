#include "stdafx.h"
#include <iostream>
#include <vector> 
#include <map>
#include <fstream>
#include <algorithm>
#include <string>

typedef std::basic_string<wchar_t> s_tring;

using namespace std;

enum  LType { ND, DIGIT }; // типы лексем НЕ ОПРЕДЕЛЕНО / ЧИСЛО 

						   // ---структура для типа лексемы---
						   // LType type;		тип лексемы 
						   // s_tring varname;  если переменная то имя
						   // int digit;	    если цифра то значение
struct lexema {

	LType type;
	s_tring varname;
	int digit;

};

// ---структура для массива строк---
// s_tring left;			     левые части
// s_tring right;			 правые части нужны были для отладки
// int value;				 вычисленные значения
// int i;					 промежуточные значения. Используются в паре со стеком, чтобы продолжить вычисления в том месте выражения, где оно ушло вглубь цепочки
// int tempcalc;			 промежуточные значения. Используются в паре со стеком
// vector<lexema> lexcouple; разбивка правых частей на лексемы
struct varstruct {
	s_tring left;
	//	s_tring right;				
	int value;
	int i;
	int tempcalc;
	vector<lexema> lexcouple;
};


map <s_tring, varstruct> vars1; // массив строк. Он же массив уникальных переменных
map <s_tring, varstruct>::iterator iter;
map <s_tring, varstruct>::iterator iter_t;

// обрезка пробелов по краям и проверка есть ли пробелы в середине
// begin начало фрагмента, q длина, *str входная строка, *rez выходная строка
void del_space(size_t begin, size_t q, s_tring *str, s_tring *rez) {
	int first = (*str).find_first_not_of(L" ", begin, q);
	int last = (*str).find_last_not_of(L" ", begin - 1 + q, q);
	(*rez) = (*str).substr(first, last - first + 1);
	if ((*rez).find_first_of(' ') < (*rez).length()) {
		(*rez) = L"";
	}

}

int main()
{
	setlocale(LC_ALL, "RUS");
	s_tring InputFileName = L"c:\/my\/input.txt";
	s_tring OutputFileName = L"c:\/my\/output.txt";
	wchar_t l_ending = '\n';							// символ конца строки
	int Max_Depth = 0;
	s_tring str;
	s_tring tempstr;
	wifstream in(InputFileName);
	size_t eqpos;
	bool valid = true;
	while (getline(in, str, l_ending)) {	// читаем строки из файла
		Max_Depth++;
		varstruct temp;
		eqpos = str.find_first_of(L"="); // делим строку на левую и правую части
		if (eqpos != s_tring::npos) {
			int strlenght = str.length();
			del_space(0, eqpos, &str, &tempstr);
			temp.left = tempstr;
			if (temp.left == L"") { valid = false; break; }

			s_tring sr = str.substr(eqpos + 1, strlenght - eqpos);

			temp.value = -1;	// значение переменной пока неизвестно, поэтому инициализируем -1
			temp.i = 0;
			temp.tempcalc = 0;
			temp.lexcouple.reserve(count(sr.begin(), sr.end(), '+') + 1); // резервируем ячейки в массиве под переменные
			size_t index = 0;
			size_t prev_index = 0;

			while (index != s_tring::npos) { // разбиваем строку на лексемы. Разделитель +
				lexema lex;
				lex.digit = 0;
				lex.type = ND;
				if (index == 0) {
					prev_index = 0;
				}
				else prev_index = index + 1;
				index = sr.find_first_of('+', prev_index);
				if (index != s_tring::npos) {
					del_space(prev_index, index - prev_index, &sr, &tempstr);
					lex.varname = tempstr;
					if (lex.varname == L"") { valid = false; break; }
				}
				else {
					del_space(prev_index, sr.length() - prev_index, &sr, &tempstr);
					lex.varname = tempstr;
				}
				if (lex.varname == L"") { valid = false; break; }
				temp.lexcouple.push_back(lex); // добавляем лексему
			}
			vars1.insert(pair<s_tring, varstruct>(temp.left, temp)); // добавляем переменную в массив
		}
	}
	in.close();
	if (valid) {

		wcout << L"Загрузили. Начали считать ...\n";
		//--------------------------------------------основной цикл-------------------------
		Max_Depth++;
		bool not_valid = false;
		{
			std::vector<map <s_tring, varstruct>::iterator> stack;	// стек для хранения пути обхода ячеек
			stack.reserve(Max_Depth);								// резервируем память под стек, чтобы снизить издержки на добавление элементов
			map <s_tring, varstruct>::iterator pointer;


			int CalcValue;
			bool resolved = false;

			for (pointer = vars1.begin(); pointer != vars1.end(); pointer++) { // цикл перебора всех переменных
				iter = pointer;
				resolved = !((*iter).second.value < 0);
				while (!resolved) { // цикл цепочки решения
					CalcValue = (*iter).second.tempcalc;

					if ((*iter).second.value < 0) {  // разрешена ли переменная на текущем уровне стека (цепочки)

						for (int i = (*iter).second.i; i < (*iter).second.lexcouple.size(); i++) //  цикл решения выражения
						{
							try {
								resolved = true;
								if ((*iter).second.lexcouple[i].type != DIGIT) {
									(*iter).second.lexcouple[i].digit = stoi((*iter).second.lexcouple[i].varname, 0, 10); // число или нет. Если нет, уходим в Catch
									(*iter).second.lexcouple[i].type = DIGIT;
								}
								CalcValue += (*iter).second.lexcouple[i].digit; // подставляем в сумму найденное число
							}
							catch (std::exception &ex) {   // пытаемся найти переменнную в других строках

								iter_t = vars1.find((*iter).second.lexcouple[i].varname); // находим перемнную и решаем ее (уходим в цикл цепочки) 
								if ((iter_t == vars1.end()) || (stack.size()>Max_Depth)) { // если не находим или превысили размер стека, значит выражение невычисляемо
									not_valid = true;
									resolved = false;
									break;
								}
								else {
									if ((*iter_t).second.value < 0) {
										(*iter).second.tempcalc = CalcValue; // если переменная не решена заносим в стек промежуточное решение
										(*iter).second.i = i;
										stack.push_back(iter);
										iter = iter_t;
										resolved = false;
										break;
									}
									else {
										(*iter).second.lexcouple[i].digit = (*iter_t).second.value; // если переменная решена,
										(*iter).second.lexcouple[i].type = DIGIT;                   //  подставляем ее в выражение 
										CalcValue += (*iter).second.lexcouple[i].digit;
									}																// и переходим к следующем переменной в том же выражении
								}

							}
						}
						if (not_valid) break;
						if (resolved) {  // проверка решено ли до конца текущее выражение
							(*iter).second.value = CalcValue;
							if (stack.size() >0) {			// если не на вершине стека, 
								iter = stack.back();		// возаращаемся в выражение 
								stack.pop_back();
								(*iter).second.lexcouple[(*iter).second.i].type = DIGIT; //и подставлям значение переменной в него
								(*iter).second.lexcouple[(*iter).second.i].digit = CalcValue;
								resolved = false;
							}
						}
					}
					else {
						if (stack.size() < 1) // проверка, что мы вернулись в вершину цепочки, значит выражение разрешено
							resolved = true;
						else {                  // возаращаемся в выражение и подставлям значение переменной в него
							CalcValue = (*iter).second.value;
							iter = stack.back();
							stack.pop_back();
							(*iter).second.lexcouple[(*iter).second.i].type = DIGIT;
							(*iter).second.lexcouple[(*iter).second.i].digit = CalcValue;
							resolved = false;
						}
					}
				}
				if (not_valid) break;
			}
		}
		//--------------------------------------конец основного цикла-------------------------



		if (!not_valid) {
			wofstream ou(OutputFileName, ios_base::out);
			if (!ou.is_open()) // если файл не открывается
				wcout << L"Файл не может быть открыт из этой папки!\n";
			else {

				for (iter = vars1.begin(); iter != vars1.end(); iter++) {
					wcout << (*iter).first << " = " << (*iter).second.value << endl;
					ou << (*iter).first << " = " << (*iter).second.value << endl;
				}
				ou.close();
			}


		}
		else {
			wcout << L"Неразрешимо!" << endl;
		}
		//		cout << "Кол-во длинных рекурсий" << " = " << iterations << endl;
		//		cout << "Максимальная глубина рекурсии" << " = " << maxdepth << endl;
	}
	else {
		wcout << L"Синтаксическая ошибка" << endl;
	}
	system("pause");
	return 0;


}
21
22
23

