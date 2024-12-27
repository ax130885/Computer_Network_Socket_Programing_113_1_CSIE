#include <iostream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <vector>
#include <memory> // c++14 智能指標 std::unique_ptr
#include <atomic> // 使用 std::atomic
#include <thread> // 使用 std::thread

#include <fstream>       // 傳檔案用
#include <sys/stat.h>    // 傳檔案用
#include <dirent.h>      // 列出資料夾內的檔案用
#include <openssl/ssl.h> // 需先 sudo yum install openssl-devel
#include <openssl/err.h> // 需先 sudo yum install openssl-devel
#include <map>

#define MAX_THREADS 10
#define CHUNCK_SIZE (4 * 1024 * 1024)
#define SERVER_IP "140.112.183.112"
#define SERVER_WELCOME_PORT 48763

// 初始化身份認證 用戶資料庫
std::unordered_map<std::string, std::string> users;
// 定義一個 map 來儲存用戶名和 SSL 連接的綁定
std::map<std::string, SSL *> logged_in_user_ssl_map;

// 初始化多執行緒
std::queue<int> client_queue;                            // mutex 只會鎖住這個變數 不會影響後續function的平行執行
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER; // 初始化互斥鎖
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;    // 初始化條件變數

// 初始化SSL context
SSL_CTX *ctx = nullptr;

// 註冊用戶
bool registerUser(const std::string &username, const std::string &password)
{
    if (users.find(username) != users.end())
    {
        return false; // 用戶已存在
    }
    users[username] = password;
    return true;
}

// 登入用戶
bool loginUser(const std::string &username, const std::string &password)
{
    auto it = users.find(username);
    if (it != users.end() && it->second == password)
    {
        return true;
    }
    return false;
}

// 定義一個結構來包含每個 pair 的 exit flag
struct ClientPair
{
    std::string source_username;
    std::string target_username;
    SSL *source_ssl;
    SSL *target_ssl;
    std::atomic<bool> exit_flag;

    // 宣告此結構接收兩個函數，並且賦值給source_ssl和target_ssl。另外將exit_flag設為false
    ClientPair(const std::string &source_user, const std::string &target_user, SSL *source, SSL *target)
        : source_username(source_user), target_username(target_user), source_ssl(source), target_ssl(target), exit_flag(false) {}
};

// 處理 client to client的訊息轉發
void handleSourceClientSend(ClientPair *client_pair)
{
    char buffer[4096];
    while (!client_pair->exit_flag)
    {
        memset(buffer, 0, 4096);
        // 接收來自source client端的訊息
        int bytes_received = SSL_read(client_pair->source_ssl, buffer, 4096);
        if (bytes_received <= 0)
        {
            std::cerr << "Error receiving message from source client (SourceClientSend)" << std::endl;
            client_pair->exit_flag = true; // 設置標誌
            break;
        }

        // 印出source client端傳來的訊息
        std::cout << "Received from " << client_pair->source_username << ", Send to " << client_pair->target_username << ": " << std::string(buffer, bytes_received) << std::endl;

        // 如果收到的訊息為 "exit" 則設置標誌，回傳確認exit訊息給source(讓client的ssl read跳出堵塞)，並且跳出循環。
        if (std::string(buffer, bytes_received) == "exit")
        {
            std::cout << "Exit command received from " << client_pair->source_username << " to " << client_pair->source_username << "  connection." << std::endl;
            client_pair->exit_flag = true;
            std::string exit_message = "確認exit";
            int bytes_sent = SSL_write(client_pair->source_ssl, exit_message.c_str(), exit_message.size());
            if (bytes_sent <= 0)
            {
                std::cerr << "Error sending exit message to target client (SourceClientSend)" << std::endl;
            }
            sleep(0.1);
            break;
        }

        // 把訊息加上前綴 "source username: "
        std::string message = client_pair->source_username + ": " + std::string(buffer, bytes_received);

        // 轉發訊息給target client端
        int bytes_sent = SSL_write(client_pair->target_ssl, message.c_str(), message.size());
        if (bytes_sent <= 0)
        {
            std::cerr << "Error sending message to target client (SourceClientSend)" << std::endl;
            client_pair->exit_flag = true; // 設置標誌
            break;
        }
    }
}

