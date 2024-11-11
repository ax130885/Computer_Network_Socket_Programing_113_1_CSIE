#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    // 建立客戶端的 socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // 設定伺服器地址結構
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;                       // 使用 IPv4
    server_addr.sin_port = htons(48763);                    // 伺服器的端口
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // 伺服器的 IP 地址

    // 連接到伺服器
    if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_addr.sin_addr, ip_str, sizeof(ip_str));
    std::cout << "Connected to server at IP: " << ip_str << " and port: " << ntohs(server_addr.sin_port) << std::endl;

    char buffer[4096];
    std::string user_input;
    bool loggedIn = false;

    // 接收並顯示伺服器的歡迎訊息(選擇註冊或登入)
    memset(buffer, 0, 4096);
    int bytes_received = recv(client_socket, buffer, 4096, 0);
    if (bytes_received > 0)
    {
        std::cout << std::string(buffer, bytes_received) << std::endl;
    }

    // 選擇登入或註冊功能
    while (true)
    {
        // 選擇1. 註冊 2. 登入 3. 離開
        std::getline(std::cin, user_input); // std::getline(std::cin, user_input)代表將用戶輸入的內容存儲到 user_input 變數中, std::cin 代表標準鍵盤輸入
        send(client_socket, user_input.c_str(), user_input.size(), 0);

        if (user_input == "1" || user_input == "2") // 註冊 或 登入
        {
            // 接收並顯示伺服器的回應 std::cout << "請輸入用戶名: ";
            memset(buffer, 0, 4096);
            bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
            std::getline(std::cin, user_input);
            send(client_socket, user_input.c_str(), user_input.size(), 0);

            // 接收並顯示伺服器的回應 std::cout << "請輸入密碼: ";
            memset(buffer, 0, 4096);
            bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
            std::getline(std::cin, user_input);
            send(client_socket, user_input.c_str(), user_input.size(), 0);

            // 接收並顯示伺服器的回應 std::cout << "註冊/登入 成功/失敗！" ;
            memset(buffer, 0, 4096);
            bytes_received = recv(client_socket, buffer, 4096, 0);
            std::string response(buffer, bytes_received);
            if (bytes_received > 0)
            {
                std::cout << response << std::endl;
            }

            // 登入成功才跳出循環，就算註冊成功也不跳出。
            if (response.find("登入成功") != std::string::npos)
            {
                std::cout << "#########################################" << std::endl;
                loggedIn = true;
                break; // 登入成功，跳出循環
            }
        }

        else if (user_input == "3")
        {
            // 接收並顯示伺服器的回應 std::cout << "程式執行完畢！" << std::endl;
            memset(buffer, 0, 4096);
            bytes_received = recv(client_socket, buffer, 4096, 0);
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
            bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }
        }
    }

    // 登入後的功能選擇
    if (loggedIn)
    {

        while (true)
        {
            // 接收並顯示伺服器的功能選單
            memset(buffer, 0, 4096);
            bytes_received = recv(client_socket, buffer, 4096, 0);
            if (bytes_received > 0)
            {
                std::cout << std::string(buffer, bytes_received) << std::endl;
            }

            // 用戶選擇功能
            std::getline(std::cin, user_input);
            send(client_socket, user_input.c_str(), user_input.size(), 0);

            if (user_input == "1") // 1. 回聲
            {
                // 接收 "現正使用1. 回聲\n請輸入訊息: "
                memset(buffer, 0, 4096);
                bytes_received = recv(client_socket, buffer, 4096, 0);
                if (bytes_received > 0)
                {
                    std::cout << std::string(buffer, bytes_received) << std::endl;
                }

                // 輸入欲回聲的訊息
                std::getline(std::cin, user_input);
                send(client_socket, user_input.c_str(), user_input.size(), 0);

                // 接收並顯示伺服器的回聲
                memset(buffer, 0, 4096);
                bytes_received = recv(client_socket, buffer, 4096, 0);
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
                bytes_received = recv(client_socket, buffer, 4096, 0);
                if (bytes_received > 0)
                {
                    std::cout << std::string(buffer, bytes_received) << std::endl;
                }

                // 輸入欲傳送的訊息
                std::getline(std::cin, user_input);
                send(client_socket, user_input.c_str(), user_input.size(), 0);

                // 如果發送的訊息為 "exit" 則跳出循環
                if (user_input == "exit")
                {
                    break;
                }
            }
            else if (user_input == "3") // 3. 離開
            {
                // 接收 "程式執行完畢！\n"
                memset(buffer, 0, 4096);
                bytes_received = recv(client_socket, buffer, 4096, 0);
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
                bytes_received = recv(client_socket, buffer, 4096, 0);
                if (bytes_received > 0)
                {
                    std::cout << std::string(buffer, bytes_received) << std::endl;
                }
            }
        }
    }

    close(client_socket); // 關閉客戶端 socket
    return 0;
}