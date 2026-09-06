#include "rQuery/BinaryQuery.h"

#include <string>

std::string BinaryQuery::rep() const {
    return "(" + lhs_.rep() + 
           " " + op_sym_ + 
           " " + rhs_.rep() + ")";
}
