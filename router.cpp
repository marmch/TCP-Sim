using namespace std;


#include "host.h"
#include "node.h"
#include "link.h"
#include "packet.h"
#include "router.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <locale> 
#include <cstdlib>
#include <queue>
#include <new>
#include <climits>
#define INF INT_MAX
extern vector<link> linkVector;
extern vector<packet> packetVector;
extern vector<router> routerVector;
//extern vector<node> nodeVector;
extern void popTimeout(int timeoutIndex);
extern void pushEvent(string e, int elapseTime);
extern vector<flow> flowVector;

router::router(string name, int ip)
:node::node(name,ip){
	this->ip = 1000 + ip;
	//host_ip=-1;		
	field f;
	f.link_id = -1;	//whatever
	f.type = 2;		//self
	f.ip = this->ip;
	f.isCTS = false;
	lVector.push_back(f);
	//discovered = false;
	rt.rname = this->name;
	rt.ip = this->ip;
	//isDefaultGateway = false;
	isCTS = false;
	routersConnected = false;
	STATE=0;

	//test code
	//rtHardCode();

}

void router::addLink(int id) {
	cout<<"adding link to router"<<endl;
		field f;
		f.link_id = id;
		f.type = -1; 	//unknown
		f.ip = -1;		//unknown
		f.isCTS = false;
		lVector.push_back(f);
}

void router::printLinks(){
	cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" lvector: "<<endl;
	cout << "{l_id,\ttype,\tip,\tisCTS}" << endl;
	for(int i=0;i<(int)lVector.size();i++){
		cout<<"{";

		if(lVector[i].link_id==-1){
			cout<<"?"<<",\t";
		}else{
			cout<<"L"<<lVector[i].link_id<<",\t";
		}

		if(lVector[i].type==0){
			cout<<"H"<<",\t";	
		}
		else if(lVector[i].type ==1){
			cout<<"R"<<",\t";
		}
		else if(lVector[i].type==-1){
			cout<<"?"<<",\t";
		}
		else if(lVector[i].type==2){
			cout<<"slf"<<",\t";
		}
		cout<<lVector[i].ip<<",\t";
		cout<<lVector[i].isCTS<<"},"<<endl;
	}
}

void router::sendCR(int i,link* link_ptr){
		packet pSend(0, 0, this, this);
		pSend.isCR = true;
		pSend.t = i;
		packetVector.push_back(pSend);
		pushPacket((int)packetVector.size() - 1,link_ptr);
}

void router::crResp(link* link_ptr,packet* p){
	cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" responding to: ";
	cout<<"CR"<<p->t<<endl;
	if(p->t==0){
		//update lVector
		lVectorUpdate(link_ptr,p);
		//printLinks();
		//cin.ignore();
		for(int i=0;i<(int)lVector.size();i++){
			if(lVector[i].type != 2){
				link* lptr = &linkVector[lVector[i].link_id];
				if(lptr != link_ptr && STATE == 0){
					cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" sending: "<<"CR"<< 0<<" on link "<<lptr->id<<endl;
					sendCR(0,lptr);
				}
				else{
					cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" sending: "<<"CR"<< 1<<" on link "<<lptr->id<<endl;
					sendCR(1,lptr);
				}	
			}	
		}
	}else{
		lVectorUpdate(link_ptr,p);
		STATE = 1;
		for (int i = 0; i < (int)lVector.size(); i++) {
			if (lVector[i].type == -1)
				STATE = 0;
		}
	}
	//printLinks();
}


void router::b_dVec(){
	for(int i=1;i<(int)lVector.size();i++){
		link* lptr = &linkVector[lVector[i].link_id];
		if (lVector[i].type == 1){
			cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" broadcasting new dVec on link: "<< lptr->id <<endl;
			packet pSend(0, (int)packetVector.size(), this, this);
			pSend.isRIP = true;
			pSend.dv = *(rt.getDv(this->ip));
			//pSend.dv.print();
			packetVector.push_back(pSend);
			pushPacket((int)packetVector.size() - 1,lptr);	
		}
	}

}

