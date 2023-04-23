
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>


namespace fs = std::filesystem;

bool name_collision(const std::filesystem::path& p1, const std::filesystem::path& p2) {
    if (std::filesystem::equivalent(p1, p2)) {
        return true; 
    }
    try {
        

        if (p1.filename() == p2.filename()) {
            if (!std::filesystem::is_directory(p1) && std::filesystem::is_directory(p2))
            {
                return true;
            }
        }
    }
        catch (const std::exception ex) {
            std::cerr << "Caught error: " << ex.what() << std::endl;
        }
    
    
    return false;
}

void read_directory(const fs::path& path, std::vector<fs::path>& files)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            files.push_back(entry.path());
            read_directory(entry.path(), files);
        }
    }
    for (const auto& entry : fs::directory_iterator(path))
    { if (entry.is_regular_file())
        {
            files.push_back(entry.path());
        }
    }
}
bool compare_files(const std::string& file1_path, const std::string& file2_path)
{
    std::ifstream file1(file1_path, std::ios::binary);
    std::ifstream file2(file2_path, std::ios::binary);

    if (!file1 || !file2) {
        return false; 
    }

    std::vector<char> buffer1(std::istreambuf_iterator<char>(file1), {});
    std::vector<char> buffer2(std::istreambuf_iterator<char>(file2), {});

    return buffer1 == buffer2;
}

void sync_folders(const std::vector<fs::path>& folder1_files, const std::vector<fs::path>& folder2_files, const std::string& folder1_path, const std::string& folder2_path, const std::string& log_path)
{
    time_t now = time(0);
    tm local_time;
    localtime_s(&local_time, &now);

    std::ofstream log_file(log_path);
    
    
        for (const auto& file1_path : folder1_files) 
        {
            std::string file2_path = folder2_path + file1_path.string().substr(folder1_path.size());


            if (std::find(folder2_files.begin(), folder2_files.end(), file2_path) != folder2_files.end())
            {
                if (name_collision(file1_path, file2_path))
                {
                    fs::remove_all(file2_path);
                    fs::copy_file(file1_path, file2_path);
                    log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << "and updated file" << std::endl;
                    std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << "and updated file" << std::endl;
                }
                if (name_collision(file2_path, file1_path))
                {
                    fs::remove(file2_path);
                    fs::create_directories(fs::path(file2_path).parent_path());
                    fs::copy_file(file1_path, file2_path);
                    log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << "and updated file" << std::endl;
                    std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << "and updated file" << std::endl;
                }

                if (!compare_files(file1_path.string(), file2_path))
                {
                    if (std::filesystem::is_directory(file2_path))
                    {
                        fs::remove_all(file2_path);
                        fs::create_directories(fs::path(file2_path).parent_path());
                        fs::copy_file(file1_path, file2_path);
                    }
                    else {
                        fs::remove(file2_path);
                        fs::copy_file(file1_path, file2_path);
                    }
                    log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
                    std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
                }

            }
            else 
            {
                fs::create_directories(fs::path(file2_path).parent_path());
                if (fs::remove_all(file2_path) > 0)
                {
                    log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << "and updated file" << std::endl;
                    std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << "and updated file" << std::endl;
                }
                if(fs::remove(file2_path))  
                {   
                    log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << "and updated file" << std::endl;
                    std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << "and updated file" << std::endl;
                }
                fs::copy_file(file1_path, file2_path);
                log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Created" << file2_path << " to " << folder2_path << std::endl;
                std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Created" << file2_path << " to " << folder2_path << std::endl;
            }
            
        }
   
    
    
    for (const auto& file2_path : folder2_files) {
        std::string file1_path = folder1_path + file2_path.string().substr(folder2_path.size());

       
        if (std::find(folder1_files.begin(), folder1_files.end(), file1_path) == folder1_files.end()) {
            if (std::filesystem::is_directory(file2_path))
            {
                fs::remove_all(file2_path);
            }
            else {
                fs::remove(file2_path);
            }
            log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << std::endl;
            std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <original_folder> <sync_folder> <log_file>\n";
        return 1;
    }
    const std::string path = argv[1];
    const std::string dest_dir = argv[2];
    const std::string log_file = argv[3];
    
    std::ofstream log_file_stream(log_file, std::ios::app);
    if (!log_file_stream) 
    {
        std::cerr << "could not create file\n";        
    }
    log_file_stream.close();

    char timestamp[32];
    //std::string log_file = "C:/Users/Silviu/Desktop/sol/Log.txt";
    //std::string path = "C:/Users/Silviu/Desktop/sol/original";
    //std::string dest_dir = "C:/Users/Silviu/Desktop/sol/destination";
    std::vector<fs::path> Sync;
    std::vector<fs::path> files;
    
    read_directory(dest_dir, Sync);
    read_directory(path, files);

    std::ofstream log(log_file, std::ios::app);
    while (true)
    {
        
        auto current_time = std::chrono::system_clock::now();
        std::time_t current_time_t = std::chrono::system_clock::to_time_t(current_time);
        ctime_s(timestamp, sizeof(timestamp), &current_time_t);

        std::cout << "Sync completed at " << timestamp;
       
        sync_folders(files,Sync,path ,dest_dir,log_file);

        
        std::this_thread::sleep_for(std::chrono::minutes(5));
    }

    
  
    return 0;
}

