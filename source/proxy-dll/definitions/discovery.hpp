#pragma once

size_t operator"" _d(const char* str, size_t len);

namespace discovery
{
	enum sig_relation
	{
		SIG_RELEVANCE_IMPOSSIBLE = -1,
		SIG_RELEVANCE_DIRECT_HIT = 0,
		SIG_RELEVANCE_LEA_FROM = 1,
		SIG_RELEVANCE_JMP_FROM = 2
	};

	struct sig_instance
	{
		const char* name;
		const char* sig;
		size_t dist;
		sig_relation relv;
	};
}