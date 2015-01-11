#include <stdio.h>
#include "EngineImpl.h"
#include <map>
#include <string>

using namespace AE;

int main (int argc, char ** argv)
{
	action_params params_map;
	params_map["a"] = "b";
	params_map["c"] = "d";
	params_map["e"] = "f";
	params_map["g"] = "h";

	Engine *e = new EngineImpl();
	e->begin();
	e->addAction(params_map);
	e->end();

	printf("main start\n");
	return 0;
}
