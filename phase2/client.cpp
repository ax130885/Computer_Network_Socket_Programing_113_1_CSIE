#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <experimental/filesystem>
#include <algorithm>
#include <openssl/ssl.h> // 需先 sudo yum install openssl-devel
#include <openssl/err.h> // 需先 sudo yum install openssl-devel
#include <thread>

#define CHUNCK_SIZE (4 * 1024 * 1024)
#define SERVER_IP "140.112.183.112"

// 初始化openssl
void initialize_openssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

// 創建SSL context
SSL_CTX *create_context()
{
    // 創建一個 SSL_METHOD 結構體，並指定使用 SSLv23_client_method() 方法
    const SSL_METHOD *method; // SSL_METHOD 結構體代表 SSL 協議的方法
    SSL_CTX *ctx;             // SSL_CTX 結構體代表 SSL 會話的上下文

    method = SSLv23_client_method(); // 使用 SSLv23_client_method() 方法

    // 創建 SSL context
    ctx = SSL_CTX_new(method);
    if (!ctx) // 如果 ctx 為空，則代表創建 SSL context 失敗。印出錯誤訊息並結束程式
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // 返回創建的 SSL context
    return ctx;
}

// 接收訊息的函數
void receiveMessages(SSL *ssl, bool &exit_flag)
{
    char buffer[4096];
    while (!exit_flag)
    {
        memset(buffer, 0, 4096);
        int bytes_received = SSL_read(ssl, buffer, 4096);
        if (bytes_received > 0)
        {
            std::cout << "Received: " << std::string(buffer, bytes_received) << std::endl;
        }
        else if (bytes_received <= 0)
        {
            std::cerr << "Error receiving message or connection closed" << std::endl;
            exit_flag = true;
            break;
        }
    }
}

// 發送訊息的函數
void sendMessages(SSL *ssl, bool &exit_flag)
{
    std::string user_input;
    while (!exit_flag)
    {
        std::getline(std::cin, user_input);
        SSL_write(ssl, user_input.c_str(), user_input.size());
        if (user_input == "exit")
        {
            exit_flag = true;
            break;
        }
    }
}

