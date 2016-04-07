#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <queue> //for std::priority_queue
using namespace std;
#include "link.h"

//GLOBAL VARIABLES for time management
std::priority_queue<string> q;
int time = 0;
string eventlog = "";
vector<link> linkVector;

// Display an ASCII View of how the network should look
void printNetwork()
{
	//
}

// Add a host
void createHost(string hostName){
	//implement later
}

// Add a router
void createRouter(string routerName){
	//implement later
}

// Add a link from a given host/router to a new Router
void createLink(string linkName, string hostA, string hostB, int a, int b, int c){
	//implement later
}

// Create flows
void createFlow(string flowName, string hostA, string hostB, int a, int b){
	//implement later
}

// Run through and record data on the Network
void SimulateNetwork(){
	
}

//Expects an event in the form of a string with standardized format, as well as time to elapse for the event
//Adds the "added time" and "time to execute" onto the start of string for use as the comparable to order events
//An example of a working format for e is LINK_0_SEND_PACKET or HOST_1_RECEIVE_PACKET
void pushEvent(string e, int elapseTime){
	int currentTime = time;
	int executeTime = time + elapseTime;
	string event = executeTime + "," + currentTime;
	event += "," + e; //csv for simplicity
	q.push(event);
}

//Pops an event from the event queue and executes the appropriate command, as contained in e and assuming correct format of e
//Pushes data onto a log for output
void popEvent(){
	if(q.empty()){
		cout << "ERROR: The event queue is empty! Cannot pop event!";
		return;
	}
	string event = q.top();
	q.pop();
	
	//Extract the original event message from the expanded event message
	int find = event.find_last_of(","); //right after the last comma lies the original event message
	string e = event.substr(find+1);
	
	//Extract the original event message's meaning
	find = e.find("_");
	string objectType = e.substr(0,find);
	
	e = e.substr(find+1); //gets the rest of the message
	find = e.find("_"); 
	string objectIndex = e.substr(0,find);
	
	string function = e.substr(find+1);
	
	//Execute the event in the event e that was initially input into pushEvent
	if(objectType == "LINK"){
		int index = stoi(objectIndex);
		if (function == "TRANSMIT_PACKET"){
			//linkVector[index].recievePacket();	
			linkVector[index].tpropagate();
		}
	}
	
	eventlog += "\n" + event; //add the event to the log
}

int main(int argc, char *argv[])
{
	cout<<"Welcome to the Network Simulator!"<<endl;
	string ln, file, temp;
	ifstream read;
	cout<<"Input the filename: ";
	cin>>file;
	read.open(file.c_str());
	while(!read.eof())
	{
		getline(read,ln);
		// if(ln!="")		
			// cout<<"reading "<<ln<<endl;
		if(ln=="Hosts:"&&!read.eof()){
			getline(read,ln);
			istringstream iss(ln);
			int numHost; iss>>numHost;
			for(int i = 0; i<numHost&&iss; i++){
				iss>>temp;
				createHost(temp);
				// cout<<"Added Host"<<endl;
			}
		}
		else if(ln=="Routers:"&&!read.eof()){
			getline(read,ln);
			istringstream iss(ln);
			int numRout; iss>>numRout;
			for(int i = 0; i<numRout&&iss; i++){
				iss>>temp;
				createRouter(temp);
				// cout<<"Added Router"<<endl;
			}
		}
		else if(ln=="Links:"&&!read.eof()){
			getline(read,ln);
			istringstream iss(ln);
			int numLink; iss>>numLink;
			for(int i = 0; i<numLink&&iss; i++){
				iss>>temp;
				string h1; iss>>h1;
				string h2; iss>>h2;
				int a; int b; int c; iss>>a; iss>>b; iss>>c;
				createLink(temp,h1,h2,a,b,c);
				// cout<<"Added Link"<<endl;
			}
		}
		else if(ln=="Flows:"&&!read.eof()){
			getline(read,ln);
			istringstream iss(ln);
			int numFlow; iss>>numFlow;
			for(int i = 0; i<numFlow&&iss; i++){
				iss>>temp;
				string h1; iss>>h1;
				string h2; iss>>h2;
				int a; int b; iss>>a; iss>>b;
				createFlow(temp,h1,h2,a,b);				
				// cout<<"Added Flow"<<endl;
			}
		}
	}
	SimulateNetwork();
	return 0;
}
