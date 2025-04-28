#pragma once

#include "InternalTypes.hpp"
#include "Constants.hpp"

#include <cmath>

namespace nsf
{

inline int sequenceGreaterThan(SequenceNumber _s1, SequenceNumber _s2)
{
    static const SequenceNumber HALF_MAX_SEQUENCE_NUMBER = static_cast<SequenceNumber>(std::ceil(MAX_SEQUENCE_NUMBER / 2.f));

    return (( _s1 > _s2 ) && ( _s1 - _s2 <= HALF_MAX_SEQUENCE_NUMBER )) || 
           (( _s1 < _s2 ) && ( _s2 - _s1  > HALF_MAX_SEQUENCE_NUMBER ));
}

inline int sequenceEqualOrGreaterThan(SequenceNumber _s1, SequenceNumber _s2)
{
    return _s1 == _s2 || sequenceGreaterThan(_s1, _s2);
}

} // namespace nsf