// 登入後的功能
int handleLoggedInUser(SSL *ssl, int client_socket)
{
    // 初始化傳送訊息的相關變數
    char buffer[4096];      // 存放從伺服器接收的數據
    std::string user_input; // 用戶輸入的內容

    while (true)
    {
        // 接收並顯示伺服器的功能選單
        memset(buffer, 0, 4096);
        // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
        int bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
        if (bytes_received > 0)
        {
            std::cout << std::string(buffer, bytes_received) << std::endl;
        }

        sleep(0.01);

        // 用戶選擇功能
        std::getline(std::cin, user_input);
        // send(client_socket, user_input.c_str(), user_input.size(), 0); // 加入SSL前
        SSL_write(ssl, user_input.c_str(), user_input.size()); // 使用SSL的版本

        if (user_input == "1") // 1. 回聲
        {
            // 接收 "現正使用1. 回聲\n請輸入訊息: "
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }

            // 輸入欲回聲的訊息
            std::getline(std::cin, user_input);
            // send(client_socket, user_input.c_str(), user_input.size(), 0); // 加入SSL前
            SSL_write(ssl, user_input.c_str(), user_input.size()); // 使用SSL的版本

            // 接收並顯示伺服器的回聲
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }

            // 如果發送的訊息為 "exit" 則跳出循環
            if (user_input == "exit")
            {
                break;
            }
        }
        else if (user_input == "2") // 2. 傳送訊息給伺服器
        {
            // 接收 "現正使用2. 傳送訊息給伺服器\n請輸入訊息: "
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }

            // 輸入欲傳送的訊息
            std::getline(std::cin, user_input);
            // send(client_socket, user_input.c_str(), user_input.size(), 0); // 加入SSL前
            SSL_write(ssl, user_input.c_str(), user_input.size()); // 使用SSL的版本

            // 如果發送的訊息為 "exit" 則跳出循環
            if (user_input == "exit")
            {
                break;
            }
        }
        else if (user_input == "3") // 3. 傳送檔案給伺服器
        {
            // 接收 "現正使用3. 傳送檔案給伺服器\n請輸入文件名: "
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }

            // 輸入文件名
            std::getline(std::cin, user_input);
            // 去除前後的引號
            std::string processed_input = user_input;
            processed_input.erase(std::remove(processed_input.begin(), processed_input.end(), '\"'), processed_input.end());

            // 取得文件名
            std::experimental::filesystem::path file_path(processed_input);
            std::string file_name = file_path.filename().string();

            // 如果路徑為資料夾，則終止連線
            if (std::experimental::filesystem::is_directory(file_path))
            {
                std::cerr << "無法傳送資料夾: " << processed_input << "終止連線" << std::endl;
                // 發送打開文件失敗
                // send(client_socket, "open_file_failed", 16, 0); // 加入SSL前
                SSL_write(ssl, "open_file_failed", 16); // 使用SSL的版本
                return 1;
            }

            // 打開文件
            std::cout << "Opening file: " << processed_input << std::endl;
            std::ifstream infile(processed_input, std::ios::binary);
            if (!infile)
            {
                std::cerr << "找不到此文件: " << processed_input << "終止連線" << std::endl;
                // 發送打開文件失敗
                // send(client_socket, "open_file_failed", 16, 0); // 加入SSL前
                SSL_write(ssl, "open_file_failed", 16); // 使用SSL的版本
                return 1;
            }

            // 發送打開文件成功
            // send(client_socket, "open_file_success", 17, 0); // 加入SSL前
            SSL_write(ssl, "open_file_success", 17); // 使用SSL的版本

            // 接收伺服器的確認訊息
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            // 如果收到"File opened successfully"，代表server端讀取文件成功
            if (bytes_received > 0 && std::string(buffer, bytes_received) == "File opened successfully")
            {
                std::cout << "Server opened file successfully" << std::endl;
            }
            else
            {
                std::cerr << "Server failed to open file" << std::endl;
                return 1;
            }

            // 發送文件名
            // send(client_socket, file_name.c_str(), file_name.size(), 0); // 加入SSL前
            SSL_write(ssl, file_name.c_str(), file_name.size()); // 使用SSL的版本
            std::cout << "Sent file name: " << file_name << std::endl;

            // 獲取檔案大小
            infile.seekg(0, std::ios::end);
            std::streamsize file_size = infile.tellg();
            infile.seekg(0, std::ios::beg);

            // 先發送檔案大小
            // send(client_socket, &file_size, sizeof(file_size), 0); // 加入SSL前
            SSL_write(ssl, reinterpret_cast<const char *>(&file_size), sizeof(file_size)); // 使用SSL的版本

            // 讀取文件並發送
            char file_buffer[CHUNCK_SIZE];
            std::streamsize total_bytes_sent = 0; // 目前已發送的字節數
            std::streamsize bytes_read;           // 每次讀取的字節數

            // 每次讀取CHUNCK_SIZE大小的文件數據，並發送
            while ((bytes_read = infile.read(file_buffer, CHUNCK_SIZE).gcount()) > 0)
            {
                // 發送文件數據
                // ssize_t bytes_sent = send(client_socket, file_buffer, bytes_read, 0); // 加入SSL前
                ssize_t bytes_sent = SSL_write(ssl, file_buffer, bytes_read); // 使用SSL的版本
                if (bytes_sent == -1)
                {
                    std::cerr << "Failed to send file data" << std::endl;
                    break;
                }
                // 更新已發送的字節數
                total_bytes_sent += bytes_sent;
                std::cout << "Sent " << total_bytes_sent << " bytes" << std::endl;
            }

            // 關閉文件
            infile.close();

            // 接收並顯示伺服器的確認訊息
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
        }
        else if (user_input == "4") // 4. 傳送訊息給其他client
        {
            // 接收 "現正使用4. 傳送訊息給其他 Client\n請先綁定要傳送訊息的用戶名:\n" + all_users + "請輸入用戶名: ";
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }

            // 輸入欲傳送訊息的用戶名
            std::getline(std::cin, user_input);
            // send(client_socket, user_input.c_str(), user_input.size(), 0); // 加入SSL前
            SSL_write(ssl, user_input.c_str(), user_input.size()); // 使用SSL的版本

            // 接收並顯示伺服器的確認訊息
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received < 0)
            {
                std::cerr << "Error receiving message" << std::endl;
                return 1;
            }

            // 印出有無選擇的用戶名是否存在
            std::cout << std::string(buffer, bytes_received) << std::endl;

            // 如果包含 "用戶名不存在" 則跳出循環
            if (std::string(buffer, bytes_received).find("用戶名不存在") != std::string::npos)
            {
                continue;
            }

            bool exit_flag(false);

            std::thread receive_thread(receiveMessages, ssl, std::ref(exit_flag));
            std::thread send_thread(sendMessages, ssl, std::ref(exit_flag));

            receive_thread.join();
            send_thread.join();

            sleep(0.01);
            continue;
        }

        else if (user_input == "9") // 9. 離開
        {
            // 接收 "程式執行完畢！\n"
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
            break;
        }

        else // 未知選項
        {
            // 接收 "未知的選項，請重新選擇功能。\n"
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
        }
    }

    return 0;
}

