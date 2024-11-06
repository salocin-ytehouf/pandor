#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <ctime>
#include <sqlite3.h>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>

#include <curl/curl.h>
#include <sstream>
#include <fstream>

#include "json.hpp"
using json = nlohmann::json;


std::string username = "nico@gmail.com";
std::string password = "12345";

// Define a structure to store camera parameter
struct CameraParameters {
    int id;
    std::string name;
    std::string location;
    std::string protocol;
    int port;
    std::string username;
    std::string password;
    int fps;
    bool rgb_feed;
    bool ir_feed;
    bool battery; // Added new field for battery
    std::string final_address;
};

// Define a structure to store db 
struct DatabaseParameters {
    int NODE_ID;
    Json::Value NODE_INFO;
    int NODE_BATTERY;
    std::string LAST_BATTERY_TS;
    std::string LAST_DETECTION_TS;
};

// Define a structure to store database parma and camer param
struct VideoConfig {
    std::vector<CameraParameters> cameras;
    std::vector<DatabaseParameters> db_params; // List of database parameters for each camera
};

//get db and camera param from config json file 
VideoConfig get_parameters(const std::string& filename) {
    VideoConfig video_config;

    // Read the JSON file
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return video_config;
    }

    // Parse the JSON data
    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string parseErrors;
    if (!Json::parseFromStream(builder, ifs, &root, &parseErrors)) {
        std::cerr << "Error parsing JSON: " << parseErrors << std::endl;
        return video_config;
    }

    // Extract camera parameters
    if (root.isMember("cameras") && root["cameras"].isArray()) {
        for (const auto& camera : root["cameras"]) {
            CameraParameters params;
            params.id = camera["id"].asInt();
            params.name = camera["name"].asString();
            params.location = camera["location"].asString();
            params.protocol = camera["protocol"].asString();
            params.port = camera["port"].asInt();
            params.username = camera["username"].asString();
            params.password = camera["password"].asString();
            params.fps = camera["fps"].asInt();
            params.rgb_feed = camera["rgb_feed"].asBool();
            params.ir_feed = camera["ir_feed"].asBool();
            params.battery = camera["battery"].asBool(); // Get the battery field
            params.final_address = camera["final_address"].asString();
            video_config.cameras.push_back(params);

            // Create the DatabaseParameters for each camera
            DatabaseParameters db_params;
            db_params.NODE_ID = params.id;
            db_params.NODE_INFO = camera;
            db_params.NODE_BATTERY = params.battery ? 1 : 0; // Convert bool to int


            // Get the current timestamp and format it
            std::time_t current_time = std::time(nullptr);
            char buffer[20];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&current_time));
            db_params.LAST_DETECTION_TS = buffer;
            db_params.LAST_BATTERY_TS = buffer;

            video_config.db_params.push_back(db_params);
        }
    }

    return video_config;
}

