#include "cpuRun.h"

int main() {
	CPU myCPU;
    myCPU.inState();
	
	Json arr = Json::array();

	while(myCPU.STAT == SAOK) {
		myCPU.fetch();
		myCPU.decode();
		myCPU.execute();
		myCPU.memory();
		myCPU.writeback();
		myCPU.updatePC();
		Json out = myCPU.outState();
		arr.push_back(out);
	}
	std::cout << std::setw(4) << arr << std::endl;
	return 0;
}
