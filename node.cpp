#include <sstream>
#include <string>
using namespace std;
#include "host.h"
#include "node.h"
#include "link.h"
#include <iostream>
// The packet constructor initializes the packet with set information of data and destination. 
//node::node(int ip) {
//	ip_addr = ip;
//}

//void node::sendPacket(node* dest, int size){
//	return;
//}

node::node(string name, int ip){
	//this->ip = ip;
	this->name = name;
	br=100;
}

//general packet transmission function
//pushes to the queue of the link
void node::pushPacket(int pnum,link* link_ptr) {
	link_ptr->prepPropagate(this, pnum);
}


string node::toString() {
stringstream ss;
	return "";
}





