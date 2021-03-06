/*
author = 'JBaig'
*/

#include "cluster.h"
using namespace std;

class Db{
public:
    static Db& inst()
    {
        static Db instance;
        if (instance.init != true) {
          cout<<"Cluster  Controller is starting..."<<'\n';   
           
          cout<<"I received a message from Cloud Contoller so let me interpret it."<<'\n';
          instance.init = true;
          instance.prev_alloc = -1;
          }
             
          return instance;
    }
    int prev_alloc;
private:
   
    bool init;
    
};





std::string cluster::createVM(const std::string& text){
    RestClient::init();

    string s(text);
    ostringstream sp;
    sp << "{\"text\":\"" ;
    sp << s; 
    sp << "\"}";
    cout<<sp.str()<<'\n';
    RestClient::Response r = RestClient::post("http://localhost:10002/node/createVM","application/json",sp.str());
    std::cout << r.body << '\n';
    return "VM created as you asked me by sending me required info";
  }




std::string cluster::createVMreq(int numVMs,int vcpu,int memory){
    //RestClient::init();
    vector <int> allavailvcpus;
    vector <int> allavailmemories;
    vector <string> allnodeuris;
    //
    ifstream file("my_ncs.txt");
    string str; 
    while (std::getline(file, str))
    {
       allnodeuris.push_back(str); 
    }
    int i;
    //
    for(i=0 ; i < allnodeuris.size();i++)
    {
      cout<<"Node Conroller"<<'\n';
      RestClient::Response r = RestClient::get("http://"+ allnodeuris[i] +"/node/getavailResources");
      std::cout << r.body<< '\n';
      //std::cout << r.body<< '\n';
      string s(r.body);
      vector<string> stringvec; 
      int avcpu;
      int avmemory;
      stringstream aa(s);
      string sub;

      while (getline(aa, sub, ':')) {
      stringvec.push_back(sub);
      }

      cout<<stringvec[2]<<endl;
      cout<<stringvec[3]<<endl;

      stringstream ab(stringvec[2]);
      getline(ab,sub,',');
      //cout<<sub<<'\n';
      stringstream(sub)>>avcpu;
      cout<<"my available vcpus : "<<avcpu<<'\n';


      stringstream ac(stringvec[3]);
      getline(ac,sub,'}');
      //cout<<sub<<'\n';
      stringstream(sub)>>avmemory;
      cout<<"my available memory: "<<avmemory<<'\n';

      allavailvcpus.push_back(avcpu);
      allavailmemories.push_back(avmemory);
    }
    
    cout<<"here are all the ncs information."<<'\n';
    for(i=0;i<allnodeuris.size();i++){
      cout<<allavailvcpus[i]<<'\n';
      cout<<allavailmemories[i]<<'\n';
    }
    cout << "Got the info from NC";
    //Total requirement.
    int total_vcpu_req = numVMs * vcpu;
    int total_mem_req = numVMs * memory;
    cout<<"total vcpu requirement : "<<total_vcpu_req<<'\n';
    cout<<" total memory requirement : "<<total_mem_req<<'\n';
    //what did we get from node controllers.
    int vcpu_we_got=0;
    long long memory_we_got=0;
    //cout<<"here are all the ncs information."<<'\n';
    for(i=0;i < allnodeuris.size();i++){
      vcpu_we_got = vcpu_we_got + allavailvcpus[i] ;
      memory_we_got = memory_we_got + allavailmemories[i] ;
    }
    cout<<"vcpu we got : "<<vcpu_we_got<<'\n';
    cout<<"memory we got : "<<memory_we_got<<'\n';
    //Check for error.
    if(vcpu_we_got < total_vcpu_req || memory_we_got < total_mem_req)
      return "Sorry, requirements can't be fulfilled using this availability zone.";
    //Greedy algorithm Starts.
    cout<<"Greedy Algorithm starts here."<<'\n';
    int flag,num,actual_num=0;//,reqmemory=1;
    int numVMs_left = numVMs;
    cout<<"numVMs_left " <<numVMs_left<<'\n';
    for(i = 0 ; i < allnodeuris.size() ; i++){
      if(numVMs_left==0) break;
      flag=1, num=1;
      while(flag){
         if( (num<= numVMs_left) && ((num * vcpu) <= allavailvcpus[i]) && ((num * memory) <= allavailmemories[i]))
             {num++; cout<<"num "<<num<<'\n'; cout<<"num vm left "<<numVMs_left<<'\n';}
         else
          flag=0;
      }
      cout<<"while loop over.\n";
      actual_num = num-1;
      numVMs_left = numVMs_left - actual_num;
      ostringstream sp;
      sp << "{\"numVMs\":" ;
      sp << actual_num; 
      sp << ",\"vcpu\":";
      sp << vcpu;
      sp << ",\"memory\":";
      sp << memory;
      sp <<"}";
      cout<<sp.str()<<'\n';
      RestClient::Response r = RestClient::post("http://"+ allnodeuris[i] +"/node/createVMreq","application/json",sp.str());
      std::cout << r.body << '\n';
    }
    cout<<"Greedy Algorithm Ends Here."<<'\n';
    //Greedy Algorithm ends here.
    //Round Robin algorithm Starts.
    /*cout<<"Round Robin Algorithm starts.";
    int numVMsRR = numVMs;
    for(i = (Db::inst().prev_alloc + 1) % allnodeuris.size(); numVMsRR > 0; i = (i+1)% allnodeuris.size()){
      if(vcpu <= allavailvcpus[i] && memory <= allavailmemories[i]){
        ostringstream sp;
        sp << "{\"numVMs\":" ;
        sp << 1 ;
        sp << ",\"vcpu\":";
        sp << vcpu;
        sp << ",\"memory\":";
        sp << memory;
        sp <<"}";
        cout<<sp.str()<<'\n';
        cout<< " creating virtual machine at "<< allnodeuris[i] <<'\n';
        ostringstream spa;
        spa <<"http://" ;
        spa << allnodeuris[i] ;
        spa << "/node/createVMreq";
        cout<<spa.str()<<'\n';
        RestClient::Response r = RestClient::post(spa.str(),"application/json",sp.str());
        std::cout << r.body << '\n';
        cout<< "prev alloc : "<<Db::inst().prev_alloc<<'\n';
        Db::inst().prev_alloc = i;
        numVMsRR--;
      }
    }
    cout<<"Round Robin ends here.\n";*/
    //Match Making Starts here.
    /*cout<<"Match Making Algorithm starts.";
    //Eliminating nodes which cant accomodate atleast one virtual machine and keeping only eligible ones.
    vector <int> allavailmemories_MM;
    vector <int> allavailvcpus_MM;
    vector <string> allnodeuris_MM;
    for(i=0;i<allnodeuris.size();i++){
      if(allavailvcpus[i] >= vcpu && allavailmemories[i] >= memory){
        allnodeuris_MM.push_back(allnodeuris[i]);
        allavailvcpus_MM.push_back(allavailvcpus[i]);
        allavailmemories_MM.push_back(allavailmemories[i]);
      }
    }
    //Ranking based on VCPU requirement.
    cout<<"Ranking based on VCPU requirement.";
    int j,n,loc,temp,tempb,min,k;
    n = allnodeuris_MM.size();
    for(i=0;i<n-1;i++)
    {
        min=allavailvcpus_MM[i];
        loc=i;
        for(j=i+1;j<n;j++)
        {
            if(min>allavailvcpus_MM[j])
            {
                min=allavailvcpus_MM[j];
                loc=j;
            }
        }
 
        temp=allavailvcpus_MM[i];
        allavailvcpus_MM[i]=allavailvcpus_MM[loc];
        allavailvcpus_MM[loc]=temp;

        tempb=allavailmemories_MM[i];
        allavailmemories_MM[i]=allavailmemories_MM[loc];
        allavailmemories_MM[loc]=tempb;
      
        const char * s1 = allnodeuris_MM[i].c_str();
        const char * s2 = allnodeuris_MM[loc].c_str();
        const char **str1_ptr = &s1;
        const char **str2_ptr = &s2;
        const char *tempc = *str1_ptr;
        *str1_ptr = *str2_ptr;
        *str2_ptr = tempc;
        string t1(s1);
        string t2(s2);
        allnodeuris_MM[i].replace(allnodeuris_MM[i].begin(),allnodeuris_MM[i].end(),t1);
        allnodeuris_MM[loc].replace(allnodeuris_MM[loc].begin(),allnodeuris_MM[loc].end(),t2);
        
    }
    //printing
    cout<<"\nSorted list is as follows\n";
    for(i=0;i<n;i++)
    {
        cout<<allnodeuris_MM[i]<<" ";
    }
    cout<<"\n";    
    for(i=0;i<n;i++)
    {
        cout<<allavailvcpus_MM[i]<<" ";
    }
    cout<<"\n";
        for(i=0;i<n;i++)
    {
        cout<<allavailmemories_MM[i]<<" ";
    }
    cout<<"\n";
    //2nd level sorting.
    int x,y;
    for(i=0;i<n;i++)
    {
      x=i;
      while(i<=n-1 && allavailvcpus_MM[i]==allavailvcpus_MM[i+1])
      i++;
      if(x==i) continue;
      y=i;
      for(j=x;j<=y-1;j++)
      {
        min=allavailmemories_MM[j];
        loc=j;
        for(k=j+1;k<=y;k++)
        {
            if(min>allavailmemories_MM[k])
            {
                min=allavailmemories_MM[k];
                loc=k;
            }
        }
 
        temp=allavailmemories_MM[j];
        allavailmemories_MM[j]=allavailmemories_MM[loc];
        allavailmemories_MM[loc]=temp;
        
        const char * s1 = allnodeuris_MM[j].c_str();
        const char * s2 = allnodeuris_MM[loc].c_str();
        const char **str1_ptr = &s1;
        const char **str2_ptr = &s2;
        const char *tempc = *str1_ptr;
        *str1_ptr = *str2_ptr;
        *str2_ptr = tempc;
        string t1(s1);
        string t2(s2);
        allnodeuris_MM[j].replace(allnodeuris_MM[j].begin(),allnodeuris_MM[j].end(),t1);
        allnodeuris_MM[loc].replace(allnodeuris_MM[loc].begin(),allnodeuris_MM[loc].end(),t2);
      }
    }
 
    cout<<"\nSorted list is as follows\n";
    for(i=0;i<n;i++)
    {
        cout<<allnodeuris_MM[i]<<" ";
    }
    cout<<"\n";
    for(i=0;i<n;i++)
    {
        cout<<allavailvcpus_MM[i]<<" ";
    }
    cout<<"\n";
        for(i=0;i<n;i++)
    {
        cout<<allavailmemories_MM[i]<<" ";
    }
    
    cout<<"\n";
    cout<<"Ranking Process completes here.";
    //Ranking Process completes here.
    cout<<"Greedy Match Making starts here.";
    //Greedy Starts from here.
    int flagm ,numm, actual_numm = 0, numVMs_leftm = numVMs;
    for(i = 0 ; i < allnodeuris_MM.size() ; i++){
      if(numVMs_leftm==0) break;
      flagm=1,numm=1;
      while(flagm){
         if( numm<= numVMs_leftm && (numm * vcpu) <= allavailvcpus_MM[i] && (numm * memory) <= allavailmemories_MM[i])
            numm++;
         else
          flagm=0;
      }
      actual_numm = numm-1;
      numVMs_leftm = actual_numm -numVMs_leftm;
      ostringstream spm,spmm;
      spm << "{\"numVMs\":" ;
      spm << actual_numm; 
      spm << ",\"vcpu\":";
      spm << vcpu;
      spm << ",\"memory\":";
      spm << memory;
      spm <<"}";
      cout<<spm.str()<<'\n';
      spmm << allnodeuris_MM[i];
      cout<<spmm.str()<<'\n';
      RestClient::Response r = RestClient::post("http://"+ spmm.str() +"/node/createVMreq","application/json",spm.str());
      std::cout << r.body << '\n';
    }
    //greedy ends here.
    cout<<"Greedy Match Making Ends here.";
    cout<<"Match MAking Algorithm ends here.";*/
    //Match Making algorithm ends here.
    return "VM created as you asked me by sending me the required info";
  }
 











