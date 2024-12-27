# 定義編譯器
CXX = g++

# 定義編譯選項
CXXFLAGS = -Wall -std=c++17 \

# 定義函式庫
LDFLAGS = -L/usr/local/lib -lpthread -lstdc++fs -lssl -lcrypto

# 定義目標
TARGETS = client server key csr

# 定義目標的依賴關係和規則
all: $(TARGETS)

key:
	@if [ ! -f server.key ]; then \
		openssl genpkey -algorithm RSA -out server.key; \
		echo "server key 生成完成"; \
	fi

csr: key
	@if [ ! -f server.csr ]; then \
		openssl req -new -key server.key -out server.csr -subj "/C=TW/ST=Taipei/L=Taipei City/O=National Taiwan University/OU=BIME_CIBILAB/CN=140.112.183.112/emailAddress=ax130885@gmail.com"; \
		openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt; \
		echo "server csr 生成完成"; \
	fi

server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp $(LDFLAGS)
	@echo "server 編譯完成"

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp $(LDFLAGS)
	@echo "client 編譯完成"

# 清理生成的文件
clean:
	rm -f $(TARGETS)
	@echo "清理完成"
