/*
 * Main.h
 *
 *  Created on: 6.7.2012
 *      Author: ondra
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <lightspeed/base/framework/app.h>

#include <tempe/interfaces.h>

namespace TempeCli {

using namespace LightSpeed;
using namespace Tempe;

class Main: public App {
public:
	virtual integer start(const Args &args) ;
};

} /* namespace TempeTest */
#endif /* MAIN_H_ */
