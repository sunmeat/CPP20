#include <iostream>
using namespace std;

#include <vector>

#include <ranges>
using namespace std::views;

#include <coroutine>

#include <chrono>
using namespace std::chrono;

#include <numbers>
using namespace std::numbers;

#include <source_location>

// CPP20 BIG FOUR:
// - concepts
// - modules
// - ranges library
// - coroutines

/*
Концепты дают возможность описать
требования для параметров шаблонов, которые будут
проверяться компилятором, и тем самым улучшают то,
как пишется обобщённый код.
- требования для параметров шаблона становятся
частью публичного интерфейса шаблона;
- перегрузка функций или специализация классов
может быть основана на концептах;
- мы получаем улучшенные сообщения об ошибках,
поскольку компилятор проверяет требования
к параметрам шаблона;
- есть уже готовые концепты как база для написания собственных;
- унифицируется использование ключевого слова auto и концептов.
Вместо auto можно использовать концепт;
- если объявление функции использует концепт, то эта функция
автоматически становится шаблонной функцией. Написание
шаблонных функций становится таким же лёгким,
как и написание обычных функций.
*/
template <typename T>
concept Integral = is_integral<T>::value;
Integral auto gcd(Integral auto a, Integral auto b) {
	if (b == 0) return a;
	return gcd(b, a % b);
}

////////////////////////////////////////////////////////////////////////////////////////

/*
Модули обеспечивают:
- более быструю компиляцию;
- уменьшение необходимости использования макросов;
- более выраженную логическую структуру кода;
- делают устаревшими заголовочные файлы;
- помощь в избавлении от уродливых макрорешений

// ???

*/

////////////////////////////////////////////////////////////////////////////////////////

/*
Библиотека диапазонов (range) поддерживает алгоритмы,
которые могут:
- работать напрямую с контейнерами, не требуя
итераторов для задания диапазона;
- выполняться отложенно (lazy);
- совмещаться/комбинироваться при помощи символа |.
Говоря проще: эта библиотека поддерживает функциональные
шаблоны. Следующий пример демонстрирует композицию
функций через символ |
*/

void range() {
	vector<int> ints{ 0, 1, 2, 3, 4, 5 };
	auto even = [](int i) { return i % 2 == 0; };
	auto square = [](int i) { return i * i; };

	for (int i : ints | filter(even) |
		views::transform(square)) {
		cout << i << ' '; // 0 4 16
	}
}

////////////////////////////////////////////////////////////////////////////////////////

/*
Корутины – это обобщённые функции, выполнение которых
можно приостанавливать и возобновлять потом,
сохраняя их состояние. Корутины очень удобны для
написания событийно-ориентированных (event-driven)
приложений. Такими приложениями могут быть симуляции,
игры, серверы, пользовательские интерфейсы и даже
алгоритмы. Корутины обычно используются для
кооперативной многозадачности.
С++20 не предоставляет конкретных корутинов, вместо
этого С++20 предоставляет фреймворк для написания
корутинов. Этот фреймворк состоит из более чем
20 функций, некоторые из которых придётся
реализовать, некоторые можно переопределить. Таким
образом можно приспособить корутины для своих целей.
Следующий фрагмент кода использует генератор для
создания потенциально бесконечного потока данных.
*/

template <typename T>
struct Generator {
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	Generator(handle_type h) : coro(h) {} 
	handle_type coro;

	~Generator() {
		if (coro) coro.destroy();
	}
	
	Generator(const Generator&) = delete;
	
	Generator& operator = (const Generator&) = delete;
	
	Generator(Generator&& oth) noexcept : coro(oth.coro) {
		oth.coro = nullptr;
	}

	Generator& operator = (Generator&& oth) noexcept {
		coro = oth.coro;
		oth.coro = nullptr;
		return *this;
	}

	T getValue() {
		return coro.promise().current_value;
	}

	bool next() { 
		coro.resume();
		return not coro.done();
	}

	struct promise_type {
		promise_type() = default;

		~promise_type() = default;

		auto initial_suspend() {
			return std::suspend_always{};
		}

		auto final_suspend() noexcept {
			return std::suspend_always{};
		}
		auto get_return_object() {
			return Generator{ handle_type::from_promise(*this) };

		}
		auto return_void() {
			return std::suspend_never{};
		}

		auto yield_value(const T value) {
			current_value = value;
			return std::suspend_always{};
		}
		void unhandled_exception() {
			std::exit(1);
		}
		T current_value;
	};
};

Generator<int> getNext(int start = 0, int step = 1) {
	auto value = start;
	while (true) {
		co_yield value;
		value += step;
	}
}

void coroutines() {
	cout << '\n';
	cout << "getNext():";
	auto gen1 = getNext();
	for (int i = 0; i <= 10; ++i) {
		gen1.next();
		cout << " " << gen1.getValue();
	}

	cout << "\n\ngetNext(100, -10):";
	auto gen2 = getNext(100, -10);
	for (int i = 0; i <= 20; ++i) {
		gen2.next();
		cout << " " << gen2.getValue();
	}

	cout << "\n";
}

////////////////////////////////////////////////////

// ну и ещё немного вкусных фишечек:
/*
Оператор трехстороннего сравнения (spaceship operator)
<=> сравнивает два значения – А и В – и определяет,
что имеет место A < B, A == B или A > B.
Если определить оператор трёхстороннего сравнения
как default, то компилятор попытается сгенерировать
соответствующий оператор для вашего класса. В этом
случае будут автоматически реализованы все шесть
операторов сравнения: ==, !=, <, <=, > и >=.
*/
class Student {
	int age;
public:
	Student(int age) : age{ age } { }
	auto operator<=>(const Student&) const = default;
};

void spaceship_test()
{
	cout << "\n";

	Student a(18);
	Student b(17);
	if (a > b)
	{
		cout << "A is older!\n";
	}
	else
	{
		cout << "A is younger!\n";
	}
}

///////////////////////////////////////////////////

void datetime()
{
	auto time = floor<milliseconds>(system_clock::now());
	auto localTime = zoned_time<milliseconds>(current_zone(), time);
	auto berlinTime = zoned_time<milliseconds>("Europe/Berlin", time);
	auto newYorkTime = zoned_time<milliseconds>("America/New_York", time);
	auto tokyoTime = zoned_time<milliseconds>("Asia/Tokyo", time);
	cout << time << '\n';
	cout << localTime << '\n';
	cout << berlinTime << '\n';
	cout << newYorkTime << '\n';
	cout << tokyoTime << '\n';
}
int main()
{
	cout << gcd(20, 25) << "\n"; // 5
	// cout << gcd(20.2, 25.5) << "\n"; // no instance of function template "gcd" matches the argument lis

	range();

	coroutines();

	spaceship_test();

	datetime();

	// форматирование аля C#
	cout << format("The answer is {}.\n", 42);

	// всякие полезные математические константы
	cout << std::numbers::pi << "\n";

	// информация о текущем файле, функции, строке кода (по новому)
	cout << source_location::current().line() << "\n";
	cout << source_location::current().file_name() << "\n";
	cout << source_location::current().function_name() << "\n";
}