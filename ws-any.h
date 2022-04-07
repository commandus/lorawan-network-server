#include "db-any.h"

#ifndef WS_ANY_H_
#define WS_ANY_H_	1

class WebServiceRunner {
public:
	const DatabaseByConfig *dbConfig;
	WebServiceRunner(const DatabaseByConfig *dbConfig);
	virtual ~WebServiceRunner();
	int start();
	int finish();
};

#endif
