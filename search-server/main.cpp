#include <iostream>
#include <string>

using namespace std;

int main() {
	// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
	// Напишите ответ здесь: 100
	int x = 0, y;
	for (int i = 1; i <= 1000; ++i) {
		y = i % 10;
		if (y == 3) {
			++x;
		}
	}
	cout << x << endl;
	// Закомитьте изменения и отправьте их в свой репозиторий.
}
