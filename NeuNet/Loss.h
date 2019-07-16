#pragma once
#include <vector>
class KeyValue
{
public:
	unsigned long int Key;
	long double Value;
	KeyValue(unsigned long int key, long double value);
};
KeyValue::KeyValue(unsigned long int key, long double value)
{
	Key = key;
	Value = value;
}

class LossVector
{
public:
	std::vector<KeyValue> Losses;
	long double GetLossesSumForLabel(int label);
	std::vector<KeyValue> GetLossesSumedVector();
	unsigned long int GetMinLossKey();
	LossVector();
	~LossVector();

private:

};

inline long double LossVector::GetLossesSumForLabel(int label)
{
	long double retval = 0.0L;
	for (unsigned long int i = 0; i < Losses.size(); i++)
		if (Losses[i].Key == label)
			retval += Losses[i].Value;
	return retval;
}

inline std::vector<KeyValue> LossVector::GetLossesSumedVector()
{
	std::vector<KeyValue> t;
	bool keyExists = false;
	unsigned long int key = 0;
	for (unsigned long int i = 0; i < Losses.size(); i++)
	{
		for (unsigned long int j = 0; j < t.size(); j++)
		{
			if (t[j].Key == Losses[i].Key)
			{
				keyExists = true;
				key = Losses[i].Key;
			}
		}
		if (!keyExists)
		{
			t.push_back(*new KeyValue(key, 0.0L));
		}
		keyExists = false;
		key = 0;
	}

	for (unsigned long int i = 0; i < t.size(); i++)
		t[i].Value = GetLossesSumForLabel(t[i].Key);
	return t;
}

inline unsigned long int LossVector::GetMinLossKey()
{
	std::vector<KeyValue> h = GetLossesSumedVector();
	long double min = h[0].Value;
	unsigned long int minKey = 0;
	for (unsigned long int i = 0; i < h.size(); i++)
	{
		if (h[i].Value < min)
		{
			min = h[i].Value;
			minKey = h[i].Key;
		}
	}
	return minKey;
}

LossVector::LossVector()
{
}

LossVector::~LossVector()
{
}