// init db 
int init_db(const std::vector<DatabaseParameters>& db_params, const std::string& db_name) {
    sqlite3* db;
    char* error_message = 0;
    int result;
    
    std::cout << "--------------------------" << std::endl;
    std::cout << "Starting initialization database." << std::endl;
    
    // Open the SQLite database
    result = sqlite3_open(db_name.c_str(), &db);
    if (result != SQLITE_OK) {
        std::cerr << "Error opening the database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return result;
    }
    // Check if there are rows in the table
    const char* check_query = "SELECT COUNT(*) FROM hw_interface_video;";
    int row_count = 0;
    result = sqlite3_exec(db, check_query, [](void* count, int argc, char** argv, char** azColName) -> int {
        int* row_count = static_cast<int*>(count);
        if (argc > 0 && argv[0])
            *row_count = std::stoi(argv[0]);
        return SQLITE_OK;
    }, &row_count, &error_message);

    if (result != SQLITE_OK) {
        std::cerr << "Error checking row count: " << error_message << std::endl;
        sqlite3_free(error_message);
        sqlite3_close(db);
        return result;
    }

    // If there are rows in the table, delete them
    if (row_count > 0) {
        const char* delete_query = "DELETE FROM hw_interface_video;";
        result = sqlite3_exec(db, delete_query, 0, 0, &error_message);
        if (result != SQLITE_OK) {
            std::cerr << "Error deleting existing rows from the table: " << error_message << std::endl;
            sqlite3_free(error_message);
            sqlite3_close(db);
            return result;
        }
    }

    // Prepare the INSERT statement
    const char* insert_query =
        "INSERT INTO hw_interface_video (NODE_ID, NODE_BATTERY, LAST_BATTERY_TS, LAST_DETECTION_TS) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt* insert_stmt;
    result = sqlite3_prepare_v2(db, insert_query, -1, &insert_stmt, nullptr);
    if (result != SQLITE_OK) {
        std::cerr << "Error preparing INSERT statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return result;
    }

    // Bind and execute the INSERT statement for each DatabaseParameters
    for (const auto& db_param : db_params) {
        // Serialize the Json::Value to a string
        sqlite3_bind_int(insert_stmt, 1, db_param.NODE_ID);
        sqlite3_bind_int(insert_stmt, 2, db_param.NODE_BATTERY);
        sqlite3_bind_text(insert_stmt, 3, db_param.LAST_BATTERY_TS.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(insert_stmt, 4, db_param.LAST_DETECTION_TS.c_str(), -1, SQLITE_STATIC);

        result = sqlite3_step(insert_stmt);
        if (result != SQLITE_DONE) {
            std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(insert_stmt);
            sqlite3_close(db);
            return result;
        }

        // Reset the prepared statement for the next iteration
        sqlite3_reset(insert_stmt);
    }

    // Finalize the prepared statement
    sqlite3_finalize(insert_stmt);

    // Close the database
    sqlite3_close(db);

    std::cout << "Database initialized successfully." << std::endl;
    std::cout << "--------------------------" << std::endl;
    return SQLITE_OK;
}

//cloud access JWT plus upload image 
// Shared JWT token
std::string jwtToken;

// Mutex to protect shared JWT token
std::mutex jwtMutex;

std::string base64_encode(const uchar* input, size_t length) {
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int i = 0;
    int j = 0;
    uchar char_array_3[3];
    uchar char_array_4[4];

    while (length--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                encoded += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j <= i; j++) {
            encoded += base64_chars[char_array_4[j]];
        }

        while (i++ < 3) {
            encoded += '=';
        }
    }

    return encoded;
}

size_t WriteCallbackJwt(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string createJwtUser(std::string username, std::string password) {
    const std::string url = "http://pandor.com/login";
    const std::string credentials = username + ":" + password;
    const int max_retry_attempts = 3;  // Maximum number of retry attempts

    // Initialize libcurl
    CURL* curl = curl_easy_init();

    if (curl) {
        CURLcode res;
        std::string response_buffer;
        bool success = false;

        for (int attempt = 1; attempt <= max_retry_attempts; ++attempt) {
            // Set the URL
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

            // Set HTTP basic authentication credentials
            curl_easy_setopt(curl, CURLOPT_USERPWD, (username + ":" + password).c_str());

            // Set the request method to POST
            curl_easy_setopt(curl, CURLOPT_POST, 1L);

            // Set an empty request body
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");

            // Set the callback function to capture the response
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackJwt);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);

            // Perform the HTTP POST request
            res = curl_easy_perform(curl);

            // Check for errors
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            } else {
                jwtMutex.lock();
                jwtToken = response_buffer;
                jwtMutex.unlock();
                std::cout << "Response: " << response_buffer << std::endl;
                // Set success to true and break out of the loop
                success = true;
                break;

            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Cleanup libcurl
        curl_easy_cleanup(curl);

        if (success) {
            return jwtToken; // Return the JWT token when successful
        }
    } else {
        std::cerr << "Failed to initialize libcurl." << std::endl;
    }

    return "";  // Return an empty string if token creation fails
}

// Callback function to write response data
size_t WriteCallbackImg(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}