// 登入後的功能
void handleLoggedInUser(SSL *ssl, int client_socket, const std::string &username)
{
    char buffer[4096];
    std::string prompt;

    while (1)
    {
        std::string menu = "\n選擇服務:\n1. 回聲\n2. 傳送訊息給伺服器\n3. 傳送檔案給伺服器\n4. 傳送訊息給其他 Client\n9. 離開\n請輸入阿拉伯數字以選擇模式: ";
        std::cout << "User " << username << "選擇服務\n"
                  << std::endl;
        SSL_write(ssl, menu.c_str(), menu.size());

        sleep(0.1);

        memset(buffer, 0, 4096);
        int bytes_received = SSL_read(ssl, buffer, 4096);
        if (bytes_received <= 0)
        {
            std::cerr << "Error receiving message in handleLoggedInUser() begin" << std::endl;
            return;
        }

        // // 印出buffer內容 檢查選擇的功能代號
        // if (bytes_received > 0)
        // {
        //     std::cout << std::string(buffer, bytes_received) << std::endl;
        // }

        std::string service_choice(buffer, bytes_received);
        service_choice = service_choice.substr(0, service_choice.find('\n')); // 去除換行符號
        if (service_choice == "1")
        {
            prompt = "現正使用1. 回聲\n請輸入訊息: ";
            std::cout << "User " << username << " 現正使用1. 回聲"
                      << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            if (bytes_received <= 0)
            {
                std::cerr << "Error receiving message in 現正使用1. 回聲" << std::endl;
                return;
            }
            std::string message(buffer, bytes_received);
            std::cout << "收到來自 " << username << " 輸入的訊息: " << message << std::endl;
            SSL_write(ssl, message.c_str(), message.size());
        }
        else if (service_choice == "2")
        {
            prompt = "現正使用2. 傳送訊息給伺服器\n請輸入訊息: ";
            std::cout << "User " << username << " 現正使用2. 傳送訊息給伺服器"
                      << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            if (bytes_received <= 0)
            {
                std::cerr << "Error receiving message in 現正使用2. 傳送訊息給伺服器" << std::endl;
                return;
            }
            std::string message(buffer, bytes_received);
            std::cout << "收到來自 " << username << " 的訊息: " << message << std::endl;
        }
        else if (service_choice == "3")
        {
            prompt = "現正使用3. 傳送檔案給伺服器 (暫不支持資料夾 僅能傳送單一檔案)\n請輸入檔案名 : ";
            std::cout << "收到來自 " << username << " 的檔案傳送請求" << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());

            // 接收client端有無讀取檔案成功
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            // 如果收到"open_file_success"，代表client端讀取檔案成功
            if (bytes_received > 0 && std::string(buffer, bytes_received) == "open_file_success")
            {
                // 發送確認訊息給客戶端
                std::string confirmation = "File opened successfully";
                SSL_write(ssl, confirmation.c_str(), confirmation.size());
                std::cout << "Client opened file successfully" << std::endl;
            }
            // 如果收到"open_file_failed"，代表client端讀取檔案失敗
            else
            {
                std::cerr << "Client failed to open file" << std::endl;
                return;
            }

            // 接收檔案名
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            // 印出client端傳來的檔案名
            std::cout << "User " << username << " 上傳檔案名: " << std::string(buffer, bytes_received) << std::endl;
            // 如果接收失敗或者接收到的檔案名為空，則關閉socket
            if (bytes_received <= 0)
            {
                std::cerr << "Failed to receive file name" << std::endl;
                return;
            }
            std::string filename(buffer, bytes_received);

            // 檢查並創建 download 資料夾
            struct stat info;
            if (stat("download", &info) != 0)
            {
                // 資料夾不存在，創建資料夾
                if (mkdir("download", 0777) == -1)
                {
                    std::cerr << "Failed to create directory: download" << std::endl;
                    return;
                }
                std::cout << "download 資料夾不存在，新建資料夾" << std::endl;
            }
            else if (!(info.st_mode & S_IFDIR))
            {
                std::cerr << "download is not a directory" << std::endl;
                return;
            }

            std::string filepath = "download/" + filename;

            // 創建檔案
            std::ofstream outfile(filepath, std::ios::binary);
            std::cout << "User " << username << " 創建檔案: " << filepath << std::endl;

            if (!outfile)
            {
                std::cerr << "Failed to open file for writing: " << filepath << std::endl;
                return;
            }

            // 接收檔案大小
            std::streamsize file_size;
            SSL_read(ssl, &file_size, sizeof(file_size));
            std::cout << "Expected file size: " << file_size << " bytes" << std::endl;

            // 接收檔案內容
            char file_buffer[CHUNCK_SIZE];
            std::streamsize total_received = 0;

            while (total_received < file_size)
            {
                memset(file_buffer, 0, CHUNCK_SIZE);
                // 計算剩餘的字節數
                size_t remaining = file_size - total_received;
                // 決定每次接收多少字節數
                std::streamsize to_receive = std::min(static_cast<std::streamsize>(remaining), static_cast<std::streamsize>(CHUNCK_SIZE));

                // 接收檔案數據
                bytes_received = SSL_read(ssl, file_buffer, to_receive);
                if (bytes_received <= 0)
                {
                    std::cerr << "Error receiving file data" << std::endl;
                    break;
                }

                // 寫入檔案
                outfile.write(file_buffer, bytes_received);
                total_received += bytes_received;

                // 印出傳輸進度
                float progress = (float)total_received / file_size * 100;
                std::cout << "\rReceiving: " << progress << "%" << std::endl;
            }
            outfile.close();
            std::cout << "File received: " << filepath << std::endl;

            // 發送確認訊息給客戶端
            std::string confirmation = "File received successfully\n";
            SSL_write(ssl, confirmation.c_str(), confirmation.size());
        }
        else if (service_choice == "4")
        {
            // 印出所有用戶名
            std::string all_users = "所有用戶名:\n";
            for (const auto &user : users)
            {
                all_users += user.first + "\n";
            }

            prompt = "現正使用4. 傳送訊息給其他 Client\n請先綁定要傳送訊息的用戶名:\n" + all_users + "請輸入用戶名: ";
            std::cout << "User " << username << " 現正使用4. 傳送訊息給其他 Client"
                      << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());

            // 接收選擇的用戶名稱 並確認是否存在
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            if (bytes_received <= 0)
            {
                std::cerr << "Error receiving message in 現正使用4. 傳送訊息給其他 Client" << std::endl;
                return;
            }
            std::string targetusername(buffer, bytes_received);
            targetusername = targetusername.substr(0, targetusername.find('\n')); // 去除換行符號

            // 如果用戶名不存在，要求重新選擇功能
            if (users.find(targetusername) == users.end())
            {
                prompt = "用戶名不存在，請重新選擇功能。\n";
                std::cout << "User " << username << " 用戶名不存在，請重新選擇功能。\n"
                          << std::endl;
                SSL_write(ssl, prompt.c_str(), prompt.size());
                continue;
            }

            // 如果用戶名稱==自己名稱，要求重新選擇功能
            if (targetusername == username)
            {
                prompt = "不能傳送訊息給自己，請重新選擇功能。\n";
                std::cout << "User " << username << " 不能傳送訊息給自己，請重新選擇功能。\n"
                          << std::endl;
                SSL_write(ssl, prompt.c_str(), prompt.size());
                continue;
            }

            // 如果用戶名稱存在，回傳綁定成功，獲取traget client對應的SSL
            prompt = "User " + username + " 與 " + targetusername + " 綁定成功！\n請輸入想要傳給對方的訊息。或是輸入exit來解除綁定，並且回到功能選單。\n";
            std::cout << "User " << username << " 與 " << targetusername << " 綁定成功！\n"
                      << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());

            // 獲取traget client對應的SSL
            SSL *target_ssl = logged_in_user_ssl_map[targetusername];

            // 創建一個新的 ClientPair 來綁定 source client 和 target client 並且記錄exit_flag
            ClientPair *client_pair = new ClientPair{username, targetusername, ssl, target_ssl};

            // 創建一個新thread 處理發送和接收訊息
            std::thread source_send_thread(handleSourceClientSend, client_pair);

            // 等待所有thread完成 join會阻塞當前thread 等到指定的thread完成後才會繼續執行
            source_send_thread.join();
            // source_receive_thread.join();

            // 刪除 client_pair
            std::cout << "User " << username << " 與 " << targetusername << " 解除綁定！\n"
                      << std::endl;
            delete client_pair;

            sleep(0.1);
            continue;
        }
        else if (service_choice == "9")
        {
            prompt = "程式執行完畢！ 登出並離開\n";
            std::cout << "User " << username << " 程式執行完畢！ 登出並離開\n"
                      << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());

            // 登出後，將username和ssl解綁
            logged_in_user_ssl_map.erase(username);
            return;
        }
        else
        {
            prompt = "未知的選項，請重新選擇功能。\n";
            std::cout << "User " << username << " 未知的選項，請重新選擇功能。\n"
                      << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());
        }
        // 延遲0.1秒 避免send的內容跟後面合在一起送出
        usleep(100000);
    }
}

