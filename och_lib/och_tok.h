#pragma once

#include "och_memrun.h"

namespace och
{
	struct destructive_tokenizer
	{
		char* text;
		const char* delimiters;

		destructive_tokenizer(char* text, const char* delimiters);

		bool is_delimiter(char c);

		char* operator()();
	};

	struct cstr_tokenizer
	{
		const char* text;
		const char* delimiters;

		cstr_tokenizer(const char* text, const char* delimiters);

		bool is_delimiter(char c);

		och::string operator()();
	};

	struct tokenizer
	{
		och::string text;
		och::string delimiters;

		tokenizer(och::string text, och::string delimiters);

		bool is_delimiter(char c);

		och::string operator()();
	};

	struct matcher
	{
		och::string text;
		och::memrun<const och::string> targets;
		uint8_t target_idx[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };

		matcher(och::string text, och::memrun<och::string> targets);

		och::string operator()();

		uint32_t count();
	};
}
