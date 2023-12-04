#include "construct.h"

Construct::Construct(Type type, std::string name)
	: m_type(type),
	m_subtype(Construct::SubType::CST_NONE),
	m_name(name),
	m_condition(false),
	m_start(nullptr)
{}

Construct::~Construct()
{}

Construct& Construct::type_change(Type type, SubType subtype)
{
	if (Construct::Type::CT__KEEP__ != type)
	{
		m_type = type;
	}
	m_subtype = subtype;
	return *this;
}

Construct::Type Construct::type()
{
	return m_type;
}

Construct::Type Construct::operator()()
{
	return m_type;
}

Construct::SubType Construct::subtype()
{
	return m_subtype;
}

Construct& Construct::name(std::string name)
{
	m_name = name;
	return *this;
}

std::string Construct::name()
{
	return m_name;
}

Construct& Construct::condition(bool condition)
{
	m_condition = condition;
	return *this;
}

bool Construct::condition(void)
{
	return m_condition;
}

Construct& Construct::mark_start(const char* p)
{
	m_start = const_cast<char*>(p);
	return *this;
}

char* Construct::mark_start(void)
{
	return m_start;
}

Construct& Construct::mark_end(const char* p)
{
	m_end = const_cast<char*>(p);
	return *this;
}

char* Construct::mark_end(void)
{
	return m_end;
}