void router::spamUpdate() {
	cout << "==============================================================SPAMMING UPDATE ROUTER " << rvID << endl;
	bool done = true;
	for (int i = 0; i < flowVector.size(); i++) {
		if (!flowVector[i].done)
			done = false;
	}
	if (!done) {
		stringstream ss;
		ss << rvID;
		string e = "ROUTER_" + ss.str() + "_UPDATE";
		pushEvent(e, UPDATE_TIMER);
		rtInit();
	}
}

void router::p_dVec(packet* p){
	cout<<"ip_except: "<<p->src->ip<<endl;
	link* link_ptr = nullptr;
	for(int i=0;i<(int)lVector.size();i++){
		if( lVector[i].ip  == p->src->ip){
			link_ptr = &linkVector[lVector[i].link_id];
		}
	}
	//there's no neighbor!!!!!!!!!!!!!!!
	for(int i=1;i<(int)lVector.size();i++){
		link* lptr = &linkVector[lVector[i].link_id];
		if ((lVector[i].type == 1)&&(lptr->id != link_ptr->id)){
			cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" propagating new dVec on link : "<< lptr->id <<endl;
			packet pSend(0,  (int)packetVector.size(), this, this);
			pSend.isRIP = true;
			//cout<<"\n\n\nsource ip is: "<<p->src->ip<<"\n\n\n"<<endl;
			pSend.dv = *(rt.getDv(p->dv.ip));
			pSend.dv.print();
			packetVector.push_back(pSend);
			pushPacket((int)packetVector.size() - 1,lptr);	
		}
	}
}

void router::b_CTS_routers(){
	cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" is almost ready to send! broadcasting CTS to routers";
	for(int i=0;i<(int)lVector.size();i++){
				if(lVector[i].type==1){
					cout<<" on L"<<lVector[i].link_id<<" ";
					link* myLink_ptr = &linkVector[lVector[i].link_id];
					packet pSend(0, 0, this, this);
					pSend.isCTS = true;
					packetVector.push_back(pSend);
					pushPacket((int)packetVector.size() - 1,myLink_ptr);	
				}
			}
			cout<<endl;
}

void router::b_CTS_hosts(){
	cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" is ready to send!  Sending CTS to hosts"<<endl;
	for(int i=0;i<(int)lVector.size();i++){
		if(lVector[i].type==0){
			link* myLink_ptr = &linkVector[lVector[i].link_id];
			packet pSend(0, 0, this, this);
			pSend.isCTS = true;
			packetVector.push_back(pSend);
			pushPacket((int)packetVector.size() - 1,myLink_ptr);	
		}
	}
}

void router::recRIP(packet* p){
	cout<<"packet is a RIP"<<endl;
			//cout<<"its dv is"<<endl;
			p->dv.print();
			//cin.ignore();
			int n = rt.update(&(p->dv));
			if((n==1)||(n==2)){
				//cout<<"forwarding dv change"<<endl;
				p_dVec(p);	
			}
			//send out OUR changed dv (by bellman ford).
			if(n==2){
				//cout<<"broadcasting MY change"<<endl;
				b_dVec();
			}
			if((n==1)||(n==2)){
				rt.print();
			}
}

void router::recCTS(packet* p){
			for(int i=0;i<(int)lVector.size();i++){
				if((lVector[i].type == 1)&&(lVector[i].ip == p->src->ip)){
					lVector[i].isCTS = true;
				}
			}
}

