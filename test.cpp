#if 0

/*struct test
{
  int a, b, c;
};

void bar(test*)
{
}

void foo(int a)
{
  test t;
  t.a = a;
  bar(&t);
}*/

inline int f() restrict(amp) { return 0; }
inline int f() restrict(cpu) { return 1; }

int g() restrict(amp) { return f() + 1; }
int g() restrict(cpu) { return f() + 2; }
//int g() restrict(amp, cpu) { return f(); }

#else

#include <iostream>

#if __CXXAMP__
#include <amp.h>

using namespace concurrency;

void MultiplyWithAMP() {
	int aMatrix[] = { 1, 4, 2, 5, 3, 6 };
	int bMatrix[] = { 7, 8, 9, 10, 11, 12 };
	int productMatrix[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	array_view<int, 2> a(3, 2, aMatrix);
	array_view<int, 2> b(2, 3, bMatrix);
	array_view<int, 2> product(3, 3, productMatrix);

	parallel_for_each(
		product.get_extent(), [=](index<2> idx) restrict(amp) {
			int row = idx[0];
			int col = idx[1];
			for (int inner = 0; inner < 2; inner++) {
				product[idx] += a(row, inner) * b(inner, col);
			}
		}
	);

	product.synchronize();

	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 3; col++) {
			std::cout << productMatrix[row*3 + col] << "  ";
			//std::cout << product(row, col) << "  ";
		}
		std::cout << "\n";
	}
}

#endif

void MultiplyWithOutAMP() {

	int aMatrix[3][2] = {{1, 4}, {2, 5}, {3, 6}};
	int bMatrix[2][3] = {{7, 8, 9}, {10, 11, 12}};
	int product[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

	for (int row = 0; row < 3; row++) {
		for (int col = 0; col < 3; col++) {
			// Multiply the row of A by the column of B to get the row, column of product.
			for (int inner = 0; inner < 2; inner++) {
				product[row][col] += aMatrix[row][inner] * bMatrix[inner][col];
			}
			std::cout << product[row][col] << "  ";
		}
		std::cout << "\n";
	}
}

int main() {
#if __CXXAMP__
	MultiplyWithAMP();
#endif
	MultiplyWithOutAMP();
	return 0;
}

#endif
