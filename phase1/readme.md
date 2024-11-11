<!-- title: 計算機網路 Socket Programing Phase 1 -->
---
Title: 計算機網路 Socket Programing Phase 1   
Student ID: R12631070  
Name: 林育新  
YouTube: 
---
# 編譯與執行環境
Linux

# Compilation instructions
編譯cpp檔案  
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
    Connected to server at IP: 127.0.0.1 and port: 48763
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
    3. 離開
    請選擇:
    1
    現正使用1. 回聲
    請輸入訊息:
    QWERTY
    QWERTY
    ```

4. 結束 Client 服務  
   在登入界面/選擇功能時輸入對應的編號，即可正常中止程式。
    ```bash
    選擇服務:
    1. 回聲
    2. 傳送訊息給伺服器
    3. 離開
    請選擇:
    3
    程式執行完畢！
    ```

5. Server 顯示紀錄
    ```bash
    Server is listening on port 48763
    Client connected: 127.0.0.1:36956
    收到來自 test 的要求回聲訊息: QWERTY
    Client disconnected: 127.0.0.1:36956
    ```

# Any additional information
重新編譯以後，需要重新註冊帳密。  
若無重新編譯，不用重新註冊!