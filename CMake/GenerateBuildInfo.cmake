string(
	TIMESTAMP
	AURA_BUILD_DATE
	"%b %d %Y"
)

string(
	TIMESTAMP
	AURA_BUILD_TIME
	"%H:%M:%S"
)

string(
	TIMESTAMP
	AURA_BUILD_TIMESTAMP
	"%a %b %d %H:%M:%S %Y"
)

configure_file(
	"${INPUT_FILE}"
	"${OUTPUT_FILE}"
	@ONLY
)