std::string cluster::suspendVM(const std::string& text){
  RestClient::init();

    string s(text);
    ostringstream sp;
    sp << "{\"text\":\"" ;
    sp << s; 
    sp << "\"}";
    cout<<sp.str()<<'\n';
  RestClient::Response r = RestClient::post("http://localhost:10002/node/suspendVM","application/json",sp.str());
  std::cout << r.body << '\n';
  return "VM suspended as you asked me by sending me "+ text;
}
    



std::string cluster::resumeVM(const std::string& text){
  RestClient::init();

    string s(text);
    ostringstream sp;
    sp << "{\"text\":\"" ;
    sp << s; 
    sp << "\"}";
    cout<<sp.str()<<'\n';
  RestClient::Response r = RestClient::post("http://localhost:10002/node/resumeVM","application/json",sp.str());
  std::cout << r.body << '\n';
  return "VM resumed as you asked me by sending me "+ text;
}
    


std::string cluster::destroyVM(const std::string& text){
  RestClient::init();

    string s(text);
    ostringstream sp;
    sp << s; 
    cout<<sp.str()<<'\n';
  RestClient::Response r = RestClient::del("http://localhost:10002/node/destroyVM/"+ sp.str());
  std::cout << r.body << '\n';
  return "VM destroyed as you asked me by sending me "+ text;
}


