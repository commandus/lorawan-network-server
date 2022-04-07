#include "ws-any.h"

#if defined(ENABLE_WS) && defined(ENABLE_DB_SQLITE)

#endif

WebServiceRunner::WebServiceRunner(const DatabaseByConfig *adbConfig)
{
	dbConfig = adbConfig;
}

int WebServiceRunner::start()
{

}

int WebServiceRunner::finish()
{

}

~WebServiceRunner::WebServiceRunner()
{

}
