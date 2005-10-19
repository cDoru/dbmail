/*
 *  Copyright (C) 2004  Paul Stevens <paul@nfg.nl>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *  $Id: check_dbmail_deliver.c 1829 2005-08-01 14:53:53Z paul $ 
 *
 *
 *  
 *
 *   Basic unit-test framework for dbmail (www.dbmail.org)
 *
 *   See http://check.sf.net for details and docs.
 *
 *
 *   Run 'make check' to see some action.
 *
 */ 

#include <check.h>
#include "check_dbmail.h"

extern char * multipart_message;
extern char * configFile;
extern db_param_t _db_params;


/* we need this one because we can't directly link imapd.o */
int imap_before_smtp = 0;
	
static void init_testuser1(void) 
{
        u64_t user_idnr;
	if (! (auth_user_exists("testuser1",&user_idnr)))
		auth_adduser("testuser1","test", "md5", 101, 1024000, &user_idnr);
}

static u64_t get_first_user_idnr(void)
{
	u64_t user_idnr;
	GList *users = auth_get_known_users();
	users = g_list_first(users);
	auth_user_exists((char *)users->data,&user_idnr);
	return user_idnr;
}

static u64_t get_mailbox_id(void)
{
	u64_t id, owner;
	auth_user_exists("testuser1",&owner);
	db_find_create_mailbox("INBOX", owner, &id);
	return id;
}

void setup(void)
{
	configure_debug(5,1,0);
	config_read(configFile);
	GetDBParams(&_db_params);
	db_connect();
	auth_connect();
	g_mime_init(0);
	init_testuser1();
}

void teardown(void)
{
	auth_disconnect();
	db_disconnect();
	config_free();
	g_mime_shutdown();
}


/****************************************************************************************
 *
 *
 * TestCases
 *
 *
 ***************************************************************************************/

START_TEST(test_dbmail_mailbox_new)
{
	struct DbmailMailbox *mb = dbmail_mailbox_new(get_mailbox_id());
	fail_unless(mb!=NULL, "dbmail_mailbox_new failed");
}
END_TEST

START_TEST(test_dbmail_mailbox_open)
{
	struct DbmailMailbox *mb = dbmail_mailbox_new(get_mailbox_id());
	mb = dbmail_mailbox_open(mb);
	fail_unless(mb!=NULL, "dbmail_mailbox_open failed");
}
END_TEST

START_TEST(test_dbmail_mailbox_dump)
{
	int c = 0;
//	FILE *o = fopen("/dev/null","w");
	struct DbmailMailbox *mb = dbmail_mailbox_new(get_mailbox_id());
	c = dbmail_mailbox_dump(mb,stdout);
	dbmail_mailbox_free(mb);
	fprintf(stderr,"dumped [%d] messages\n", c);
}
END_TEST

START_TEST(test_dbmail_mailbox_orderedsubject)
{
	char *res;
	unsigned setlen = 3;
	struct DbmailMailbox *mb = dbmail_mailbox_new(get_mailbox_id());
	u64_t *rset = g_new0(u64_t, setlen);
	rset[0] = 1;
	rset[1] = 2;
	rset[2] = 1074946;
	res = dbmail_mailbox_orderedsubject(mb, rset, setlen);
	printf("threads [%s]\n", res);

}
END_TEST

Suite *dbmail_mailbox_suite(void)
{
	Suite *s = suite_create("Dbmail Mailbox");

	TCase *tc_mailbox = tcase_create("Mailbox");
	suite_add_tcase(s, tc_mailbox);
	tcase_add_checked_fixture(tc_mailbox, setup, teardown);
	tcase_add_test(tc_mailbox, test_dbmail_mailbox_new);
	tcase_add_test(tc_mailbox, test_dbmail_mailbox_open);
	tcase_add_test(tc_mailbox, test_dbmail_mailbox_dump);
	tcase_add_test(tc_mailbox, test_dbmail_mailbox_orderedsubject);
	
	return s;
}

int main(void)
{
	int nf;
	Suite *s = dbmail_mailbox_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
	

