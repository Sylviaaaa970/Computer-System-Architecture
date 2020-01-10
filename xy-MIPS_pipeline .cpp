#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.


bitset<32> b_add(bitset<32> ad1,bitset<32> ad2);//function declaration


struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
     bool     is_beq;
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
     bitset<6>  op_code;
     bitset<6>  funct;
   bool   stall;
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<32>  Imm;
    bitset<16>  Imm_out;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
      bool     is_store;
    bool     is_load;
   bool  halt;
   bool stall;
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
    bool    halt;

};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
    bool    halt;
    bool  stall;
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory

            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm_out<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 

int main()
{
    
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
	

    stateStruct state,newState;

    bitset<32> bit4;
    bit4=4;
    int a=0;
    int times=1;

newState.WB.nop=1;
newState.MEM.nop=1;
newState.EX.nop=1;
newState.ID.nop=1;
newState.IF.nop=1;


state.WB.nop=1;
state.MEM.nop=1;
state.EX.nop=1;
state.ID.nop=1;
state.IF.nop=1;


newState.EX.rd_mem=0;
newState.EX.wrt_mem=0;
newState.EX.alu_op=0;
newState.EX.wrt_enable=0;
newState.EX.is_I_type=0;
newState.MEM.rd_mem=0;
newState.MEM.wrt_mem=0;
newState.MEM.wrt_enable=0;
newState.WB.wrt_enable=0;


state.EX.rd_mem=0;
state.EX.wrt_mem=0;
state.EX.wrt_enable=0;
state.MEM.wrt_mem=0;
state.MEM.rd_mem=0;
             
state.IF.is_beq=0;
newState.IF.is_beq=0;
state.IF.PC=0;


state.WB.halt=0;
newState.WB.halt=0;
state.MEM.halt=0;
newState.MEM.halt=0;
state.EX.halt=0;
newState.EX.halt=0;



state.MEM.wrt_enable=0;
state.WB.wrt_enable=0;


state.WB.stall=0;
state.EX.stall=0;
newState.WB.stall=0;
newState.EX.stall=0;
state.ID.stall=0;
newState.ID.stall=0;

    while (1) {

             
        /* --------------------- WB stage --------------------- */
if (state.IF.PC==16 && state.WB.halt==0)
{state.WB.nop=0;
newState.WB.nop=0;
}
// The first instruction

if(state.WB.nop==0)
{
    if (state.WB.wrt_enable==1)
    myRF.writeRF(state.WB.Wrt_reg_addr,state.WB.Wrt_data);

}



if(state.WB.halt==1)
    {newState.WB.nop=1;}







        /* --------------------- MEM stage --------------------- */
      if (state.IF.PC==12 && state.MEM.halt==0)
{state.MEM.nop=0;
newState.MEM.nop=0;}
// The first instruction



//stall recover
if(state.WB.stall==0 && times%2==1 && state.WB.halt==0 && state.IF.PC.to_ulong()>=16)
{
    newState.WB.nop=0;


}


newState.WB.Rs=state.MEM.Rs;
newState.WB.Rt=state.MEM.Rt;
newState.WB.Wrt_reg_addr=state.MEM.Wrt_reg_addr;
newState.WB.wrt_enable=state.MEM.wrt_enable;
newState.WB.halt=state.MEM.halt;


//add,sub
newState.WB.Wrt_data=state.MEM.ALUresult;
if(newState.WB.halt==0 && (state.IF.PC.to_ulong()>=16))
newState.WB.nop=0;

if(state.MEM.nop==0)
{

   //load
    if(state.MEM.rd_mem==1)
    {
        newState.WB.Wrt_data=myDataMem.readDataMem(state.MEM.ALUresult);
        newState.WB.nop=0;
}
    


    //store 
   else{  if(state.MEM.wrt_mem==1)
           {

//load-store  MEM-MEM   fowarding
            if(state.MEM.Rt==state.WB.Wrt_reg_addr && state.WB.wrt_enable==1)
                state.MEM.Store_data=state.WB.Wrt_data;

            myDataMem.writeDataMem(state.MEM.ALUresult,state.MEM.Store_data);
            newState.WB.nop=0;
             }
           
        }
}


//halt
if (state.MEM.halt==1)
    newState.MEM.nop=1;








        /* --------------------- EX stage --------------------- */
 if (state.IF.PC==8 && state.EX.halt==0)
{state.EX.nop=0;
newState.EX.nop=0;}
// The first instruction    
  

    newState.MEM.Rs=state.EX.Rs;
    newState.MEM.Rt=state.EX.Rt;
    newState.MEM.Wrt_reg_addr=state.EX.Wrt_reg_addr;
    newState.MEM.rd_mem=state.EX.rd_mem;
    newState.MEM.wrt_mem=state.EX.wrt_mem;
    newState.MEM.wrt_enable=state.EX.wrt_enable;
    newState.MEM.halt=state.EX.halt;
newState.MEM.Store_data=state.EX.Read_data2;


newState.WB.stall=state.EX.stall;


// WB stall
if(state.WB.stall==1 && times%2 ==0)
   { newState.WB.nop=1;
newState.EX.stall=0;
newState.WB.stall=0;
times++;}



if(state.EX.nop==0)
{if (state.EX.alu_op==0)
//sub
   { 
   //add-add + load-add   MEM-EX forwarding
if (state.EX.Rs==state.WB.Wrt_reg_addr && state.WB.wrt_enable==1)
    state.EX.Read_data1=state.WB.Wrt_data;
if (state.EX.Rt==state.WB.Wrt_reg_addr && state.WB.wrt_enable==1)
    {state.EX.Read_data2=state.WB.Wrt_data;}

    //ADD-ADD   EX-EX forwarding
    if(state.EX.Rs==state.MEM.Wrt_reg_addr && state.MEM.wrt_enable==1 && state.EX.stall==0)
    state.EX.Read_data1=state.MEM.ALUresult;
if(state.EX.Rt==state.MEM.Wrt_reg_addr && state.MEM.wrt_enable==1 && state.EX.stall==0)
state.EX.Read_data2=state.MEM.ALUresult;


    newState.MEM.ALUresult=state.EX.Read_data1.to_ulong()-state.EX.Read_data2.to_ulong();
    newState.MEM.nop=0;}

    else 
{ //load,store
        if(state.EX.is_store||state.EX.is_load)
        {
   // add-load MEM-EX forwarding
            if(state.EX.Rs==state.WB.Wrt_reg_addr && state.WB.wrt_enable==1)
                state.EX.Read_data1=state.WB.Wrt_data;
if (state.WB.Wrt_reg_addr==newState.EX.Rt && state.WB.wrt_enable==1)
{newState.MEM.Store_data=state.WB.Wrt_data;}




    //ADD-LOAD/STORE   EX-EX forwarding
    if(state.EX.Rs==state.MEM.Wrt_reg_addr && state.MEM.wrt_enable==1)
    state.EX.Read_data1=state.MEM.ALUresult;
if (state.MEM.Wrt_reg_addr==newState.EX.Rt && state.WB.wrt_enable==1)
{newState.MEM.Store_data=state.MEM.ALUresult;}


         
//IMM <0
   if(newState.EX.Imm.test(31))
{
//CHANGE TO Unsighed
    newState.EX.Imm.reset(31);
newState.EX.Imm=bitset<32>(newState.EX.Imm.to_ulong()-1);
for(int k=0;k<15;k++)
newState.EX.Imm.flip(k);}


            newState.MEM.ALUresult=state.EX.Read_data1.to_ulong()+newState.EX.Imm.to_ulong();
            newState.MEM.nop=0;
            }

//add
        else  {
      //add-add + load-add   MEM-EX forwarding


if (state.EX.Rs==state.WB.Wrt_reg_addr && state.WB.wrt_enable==1)
    state.EX.Read_data1=state.WB.Wrt_data;
    
if (state.EX.Rt==state.WB.Wrt_reg_addr && state.WB.wrt_enable==1)
    state.EX.Read_data2=state.WB.Wrt_data;


    //ADD-ADD   EX-EX forwarding
    if(state.EX.Rs==state.MEM.Wrt_reg_addr && state.MEM.wrt_enable==1 && state.EX.stall==0)
    state.EX.Read_data1=state.MEM.ALUresult;
    
if(state.EX.Rt==state.MEM.Wrt_reg_addr && state.MEM.wrt_enable==1 && state.EX.stall==0)
state.EX.Read_data2=state.MEM.ALUresult;

            newState.MEM.ALUresult=state.EX.Read_data1.to_ulong()+state.EX.Read_data2.to_ulong();
        newState.MEM.nop=0;}

    }


}
 
//halt

if (state.EX.halt==1)
    newState.EX.nop=1;



 if(state.EX.nop==1)
newState.MEM.nop=1;
        /* --------------------- ID stage --------------------- */
if (state.IF.PC==4)
{state.ID.nop=0;
newState.ID.nop=0;}
// The first instruction


//load-add hazard stall
    int i=31;
while (i>25)
{if(state.ID.Instr.test(i))
    newState.ID.op_code.set(i-26);
else  newState.ID.op_code.reset(i-26);
i--;
}//op-code


 int k=25;
while (k>20)
{if(state.ID.Instr.test(k))
newState.EX.Rs.set(k-21);
else newState.EX.Rs.reset(k-21);
k--;
}//RS
newState.EX.Read_data1=myRF.readRF(newState.EX.Rs);


 int j=20;
while (j>15)
{if(state.ID.Instr.test(j))
   newState.EX.Rt.set(j-16);
else  newState.EX.Rt.reset(j-16);
j--;
}//RT
newState.EX.Read_data2=myRF.readRF(newState.EX.Rt);



if(state.EX.is_load==1 && (state.EX.Wrt_reg_addr==newState.EX.Rs || state.EX.Wrt_reg_addr==newState.EX.Rt) && newState.ID.op_code==0 && state.IF.nop!=1)
{state.ID.nop=1;
    state.IF.nop=1;
    newState.IF.nop=1;
    newState.EX.stall=1;
    times++;
}
else 
{if(state.EX.halt==0 && state.IF.nop==1 && state.IF.PC!=0)
    {state.ID.nop=0;
    state.IF.nop=0;
   newState.IF.nop=0;}}



if (state.ID.nop==0)
{


 //   int i=31;
//while (i>25)
//{if(state.ID.Instr.test(i))
 //   newState.ID.op_code.set(i-26);
//else  newState.ID.op_code.reset(i-26);
//i--;}
//op-code
//cout<<"opcode"<<newState.ID.op_code<<endl;




int j=5;
while(j>=0)
{if(state.ID.Instr.test(j))
    newState.ID.funct.set(j);
else  newState.ID.funct.reset(j);
j--;
}//funct
//cout<<"funct"<<newState.ID.funct<<endl;


//Signextend Immediate
newState.EX.Imm=state.ID.Instr<<16;
newState.EX.Imm=newState.EX.Imm>>16;
 if (newState.EX.Imm[15]==1)
  {newState.EX.Imm.set(31);
   newState.EX.Imm.reset(15);}




//RF
// int k=25;
//while (k>20)
//{if(state.ID.Instr.test(k))
//newState.EX.Rs.set(k-21);
//else newState.EX.Rs.reset(k-21);
//k--;}

//newState.EX.Read_data1=myRF.readRF(newState.EX.Rs);
//RS
//cout<<"RS address"<<newState.EX.Rs<<endl;
//cout<<"RS"<<newState.EX.Read_data1<<endl;


//Rtype
if(newState.ID.op_code==0)
{
   newState.EX.is_I_type=0;



 //int j=20;
//while (j>15)
//{if(state.ID.Instr.test(j))
 //  newState.EX.Rt.set(j-16);
//else  newState.EX.Rt.reset(j-16);
//j--;}

//RT

//newState.EX.Read_data2=myRF.readRF(newState.EX.Rt);
//cout<<"RT address"<<newState.EX.Rt<<endl;
//cout<<"RT"<<newState.EX.Read_data2<<endl;



 int i=15;
while (i>10)
{if(state.ID.Instr.test(i))
   newState.EX.Wrt_reg_addr.set(i-11);
else  newState.EX.Wrt_reg_addr.reset(i-11);
i--;
}//RD
//cout<<"RD address"<<newState.EX.Wrt_reg_addr<<endl;


newState.EX.is_store=0;
   newState.EX.is_load=0;
   newState.IF.is_beq=0;
  


    if(newState.ID.funct==0x21)
   //add
    {newState.EX.alu_op=1;
newState.EX.wrt_enable=1;
    newState.EX.rd_mem=0;
    newState.EX.wrt_mem=0;
newState.EX.nop=0;
 newState.IF.is_beq=0;
cout<<"add"<<endl;


}

    //sub
    else  {if(newState.ID.funct==0x23)  
        {
            newState.EX.alu_op=0;
        newState.EX.wrt_enable=1;
    newState.EX.rd_mem=0;
    newState.EX.wrt_mem=0;
newState.EX.nop=0;
 newState.IF.is_beq=0;
cout<<"sub"<<endl;
    }

    else cout<<"no such funct"<<newState.ID.funct<<endl;
    }
}

//I type
else
{newState.EX.is_I_type=1;



 int j=20;
while (j>15)
{if(state.ID.Instr.test(j))
   newState.EX.Wrt_reg_addr.set(j-16);
else  newState.EX.Wrt_reg_addr.reset(j-16);
j--;
}//RT

//cout<<"RT address"<<newState.EX.Wrt_reg_addr<<endl;




 //int k=20;
//while (k>15)
//{if(state.ID.Instr.test(k))
  // newState.EX.Rt.set(k-16);
//else  newState.EX.Rt.reset(k-16);
//k--;}
//RT:for forwarding
//newState.EX.Read_data2=myRF.readRF(newState.EX.Rt);






    switch(newState.ID.op_code.to_ulong())
  {//load
    case 0x23 :
  { newState.EX.wrt_enable=1;
    newState.EX.rd_mem=1;
    newState.EX.wrt_mem=0;
    newState.EX.alu_op=1;
    newState.EX.nop=0;
    newState.EX.is_store=0;
   newState.EX.is_load=1;
   newState.IF.is_beq=0;
  

    cout<<"load"<<endl;
    break;}


//store
   case 0x2B:
  { newState.EX.wrt_enable=0;
newState.EX.rd_mem=0;
    newState.EX.wrt_mem=1;
    newState.EX.alu_op=1;
    newState.EX.nop=0;
    newState.EX.is_store=1;
   newState.EX.is_load=0;
   newState.IF.is_beq=0;
  
    cout<<"store"<<endl;
   break;}

//beq
   case 0x04:
      {newState.EX.wrt_enable=0;
newState.EX.rd_mem=0;
    newState.EX.wrt_mem=0;
    newState.EX.is_store=0;
   newState.EX.is_load=0;
    newState.IF.is_beq=0;
    cout<<"beq"<<endl;
    if(newState.EX.Read_data1!=newState.EX.Read_data2)
    {   state.IF.is_beq=1;
        //newState.IF.is_beq=0;
        break;
    }
    

  }



}

   

}


}

if(state.ID.nop==1)
    newState.EX.nop=1;
if(state.ID.stall==1)
{newState.EX.stall=1;
    times++;
}
        
        /* --------------------- IF stage --------------------- */
if (state.IF.PC==0)
{state.IF.nop=0;
    newState.IF.nop=0;
}// The first instruction



if (state.IF.nop==0)
{
//not branch
    

    if(state.IF.is_beq==0)
   {
    newState.ID.Instr=myInsMem.readInstr(newState.IF.PC);
     newState.IF.PC=b_add(newState.IF.PC,bit4);
newState.ID.stall=0;
newState.ID.nop=0;}




//branch
   else
{

 //IMM <0
   if(newState.EX.Imm.test(31))
{newState.EX.Imm=newState.EX.Imm<<2;


//CHANGE TO Unsighed
newState.EX.Imm=bitset<32>(newState.EX.Imm.to_ulong()-1);
for(int k=0;k<17;k++)
newState.EX.Imm.flip(k);


newState.IF.PC=bitset<32>(newState.IF.PC.to_ulong()-newState.EX.Imm.to_ulong());}


//IMM >0
else 
{newState.EX.Imm=newState.EX.Imm<<2;

newState.IF.PC=bitset<32>(newState.IF.PC.to_ulong()+newState.EX.Imm.to_ulong());
}


    newState.ID.Instr=myInsMem.readInstr(newState.IF.PC);
    newState.ID.nop=1;
    newState.ID.stall=1;
        

}




}




if (newState.ID.Instr.count()==32)
    {newState.IF.nop=1;
    newState.ID.nop=1;
newState.EX.halt=1;
newState.IF.PC=newState.IF.PC.to_ulong()-4;}


//FINISH

        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
                
       


 if (newState.EX.Imm[31]==1)
  {newState.EX.Imm_out.set(15);
   newState.EX.Imm_out.reset(15);}
for(int i=14;i>=0;i--)
  {if(newState.EX.Imm[i]==1)
  	newState.EX.Imm_out.set(i);
  	else
  		newState.EX.Imm_out.reset(i);
  }
   

                printState(newState, a);
        cout<<"cycle"<<a<<" : "<<newState.ID.Instr<<endl;
        //print states after executing cycle 0, cycle 1, cycle 2 ... 
         a++; 
             
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */ 
               	
    


    }
    myRF.outputRF(); // dump RF;	
	myDataMem.outputDataMem(); // dump data mem 
	
	return 0;
}


bitset<32> b_add(bitset<32> ad1,bitset<32> ad2)
{bitset<32> carry,sum,tmps;
  carry=ad2;
  sum=ad1;
while(carry.any())
{tmps=sum;
  sum=tmps^carry;
  carry=(tmps&carry)<<1;

}
return sum;
}