void uploadImage(const cv::Mat& image1, const cv::Mat& image2) {
    const std::string urlToPost = "http://pandor.com/upload";

    // Encode the images as JPEG
    std::vector<uchar> image1_buffer, image2_buffer;
    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
    compression_params.push_back(95);

    cv::imencode(".jpeg", image1, image1_buffer, compression_params);
    cv::imencode(".jpeg", image2, image2_buffer, compression_params);

    std::string image1_base64 = base64_encode(image1_buffer.data(), image1_buffer.size());
    std::string image2_base64 = base64_encode(image2_buffer.data(), image2_buffer.size());


    // Get current timestamp
    auto currentTime = std::chrono::system_clock::now();
    auto currentTimestamp = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()).count();

    // Prepare metadata for each image
    json image1Metadata, image2Metadata;
    image1Metadata["worker_id"] = "aaaaaa";
    image1Metadata["timestamp"] = static_cast<int>(currentTimestamp);
    image1Metadata["camera_id"] = "camera1";
    image1Metadata["is_door"] = true; // Use boolean value here
    image2Metadata["worker_id"] = "aaaaaa";
    image2Metadata["timestamp"] = static_cast<int>(currentTimestamp);
    image2Metadata["camera_id"] = "camera2";
    image2Metadata["is_door"] = false; // Use boolean value here

    // Create the payload JSON
    json payload;
    payload["image1"]["data"] = image1_base64;
    payload["image1"]["metadata"] = image1Metadata;
    payload["image2"]["data"] = image2_base64;
    payload["image2"]["metadata"] = image2Metadata;

    // Serialize JSON payload to a string
    std::string payloadJson = payload.dump();
    
    // Save payload to a file
    std::ofstream payloadFile("payload.json");
    payloadFile << payloadJson;
    payloadFile.close();

    // Build and execute the cURL command
    std::string curlCommand = "curl -X POST -H \"Authorization: Bearer " +  jwtToken + "\" -H \"Content-Type: application/json\" -d @payload.json " + urlToPost;
    std::cout << "cURL command: " << curlCommand << std::endl;
    int result = system(curlCommand.c_str());

    if (result == 0) {
        std::cout << "Image upload successful." << std::endl;
    } else {
        std::cerr << "Image upload failed." << std::endl;
    }
}



// ######################   WORKER   CAPTURE   IMAGE    #######################
// Define a structure to store image data and associated information
struct ImageData {
    int camera_id;
    cv::Mat image;
    std::chrono::system_clock::time_point timestamp; // Add a timestamp

};

// Define a vector of queues to store the image data for each camera
std::vector<std::queue<ImageData>> image_queues;

// Define a vector of mutexes to protect the access to the image_queues
std::vector<std::unique_ptr<std::mutex>> queue_mutexes;

// Function to initialize the mutexes abd queues for each camera
void initializeMutexes(const VideoConfig& videoConfig) {
    // Resize the vectors to hold the required number of queues and mutexes
    image_queues.resize(videoConfig.cameras.size());
    queue_mutexes.resize(videoConfig.cameras.size());
    // Create and assign a unique_ptr to std::mutex for each camera
    for (size_t i = 0; i < videoConfig.cameras.size(); ++i) {
        queue_mutexes[i] = std::make_unique<std::mutex>();
    }
}

// print queues
void printImageQueues() {
    std::cout << "Contents of image_queues:" << std::endl;
    for (size_t i = 0; i < image_queues.size(); ++i) {
        std::cout << "Camera " << i << " queue size: " << image_queues[i].size() << std::endl;
    }
    std::cout << "--------------------------" << std::endl;
}

// print mutexes
void printQueueMutexes() {
    std::cout << "Contents of queue_mutexes:" << std::endl;
    for (size_t i = 0; i < queue_mutexes.size(); ++i) {
        std::cout << "Camera " << i << " mutex address: " << queue_mutexes[i].get() << std::endl;
    }
    std::cout << "--------------------------" << std::endl;
}

// Declare the threads vector for capture image before the main function
std::vector<std::thread> threads;

//worker for capture image
void worker_capture_images(const std::string& video_stream, int camera_id, int target_fps) {
    cv::VideoCapture cap(video_stream);
    if (!cap.isOpened()) {
        std::cerr << "Error opening video stream: " << video_stream << std::endl;
        return;
    }
    
    //set buffer size to one 
    bool buffer_size_set = cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    if (!buffer_size_set) {
        std::cerr << "Error setting buffer size" << std::endl;
        return;
    }
    else{
        std::cout << "succes setting buffer size" << std::endl;
    }

    int frame_delay = 1000 / target_fps; // Delay in milliseconds between frames
    cv::Mat frame;
    while (true) {
        auto start_time = std::chrono::high_resolution_clock::now();
        cap.grab();
        if (!cap.read(frame)) {
            std::cerr << "Error reading frame from camera: " << camera_id << std::endl;
            continue;
        }
        
        //std::cout << "      Final Address: " << camera.final_address << std::endl;
        // Push the image data into the queue for this camera
        cv::imshow("Frame "+ std::to_string(camera_id), frame);
        if (cv::waitKey(1) == 'q') {
            break;
        }
        
        ImageData data;
        data.camera_id = camera_id;
        data.image = frame.clone(); // Make a copy of the frame to avoid data corruption not sure its neccessary ??
        data.timestamp = std::chrono::system_clock::now(); // Get the current timestamp

            {
            // Lock the mutex before accessing the image_queue for this camera
            std::lock_guard<std::mutex> lock(*queue_mutexes[camera_id]);
            image_queues[camera_id].push(data);
            } // The std::unique_lock automatically unlocks the mutex upon going out of scope

        // Calculate the time spent processing the frame and sleep if needed to achieve the target FPS
        auto end_time = std::chrono::high_resolution_clock::now();
        auto elapsed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        int remaining_delay = frame_delay - elapsed_time_ms;
        if (remaining_delay > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(remaining_delay));
        }
    }
    // Release the VideoCapture and close any OpenCV windows
    cap.release();
    cv::destroyAllWindows();
}

