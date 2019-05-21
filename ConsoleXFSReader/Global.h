#pragma once

#include <iostream>
#include <wtypes.h>
#include <vector>

using namespace std;
typedef vector<BYTE> BinaryBlock;
// Итератор
//---------------------------------------------------------------------------
template<class Type>
class Iterator
{
protected:
	Iterator() {}

public:
	virtual ~Iterator() {}
	virtual void First() = 0;
	virtual void Next() = 0;
	virtual bool IsDone() const = 0;
	virtual Type GetCurrent() const = 0;
};
