#include "Simhasher.h"
using namespace Simhash;
using namespace std;
uint64_t Simhasher::make(vector<pair<string,double> >& wordWeight) const   //根据词和权重获得64位的simhash
{
    vector<pair<uint64_t, double> > hashvalues;
    for(const pair<std::string,double>& w: wordWeight)
    {
        uint64_t hash = hasher_(w.first.c_str(),w.first.size(),0);
        double weight = w.second;
        hashvalues.push_back(make_pair(hash,weight));
    }

    vector<double> weights(BITS_LENGTH, 0.0);
    const uint64_t u64_1(1);
    for(size_t i = 0; i < hashvalues.size(); i++)
    {
        for(size_t j = 0; j < BITS_LENGTH; j++)
        {
            weights [j] += (((u64_1 << j) & hashvalues[i].first) ? 1: -1) * hashvalues[i].second;
        }
    }

    uint64_t v64 = 0;
    for(size_t j = 0; j < BITS_LENGTH; j++)
    {
        if(weights[j] > 0.0)
        {
            v64 |= (u64_1 << j);
        }
    }
    return v64;
}

bool Simhasher::isEqual(uint64_t lhs, uint64_t rhs, unsigned short n)
{
    unsigned short cnt = 0;
    lhs ^= rhs;
    while(lhs && cnt <= n)
    {
        lhs &= lhs - 1;
        cnt++;
    }
    if(cnt <= n)
    {
        return true;
    }
    return false;
}

void Simhasher::toBinaryString(uint64_t req, string& res)
{
    res.resize(64);
    for(signed i = 63; i >= 0; i--)
    {
        req & 1 ? res[i] = '1' : res[i] = '0';
        req >>= 1;
    }
}

uint64_t Simhasher::binaryStringToUint64(const string& bin)
{
    uint64_t res = 0;
    for(size_t i = 0; i < bin.size(); i++)
    {
        res <<= 1;
        if(bin[i] == '1')
        {
            res += 1;
        }
    }
    return res;
}


