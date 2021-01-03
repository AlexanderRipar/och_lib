#include "och_tok.h"

namespace och
{
	bool destructive_tokenizer::is_delimiter(char c)
	{
		for (int i = 0; delimiters[i]; ++i)
		{
			if (c == delimiters[i])
				return true;
		}

		return false;
	}

	destructive_tokenizer::destructive_tokenizer(char* text, const char* delimiters) : text{ text }, delimiters{ delimiters } {}

	char* destructive_tokenizer::operator()()
	{

		while (is_delimiter(*text))
			++text;

		if (!(*text))
			return nullptr;

		char* ret = text;

		while (!is_delimiter(*text))
		{
			if (!(*text))
				return ret;

			++text;
		}

		*(text++) = '\0';

		return ret;
	}



	cstr_tokenizer::cstr_tokenizer(const char* text, const char* delimiters) : text{ text }, delimiters{ delimiters } {}

	bool cstr_tokenizer::is_delimiter(char c)
	{
		for (int i = 0; delimiters[i]; ++i)
		{
			if (c == delimiters[i])
				return true;
		}

		return false;
	}

	och::string cstr_tokenizer::operator()()
	{
		while (is_delimiter(*text))
			++text;

		if (!(*text))
			return { nullptr, nullptr };

		const char* beg = text;

		while (*text && !is_delimiter(*text))
			++text;

		return { beg, text++ };
	}

	tokenizer::tokenizer(och::string text, och::string delimiters) : text{ text }, delimiters{ delimiters } {}

	bool tokenizer::is_delimiter(char c)
	{
		for (const char& d : delimiters)
		{
			if (c == d)
				return true;
		}

		return false;
	}

	och::string tokenizer::operator()()
	{
		while (is_delimiter(*text.beg))
			++text.beg;

		if (!text.len())
			return { nullptr, nullptr };

		const char* beg = text.beg;

		while (*text.beg && !is_delimiter(*text.beg))
			++text.beg;

		return { beg, text.beg++ };
	}
}

och::matcher::matcher(och::string text, och::memrun<och::string> targets) : text{ text }, targets{ targets.beg, targets.end } {}

och::string och::matcher::operator()()
{
	for (char c = *(text.beg++); text.beg <= text.end; c = *(text.beg++))
		for (int i = 0; i != targets.len(); ++i)
			if (targets[i][target_idx[i]] == c)
			{
				if (++target_idx[i] == targets[i].len())
				{
					target_idx[i] = 0;
					return { text.beg - targets[i].len(), text.beg };
				}
			}
			else
				target_idx[i] = 0;

	return { nullptr, nullptr };
}

uint32_t och::matcher::count()
{
	uint32_t hits = 0;

	for (char c = *(text.beg++); text.beg <= text.end; c = *(text.beg++))
		for (int i = 0; i != targets.len(); ++i)
			if (targets[i][target_idx[i]] == c)
			{
				if (++target_idx[i] == targets[i].len())
				{
					target_idx[i] = 0;
					++hits;
				}
			}
			else
				target_idx[i] = 0;

	return hits;
}