// 建立連線後 選擇註冊或登入
void *handleClient(void *arg)
{
    std::unique_ptr<int> client_socket_ptr = std::make_unique<int>(*static_cast<int *>(arg)); // 將 void* 轉換為 unique_ptr
    int client_socket = *client_socket_ptr;                                                   // 使用解引用

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_socket);
    if (SSL_accept(ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
        close(client_socket);
        return nullptr;
    }

    char buffer[4096];
    std::string username;

    // 歡迎訊息
    std::string welcomeMsg = "歡迎來到伺服器！\n1. 註冊\n2. 登入\n9. 離開\n請輸入阿拉伯數字以選擇模式: ";
    SSL_write(ssl, welcomeMsg.c_str(), welcomeMsg.size());

    std::string prompt;

    while (true)
    {
        memset(buffer, 0, 4096);
        int bytes_received = SSL_read(ssl, buffer, 4096);
        if (bytes_received <= 0)
        {
            break; // 跳出循環，進行資源釋放
        }

        std::string choice(buffer, bytes_received);

        if (choice == "1") // 註冊
        {
            prompt = "請輸入用戶名: ";
            SSL_write(ssl, prompt.c_str(), prompt.size());
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            if (bytes_received <= 0)
            {
                break; // 跳出循環，進行資源釋放
            }
            std::string username(buffer, bytes_received);

            prompt = "請輸入密碼: ";
            SSL_write(ssl, prompt.c_str(), prompt.size());
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            if (bytes_received <= 0)
            {
                break; // 跳出循環，進行資源釋放
            }
            std::string password(buffer, bytes_received);

            if (registerUser(username, password))
            {
                prompt = "註冊成功！\n請重新選擇功能。\n";
                std::cout << "User " << username << " 註冊成功\n"
                          << std::endl;
                SSL_write(ssl, prompt.c_str(), prompt.size());
            }
            else
            {
                prompt = "用戶已存在，註冊失敗。\n請重新選擇功能。\n";
                std::cout << "User " << username << " 用戶已存在，註冊失敗\n"
                          << std::endl;
                SSL_write(ssl, prompt.c_str(), prompt.size());
            }
        }
        else if (choice == "2") // 登入
        {
            prompt = "請輸入用戶名: ";
            SSL_write(ssl, prompt.c_str(), prompt.size());
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            if (bytes_received <= 0)
            {
                break; // 跳出循環，進行資源釋放
            }
            username = std::string(buffer, bytes_received);

            prompt = "請輸入密碼: ";
            SSL_write(ssl, prompt.c_str(), prompt.size());
            memset(buffer, 0, 4096);
            bytes_received = SSL_read(ssl, buffer, 4096);
            if (bytes_received <= 0)
            {
                break; // 跳出循環，進行資源釋放
            }
            std::string password(buffer, bytes_received);

            if (loginUser(username, password))
            {
                prompt = "登入成功！\n";
                std::cout << "User " << username << " 登入成功\n"
                          << std::endl;
                SSL_write(ssl, prompt.c_str(), prompt.size());
                // 延遲0.1秒 避免send的內容跟後面合在一起送出
                usleep(100000);

                // 將username和ssl綁定
                logged_in_user_ssl_map[username] = ssl;

                // 重點:處理登入後功能
                handleLoggedInUser(ssl, client_socket, username);
                break;
            }
            else
            {
                prompt = "登入失敗，用戶名或密碼錯誤。\n請重新選擇功能。\n";
                std::cout << "User " << username << " 登入失敗，用戶名或密碼錯誤\n"
                          << std::endl;
                SSL_write(ssl, prompt.c_str(), prompt.size());
            }
        }
        else if (choice == "9") // 離開
        {
            prompt = "註冊選單執行完畢！ 離開\n";
            std::cout << "User " << username << " 註冊選單執行完畢！ 離開\n"
                      << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());
            break; // 跳出循環，進行資源釋放
        }
        else
        {
            prompt = "未知的選項，請重新輸入功能代號。\n";
            std::cout << "User " << username << " 未知的選項，請重新輸入功能代號\n"
                      << std::endl;
            SSL_write(ssl, prompt.c_str(), prompt.size());
        }
        // 延遲0.1秒 避免send的內容跟後面合在一起送出
        usleep(100000);
    }

    // 資源釋放
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_socket);
    return nullptr;
}

