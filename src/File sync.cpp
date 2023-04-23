
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>


namespace fs = std::filesystem;
//Recursively scans from a folder and adds all files and folders into a vector of paths
void read_directory(const fs::path& path, std::vector<fs::path>& files)
{
    for (const auto& entry : fs::directory_iterator(path))
    {

        //Check if the file is a folder and if it is it will call the function again to check the files from that folder and add it to the vector of paths
        if (entry.is_directory())
        {
            files.push_back(entry.path());
            read_directory(entry.path(), files);
        }
    }
    for (const auto& entry : fs::directory_iterator(path))
    { 
        // Check if the file is a regular file and if it is it will be added to the path vector
        if (entry.is_regular_file())
        {
            files.push_back(entry.path());
        }
    }
}
//Compare 2 files
bool compare_files(const std::string& file1_path, const std::string& file2_path)
{
    // Check if the file sizes are different
    if (fs::file_size(file1_path) != fs::file_size(file2_path))
    {
        return false;
    }
    // Open the files in binary mode for comparison
    std::ifstream file1(file1_path, std::ios::binary);
    std::ifstream file2(file2_path, std::ios::binary);
    
    // Check if the files were opened successfully
    if (!file1 || !file2) 
    {
        return false; 
    }
    
    char c1, c2;
    // Compare each byte of the two files
    while (file1.get(c1) && file2.get(c2))
    {
        if (c1 != c2)
            return false;
    }
    // If the loop completes, the files are identical
    return true;
}
//Checks if folder 1 and folder 2 have the same files and if not it will edit folder 2 to match folder 1
void sync_folders(const std::vector<fs::path>& folder1_files, const std::vector<fs::path>& folder2_files, const std::string& folder1_path, const std::string& folder2_path, const std::string& log_path)
{
    // Get localt time
    time_t now = time(0);
    tm local_time;
    localtime_s(&local_time, &now);

    std::ofstream log_file(log_path);
    

    //Checks if files in folder 1 are also in folder 2 or update cheages to existing documents or create documents if they exist in folder 1 but not in folder 2
    for (const auto& file1_path : folder1_files) 
    {
        std::string file2_path = folder2_path + file1_path.string().substr(folder1_path.size());

        //Checks if files exist in both folders
        if (std::find(folder2_files.begin(), folder2_files.end(), file2_path) != folder2_files.end()) 
        {
            //If file1 is a folder and file2 a file updates file2 to be the same type as file1 and logs the change
            if (fs::is_directory(file1_path) && !fs::is_directory(file2_path))
            {
                fs::remove(file2_path);
                fs::create_directory(file2_path);
                log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
                std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
            
            }
            //If file1 is a file and file2 is a folder remove the folder and its contents and copy file1 in the destination folder and logs the change
            else if (!fs::is_directory(file1_path) && fs::is_directory(file2_path))
            {
                fs::remove_all(file2_path);
                fs::copy_file(file1_path, file2_path);
                log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
                std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
            
            }

            //If both files are both files are folders then continue with the next file no chenge needs to be made
            else if(fs::is_directory(file1_path) && fs::is_directory(file2_path))
            {
                continue;
            }

            //If both files are the same type it will compare them to see if the contents are the same if not the file will be updated and a log will be made of the change
            else
            {
                if (!compare_files(file1_path.string(), file2_path))
                {
                    fs::remove(file2_path);
                    fs::copy_file(file1_path, file2_path);
                    log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
                    std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Updated " << file2_path << " to " << folder2_path << std::endl;
                }
            }
        }
        //If a file exists in folder1 but not in folder 2 it will be created and a log will be made
        else 
        {
            fs::create_directories(fs::path(file2_path).parent_path());
            if (fs::is_directory(file1_path))
            {
                fs::create_directory(file2_path);
            }
            else
            {
                fs::copy_file(file1_path, file2_path);
            }
            log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Created " << file2_path << " to " << folder2_path << std::endl;
            std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Created " << file2_path << " to " << folder2_path << std::endl;
        }
    }

    // Checks if the files exist in folder 2 but not in folder 1 and removes them
    for (const auto& file2_path : folder2_files) 
    {
        std::string file1_path = folder1_path + file2_path.string().substr(folder2_path.size());

        
        if (std::find(folder1_files.begin(), folder1_files.end(), file1_path) == folder1_files.end()) 
        {
            if(fs::is_directory(file2_path))
            {
                fs::remove_all(file2_path);
            }
            else
            {
                fs::remove(file2_path);
            }
            log_file << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << std::endl;
            std::cout << "[" << std::setfill('0') << std::setw(2) << local_time.tm_hour << ":" << std::setfill('0') << std::setw(2) << local_time.tm_min << ":" << std::setfill('0') << std::setw(2) << local_time.tm_sec << "] " << "Removed " << file2_path << " from " << folder2_path << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    // Command line inputs

    if (argc < 5) 
    {
        std::cerr << "Usage: " << argv[0] << " <original_folder> <sync_folder> <log_file> <interval>\n";
        return 1;
    }
    const std::string path = argv[1];
    const std::string dest_dir = argv[2];
    const std::string log_file = argv[3];
    const std::string interval = argv[4];
    
    // Create the log file and propnts a error if it is unable

    std::ofstream log_file_stream(log_file, std::ios::app);
    if (!log_file_stream) 
    {
        std::cerr << "Unable to create log\n";        
    }
    log_file_stream.close();

    //Created the destination folder if its not present where the files will be updated
    fs::create_directory(dest_dir);

    //Loop the sync indefinite and sleep for the number of secounds provided from the command line before starting the loop again
    char timestamp[32];        
    std::ofstream log(log_file, std::ios::app);
    while (true)
    {
        std::vector<fs::path> Sync;
        std::vector<fs::path> files;
        read_directory(dest_dir, Sync);
        read_directory(path, files);
        auto current_time = std::chrono::system_clock::now();
        std::time_t current_time_t = std::chrono::system_clock::to_time_t(current_time);
        ctime_s(timestamp, sizeof(timestamp), &current_time_t);

        
       
        sync_folders(files,Sync,path ,dest_dir,log_file);
        std::cout << "Sync completed at " << timestamp;
        
        std::this_thread::sleep_for(std::chrono::seconds(std::stoi(interval)));
    }

    
  
    return 0;
}

