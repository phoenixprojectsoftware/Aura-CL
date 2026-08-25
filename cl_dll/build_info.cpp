#include "build_info.h"
#include "generated/build_info_generated.h"

const char* AuraBuildDate()
{
	return AURA_BUILD_DATE;
}

const char* AuraBuildTime()
{
	return AURA_BUILD_TIME;
}

const char* AuraBuildTimestamp()
{
	return AURA_BUILD_TIMESTAMP;
}
