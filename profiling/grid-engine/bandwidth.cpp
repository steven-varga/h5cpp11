#include <vector>
#include <armadillo>
#include <boost/program_options.hpp>
#include <iostream>
#include <glog/logging.h>
#include <cstdlib>
#include <h5cpp/core>
	#include <tests/struct.h>
#include <h5cpp/io>

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>


#include "read.hpp"

using namespace std;
namespace po = boost::program_options;

// used when capturing /proc/... system info 
std::string exec(const char* cmd) {
	const int buf_size = 1024;
    std::array<char, buf_size> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), buf_size, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}

int main(int argc, char **argv) {
	std::string input,output,source,destination,glog_dir;
	bool glog_stderr;
    unsigned int glog_minloglevel;
    po::options_description desc("Allowed options",120);

	std::map<std::string,std::function<void(int)>> dispatch;

	dispatch["raw-float"] = harness<float>;
	dispatch["raw-double"] = harness<double>;
	dispatch["raw-int8"] = harness<int8_t>;
	dispatch["raw-int16"] = harness<int16_t>;
	dispatch["raw-int32"] = harness<int32_t>;
	dispatch["raw-int64"] = harness<int64_t>;

	dispatch["stl-vec-string"] = harness<std::vector<std::string>>;
	dispatch["stl-vec-float"] = harness<std::vector<float>>;
	dispatch["stl-vec-double"] = harness<std::vector<double>>;
	dispatch["stl-vec-int8"] = harness<std::vector<int8_t>>;
	dispatch["stl-vec-int16"] = harness<std::vector<int16_t>>;
	dispatch["stl-vec-int32"] = harness<std::vector<int32_t>>;
	dispatch["stl-vec-int64"] = harness<std::vector<int64_t>>;

	dispatch["arma-mat-string"] = harness<arma::Mat<std::string>>;
	dispatch["arma-mat-float"] = harness<arma::Mat<float>>;
	dispatch["arma-mat-double"] = harness<arma::Mat<double>>;
	dispatch["arma-mat-int8"] = harness<arma::Mat<int8_t>>;
	dispatch["arma-mat-int16"] = harness<arma::Mat<int16_t>>;
	dispatch["arma-mat-int32"] = harness<arma::Mat<int32_t>>;
	dispatch["arma-mat-int64"] = harness<arma::Mat<int64_t>>;

    desc.add_options()
            ("case,c", po::value<std::string>(),"one of the test cases queried by --list option")
            ("multiplier,m", po::value<unsigned long>()->default_value(10),"object size on heap := multiplier * size")
            ("size,s", po::value<unsigned long>()->default_value(1),"size in MB written onto disk")
			("gzip,g", po::value<unsigned int>()->default_value(0), "0-9 0 for no compression, 9 for highest")
            ("chunk", po::value<unsigned int>()->default_value(1), "number of days in blocks/hdf5-chunks, 0 no-chunks")
            ("list,l","list test cases...\n")

            ("glog-dir", po::value<string>()->default_value("./"),"glog output directory") 
            ("glog-stderr", po::value<bool>()->default_value(true),"glog output to stderr if true") 
            ("glog-minloglevel", po::value<unsigned int>()->default_value(0),"glog log level:  INFO=0 WARNING=1 ERROR=2 FATAL=3\n")
            ("help,h", "produce help message")
            ;

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            cout << "bandwidth profiler harness... "<<endl;
            cout <<endl;
            cout << desc << endl <<endl;

            cout <<"example:" <<endl;
            cout <<"   "<< argv[0] <<"   \n\n" <<endl;
            return 0;
        }
		if( vm.count("list") ){
			for( auto i:dispatch )
				std::cout << i.first << std::endl;
			return 0;
		}

	std::string cpu_freq = exec("cat /proc/cpuinfo | grep 'cpu MHz'| head -n 1|cut -d':' -f2|tr -d '\n \t'");
	std::string cpu_type = exec("cat /proc/cpuinfo | grep 'model name'| head -n 1| cut -d':' -f2|cut -d' ' -f4 | tr -d '\n \t'");
	std::string load_avg = exec("cat /proc/loadavg | cut -d' ' -f1,2,3 | tr -d '\n' | tr ' ' '\t'");
//	dispatch["string"](134);

	std::cout<< cpu_freq <<" " << cpu_type << " " << load_avg  << std::endl;

	} catch( ... ){
        cout << endl << "Error parsing arguments!!! "<<endl<<endl;
        cout << desc <<endl<<endl;
    }

    return 0;
}

