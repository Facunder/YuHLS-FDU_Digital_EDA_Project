#include "HLS.h"
#include <iostream>

int main(int argc, char ** argv)
{
     if(argc != 2)
     {
         std::cout << "Ustdsage:" << argv[0] << "filename\n";
         return -1;
     }
    //std::string file_name = "test_new.ll";
    //std::cout<<file_name <<std::endl;
    parser p(argv[1]);
    //parser p(file_name);
    if(p.parse() != 0)
    {
        std::cout << "parsing error" << p.get_current_line() << std::endl;
        return -1;
    }

    // dump parsed results
    std::vector<basic_block> bbs = p.get_basic_blocks();
    std::string fn_name = p.get_function_name();
    std::vector<var> vars = p.get_function_params();
    int ret_type = p.get_ret_type();

    if(ret_type == RET_INT)
        std::cout << "ret type: int" << std::endl;
    else
        std::cout << "ret type: void" << std::endl;

    std::cout << "function name" << fn_name << std::endl;

    for(int i = 0; i < vars.size(); ++i)
    {
        if(vars[i]._array_flag)
            std::cout << "array" << std::endl;
        else
            std::cout << "non-array" << std::endl;
        std::cout << vars[i]._name << std::endl;
    }

    for(int i = 0; i < bbs.size(); ++i)
    {
        std::cout << "Basic Block label: " << bbs[i].get_label_name() << std::endl;
        std::vector<statement> ss = bbs[i].get_statements();
        for(int j = 0; j < ss.size(); ++j)
        {
            int type = ss[j].get_type();
            if(type != OP_STORE || type != OP_RET)
            {
                std::cout << "    " << "value " << ss[j].get_var() <<  " " << std::endl;
            }

            std::cout << "    " << "OP TYPE:" <<  ss[j].get_type() << std::endl;
            std::cout<<"    ";
            for(int k = 0; k < ss[j].get_num_oprands(); ++k)
                std::cout << ss[j].get_oprand(k) << " ";

            std::cout << std::endl;

        }
    }
    HLS hls(p);
	// Generate DFG->CFG
	hls.genCDGF();
    // Scheduling
	hls.scheduling();
	// Resource binding
	hls.registerAllocationAndBinding();
	// hls.calculateAllocationAndBinding();
	hls.controlLogicSynthesis();
    // Write out RTL code
	hls.writeOutFile();
}
