#include <iostream>
#include <fstream>
#include "trans.h"
#include "Y86.h"

int main() {
	CPU myCPU;
    myCPU.inState();
	
	Json arr = Json::array();
	Json ins = Json::array();
	Json total = Json::array();

	while(myCPU.STAT == SAOK) {
		myCPU.fetch();
		Json insOut = myCPU.outIns();
		ins.push_back(insOut);
		myCPU.decode();
		myCPU.execute();
		myCPU.memory();
		myCPU.writeback();
		myCPU.updatePC();
		Json out = myCPU.outState();
		arr.push_back(out);
	}

	total.push_back(arr);
	total.push_back(ins);
	std::cout << std::setw(4) << total << std::endl;

	return 0;
}
