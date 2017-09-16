#include <restclient-cpp/restclient.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;
int returnHashOf(string fileName);


int returnHashOf(string fileName){
  int ascii;
  int result = 0;
  for(char c : fileName){
    ascii = (int)(c);
    result += ascii;
  }
  cout << result << endl;
  return result % 2;
}







int main() {
  RestClient::init();
  //cout<<"{\"numVMs\":2,\"vcpu\":1,\"memory\":1024}";
  RestClient::Response r; 
  while(1){
    int x=7;
    string name;
    cout << "Enter command no. to execute: ";
    cin >> x;

    if(x==1 || x==3 || x==4 || x==5){
      cout << "Enter VM Name: ";
      cin >> name;
    }
    
    if(x==1)
    {
    
  	r = RestClient::post("http://localhost:10001/cluster/createVM","application/json","{\"text\":\""+name+"\"}");
  	std::cout << r.body << '\n';
    }
    else if(x==2)
    {
      string numVm, vcpu, mem;
      cout<<"Enter num of vms, vcpu and mem: ";
      cin >> numVm;
      cin >> vcpu;
      cin >> mem;

      cout<<"sending command to CC1.\n";
      r = RestClient::post("http://localhost:10001/cluster/createVMreq","application/json","{\"numVMs\":"+numVm+",\"vcpu\":"+vcpu+",\"memory\":"+mem+"}"); //CC1
  	std::cout << r.body << '\n';
  	//cout<<"sending command to CC2.\n";
  	//r = RestClient::post("http://172.50.88.27:10001/cluster/createVMreq","application/json","{\"numVMs\":2,\"vcpu\":1,\"memory\":1024}");  //CC2
  	//std::cout << r.body << '\n';
    }
    else if(x==3)
    {
      r = RestClient::post("http://localhost:10001/cluster/suspendVM","application/json","{\"text\":\"" + name +"\"}");
      std::cout << r.body << '\n';
    }
    else if(x==4)
    {
      r = RestClient::post("http://localhost:10001/cluster/resumeVM","application/json","{\"text\":\"" + name +"\"}");
      std::cout << r.body << '\n';
    }
    else if(x==5)
    {
      r = RestClient::del("http://localhost:10001/cluster/destroyVM/"+name);
      std::cout << r.body << '\n';
    }
    else if(x==6) //auconfiguration.
    {
    	bool autoconfig = 1;
    	ostringstream au;
    	au << "{\"autoconfig\":";
    	au << autoconfig;
    	au << "}";
      r = RestClient::post("http://localhost:10001/cluster/autoconfigVM","application/json",au.str());
  	std::cout << r.body << '\n';
    }
    else if(x==7)  //server consolidation.
    {
      vector <float> allvolumeinfo;
      vector <float> allvolume_memory_ratios;
      vector <int> allbusycpus;
      vector <unsigned long long> allbusymemories;
      vector <string> allnodeuris;
      ifstream file("ncs.txt");
      string str; 
      while (std::getline(file, str))
      allnodeuris.push_back(str); 
      //getting busy resorces information from nodes.
      int i;
      for(i=0 ; i < allnodeuris.size();i++)
      {
        cout<<"I am talking to Node Conrollers"<<'\n';
        RestClient::Response r = RestClient::get("http://"+ allnodeuris[i] +"/node/getbusyResources");
        std::cout << r.body<< '\n';
        //std::cout << r.body<< '\n';
        string s(r.body);
        vector<string> stringvec; 
        int busycpu;
        unsigned long long busymemory;
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
        stringstream(sub)>>busycpu;
        cout<<"my busy vcpus : "<<busycpu<<'\n';


        stringstream ac(stringvec[3]);
        getline(ac,sub,'}');
        //cout<<sub<<'\n';
        stringstream(sub)>>busymemory;
        cout<<"my busy memory: "<<busymemory<<'\n';

        allbusycpus.push_back(busycpu);
        allbusymemories.push_back(busymemory);
      }
      cout<<"here are all the ncs information."<<'\n';
      for(i=0;i<allnodeuris.size();i++){
        cout<<allbusycpus[i]<<'\n';
        cout<<allbusymemories[i]<<'\n';
      }
      //getting all resources info from nodes.
      vector <int> allcpus;
      vector <unsigned long long> allmemories;
      for(i=0 ; i < allnodeuris.size();i++)
      {
        cout<<"I am talking to Node Conrollers to get all resouces info."<<'\n';
        RestClient::Response ra = RestClient::get("http://"+ allnodeuris[i] +"/node/getResources");
        std::cout << ra.body<< '\n';
        //std::cout << r.body<< '\n';
        string sa(ra.body);
        vector<string> stringveca; 
        int allcpu;
        unsigned long long allmemory;
        stringstream aaa(sa);
        string suba;

        while (getline(aaa, suba, ':')) {
        stringveca.push_back(suba);
        }

        cout<<stringveca[2]<<endl;
        cout<<stringveca[3]<<endl;

        stringstream aba(stringveca[2]);
        getline(aba,suba,',');
        //cout<<sub<<'\n';
        stringstream(suba)>>allcpu;
        cout<<"my all vcpus : "<<allcpu<<'\n';


        stringstream aca(stringveca[3]);
        getline(aca,suba,'}');
        //cout<<sub<<'\n';
        stringstream(suba)>>allmemory;
        cout<<"my all memory: "<<allmemory<<'\n';

        allcpus.push_back(allcpu);
        allmemories.push_back(allmemory);
      }
      cout<<"here are all the ncs information."<<'\n';
      for(i=0;i<allnodeuris.size();i++){
        cout<<allcpus[i]<<'\n';
        cout<<allmemories[i]<<'\n';
      }
      //Calculating volumes and volume ratios for all the nodes.
      cout<<"Calculating volumes and volume ratios for all the nodes.\n";
      for(i=0;i<allnodeuris.size();i++){
        float cpur = (float) allbusycpus[i] / allcpus[i];
        cout<<"cpur "<<cpur<<'\n';
        float memoryr = (float) allbusymemories[i] / allmemories[i];
        cout<<"memoryrr "<<memoryr<<'\n';
        float vol = (float)(1.0/(1.0-cpur)) + (float)(1.0/(1.0-memoryr)) ;
        allvolumeinfo.push_back(vol); 
        cout<<"my volume is "<<allvolumeinfo[i]<<'\n';
        float vol_ratio = (float)allvolumeinfo[i]/allbusymemories[i];
        allvolume_memory_ratios.push_back(vol_ratio);
        cout<<"my volume memory ratio is "<<allvolume_memory_ratios[i]<<'\n';
      }
      //sorting all the nodes according to volume memory ratios.
      cout<<"sorting all the nodes according to volume memory ratios.\n";
      std::vector <string> allnodeuris_VR;
      for(i=0;i<allnodeuris.size();i++){
        allnodeuris_VR.push_back(allnodeuris[i]);
      }
      int j,n,loc;
      float temp,min;
      n=allnodeuris_VR.size();
      for(i=0;i<n-1;i++)
      {
          min=allvolume_memory_ratios[i];
          loc=i;
          for(j=i+1;j<n;j++)
          {
              if(min>allvolume_memory_ratios[j])
              {
                  min=allvolume_memory_ratios[j];
                  loc=j;
              }
          }
   
          temp=allvolume_memory_ratios[i];
          allvolume_memory_ratios[i]=allvolume_memory_ratios[loc];
          allvolume_memory_ratios[loc]=temp;
        
          const char * s1 = allnodeuris_VR[i].c_str();
          const char * s2 = allnodeuris_VR[loc].c_str();
          const char **str1_ptr = &s1;
          const char **str2_ptr = &s2;
          const char *tempc = *str1_ptr;
          *str1_ptr = *str2_ptr;
          *str2_ptr = tempc;
          string t1(s1);
          string t2(s2);
          allnodeuris_VR[i].replace(allnodeuris_VR[i].begin(),allnodeuris_VR[i].end(),t1);
          allnodeuris_VR[loc].replace(allnodeuris_VR[loc].begin(),allnodeuris_VR[loc].end(),t2);
          
      }
      //printing
      cout<<"\nSorted list is as follows\n";
      for(i=0;i<n;i++)
      {
          cout<<allnodeuris_VR[i]<<" ";
      }
      cout<<"\n";    
      for(i=0;i<n;i++)
      {
          cout<<allvolume_memory_ratios[i]<<" ";
      }
      cout<<"\n";
      string source = allnodeuris_VR[0];
      string target ;
      cout<<"Got the source "<<source<<" chosen for server consolidation and initial target "<< target << " for migration.\n";
      cout<<"Now CLC has to find source_busy_cpu, source_busy_memory\n";
      RestClient::Response rb = RestClient::get("http://"+ source +"/node/getbusyResources");
      std::cout << rb.body<< '\n';
      //std::cout << r.body<< '\n';
      string sb(rb.body);
      vector<string> stringvecb; 
      int source_busy_cpu;
      unsigned long long source_busy_memory;
      stringstream aab(sb);
      string subb;
      while (getline(aab, subb, ':'))
        stringvecb.push_back(subb);
      cout<<stringvecb[2]<<endl;
      cout<<stringvecb[3]<<endl;
      stringstream abb(stringvecb[2]);
      getline(abb,subb,',');
      //cout<<sub<<'\n';
      stringstream(subb)>>source_busy_cpu;
      cout<<"source_busy_cpus : "<<source_busy_cpu<<'\n';
      stringstream acb(stringvecb[3]);
      getline(acb,subb,'}');
      //cout<<sub<<'\n';
      stringstream(subb)>>source_busy_memory;
      cout<<"source_busy_memory : "<<source_busy_memory<<'\n';
      //loop for finding suitable target for vm migration.
      cout<<"loop for finding suitable target for vm migration.\n";
      for(int x = allnodeuris_VR.size()-1; x > 0; x--){
        target = allnodeuris_VR[x];
        cout<<"source is : "<<source<<'\n';
        cout<<"target is : "<<target<<'\n';
        cout<<"Now CLC has to find target_avail_cpu, target_avail_memory.\n";
        //target info
        RestClient::Response rc = RestClient::get("http://"+ target +"/node/getavailResources");
        std::cout << rc.body<< '\n';
        //std::cout << r.body<< '\n';
        string sc(rc.body);
        vector<string> stringvecc; 
        int target_avail_cpu;
        unsigned long long target_avail_memory;
        stringstream aac(sc);
        string subc;
        while (getline(aac, subc, ':'))
          stringvecc.push_back(subc);
        cout<<stringvecc[2]<<endl;
        cout<<stringvecc[3]<<endl;
        stringstream abc(stringvecc[2]);
        getline(abc,subc,',');
        //cout<<sub<<'\n';
        stringstream(subc)>>target_avail_cpu;
        cout<<"target_avail_cpu : "<<target_avail_cpu<<'\n';
        stringstream acc(stringvecc[3]);
        getline(acc,subc,'}');
        //cout<<sub<<'\n';
        stringstream(subc)>>target_avail_memory;
        cout<<"target_avail_memory : "<<target_avail_memory<<'\n';
        cout<<"http://" + source + "/cluster/migrateVM","application/json","{\"text\":\"" + target +"\"}";
        if(source_busy_cpu<=target_avail_cpu && source_busy_memory<=target_avail_memory){
      	 r = RestClient::post("http://" + source + "/node/migrateVMs","application/json","{\"text\":\"" + target +"\"}");
           cout<<"i am checking.'\n'";
           //std::cout << r.body << '\n';
           break;
        }
        else
        	continue;
      }//for loop over
    }//else if part is over.
    else if(x==8) //create volume.
    {
      cout << "Enter domain, cap, and unit: ";
      cout<<"sending command to CC1 to create volume.\n";
      string domain, cap, unit;
      cin >> domain;
      cin >> cap;
      cin >> unit;
      RestClient::Response r = RestClient::post("http://localhost:10001/cluster/createVolume","application/json","{\"domain\":\""+domain+"\",\"cap\":\""+cap+"\",\"unit\":\""+unit+"\"}"); //CC1
      std::cout << r.body << '\n';
    } 
    else if(x==9) //getting pool list.
    {
      cout<<"getting pool list.\n";
      RestClient::Response r = RestClient::get("http://localhost:10001/cluster/getPoolList");
      std::cout << r.body<< '\n';
    } 
    else if(x==10) //getting volume list.
    {
      cout << "Enter pool name: ";
      string pool;
      cin >> pool;
      cout<<"getting volume list.\n";
      RestClient::Response r = RestClient::post("http://localhost:10001/cluster/getVolumeList","application/json","{\"pool\":\""+pool+"\"}");
      std::cout << r.body<< '\n';
    } 
    else if(x==11)//attach volume to vm.
    {
      cout << "Enter domain, pool and volume names: ";
      string domain, pool, vol;
      cin >> domain;
      cin >> pool;
      cin >> vol;
      cout<<"sending command to CC1 to attach volume to vm.\n";
      RestClient::Response r = RestClient::post("http://localhost:10001/cluster/attachVolumeToVM","application/json","{\"domain\":\""+domain+"\",\"pool\":\""+pool+"\",\"volume\":\""+vol+"\"}"); //CC1
      std::cout << r.body << '\n';
    } 
    else if(x==12)//upload object
    {
      cout<<"upload file.\n";
      int hash = returnHashOf("fileName1");
      string ip;
      if(hash==0)
         ip="localhost:10001";
      else
         ip="172.50.88.27:10001"; 
      RestClient::Response r = RestClient::post("http://"+ ip +"/cluster/objectUpload","application/json","{\"fileName\":\"filename2\",\"content\":\"content2\"}"); //CC1
      std::cout << r.body << '\n';
    } 
    else //download object
    {
      cout<<"download file.\n";
      int hash = returnHashOf("fileName1");
      string ip;
      if(hash==0)
         ip="localhost:10001";
      else
         ip="172.50.88.27:10001"; 
       cout<<ip<<"\n";
      RestClient::Response r = RestClient::post("http://"+ ip +"/cluster/objectDownload","application/json","{\"fileName\":\"filename2\"}"); //CC1
      std::cout << r.body << '\n';
    } 
  }
  return -0;

}