// 建立連線後 選擇註冊或登入
int handleClient(SSL *ssl, int client_socket)
{

    // 初始化傳送訊息的相關變數
    char buffer[4096];            // 存放從伺服器接收的數據
    std::string user_input = "0"; // 用戶輸入的內容

    // 接收並顯示伺服器的歡迎訊息(選擇註冊或登入)
    memset(buffer, 0, 4096);
    // int bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
    int bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
    if (bytes_received > 0)
    {
        std::cout << std::string(buffer, bytes_received) << std::endl;
    }

    // 選擇登入或註冊功能
    while (true)
    {
        // 選擇1. 註冊 2. 登入 9. 離開
        std::getline(std::cin, user_input); // std::getline(std::cin, user_input)代表將用戶輸入的內容存儲到 user_input 變數中, std::cin 代表標準鍵盤輸入
        // send(client_socket, user_input.c_str(), user_input.size(), 0); // 加入SSL前
        SSL_write(ssl, user_input.c_str(), user_input.size()); // 使用SSL的版本

        if (user_input == "1" || user_input == "2") // 註冊 或 登入
        {
            // 接收並顯示伺服器的回應 std::cout << "請輸入用戶名: ";
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
            std::getline(std::cin, user_input);
            // send(client_socket, user_input.c_str(), user_input.size(), 0); // 加入SSL前
            SSL_write(ssl, user_input.c_str(), user_input.size()); // 使用SSL的版本

            // 接收並顯示伺服器的回應 std::cout << "請輸入密碼: ";
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
            std::getline(std::cin, user_input);
            // send(client_socket, user_input.c_str(), user_input.size(), 0); // 加入SSL前
            SSL_write(ssl, user_input.c_str(), user_input.size()); // 使用SSL的版本

            // 接收並顯示伺服器的回應 std::cout << "註冊/登入 成功/失敗！" ;
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            std::string response(buffer, bytes_received);
            if (bytes_received > 0)
            {
                std::cout << response << std::endl;
            }

            // 登入成功才跳出循環，就算註冊成功也不跳出。
            if (response.find("登入成功") != std::string::npos)
            {
                std::cout << "#########################################" << std::endl;
                handleLoggedInUser(ssl, client_socket);
                break; // 登入成功，跳出循環
            }
        }

        else if (user_input == "9")
        {
            // 接收並顯示伺服器的回應 std::cout << "程式執行完畢！" << std::endl;
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
            close(client_socket);
            return 0;
        }
        else // 未知選項
        {
            // 接收並顯示伺服器的回應 std::cout << "未知的選項，請重新輸入。" << std::endl;
            memset(buffer, 0, 4096);
            // bytes_received = recv(client_socket, buffer, 4096, 0); // 加入SSL前
            bytes_received = SSL_read(ssl, buffer, 4096); // 使用SSL的版本
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
        }
    }

    return 0;
}

int main()
{
    // 初始化 OpenSSL
    initialize_openssl();            // 初始化 OpenSSL (上方自定義的函數)
    SSL_CTX *ctx = create_context(); // 創建 SSL context (上方自定義的函數)

    // 建立客戶端的 socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // 設定伺服器地址結構
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;                     // 使用 IPv4
    server_addr.sin_port = htons(48763);                  // 伺服器的端口
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr); // 伺服器的 IP 地址
    // inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // 伺服器的 IP 地址

    // 連接到伺服器
    if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    // 創建 SSL 連接
    SSL *ssl = SSL_new(ctx);        // 創建 SSL 連接
    SSL_set_fd(ssl, client_socket); // 將 SSL 連接綁定到客戶端 socket

    if (SSL_connect(ssl) <= 0) // 連接 SSL 連接 (如果返回值小於等於 0，則代表連接失敗)
    {
        ERR_print_errors_fp(stderr);
    }
    else // 連接成功
    {
        std::cout << "Connected with " << SSL_get_cipher(ssl) << " encryption" << std::endl;
    }

    // 取得伺服器的 IP 位址和 port
    char ip_str[INET_ADDRSTRLEN];                                      // 初始化下一行用來存放 IP 位址的字串
    inet_ntop(AF_INET, &server_addr.sin_addr, ip_str, sizeof(ip_str)); // 將二進制的 IP 位址轉換為點分十進制的 IP 位址; AF_INET:IPv4, sin_addr: (source 二進制)伺服器的 IP 位址, ip_str: (destination 十進制)用來存放 IP 位址的字串, sizeof(ip_str): 字串的大小
    std::cout << "Connected to server at IP: " << ip_str << " and port: " << ntohs(server_addr.sin_port) << std::endl;

    handleClient(ssl, client_socket); // 建立連線後 選擇註冊或登入

    SSL_shutdown(ssl);    // 關閉 SSL 連接
    SSL_free(ssl);        // 釋放 SSL 連接
    SSL_CTX_free(ctx);    // 釋放 SSL context
    EVP_cleanup();        // 清理 OpenSSL
    close(client_socket); // 關閉客戶端 socket
    return 0;
}