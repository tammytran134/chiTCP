#ifndef FIXTURES_H_
#define FIXTURES_H_

#include "chitcp/chitcpd.h"
#include "serverinfo.h"

extern serverinfo_t *si;
extern chitcp_tester_t *tester;

void chitcpd_and_tester_setup(void);
void chitcpd_and_tester_teardown(void);
void tester_run(void);

#endif /* FIXTURES_H_ */