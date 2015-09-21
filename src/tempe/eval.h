#pragma once
#include "interfaces.h"
#include <lightspeed/base/containers/arrayref.h>

namespace Tempe {

	Value fnEval(IExprEnvironment& env, ArrayRef<Value> values);


}