void router::receivePacket(link* link_ptr) {
	//cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" has recieved packet:"<<endl;
	packet* p = &packetVector[link_ptr->pnum];
	//cout<<"packet recieved info: "<<endl;
	//cout<<"pnum: "<<p->num<<endl;
	//cout<<"pnum: "<<link_ptr->pnum<<endl;
	int tnum = link_ptr->pnum;
	link_ptr->pnum = -1;
	if(STATE==0){ //Links Unknown
		if(p->isCR){
			crResp(link_ptr,p);
			if(discoveryComplete()){
				cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" has completed discovery:"<<endl;
				printLinks();
				STATE=1;
				cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" initial routing table:"<<endl;
				rtInit();
				//cin.ignore();
				if(RIPbuf.empty()){	
					cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" empty RIPbuf:"<<endl;
					//inform(1,this->ip,link_ptr);
					//we've created an empty routing table. we have to send out our own distance vector EVERYWHERE.
					b_dVec();
				}
				else{
					cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" nonEmpty RIPbuf:"<<endl;
				
					int k=0;
					//int ipRet=-1;
					while(!RIPbuf.empty()){
						//cout<<"front is: "<<RIPbuf.front()<<endl;
						packet* pp = &packetVector[RIPbuf.front()];
						//cout<<"pnum is: "<< pp->num <<endl;
						int n = rt.update(&pp->dv);
						//cout<<"inform "<<n<<endl;
						//get ip from RIPbuf
						RIPbuf.pop();
						if (n>k){
							k = n;
						}
						b_dVec();
					}
					//cin.ignore();
					rt.print();
				}		
				STATE=1;
				if(rt.isComplete()){
					STATE=2;
					cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" routing table is complete:"<<endl;
					cout<<"final routing table:"<<endl;
					rt.print();
					//inform(3,p->src->ip,link_ptr);
					b_dVec();
					cout<<this->name<<"STATE: "<<STATE<<endl;
					b_CTS_routers();
				}
				if(!routersConnected){
					cout<<"no routers connected!"<<endl;
					if(rt.isComplete()){
						cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" routing table is complete:"<<endl;
						cout<<"final routing table:"<<endl;
						rt.print();
						STATE=2;
						b_CTS_hosts();
						this->isCTS = true;
					}
				}
			}
		}
		else if (p->isRIP){
			cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" is buffering DVEC packet:"<<endl;
			p->dv.print();
			RIPbuf.push(p->num);
		}
		else if(p->isCTS){
			cout<<"packet is CTS"<<endl;
			recCTS(p);
		}
	}
	else if(STATE==1){ //building routing table
		cout<<this->name<<" state=1"<<endl;
		
		if(p->isRIP){
			recRIP(p);
		}
		else if(p->isCR){
			//we sent to a router that wasn't ready for RIP yet
			crResp(link_ptr,p);
		}
		else if(p->isCTS){
			recCTS(p);
		}
		if(rt.isComplete()){
			STATE=2;
			cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" routing table is complete:"<<endl;
			cout<<"final routing table:"<<endl;
			rt.print();
			//inform(3,p->src->ip,link_ptr);
			b_dVec();
			cout<<this->name<<"STATE: "<<STATE<<endl;
			b_CTS_routers();
			spamUpdate();
		}
	}
	else if(STATE==2){ //routing table complete, waiting on CTS from other routers!
		//cout<<this->name<<" state=2"<<endl;
		if(p->isRIP){
			recRIP(p);
		}
		else if(p->isCR){
			crResp(link_ptr,p);
		}
		else if(p->isCTS){
			recCTS(p);
			if(!this->isCTS){
				isCTS = clearToSend();
				if(this->isCTS){
					b_CTS_hosts();
				}
			}
		}
		if((p->f != nullptr)&&(this->isCTS)){
			//cout<<"data packet recieved on "<< this->name <<" choosing link"<<endl;
			link* l = chooseLink(p);
			//cout<<"link "<<l->id<<" chosen"<<endl;
			if(l){
				pushPacket(tnum,l);
			}else{
				cout<<this->name<<" STATE: "<<this->STATE<<" :"<<" chooseLink failure"<<endl;
			}
		}
		
	}
}

