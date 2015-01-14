#include <stdio.h>
#include "EngineImpl.h"
#include <map>
#include <string>

using namespace AE;

int main (int argc, char ** argv)
{
	action_params params_map;
	params_map["a"] = AE::variant("b");
	params_map["c"] = AE::variant("d");
	params_map["e"] = AE::variant("f");
	params_map["g"] = AE::variant("h");

	Engine *e = new EngineImpl();
	e->begin();
	e->addAction(params_map);
	e->end();

	printf("main start\n");
	return 0;
}
