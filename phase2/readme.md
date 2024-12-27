<!-- title: 計算機網路 Socket Programing Phase 2 -->
---
Title: 計算機網路 Socket Programing Phase 2   
Student ID: R12631070  
Name: 林育新  
YouTube: 
---
# 編譯與執行環境
Linux  
openssl (CentOS7 當中使用 sudo yum install openssl-devel安裝)

# Compilation instructions
1. 將 server.cpp 和 client.cpp 當中的 #define SERVER_IP 修改為自己的IP
    ```cpp
    #define SERVER_IP "140.112.xxx.xxx"
    ```
2. 編譯cpp檔案  
    ```bash
    make
    ```

# Usage guide

1. 啟動 Server 和 Client
    ```bash
    ./server
    ```

    ```bash
    ./client
    ```

2. Client 登入界面  

    首先輸入1完成註冊以後，接著輸入2再進行登入。  
    ```bash
    Connected to server at IP: 140.112.183.112 and port: 48763
    歡迎來到伺服器！
    1. 註冊
    2. 登入
    3. 離開
    請輸入阿拉伯數字以選擇模式:
    1
    請輸入用戶名:
    test
    請輸入密碼:
    test
    註冊成功！
    請重新選擇功能。

    2
    請輸入用戶名:
    test
    請輸入密碼:
    test
    登入成功！
    ```

3. Client 功能選單  
    登入成功後，會顯示如下畫面。  
    每次發送訊息前都需要選擇功能，再輸入欲傳送的訊息。
    ```bash
    #########################################

    選擇服務:
    1. 回聲
    2. 傳送訊息給伺服器
    3. 傳送檔案給伺服器
    4. 傳送訊息給其他 Client
    9. 離開
    請輸入阿拉伯數字以選擇模式:
    1
    現正使用1. 回聲
    請輸入訊息:
    qwerty
    qwerty
    ```

4. 檔案傳送功能

    #### 注意事項
    1. 僅能從client傳到server
    2. 檔案存在server/download內
    3. 允許任意種類檔案
    4. 輸入檔案名可以包含"" 也可以不包含
    5. 可以使用相對路徑和絕對路徑

    #### 執行過程
    #### client端
    ```bash
    選擇服務:
    1. 回聲
    2. 傳送訊息給伺服器
    3. 傳送檔案給伺服器
    4. 傳送訊息給其他 Client
    9. 離開
    請輸入阿拉伯數字以選擇模式:
    3
    現正使用3. 傳送檔案給伺服器 (暫不支持資料夾 僅能傳送單一檔案)
    請輸入檔案名 :
    "/home/graduate/yuxin/computer_network/phase2/readme.md"
    Opening file: /home/graduate/yuxin/computer_network/phase2/readme.md
    Server opened file successfully
    Sent file name: readme.md
    Sent 2120 bytes
    File received successfully
    ```
    #### server 端
    ```bash
    收到來自 112 的檔案傳送請求
    Client opened file successfully
    User 112 上傳檔案名: readme.md
    download 資料夾不存在，新建資料夾
    User 112 創建檔案: download/readme.md
    Expected file size: 2120 bytes
    Receiving: 100%
    File received: download/readme.md
    ```

5. 透過server轉傳訊息給其他client

    #### 使用說明
    選擇功能進入4. 傳送訊息給其他 Client以後，伺服器會回傳目前連線中的所有用戶名稱。  
    先輸入欲傳送訊息的用戶，確認綁定成功以後，再開始傳送訊息。  
    傳輸訊息輸入exit即可回到功能選單。

    #### 執行過程
    #### server 端
    ```bash
    User 112 現正使用4. 傳送訊息給其他 Client
    User 112 與 111 綁定成功！

    User 111 現正使用4. 傳送訊息給其他 Client
    User 111 不能傳送訊息給自己，請重新選擇功能。

    User 111選擇服務

    User 111 現正使用4. 傳送訊息給其他 Client
    User 111 與 112 綁定成功！

    Received from 112, Send to 111: hello
    Received from 111, Send to 112: how
    Received from 112, Send to 111: are
    Received from 111, Send to 112: you
    Received from 112, Send to 111: exit
    Exit command received from 112 to 112  connection.
    User 112 與 111 解除綁定！
    ```

    #### client 1 端
    ```bash
    選擇服務:
    1. 回聲
    2. 傳送訊息給伺服器
    3. 傳送檔案給伺服器
    4. 傳送訊息給其他 Client
    9. 離開
    請輸入阿拉伯數字以選擇模式:
    4
    現正使用4. 傳送訊息給其他 Client
    請先綁定要傳送訊息的用戶名:
    所有用戶名:
    112
    111
    請輸入用戶名:
    111
    User 112 與 111 綁定成功！
    請輸入想要傳給對方的訊息。或是輸入exit來解除綁定，並且回到功能選單。

    hello
    Received: 111: how
    are
    Received: 111: you
    exit
    Received: 確認exit
    ```

    #### client 2 端
    ```bash
    選擇服務:
    1. 回聲
    2. 傳送訊息給伺服器
    3. 傳送檔案給伺服器
    4. 傳送訊息給其他 Client
    9. 離開
    請輸入阿拉伯數字以選擇模式:
    4
    Received: 現正使用4. 傳送訊息給其他 Client
    請先綁定要傳送訊息的用戶名:
    所有用戶名:
    112
    111
    請輸入用戶名:
    112
    Received: User 111 與 112 綁定成功！
    請輸入想要傳給對方的訊息。或是輸入exit來解除綁定，並且回到功能選單。

    Received: 112: hello
    how
    Received: 112: are
    you
    ```



6. 結束 Client 服務  
   在登入界面/選擇功能時輸入對應的編號，即可正常中止程式。
    ```bash
    選擇服務:
    1. 回聲
    2. 傳送訊息給伺服器
    3. 傳送檔案給伺服器
    4. 傳送訊息給其他 Client
    9. 離開
    請輸入阿拉伯數字以選擇模式:
    9
    程式執行完畢！ 登出並離開
    ```

# Any additional information
重新編譯以後，需要重新註冊帳密。  
若無重新編譯，不用重新註冊!