// std::string cluster::askingforVMs(int numofVMs,const std::string& textinfo){
//   RestClient::init();

//     string s(text);
//     ostringstream sp;
//     sp << "{\"text\":\"" ;
//     sp << s; 
//     sp << "\"}";
//     cout<<sp.str()<<'\n';
//   RestClient::Response r = RestClient::get("http://localhost:10006/node/getResources");
//   std::cout << r.body << '\n';
//   return "VM destroyed as you asked me by sending me "+ textinfo;
// }


std::string cluster::destroyVMdir(){
  RestClient::init();
  RestClient::Response r = RestClient::del("http://localhost:10002/node/destroyVMdir");
  std::cout << r.body << '\n';
  return "VM destroyed as you asked me because of less traffic.\n";
}




std::string cluster::autoconfigVM(bool autoconfig){
  //requesting to the apache web server for getting request rate ;
  //int request_rate;
  //get request rate from the apache web serever status.
  unsigned int microseconds = 30000000; 
  float prev_request_rate = 0;
  for(int l=0;l<8;l++){
    cout<<"loop number : "<<l<<"\n";
    usleep(microseconds);
    RestClient::Response r = RestClient::get("http://172.50.88.15:80/server-status");
    //std::cout << r.body << '\n';
    cout<<"String starts here.\n";
    //cout<<r.body<<endl;
    cout<<"String ends here.\n";
    int k=r.body.find("requests/sec");
    const char* strr = r.body.c_str();
    int j=k,flag=1;
    while(flag){
      if(strr[j] =='>')
        flag=0;
      j--;
    //cout<<"i m in loop\n";
    }
    string reqrate="";
    for(int i=j+2;i<k;i++)
    reqrate=reqrate+strr[i];
    cout<<reqrate<<'\n';
    float request_rate = atof(reqrate.c_str());
    cout<<"request rate "<<request_rate<<'\n';
    //condition for creating new vm.
    float curr_request_rate=request_rate;
    cout<<"prev_request_rate : "<<prev_request_rate<<'\n';
    cout<<"curr_request_rate : "<<curr_request_rate<<'\n';
    if(prev_request_rate >=5 && curr_request_rate >=prev_request_rate)
      { createVMreq(1,1,1024); cout<<"hi,i send request to autoscaleup.\n"; }
    //else
      //{ prev_request_rate = curr_request_rate; continue; }
    if(prev_request_rate >= 9.7 && curr_request_rate <= (prev_request_rate*0.8) )
      { destroyVMdir(); cout<<"hi,i send request to autoscaledown.\n"; }
    //else
      //{ prev_request_rate = curr_request_rate; continue; }
    prev_request_rate = curr_request_rate;

    }
  return "Hi,I am autoconfig. I have completed my job.\n";
}