void *workerThread(void *arg)
{
    while (true)
    {
        std::unique_ptr<int> client_socket = nullptr; // 因為要傳給handleClient，所以要用手動管理記憶體。否則直接給定局部變數，會在handleClient執行完畢後被釋放。

        // 先鎖住mutex，然後檢查佇列是否為空，如果是空的，就等待條件變數觸發釋放這個鎖
        pthread_mutex_lock(&queue_mutex);
        while (client_queue.empty()) // 如果佇列為空，就等待條件變數，並且釋放queue_mutex。等到queue_cond被觸發，才會繼續執行
        {
            pthread_cond_wait(&queue_cond, &queue_mutex); // pthread_cond_wait(條件變數, 互斥鎖) 只有等待的時候會自動釋放互斥鎖，觸發的時候需要手動上鎖(在main的while(true)當中)
        }

        // 如果佇列不為空，就取出佇列的第一個元素
        client_socket = std::make_unique<int>(client_queue.front()); // 動態分配記憶體，並從佇列中取出元素 // 取出佇列的第一個元素
        client_queue.pop();                                          // 將取出的元素從佇列中刪除
        pthread_mutex_unlock(&queue_mutex);                          // 解鎖互斥鎖

        // 建立連線後 選擇註冊或登入
        handleClient(client_socket.get()); // 因為workerThread已經被多執行緒執行，因此handleClient也會被多執行緒執行。不需要再創建新的執行緒
    }

    return nullptr;
}

