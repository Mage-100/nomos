#pragma once

#include <string>

struct Entry {
	int row;
	int col;
	double value;
};

enum class DatasetType
{
    Afiro,
    Agg,
    Agg2,
    Agg3,
    Bandm,
    Bnl1,
    Bnl2,
};