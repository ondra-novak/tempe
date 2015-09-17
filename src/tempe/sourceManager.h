/*
 * includeManager.h
 *
 *  Created on: 17. 9. 2015
 *      Author: ondra
 */

#ifndef TEMPE_INCLUDEMANAGER_H_
#define TEMPE_INCLUDEMANAGER_H_

namespace Tempe {

class SourceManager {
public:

	virtual PExprNode compile(ConstStrA scriptName);

	virtual ~AbstractIncludeManager() {}

};


}




#endif /* TEMPE_INCLUDEMANAGER_H_ */