bool router::clearToSend(){
	for(int i=0;i<(int)lVector.size();i++){
		if((lVector[i].type==1)&&(!lVector[i].isCTS)){
			return false;
		}
	}
	return true;
}

void router::lVectorUpdate(link* link_ptr,packet* p){
	for(int i=0;i<(int)lVector.size();i++){
		if(lVector[i].link_id == link_ptr->id){
			if(p->src->name[0] == 'R'){
				lVector[i].type = 1;
				lVector[i].ip = p->src->ip;
				routersConnected = true;
				//	cout<<"router connected!"<<endl;
			}
			else if(p->src->name[0] == 'H'){
				lVector[i].type = 0;
				lVector[i].ip = p->src->ip;
				//isDefaultGateway = true;
			}
		}
	}
}

//generate distance vector from 
dVec* router::genDv(dVec& dv){
	dv.ip = -1;
	for(int i=0;i<(int)lVector.size();i++){
		if(lVector[i].type == 1){ //router
			link* link_ptr = &linkVector[lVector[i].link_id];
			dVec::entry ent; 
			ent.cost = link_ptr->cost;
			ent.ip = lVector[i].ip;
			dv.e.push_back(ent);
		}
		else if(lVector[i].type == 0){ //host
			dv.h.push_back(lVector[i].ip);
		}else if(lVector[i].type == -1){ //unknown
			//shouldn't occur
		}else if(lVector[i].type == 2){ //self
			dVec::entry s;
			s.ip = lVector[i].ip;
			s.cost = 0;
			dv.ip=lVector[i].ip;
			dv.e.push_back(s);
		}
	}
	return &dv;
}

bool router::discoveryComplete(){
	for(int i=0;i<(int)lVector.size();i++){
		if(lVector[i].type==-1){
			return false;
		}
	}
	return true;
}


link* router::chooseLink(packet* p){
	//int inf = std::numeric_limits<int>::max();
	//IF DEST CORRESPONDS TO CONNECTED HOST
	for(int i=0; i<(int)lVector.size();i++){
		if((lVector[i].type == 0)&&(lVector[i].ip == p->dest->ip)){
			return &linkVector[lVector[i].link_id];

		}
	}
	//ELSE PICK LEAST-COST NEXT STEP, GIVEN DESTINATION IP
	link* min_link=NULL;
	int min_step=INF;
	//if the destination of the packet corresponds to a default gateway router of any router in the table
	//determine the least-cost path to that router


	for(int i=0;i<(int)rt.dvv.size();i++){ 
		for(int j=0;j<(int)rt.dvv[i].h.size();j++){
			if(rt.dvv[i].h[j] == p->dest->ip){
				//therefore we want to get to rt.dvv[i].ip
				for(int k=0; k < (int)lVector.size();k++){
					if(lVector[k].type == 1){
						int step_cost = rt.getCost(this->ip,lVector[k].ip) + rt.getCost(lVector[k].ip,rt.dvv[i].ip);
						if (step_cost < 0) {
							step_cost = INF;
						}
						if(step_cost<min_step){
							min_link= &linkVector[lVector[k].link_id];
							min_step = step_cost;
						}
					}
				}
			} 
		}		
	}
	return min_link;
}

/*
void router::sendDVec(int ip_from){
	for(int i=0;i<(int)lVector.size();i++){
		if(lVector[i].type != 2){
			link* link_ptr = &linkVector[lVector[i].link_id];
			int size= 1;
			int num= -1;
			packet p(size, num, this, this);
			p.isRIP = true;
			dVec* dv = rt.getDv(ip_from);
			packetVector.push_back(p);
			pushPacket(packetVector.size()-1,link_ptr);
		}
	}
}

*/

//assumes LinkVector is fully updated.
void router::rtInit(){
	dVec dv;
	cout<<"creating routing table "<<endl;
	rt.update(genDv(dv));
	rt.print();
}