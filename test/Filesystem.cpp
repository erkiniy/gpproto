//
// Created by ProDigital on 7/30/18.
//

#define BOOST_TEST_MODULE FilesystemTests
#include <boost/test/unit_test.hpp>
#include "gp/utils/Logging.h"
#include "gp/utils/filesystem/NativeFileSystem.h"
#include "gp/utils/filesystem/NativeFile.h"
#include <boost/filesystem.hpp>
#include <iostream>

using namespace gpproto;

BOOST_AUTO_TEST_CASE(create_folder) {

    std::string rootHomeDir(getenv("HOME"));
    rootHomeDir += "/Documents/MyDir/AnotherDir/YourDir/";

    std::string path(rootHomeDir);

    if (boost::filesystem::exists(path)) {
        printf("Boost exists path %s\n", path.c_str());
    }
    else {
        printf("Boost dir doesn't exist %s\n", path.c_str());
        boost::filesystem::create_directories(rootHomeDir);
    }

    try {
        std::cout << boost::filesystem::absolute(rootHomeDir, "/") << "\n";
    }
    catch (boost::filesystem::filesystem_error& e) {
        std::cerr << e.what() << "\n";
    }

};

BOOST_AUTO_TEST_CASE(file_path_init) {

    std::string rootHomeDir(getenv("HOME"));
    rootHomeDir += "/Documents/MyDir/AnotherDir/";

    if (true) {
        auto fs = new NativeFileSystem(rootHomeDir);

        fs->Initialize();

        LOGV("Base path for this is %s", fs->BasePath().c_str());

        auto fileInfo = FileInfo("/jalol.txt");

        auto nativeFile = fs->OpenFile(fileInfo, File::FileMode::readWrite);
        char buffer[] = "Jaloliliddin Erkiniy asdasa";

        auto size = nativeFile->Write(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));

        LOGV("Written file size is %lld", size);
        fs->CloseFile(nativeFile);

        fs->Shutdown();
        delete fs;
    }

    {
        auto fs = new NativeFileSystem(rootHomeDir);

        fs->Initialize();

        auto fileInfo = FileInfo("/jalol.txt");

        auto nativeFile = fs->OpenFile(fileInfo, File::FileMode::readWrite);

        char buffer[1024];

        auto size = nativeFile->Read(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer));

        LOGV("Read file size is %lld", size);

        if (size > 0)
            LOGV("Read file = %s", buffer);

        fs->Shutdown();
        delete fs;
    }


    BOOST_ASSERT(true);
};

