// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#include <munit.h>

extern MunitTest tests_seq_num[];
extern MunitTest tests_key_state[];
extern MunitTest tests_reorder_queue[];
extern MunitTest tests_http[];
extern MunitTest tests_rpcrypt[];
extern MunitTest tests_gkcrypt[];
extern MunitTest tests_takion[];
extern MunitTest tests_fec[];
extern MunitTest tests_regist[];

static MunitSuite suites[] = {
	{
		"/seq_num",
		tests_seq_num,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{
		"/key_state",
		tests_key_state,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{
		"/reorder_queue",
		tests_reorder_queue,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{
		"/http",
		tests_http,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{
		"/rpcrypt",
		tests_rpcrypt,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{
		"/gkcrypt",
		tests_gkcrypt,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{
		"/takion",
		tests_takion,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{
		"/fec",
		tests_fec,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{
		"/regist",
		tests_regist,
		NULL,
		1,
		MUNIT_SUITE_OPTION_NONE
	},
	{ NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }
};

static const MunitSuite suite_main = {
	"/chiaki",
	NULL,
	suites,
	1,
	MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char *argv[])
{
	return munit_suite_main(&suite_main, NULL, argc, argv);
}