string cluster::createVolume(std::string domain, std::string cap, std::string unit) {
   RestClient::init();
    ostringstream sp;
    sp << "{\"domain\":\"" ;
    sp << domain;
    sp << "\",";
    sp << "\"cap\":\"";
    sp << cap;
    sp << "\",\"unit\":\"";
    sp << unit; 
    sp << "\"}";
    cout<<sp.str()<<'\n';
  RestClient::Response r = RestClient::post("http://localhost:10002/node/createVolume","application/json",sp.str());
  std::cout << r.body << '\n';
  return r.body;
}


std::string cluster::getPoolList() {
  RestClient::init();
  RestClient::Response r = RestClient::get("http://localhost:10002/node/getPoolList");
  return r.body;
}


std::string cluster::getVolumeList(std::string pool) {
  RestClient::init();
  ostringstream sp;
  sp << "{\"pool\":\"" ;
  sp << pool;
  sp << "\"}";
  cout<<sp.str()<<'\n';
  RestClient::Response r = RestClient::post("http://localhost:10002/node/getVolumeList","application/json",sp.str());
  return r.body;
}


string cluster::attachVolumeToVM(std::string domain, std::string pool, std::string volume) {
   RestClient::init();
    ostringstream sp;
    sp << "{\"domain\":\"" ;
    sp << domain;
    sp << "\",";
    sp << "\"pool\":\"";
    sp << pool;
    sp << "\",";
    sp << "\"volume\":\"";
    sp << volume; 
    sp << "\"}";
    cout<<sp.str()<<'\n';
  RestClient::Response r = RestClient::post("http://localhost:10002/node/attachVolumeToVM","application/json",sp.str());
  std::cout << r.body << '\n';
  return r.body;
}



std::string cluster::objectUpload(std::string fileName, std::string content){
    ostringstream sp;
    sp << "{\"fileName\":\"" ;
    sp << fileName;
    sp << "\",";
    sp << "\"content\":\"";
    sp << content;
    sp << "\"}";
    cout<<sp.str()<<'\n';
   RestClient::Response r = RestClient::post("http://localhost:10002/node/objectUpload","application/json",sp.str());
   std::cout << r.body << '\n';
   return r.body;
}


std::string cluster::objectDownload(std::string fileName){
  ostringstream sp;
    sp << "{\"fileName\":\"" ;
    sp << fileName;
    sp << "\"}";
    cout<<sp.str()<<'\n';
   RestClient::Response r = RestClient::post("http://localhost:10002/node/objectDownload","application/json",sp.str());
   std::cout << r.body << '\n';
   return r.body;
}