// ######################   WORKER   MOTION DETECTOR    #######################

std::string time_point_to_string(const std::chrono::system_clock::time_point& timestamp) {
    std::time_t time_t_stamp = std::chrono::system_clock::to_time_t(timestamp);
    std::tm timeinfo;
    localtime_r(&time_t_stamp, &timeinfo);

    char buffer[20]; // 19 characters for "%Y-%m-%d %H:%M:%S" plus null-terminator
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);

    return std::string(buffer);
}


void update_database(std::chrono::system_clock::time_point timestamp, int camera_id, const std::string& db_name) {
    // Open SQLite database "pandore.db"
    sqlite3* db;
    int rc = sqlite3_open(db_name.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // Check if a row exists with the given NODE_ID
    std::string check_query = "SELECT COUNT(*) FROM hw_interface_video WHERE NODE_ID = ?";
    sqlite3_stmt* check_stmt;
    rc = sqlite3_prepare_v2(db, check_query.c_str(), -1, &check_stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error preparing check query: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // Bind the camera_id to the check statement
    rc = sqlite3_bind_int(check_stmt, 1, camera_id);
    if (rc != SQLITE_OK) {
        std::cerr << "Error binding camera_id: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(check_stmt);
        sqlite3_close(db);
        return;
    }

    // Execute the check query and get the count of rows with the given NODE_ID
    int row_count = 0;
    while ((rc = sqlite3_step(check_stmt)) == SQLITE_ROW) {
        row_count = sqlite3_column_int(check_stmt, 0);
    }

    sqlite3_finalize(check_stmt);

    if (row_count > 0) {
        // Node exists, update the timestamp
        std::string update_query = "UPDATE hw_interface_video SET LAST_DETECTION_TS = ? WHERE NODE_ID = ?";
        sqlite3_stmt* update_stmt;
        rc = sqlite3_prepare_v2(db, update_query.c_str(), -1, &update_stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing update query: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return;
        }
        std::string timestamp_str = time_point_to_string(timestamp);
        std::cout << "Formatted timestamp: " << timestamp_str << std::endl;
        std::cout << "Type of timestamp: " << typeid(timestamp).name() << std::endl;
        // Bind the timestamp and camera_id to the update statement
        rc = sqlite3_bind_text(update_stmt, 1, timestamp_str.c_str(), -1, SQLITE_STATIC);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding timestamp: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(update_stmt);
            sqlite3_close(db);
            return;
        }

        rc = sqlite3_bind_int(update_stmt, 2, camera_id);
        if (rc != SQLITE_OK) {
            std::cerr << "Error binding camera_id: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(update_stmt);
            sqlite3_close(db);
            return;
        }

        // Execute the update statement
        rc = sqlite3_step(update_stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error updating data: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_finalize(update_stmt);
    } else {
        // Node does not exist, you may handle this case accordingly (e.g., insert a new row).
        // This implementation only updates the timestamp if the node exists in the table.
        std::cout << "Node with NODE_ID " << camera_id << " not found in the table." << std::endl;
    }

    // Close the database connection
    sqlite3_close(db);
}


bool motion_detection(const cv::Mat& frame_1, const cv::Mat& frame_2, int min_area) {
    // Resize the frame_1, convert it to grayscale, and blur it
    cv::Mat resized_frame_1, resized_frame_2;

    cv::resize(frame_1, resized_frame_1, cv::Size(500, frame_1.rows * 500 / frame_1.cols));
    cv::resize(frame_2, resized_frame_2, cv::Size(500, frame_2.rows * 500 / frame_2.cols));
    
    cv::Mat gray_1, gray_2;
    cv::cvtColor(resized_frame_1, gray_1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(resized_frame_2, gray_2, cv::COLOR_BGR2GRAY);

    cv::GaussianBlur(gray_1, gray_1, cv::Size(21, 21), 0);
    cv::GaussianBlur(gray_2, gray_2, cv::Size(21, 21), 0);

    cv::Mat frameDelta;
    cv::absdiff(gray_1, gray_2, frameDelta);
    cv::imshow("frameDelta, Frame 188888888", frameDelta);
    cv::threshold(frameDelta, frameDelta, 25, 255, cv::THRESH_BINARY);
    cv::imshow("frameDelta, Frame 2999999", frameDelta);


    cv::dilate(frameDelta, frameDelta, cv::Mat(), cv::Point(-1, -1), 2);
    cv::imshow("frameDelta", frameDelta);
 
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(frameDelta, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Loop over the contours
    for (const auto& c : contours) {
        // If the contour is too small, ignore it
        if (cv::contourArea(c) > min_area) {
            return true;
        }
    }

    return false;
}

// Motion detector worker function
void motion_worker(const std::vector<std::queue<ImageData>>& queues, int min_area, const std::string& db_name) {
    while (true) {
        for (size_t i = 0; i < image_queues.size(); ++i){           
            if (image_queues[i].size() >= 2) {
                std::lock_guard<std::mutex> lock(*queue_mutexes[i]); // Lock the mutex before accessing the queue
                std::cout << "Camera " << i << " queue size: " << image_queues[i].size() << std::endl;
                // Get the first two frames from the queue
                ImageData frame_1 = image_queues[i].front();
                image_queues[i].pop();        
                ImageData frame_2 = image_queues[i].front();
                image_queues[i].pop();               
                // Calculate the elapsed time between the two timestamps
                auto duration = frame_2.timestamp - frame_1.timestamp;
                // Convert the duration to milliseconds (or any other desired unit)
                auto elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
                // Print the elapsed time
                std::cout << "Elapsed time: " << elapsed_milliseconds << " milliseconds" << std::endl;
                auto time_1 = std::chrono::system_clock::to_time_t(frame_1.timestamp);
                auto time_2 = std::chrono::system_clock::to_time_t(frame_2.timestamp);
                // Perform motion detection between frame_1 and frame_2
                bool motion_detected = motion_detection(frame_1.image, frame_2.image, min_area);
                // Update the database with motion detection results
                if (motion_detected){
                    update_database(frame_1.timestamp, frame_1.camera_id, db_name);
                    uploadImage(frame_1.image, frame_2.image);
                }
                
            }
        }
    }
}



/*
#################################################################################
#################################################################################
MAIN
#################################################################################
#################################################################################
*/

int main() {
    // Open a log file to redirect stderr
    std::ofstream logFile("error_log.txt");
    // Redirect stderr to the log file
    std::streambuf* originalStderr = std::cerr.rdbuf(logFile.rdbuf());

    std::string filename = "../video_config.json";
    std::string db_name = "../pandore.db";

    VideoConfig video_config = get_parameters(filename);

    // Print the extracted parameters for each camera
    for (const auto& camera : video_config.cameras) {
        std::cout << "Camera " << camera.id << " - Name: " << camera.name << ", Location: " << camera.location<< std::endl;
    }

    // Print the database parameters for each camera
    for (const auto& db_params : video_config.db_params) {
        std::cout << "Camera " << db_params.NODE_ID
                  << ", LAST_STATE_TS: " << db_params.LAST_DETECTION_TS << std::endl;
    }

    // Initialize the database with the extracted database parameters
    int init_result = init_db(video_config.db_params, db_name);
    if (init_result != SQLITE_OK) {
        std::cerr << "Error initializing the database." << std::endl;
        return init_result;
    }

    // initialize queues vector and mutexes vectors  
    initializeMutexes(video_config);

    // Print the contents of the vectors
    printImageQueues();
    printQueueMutexes();

    for (const auto& camera : video_config.cameras) {
        std::cout << "Starting worker with id :" << camera.id << std::endl;
        std::cout << "      Camera ID: " << camera.id << std::endl;
        std::cout << "      Final Address: " << camera.final_address << std::endl;
        std::cout << "      FPS: " << camera.fps << std::endl;
        // comprendre ce qu'est une lambda function deja vu ca ave cthinker 
        threads.emplace_back([&camera]() {
            worker_capture_images(camera.final_address, camera.id, camera.fps);
        });
        std::cout << "--------------------------" << std::endl;

    }

    jwtToken = createJwtUser(username, password);

    // Start the motion_detector worker function in a separate thread
    int min_area = 500; // You can adjust this threshold based on your requirements
    std::thread motion_detector_thread(motion_worker, std::ref(image_queues), min_area, db_name);

    // Wait for all threads to finish
    for (auto& thread : threads) {
        thread.join();
    }
    // Join the motion_detector_thread to properly terminate the program
    motion_detector_thread.join();

    // Restore stderr to its original stream buffer
    std::cerr.rdbuf(originalStderr);
    // Close the log file
    logFile.close();

    return 0;
}
