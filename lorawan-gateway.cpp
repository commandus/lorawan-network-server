/**
 * LoRaWAN gateway server skeleton
 * Copyright (c) 2021 {@link mailto:andrey.ivanov@ikfia.ysn.ru} Yu.G. Shafer Institute
 * of Cosmophysical Research * and Aeronomy of Siberian Branch of the Russian Academy
 * of Sciences
 * MIT license {@link file://LICENSE}
 */
#include <iostream>
#include <iomanip>
#include <cstring>
#include <csignal>
#include <climits>

#ifdef _MSC_VER
#else
#include <execinfo.h>
#endif

#include "argtable3/argtable3.h"

#include "platform.h"
#include "utilstring.h"
#include "utildate.h"
#include "daemonize.h"
#include "errlist.h"
#include "config-json.h"
#include "config-filename.h"
#include "utilfile.h"


int main(
	int argc,
	char *argv[])
{
}

