#include "argumentsparser.h"

#include <string>
#include <iostream>
#include <boost/program_options.hpp>

namespace twPro {

    twPro::ApplicationParameters ArgumentsParser::parseParameters(int argc, const char * argv[]) noexcept
    {
        twPro::ApplicationParameters params;

        try
        {
            boost::program_options::options_description desc{ "Options" };
            desc.add_options()
                ("help,h", "Help screen")
                ("source,s", boost::program_options::value<std::string>(), "Some data source (file path, ip)")
                ("destination,d", boost::program_options::value<std::string>(), "Some data storage of results (file path, ip)")
                ("algorithm,a", boost::program_options::value<std::string>(), "Type of an algorithm for working on data")
                ("algorithms", "Available algorithms:\n"
                               "md5_hex - Hex (32 bits) output type md5\n"
                               "md5_byte - 2 bytes (16 bits) output type md5 (IN DEVELOPMENT)")
                ("block,b", boost::program_options::value<size_t>(), "Data block size for an algorithm");
             
            boost::program_options::variables_map vm;
            store(parse_command_line(argc, argv, desc), vm);

            // Default block size and worker
            params.blockSize = 1024 * 1024; // 1 MiB (1 mebi-bytes = 2^20 bytes)
            params.workerType = WorkersTypes::MD5_hex;

            if (vm.count("help")) {
                std::cout << "\n" << desc << '\n';
            } 
            if (vm.count("source")) {
                params.source = vm["source"].as<std::string>();
            } 
            if (vm.count("destination")) {
                params.destination = vm["destination"].as<std::string>();
            } 
            if (vm.count("algorithm")) {
                params.workerType = workTypeByString(vm["algorithm"].as<std::string>());
            } 
            if (vm.count("block")) {
                params.blockSize = vm["block"].as<size_t>();
            }
        
        }
        catch (const boost::program_options::error &ex)
        {
            std::cerr << ex.what() << '\n';
        }

        return params;
    }

    ArgumentsParser::ArgumentsParser() noexcept
    {
    }

    ArgumentsParser::~ArgumentsParser() noexcept
    {
    }
     
    twPro::WorkersTypes ArgumentsParser::workTypeByString(const std::string & _val) noexcept
    {
        if (_val == "md5_hex" || _val == "md5") {
            return twPro::WorkersTypes::MD5_hex;
        }
        else if (_val == "md5_byte") {
            return twPro::WorkersTypes::MD5_byte;
        }

        return twPro::WorkersTypes::Unknown;
    }

}
