#pragma once
class Hooks
{
public:
	Hooks() = default;
	~Hooks() = default;

private:
	HMODULE _hModule = NULL;

public:
	void attach();
	void detach();
};

