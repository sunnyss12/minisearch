#ifndef SIMHASH_SIMHASHER_HPP
#define SIMHASH_SIMHASHER_HPP

#include "hashes/jenkins.h"
#include <vector>
#include <string>


namespace Simhash
{
    class Simhasher
    {
        private:
            enum{BITS_LENGTH = 64};
            jenkins hasher_;
        public:
            Simhasher(){}
            ~Simhasher(){}
        public:
            uint64_t make(std::vector<std::pair<std::string,double> >& wordWeight)const;
            static bool isEqual(uint64_t lhs, uint64_t rhs, unsigned short n = 3);
            static void toBinaryString(uint64_t req, std::string& res);
            static uint64_t binaryStringToUint64(const std::string& bin);

    };
}
#endif

