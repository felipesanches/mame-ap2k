static const char ureg_names[256][16] =
{
	"R0",		"R1",		"R2",		"R3",		"R4",		"R5",		"R6",		"R7",
	"R8",		"R9",		"R10",		"R11",		"R12",		"R13",		"R14",		"R15",
	"I0",		"I1",		"I2",		"I3",		"I4",		"I5",		"I6",		"I7",
	"I8",		"I9",		"I10",		"I11",		"I12",		"I13",		"I14",		"I15",
	"M0",		"M1",		"M2",		"M3",		"M4",		"M5",		"M6",		"M7",
	"M8",		"M9",		"M10",		"M11",		"M12",		"M13",		"M14",		"M15",
	"L0",		"L1",		"L2",		"L3",		"L4",		"L5",		"L6",		"L7",
	"L8",		"L9",		"L10",		"L11",		"L12",		"L13",		"L14",		"L15",
	"B0",		"B1",		"B2",		"B3",		"B4",		"B5",		"B6",		"B7",
	"B8",		"B9",		"B10",		"B11",		"B12",		"B13",		"B14",		"B15",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"FADDR",	"DADDR",	"???",		"PC",		"PCSTK",	"PCSTKP",	"LADDR",	"CURLCNTR",
	"LCNTR",	"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"USTAT1",	"USTAT2",	"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"IRPTL",	"MODE2",	"MODE1",	"ASTAT",	"IMASK",	"STKY",		"IMASKP",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"PX",		"PX1",		"PX2",		"TPERIOD",	"TCOUNT",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???",
	"???",		"???",		"???",		"???",		"???",		"???",		"???",		"???"
};

static const char bopnames[8][8] =
{
	"SET",		"CLEAR",	"TOGGLE",	"???",		"TEST",		"XOR",		"???",		"???"
};

static const char condition_codes_if[32][32] =
{
	"EQ",			"LT",			"LE",			"AC",
	"AV",			"MV",			"MS",			"SV",
	"SZ",			"FLAG0_IN",		"FLAG1_IN",		"FLAG2_IN",
	"FLAG3_IN",		"TF",			"BM",			"NOT LCE",
	"NE",			"GE",			"GT",			"NOT AC",
	"NOT AV",		"NOT MV",		"NOT MS",		"NOT SV",
	"NOT SZ",		"NOT FLAG0_IN",	"NOT FLAG1_IN",	"NOT FLAG2_IN",
	"NOT FLAG3_IN",	"NOT TF",		"NBM",			""
};

static const char condition_codes_do[32][32] =
{
	"EQ",			"LT",			"LE",			"AC",
	"AV",			"MV",			"MS",			"SV",
	"SZ",			"FLAG0_IN",		"FLAG1_IN",		"FLAG2_IN",
	"FLAG3_IN",		"TF",			"BM",			"LCE",
	"NE",			"GE",			"GT",			"NOT AC",
	"NOT AV",		"NOT MV",		"NOT MS",		"NOT SV",
	"NOT SZ",		"NOT FLAG0_IN",	"NOT FLAG1_IN",	"NOT FLAG2_IN",
	"NOT FLAG3_IN",	"NOT TF",		"NBM",			"FOREVER"
};

static const char mr_regnames[16][8] =
{
	"MR0F",	"MR1F",	"MR2F",	"MR0B",	"MR1B",	"MR2B", "???",	"???",
	"???",	"???",	"???",	"???",	"???",	"???",	"???",	"???"
};

typedef struct
{
	UINT32 op_mask;
	UINT32 op_bits;
	UINT32 (* handler)(UINT32, UINT64);
} SHARC_DASM_OP;
