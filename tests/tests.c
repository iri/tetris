#include "tetris.h"
#include "minunit.h"

MU_TEST(test_min4_01) {
	mu_check(min4(-7,4,5,2) == -7);
}
MU_TEST(test_min4_02) {
	mu_check(min4(0,-4,5,2) == -4);
}
MU_TEST(test_min4_03) {
	mu_check(min4(0,-4,-5,2) == -5);
}
MU_TEST(test_min4_04) {
	mu_check(min4(0,-4,5,-12) == -12);
}
MU_TEST(test_max4_01) {
	mu_check(max4(7,-2,0,5) == 7);
}
MU_TEST(test_max4_02) {
	mu_check(max4(-1,8,0,5) == 8);
}
MU_TEST(test_max4_03) {
	mu_check(max4(1,-2,9,5) == 9);
}
MU_TEST(test_max4_04) {
	mu_check(max4(1,-2,0,15) == 15);
}
MU_TEST_SUITE(test_suite_tetris) {
	MU_RUN_TEST(test_min4_01);
	MU_RUN_TEST(test_min4_02);
	MU_RUN_TEST(test_min4_03);
	MU_RUN_TEST(test_min4_04);
	MU_RUN_TEST(test_max4_01);
	MU_RUN_TEST(test_max4_02);
	MU_RUN_TEST(test_max4_03);
	MU_RUN_TEST(test_max4_04);
}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_suite_tetris);
	MU_REPORT();
	return MU_EXIT_CODE;
}

