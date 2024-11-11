#include <iostream>
#include <string>
#include <unordered_map>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// 用戶資料庫
std::unordered_map<std::string, std::string> users;

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

// 處理客戶端連線
void handleClient(int client_socket)
{
    char buffer[4096];
    std::string username;
    bool loggedIn = false;

    // 歡迎訊息
    std::string welcomeMsg = "歡迎來到伺服器！\n1. 註冊\n2. 登入\n3. 離開\n請輸入阿拉伯數字以選擇模式: ";
    send(client_socket, welcomeMsg.c_str(), welcomeMsg.size(), 0);

    std::string prompt;

    while (true)
    {
        // 接收用戶選擇
        memset(buffer, 0, 4096);
        int bytes_received = recv(client_socket, buffer, 4096, 0);
        if (bytes_received <= 0)
        {
            close(client_socket);
            return;
        }
        std::string choice(buffer, bytes_received);

        if (choice == "1") // 註冊
        {
            prompt = "請輸入用戶名: ";
            send(client_socket, prompt.c_str(), prompt.size(), 0);

            memset(buffer, 0, 4096); // 清空 buffer
            bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received <= 0)
            {
                std::cerr << "Failed to receive username" << std::endl;
                close(client_socket);
                return;
            }
            std::string username(buffer, bytes_received); // 將buffer中前bytes_received個bytes轉換為string

            prompt = "請輸入密碼: ";
            send(client_socket, prompt.c_str(), prompt.size(), 0);

            memset(buffer, 0, 4096); // 清空 buffer
            bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received <= 0)
            {
                std::cerr << "Failed to receive password" << std::endl;
                close(client_socket);
                return;
            }
            std::string password = std::string(buffer, bytes_received); // 將buffer中前bytes_received個bytes轉換為string

            // 註冊帳密
            if (registerUser(username, password))
            {
                prompt = "註冊成功！\n請重新選擇功能。\n";
                send(client_socket, prompt.c_str(), prompt.size(), 0);
            }
            else
            {
                prompt = "用戶已存在，註冊失敗。\n請重新選擇功能。\n";
                send(client_socket, prompt.c_str(), prompt.size(), 0);
            }
        }
        else if (choice == "2") // 登入
        {
            prompt = "請輸入用戶名: ";
            send(client_socket, prompt.c_str(), prompt.size(), 0);
            memset(buffer, 0, 4096); // 清空 buffer
            bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received <= 0)
            {
                std::cerr << "Failed to receive username" << std::endl;
                close(client_socket);
                return;
            }

            username = std::string(buffer, bytes_received); // 將buffer中前bytes_received個bytes轉換為string

            prompt = "請輸入密碼: ";
            send(client_socket, prompt.c_str(), prompt.size(), 0);
            memset(buffer, 0, 4096); // 清空 buffer
            bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received <= 0)
            {
                std::cerr << "Failed to receive password" << std::endl;
                close(client_socket);
                return;
            }
            std::string password = std::string(buffer, bytes_received); // 將buffer中前bytes_received個bytes轉換為string

            // 登入
            if (loginUser(username, password))
            {
                prompt = "登入成功！\n";
                loggedIn = true;
                send(client_socket, prompt.c_str(), prompt.size(), 0);
                // 加入短暫延遲 避免send衝突
                usleep(100000);
                break;
            }
            else
            {
                prompt = "登入失敗，用戶名或密碼錯誤。\n請重新選擇功能。\n";
                send(client_socket, prompt.c_str(), prompt.size(), 0);
            }
        }
        else if (choice == "3")
        {
            prompt = "程式執行完畢！\n";
            send(client_socket, prompt.c_str(), prompt.size(), 0);
            close(client_socket);
            return;
        }
        else
        {
            prompt = "未知的選項，請重新輸入功能代號。\n";
            send(client_socket, prompt.c_str(), prompt.size(), 0);
        }
        // 加入短暫延遲 避免send衝突
        usleep(100000);
    }

    if (loggedIn)
    {
        while (true)
        {
            // 傳送功能選單
            std::string menu = "選擇服務:\n1. 回聲\n2. 傳送訊息給伺服器\n3. 離開\n請選擇: ";
            send(client_socket, menu.c_str(), menu.size(), 0);

            // 接收用戶選擇功能
            memset(buffer, 0, 4096); // 清空 buffer
            int bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received <= 0)
            {
                std::cerr << "Failed to receive service choice" << std::endl;
                close(client_socket);
                return;
            }

            std::string service_choice(buffer, bytes_received);

            if (service_choice == "1") // 回聲服務
            {
                prompt = "現正使用1. 回聲\n請輸入訊息: ";
                send(client_socket, prompt.c_str(), prompt.size(), 0);

                // 接收想回聲的訊息
                memset(buffer, 0, 4096); // 清空 buffer
                bytes_received = recv(client_socket, buffer, 4096, 0);
                if (bytes_received <= 0)
                {
                    std::cerr << "Failed to receive message" << std::endl;
                    close(client_socket);
                    return;
                }

                // 將收到的訊息轉換為 string 並且檢查是否為 exit
                std::string message(buffer, bytes_received);
                if (message == "exit")
                {
                    break;
                }
                std::cout << "收到來自 " << username << " 的要求回聲訊息: " << message << std::endl;
                send(client_socket, buffer, bytes_received, 0); // 回傳訊息
            }
            else if (service_choice == "2") // 傳送訊息給伺服器
            {
                prompt = "現正使用2. 傳送訊息給伺服器\n請輸入訊息: ";
                send(client_socket, prompt.c_str(), prompt.size(), 0);

                // 接收訊息(不回傳)
                memset(buffer, 0, 4096); // 清空 buffer
                bytes_received = recv(client_socket, buffer, 4096, 0);
                if (bytes_received <= 0)
                {
                    std::cerr << "Failed to receive message" << std::endl;
                    close(client_socket);
                    return;
                }

                std::string message(buffer, bytes_received);
                if (message == "exit")
                {
                    break;
                }
                std::cout << "收到來自 " << username << " 傳送的訊息: " << message << std::endl;
            }
            else if (service_choice == "3") // 離開
            {
                prompt = "程式執行完畢！\n";
                send(client_socket, prompt.c_str(), prompt.size(), 0);
                close(client_socket);
                return;
            }
            else
            {
                prompt = "未知的選項，請重新選擇功能。\n";
                send(client_socket, prompt.c_str(), prompt.size(), 0);
            }
            // 加入短暫延遲 避免send衝突
            usleep(100000);
        }
    }

    close(client_socket); // 關閉客戶端 socket
}

int main()
{
    // 創建伺服器 socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // 設定伺服器地址
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(48763);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // 綁定 socket 到指定的 IP 和 port
    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to bind to port 48763" << std::endl;
        return 1;
    }

    // 監聽連線
    if (listen(server_socket, SOMAXCONN) == -1)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        return 1;
    }

    std::cout << "Server is listening on port 48763" << std::endl;

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
        handleClient(client_socket);
        std::cout << "Client disconnected: " << client_ip << ":" << client_port << std::endl;
    }

    close(server_socket); // 關閉伺服器 socket
    return 0;
}