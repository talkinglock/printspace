#include <iostream>
#include <cstdio>
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <mntent.h>
#include <filesystem>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <cmath>
namespace fs = std::filesystem;

const int TOTAL_PARTS = 20;

int GetPartsFilled(float partTotal, float ratio){
    return std::round(partTotal * ratio);
}
    
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;

    std::string cmdSilent = std::string(cmd) + " 2>/dev/null";

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string GetDisplayNameFromDir(std::string dir)
{
    std::string displayname;
    std::vector<std::string> seperations;
    std::string namePos;
    int i = 0;
    for (char c : dir)
    {
        i++;
        namePos += c;
        if (c == '/' or i == dir.size())
        {
            seperations.push_back(namePos);
            namePos = "";
        }
    }
    displayname = seperations[seperations.size() - 1];
    if (displayname == "/")
    {
        displayname = "root";
    }
    return displayname;
}

float GetSizeFromDir(std::string dir)
{
    float result = std::stof(exec(("df --output=used -B1 " + dir +" | tail -n1").c_str()));

    return result;
}

int main()
{
    FILE *fp;
    struct mntent *ent;

    fp = setmntent("/etc/mtab", "r");

    if (fp == NULL)
    {
        perror("setmntent");
        return -1;
    }
    while ((ent = getmntent(fp)) != NULL)
    {
        struct statvfs filesys;
        if (statvfs(ent->mnt_dir, &filesys) == 0)
        {
            std::string filename = ent->mnt_fsname;
            std::string mnttype = ent->mnt_type;
            std::string dir = ent->mnt_dir;
            std::string displayname;
            if (filename.find("/dev/") == std::string::npos)
            {
                continue;
            }
            if (mnttype == "vfat")
            {
                continue;
            }
            // std::cout << filename << '\n';

            displayname = GetDisplayNameFromDir(dir);
            float sizeTaken = GetSizeFromDir(dir);
            float sizeTotal = filesys.f_blocks * filesys.f_frsize;
            float sizeRatio = sizeTaken / sizeTotal;
            float parts = GetPartsFilled(TOTAL_PARTS, sizeRatio);
            std::string cubes;
            std::string filler;

            for (char c : displayname){
                filler += "#";
            }

            for (int i = 0; i < TOTAL_PARTS; ++i){
                if (i < parts){
                    cubes += "■";
                }
                else
                {
                    cubes += "□";
                } 
            }


            std::cout << "##### " + displayname + " #####" << '\n';
            std::cout << '\n';
            std::cout << sizeTaken / 1000000000 << " / " << sizeTotal / 1000000000 << '\n';
            std::cout << cubes << '\n';
            std::cout << '\n';
            std::cout << "######" + filler + "######" << '\n';
            std::cout << '\n';

        }
    }

    endmntent(fp);
    return 0;
}