int main()
{
    // 初始化 OpenSSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    const SSL_METHOD *method = SSLv23_server_method(); // 使用 SSLv23_server_method() 方法
    ctx = SSL_CTX_new(method);                         // 創建 SSL context
    if (!ctx)
    {
        std::cerr << "Unable to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        return 1;
    }

    // 設定 SSL 認證
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        return 1;
    }

    // 創建伺服器 socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // 設定伺服器地址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;                  // 使用IPv4
    server_addr.sin_port = htons(SERVER_WELCOME_PORT); // 設定port
    server_addr.sin_addr.s_addr = INADDR_ANY;          // 允許所有可用的網路介面 可以設置為特定的IP地址

    // 綁定 socket 到指定的 IP 和 port
    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to bind to port " << SERVER_WELCOME_PORT << std::endl;
        return 1;
    }

    // 監聽連線
    if (listen(server_socket, SOMAXCONN) == -1)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port " << SERVER_WELCOME_PORT << std::endl;

    // 創建工作執行緒
    std::vector<pthread_t> threads(MAX_THREADS);
    for (int i = 0; i < MAX_THREADS; ++i)
    {
        if (pthread_create(&threads[i], nullptr, workerThread, nullptr) != 0) // pthread_create(占用的執行緒tid, 執行緒的屬性(默認NULL), 執行函數, 執行函數的參數)
        {
            std::cerr << "Failed to create thread" << std::endl;
            return 1;
        }
    }

    while (true)
    {
        sockaddr_in client_addr;
        socklen_t client_size = sizeof(client_addr);
        // accept 會暫停直到有客戶端連線進來 並且回傳一個新的 socket 來處理這個連線
        int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_size);
        if (client_socket == -1)
        {
            std::cerr << "Failed to grab connection" << std::endl;
            continue;
        }

        // 儲存客戶端的 IP 和 port
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);

        std::cout << "Client connected: " << client_ip << ":" << client_port << std::endl;

        // 將客戶端連接加入佇列
        pthread_mutex_lock(&queue_mutex);   // 鎖住互斥鎖
        client_queue.push(client_socket);   // 將 client_socket 加入 client_queue 佇列
        pthread_cond_signal(&queue_cond);   // 喚醒一個等待的執行緒
        pthread_mutex_unlock(&queue_mutex); // 解鎖互斥鎖
    }

    close(server_socket); // 關閉伺服器 socket

    // 回收所有工作執行緒
    for (pthread_t &thread : threads)
    {
        pthread_join(thread, nullptr);
    }

    // 銷毀互斥鎖和條件變數
    pthread_mutex_destroy(&queue_mutex);
    pthread_cond_destroy(&queue_cond);

    return 0;
}